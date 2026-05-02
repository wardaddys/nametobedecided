/**
 * @file SerialManager.cpp
 * @brief Implementation of SerialManager
 *
 * Implements:
 *  - New-protocol framing (CRC32 on all send/receive)
 *  - DTR reset byte handling (BUG-003)
 *  - Command context tracking (BUG-007)
 *  - Proper response routing by CommandType instead of packet-size heuristics
 *  - ECU restart detection via secl monitoring
 */

#include "SerialManager.h"
#include "../utils/Logger.h"
#include <QThread>

SerialManager::SerialManager(QObject *parent)
    : QObject(parent), m_serialPort(new QSerialPort(this)),
      m_protocol(new SpeeduinoProtocol(this)),
      m_status(ConnectionStatus::Disconnected), m_awaitingResponse(false),
      m_reconnectAttempts(0), m_maxReconnectAttempts(10),
      m_isPolling(false), m_pollingInterval(50),
      m_lastSecl(0),
      m_packetsSent(0), m_packetsReceived(0), m_crcErrors(0) {

    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(ProtocolTiming::HEARTBEAT_INTERVAL_MS);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &SerialManager::sendHeartbeat);

    m_dataPollingTimer = new QTimer(this);
    connect(m_dataPollingTimer, &QTimer::timeout, this, &SerialManager::requestRealTimeData);

    m_commandTimeoutTimer = new QTimer(this);
    m_commandTimeoutTimer->setSingleShot(true);
    connect(m_commandTimeoutTimer, &QTimer::timeout, this, &SerialManager::onCommandTimeout);

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(2000);
    connect(m_reconnectTimer, &QTimer::timeout, this, &SerialManager::attemptReconnection);

    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialManager::onReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialManager::onErrorOccurred);
}

bool SerialManager::loadEcuDefinition(const QString &filePath) {
    // Load definition into m_ecuDefinition for signature validation
    bool defLoaded = m_ecuDefinition.load(filePath);
    if (defLoaded) {
        Logger::info("ECU Definition loaded for signature validation: " + filePath);
        // Also load into protocol for constants/etc.
        return m_protocol->loadDefinition(filePath);
    } else {
        Logger::warning("Failed to load ECU Definition: " + filePath);
        return false;
    }
}

SerialManager::~SerialManager() { disconnectFromDevice(); }

QList<QSerialPortInfo> SerialManager::detectDevices() {
    return QSerialPortInfo::availablePorts();
}

// ============================================================================
//  Simulation Mode
// ============================================================================

void SerialManager::setSimulationMode(bool enabled) {
    m_isSimulation = enabled;
    if (enabled) {
        Logger::info("Simulation mode enabled");
        for (int i = 0; i < 256; ++i) {
            m_simulatedMemory[i].fill(0, 4096);
        }
    } else {
        Logger::info("Simulation mode disabled");
    }
}

// ============================================================================
//  Connection — with DTR reset handling (BUG-003 fix)
// ============================================================================

bool SerialManager::connectToDevice(const QString &portName, qint32 baudRate) {
    if (isConnected()) {
        disconnectFromDevice();
    }

    // Always require fresh validation for each connection session.
    m_signatureValidated = false;

    m_portName = portName;
    m_baudRate = baudRate;

    if (m_isSimulation) {
        if (m_serialPort->isOpen()) m_serialPort->close();
        m_status = ConnectionStatus::Connected;
        emit connectionStatusChanged(m_status);

        m_signature.firmwareVersion = "Speeduino 2025.01 (Sim)";
        m_signature.boardType = "Simulator";
        m_signature.protocolVersion = 2;
        m_signature.blockingFactor = 251;
        m_signature.tableBlockingFactor = 256;
        emit connected(m_signature);

        Logger::info("Connected to Simulated ECU");

        if (m_isPolling) m_dataPollingTimer->start(m_pollingInterval);
        m_heartbeatTimer->start();
        return true;
    }

    m_serialPort->setPortName(portName);
    
    // Clear any previous error states
    m_serialPort->clearError();

    // FORCE CLOSE: If the port is already open (perhaps from a previous crash or attempt),
    // we must close it before we can re-open it with new settings.
    if (m_serialPort->isOpen()) {
        Logger::info("Port was already open, force-closing before re-connect.");
        m_serialPort->close();
    }

    if (m_serialPort->open(QIODevice::ReadWrite)) {
        // Apply parameters AFTER opening (more robust for some Windows drivers)
        m_serialPort->setBaudRate(baudRate);
        m_serialPort->setDataBits(QSerialPort::Data8);
        m_serialPort->setParity(QSerialPort::NoParity);
        m_serialPort->setStopBits(QSerialPort::OneStop);
        m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
        
        // CRITICAL: Explicitly set DTR and RTS to wake up the AtMega16U2 serial chip 
        // on original Arduino Mega boards.
        m_serialPort->setDataTerminalReady(true);
        m_serialPort->setRequestToSend(true);

        m_status = ConnectionStatus::Connecting;
        emit connectionStatusChanged(m_status);
        Logger::info("Serial port opened: " + portName);

        // Increased to 2000ms for original Mega bootloader stability and triple-flush
        QTimer::singleShot(2000, this, [this]() {
            if (!m_serialPort->isOpen()) return;

            // Triple-flush to remove all reset/stale junk
            for (int i=0; i<3; ++i) {
                m_serialPort->readAll();
                m_serialPort->flush();
            }
            
            // Start connection handshake: Use LEGACY 'S' first for maximum compatibility
            SerialCommand cmd;
            cmd.data = "S"; 
            cmd.type = CommandType::Signature;
            cmd.timeoutMs = 1500; // More generous for signature
            cmd.retryCount = 5;
            cmd.isRaw = true; // DO NOT wrap the first command
            
            Logger::info("Handshake: Sending Legacy Signature Request ('S')...");
            queueCommand(cmd);
        });

        return true;
    } else {
        QString err = m_serialPort->errorString();
        Logger::error("Failed to open serial port: " + err);
        emit error("Failed to open port: " + err);
        return false;
    }
}

void SerialManager::disconnectFromDevice() {
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }

    m_status = ConnectionStatus::Disconnected;
    emit connectionStatusChanged(m_status);
    emit disconnected();

    m_heartbeatTimer->stop();
    m_dataPollingTimer->stop();
    m_commandTimeoutTimer->stop();
    m_reconnectTimer->stop();
    m_commandQueue.clear();
    m_receiveBuffer.clear();
    m_isPolling = false;
    m_awaitingResponse = false;
    m_signatureValidated = false;

    Logger::info("Disconnected from device");
}

bool SerialManager::isConnected() const {
    if (m_isSimulation) {
        return m_status == ConnectionStatus::Connected;
    }
    return m_serialPort->isOpen() && m_status == ConnectionStatus::Connected;
}

bool SerialManager::isSimulationConnected() const {
    return m_isSimulation && m_status == ConnectionStatus::Connected;
}

ConnectionStatus SerialManager::getStatus() const { return m_status; }

ECUSignature SerialManager::getSignature() const { return m_signature; }

// ============================================================================
//  Data Polling
// ============================================================================

void SerialManager::startDataPolling(int intervalMs) {
    if (intervalMs < 10) intervalMs = 10;
    if (intervalMs > 1000) intervalMs = 1000;

    m_pollingInterval = intervalMs;
    m_isPolling = true;

    if (isConnected()) {
        m_dataPollingTimer->start(m_pollingInterval);
        emit pollingRateChanged(1000.0 / m_pollingInterval);
    }
}

void SerialManager::stopDataPolling() {
    m_isPolling = false;
    m_dataPollingTimer->stop();
}

void SerialManager::setPollingRate(int intervalMs) {
    startDataPolling(intervalMs);
}

// ============================================================================
//  Table Operations — with CommandType tracking (BUG-007 fix)
// ============================================================================

void SerialManager::readTable(quint8 table, quint16 offset, quint16 size) {
    SerialCommand cmd;
    cmd.data = m_protocol->createReadPageRequest(table, offset, size);
    cmd.type = CommandType::ReadPage;
    cmd.expectedResponse = -1; // Variable: RC + data bytes
    cmd.timeoutMs = ProtocolTiming::TIMEOUT_MS;
    cmd.retryCount = ProtocolTiming::RETRY_COUNT;
    cmd.pageNum = table;
    cmd.pageOffset = offset;
    queueCommand(cmd);
}

void SerialManager::writeTable(quint8 table, quint16 offset,
                               const QByteArray &data) {
    // ===== SIGNATURE VALIDATION GATE: Block writes if signature not validated =====
    if (!m_signatureValidated && !m_isSimulation) {
        Logger::error("WRITE BLOCKED: ECU signature has not been validated. "
                      "Definition must match ECU firmware before writes are allowed.");
        emit error("Write blocked: ECU signature not validated against definition");
        return;
    }
    
    SerialCommand cmd;
    cmd.data = m_protocol->createWritePageRequest(table, offset, data);
    cmd.type = CommandType::WritePage;
    cmd.expectedResponse = -1; // RC only
    cmd.timeoutMs = ProtocolTiming::TIMEOUT_MS;
    cmd.retryCount = 0;
    cmd.pageNum = table;
    queueCommand(cmd);
}

void SerialManager::sendBurnCommand(quint8 page) {
    // ===== SIGNATURE VALIDATION GATE: Block burn if signature not validated =====
    if (!m_signatureValidated && !m_isSimulation) {
        Logger::error("BURN BLOCKED: ECU signature has not been validated. "
                      "Definition must match ECU firmware before burn operations are allowed.");
        emit error("Burn blocked: ECU signature not validated against definition");
        return;
    }
    
    SerialCommand cmd;
    cmd.data = m_protocol->createBurnPageRequest(page);
    cmd.type = CommandType::BurnPage;
    cmd.expectedResponse = -1;
    cmd.timeoutMs = ProtocolTiming::BURN_TIMEOUT_MS;
    cmd.retryCount = 2;
    cmd.pageNum = page;
    queueCommand(cmd);
}

// ============================================================================
//  Data Reception — New protocol frame parsing (BUG-002 fix)
// ============================================================================

void SerialManager::onReadyRead() {
    m_receiveBuffer.append(m_serialPort->readAll());

    // DEBUG: Hex dump first 16 bytes of any incoming data to trace ECU behavior
    if (!m_receiveBuffer.isEmpty()) {
        Logger::info("RX Buffer (" + QString::number(m_receiveBuffer.size()) + " bytes): " + 
                     m_receiveBuffer.left(16).toHex(' ').toUpper());
    }

    // --- UNIVERSAL PACKET RECOGNIZER ---
    // We try all possible Speeduino response formats in order of likelihood.
    
    // 1. Try New Protocol Frame (Wrapped CRC)
    if (tryParseNewProtocolFrame()) return;

    // 2. Try Raw Telemetry (Awaiting 'A' response)
    if (m_awaitingResponse && m_currentCommand.type == CommandType::RealTimeData) {
        
        // Speeduino may prefix raw responses with 0x06 (RC_OK) in some firmware
        int offset = 0;
        if (m_receiveBuffer.size() > 0 && static_cast<uint8_t>(m_receiveBuffer.at(0)) == 0x06) {
            offset = 1;
        }

        if (m_receiveBuffer.size() >= (120 + offset)) {
            try {
                // Peek at the data: SECL should be a small counter, MAP should be ~100
                QByteArray dataToParse = m_receiveBuffer.mid(offset);
                RealTimeData rtData;
                
                if (!m_ecuDefinition.getOutputChannels().isEmpty()) {
                    SpeeduinoProtocol::DynamicRTData dyn = m_protocol->parseRealTimeDataDynamic(dataToParse, m_ecuDefinition);
                    rtData = dyn.legacy;
                } else {
                    rtData = m_protocol->parseRealTimeData(dataToParse);
                }
                
                emit dataReceived(rtData);
                m_receiveBuffer.clear();
                m_awaitingResponse = false;
                m_commandTimeoutTimer->stop();
                processCommandQueue();
                return;
            } catch (...) {
                // If it looks like a packet but parsing failed, slide the window
                m_receiveBuffer.remove(0, 1);
            }
        }
    }

    // 3. Fallback: Generic Command Handling (Signature, etc.)
    // SCAN FOR SIGNATURE ANYWHERE IN BUFFER (Handles frame-wrapped or noisy signatures)
    if (m_receiveBuffer.contains("Speeduino")) {
        int index = m_receiveBuffer.indexOf("Speeduino");
        QByteArray sigData = m_receiveBuffer.mid(index);
        m_signature = m_protocol->parseSignature(sigData);
        Logger::info("Signature identified (offset " + QString::number(index) + "): " + m_signature.toString());
        
        m_awaitingResponse = false;
        m_commandTimeoutTimer->stop();
        processResponse(sigData);
        m_receiveBuffer.clear();
        return;
    }

    if (m_awaitingResponse && m_currentCommand.isRaw) {
        // Handle 'f' (Capabilities)
        if (m_currentCommand.type == CommandType::Capabilities && m_receiveBuffer.size() >= 5) {
            processResponse(m_receiveBuffer);
            m_receiveBuffer.clear();
            m_awaitingResponse = false;
            m_commandTimeoutTimer->stop();
            processCommandQueue();
            return;
        }
    }

    // --- CASE 3: FALLBACK / ERROR RECOVERY ---
    if (m_receiveBuffer.size() > 1024) {
        Logger::warning("Receive buffer overflow, clearing.");
        m_receiveBuffer.clear();
    }

    // --- CASE 3: FALLBACK / ERROR RECOVERY ---
    if (m_receiveBuffer.size() > 1024) {
        Logger::warning("Receive buffer overflow, clearing.");
        m_receiveBuffer.clear();
    }
}

bool SerialManager::tryParseNewProtocolFrame() {
    // New protocol response: [Length 2B BE] [Payload: RC + data...] [CRC32 4B BE]
    while (m_receiveBuffer.size() >= 6) { // Minimum frame: 2 + 0 + 4 = 6

        // Read length header (2 bytes big-endian)
        uint16_t payloadLen = (static_cast<uint8_t>(m_receiveBuffer.at(0)) << 8) |
                               static_cast<uint8_t>(m_receiveBuffer.at(1));

        // Sanity check: payload can't be bigger than ~2KB
        if (payloadLen > 2048) {
            // Bad framing — discard first byte and try again
            Logger::warning("Bad frame length: " + QString::number(payloadLen) + ", resyncing");
            m_receiveBuffer.remove(0, 1);
            continue;
        }

        int frameSize = 2 + payloadLen + 4; // header + payload + CRC

        if (m_receiveBuffer.size() < frameSize) {
            // Not enough data yet, wait for more
            return false;
        }

        // Extract complete frame
        QByteArray frame = m_receiveBuffer.left(frameSize);
        m_receiveBuffer.remove(0, frameSize);

        // Validate CRC32
        QByteArray payload;
        if (SpeeduinoProtocol::unwrapNewProtocol(frame, payload)) {
            m_packetsReceived++;
            m_commandTimeoutTimer->stop();
            m_awaitingResponse = false;

            // Skip RC byte (first byte of payload) for data extraction
            uint8_t returnCode = payload.isEmpty() ? 0xFF : static_cast<uint8_t>(payload.at(0));

            // Check for error return codes
            if (returnCode == SpeeduinoRC::RC_BUSY_ERR) {
                Logger::warning("ECU busy (BUSY_ERR), will retry");
                if (m_currentCommand.retryCount > 0) {
                    m_currentCommand.retryCount--;
                    QTimer::singleShot(ProtocolTiming::BUSY_RETRY_DELAY_MS, this, [this]() {
                        m_commandQueue.prepend(m_currentCommand);
                        processCommandQueue();
                    });
                    return true;
                }
            } else if (returnCode == SpeeduinoRC::RC_CRC_ERR) {
                m_crcErrors++;
                Logger::warning("ECU reported CRC error, retrying");
                if (m_currentCommand.retryCount > 0) {
                    m_currentCommand.retryCount--;
                    QTimer::singleShot(150, this, [this]() {
                        m_commandQueue.prepend(m_currentCommand);
                        processCommandQueue();
                    });
                } else {
                    processCommandQueue(); 
                }
                return true;
            } else if (returnCode == SpeeduinoRC::RC_RANGE_ERR) {
                Logger::error("Range error — offset+length exceeds page size");
                processCommandQueue();
                return true;
            } else if (returnCode == SpeeduinoRC::RC_UKWN_ERR) {
                Logger::error("Unknown command — firmware may be outdated");
                processCommandQueue();
                return true;
            }

            // Route response based on command type
            processResponse(payload);
            
            m_awaitingResponse = false;
            m_commandTimeoutTimer->stop();
            processCommandQueue();
            return true;
        } else {
            // CRC validation failed
            m_crcErrors++;
            Logger::warning("CRC32 validation failed, packet discarded");
            m_awaitingResponse = false;
            m_commandTimeoutTimer->stop();
            processCommandQueue();
            return true;
        }
    }
    return false;
}

void SerialManager::onErrorOccurred(QSerialPort::SerialPortError portError) {
    if (portError == QSerialPort::NoError) return;

    Logger::error("Serial port error: " + m_serialPort->errorString());

    if (portError == QSerialPort::ResourceError ||
        portError == QSerialPort::PermissionError) {
        // FIX: clear error before close so the errorOccurred signal isn't
        // re-emitted when we call close() on the still-open port
        m_serialPort->clearError();
        if (m_serialPort->isOpen()) m_serialPort->close();

        m_status = ConnectionStatus::Error;
        m_heartbeatTimer->stop();
        m_dataPollingTimer->stop();
        m_commandTimeoutTimer->stop();
        m_commandQueue.clear();
        m_receiveBuffer.clear();
        m_awaitingResponse = false;

        emit connectionStatusChanged(m_status);
        emit disconnected();
        Logger::info("Disconnected from device");

        if (portError == QSerialPort::ResourceError) {
            m_reconnectAttempts = 0;
            m_reconnectTimer->start();
        }
    }
}

// ============================================================================
//  Command Queue
// ============================================================================

void SerialManager::processCommandQueue() {
    if (m_commandQueue.isEmpty()) return;
    if (m_awaitingResponse) return; // Still waiting for previous response

    m_currentCommand = m_commandQueue.dequeue();
    sendCommand(m_currentCommand.data);
    m_awaitingResponse = true;

    m_commandTimeoutTimer->start(m_currentCommand.timeoutMs);
}

void SerialManager::sendCommand(const QByteArray &command) {
    if (m_isSimulation) {
        if (command.isEmpty()) return;

        char cmdType = command.at(0);
        QByteArray responsePayload;

        if (cmdType == SpeeduinoCommands::CMD_REALTIME_DATA) { // 'A'
            // FIX-002: Complete 130-byte RT data at exact spec offsets
            // All offsets per Phase 1 §1.4, +1 for RC byte prefix at index 0
            responsePayload.resize(131);  // 1 RC + 130 data
            responsePayload.fill(0);
            responsePayload[0] = SpeeduinoRC::RC_OK;

            // secl (offset 0) — incrementing seconds counter
            static uint8_t simSecl = 0;
            responsePayload[1] = static_cast<char>(simSecl++);
            // status1 (offset 1) — INJ1 scheduled
            responsePayload[2] = 0x01;
            // engine (offset 2) — BIT_ENGINE_RUN
            responsePayload[3] = 0x01;
            // syncLossCounter (offset 3)
            responsePayload[4] = 0;
            // MAP U16LE (offset 4-5) = 101 kPa
            responsePayload[5] = 101; responsePayload[6] = 0;
            // IAT (offset 6) = 25°C → raw 65
            responsePayload[7] = 65;
            // CLT (offset 7) = 90°C → raw 130
            responsePayload[8] = static_cast<char>(130);
            // batCorrection (offset 8) = 100%
            responsePayload[9] = 100;
            // battery10 (offset 9) = 13.8V → raw 138
            responsePayload[10] = static_cast<char>(138);
            // O2 (offset 10) = AFR 14.7 → raw 147
            responsePayload[11] = static_cast<char>(147);
            // egoCorrection (offset 11) = 100%
            responsePayload[12] = 100;
            // iatCorrection (offset 12) = 100%
            responsePayload[13] = 100;
            // wueCorrection (offset 13) = 100%
            responsePayload[14] = 100;
            // RPM U16LE (offset 14-15) = 1000
            responsePayload[15] = static_cast<char>(0xE8);
            responsePayload[16] = static_cast<char>(0x03);
            // aeAmount (offset 16)
            responsePayload[17] = 0;
            // corrections U16LE (offset 17-18) = 100
            responsePayload[18] = 100; responsePayload[19] = 0;
            // ve1 (offset 19) = 75
            responsePayload[20] = 75;
            // ve2 (offset 20) = 0
            responsePayload[21] = 0;
            // afrTarget (offset 21) = 14.7 → raw 147
            responsePayload[22] = static_cast<char>(147);
            // tpsDOT S16LE (offset 22-23) = 0
            responsePayload[23] = 0; responsePayload[24] = 0;
            // advance S08 (offset 24) = 15°
            responsePayload[25] = 15;
            // TPS (offset 25) = 20% → raw 40 (×0.5)
            responsePayload[26] = 40;
            // loopsPerSecond U16LE (offset 26-27) = 200
            responsePayload[27] = static_cast<char>(200); responsePayload[28] = 0;
            // freeRAM U16LE (offset 28-29) = 8000
            responsePayload[29] = static_cast<char>(0x40); responsePayload[30] = static_cast<char>(0x1F);
            // boostTarget (offset 30) = 0
            responsePayload[31] = 0;
            // boostDuty (offset 31) = 0
            responsePayload[32] = 0;
            // status2 (offset 32) — full sync
            responsePayload[33] = static_cast<char>(0x80);
            // rpmDOT S16LE (offset 33-34) = 0
            responsePayload[34] = 0; responsePayload[35] = 0;
            // ethanolPct (offset 35) = 0
            responsePayload[36] = 0;
            // flexCorrection (offset 36) = 100
            responsePayload[37] = 100;
            // flexIgnCorrection (offset 37) = 0
            responsePayload[38] = 0;
            // idleLoad (offset 38) = 30
            responsePayload[39] = 30;
            // testOutputs (offset 39) = 0
            responsePayload[40] = 0;
            // O2_2 (offset 40) = 14.7 → raw 147
            responsePayload[41] = static_cast<char>(147);
            // baro (offset 41) = 101 kPa
            responsePayload[42] = 101;
            // canin[0-15] (offset 42-73) = all zero (already filled)
            // tpsADC (offset 74) = 80
            responsePayload[75] = 80;
            // errorByte (offset 75) = 0
            responsePayload[76] = 0;
            // PW1 U16LE (offset 76-77) = 3500 µs
            responsePayload[77] = static_cast<char>(0xAC); responsePayload[78] = static_cast<char>(0x0D);
            // PW2-4 (offset 78-83) = 0 (already filled)
            // status3 (offset 84) = nSquirts=2 → (2 << 5) = 0x40
            responsePayload[85] = 0x40;
            // engineProtectStatus (offset 85) = 0
            responsePayload[86] = 0;
            // fuelLoad S16LE (offset 86-87) = 101
            responsePayload[87] = 101; responsePayload[88] = 0;
            // ignLoad S16LE (offset 88-89) = 101
            responsePayload[89] = 101; responsePayload[90] = 0;
            // dwell U16LE (offset 90-91) = 30 → 3.0ms
            responsePayload[91] = 30; responsePayload[92] = 0;
            // CLIdleTarget (offset 92) = 85 → 850 RPM
            responsePayload[93] = 85;
            // mapDOT S16LE (offset 93-94) = 0
            responsePayload[94] = 0; responsePayload[95] = 0;
            // vvt1Angle S16LE (offset 95-96) = 0
            responsePayload[96] = 0; responsePayload[97] = 0;
            // vvt1TargetAngle (offset 97) = 0
            responsePayload[98] = 0;
            // vvt1Duty (offset 98) = 0
            responsePayload[99] = 0;
            // flexBoostCorrection S16LE (offset 99-100) = 0
            responsePayload[100] = 0; responsePayload[101] = 0;
            // baroCorrection (offset 101) = 100
            responsePayload[102] = 100;
            // VE (offset 102) = 75
            responsePayload[103] = 75;
            // ASEValue (offset 103) = 0
            responsePayload[104] = 0;
            // vss U16LE (offset 104-105) = 0
            responsePayload[105] = 0; responsePayload[106] = 0;
            // gear (offset 106) = 0
            responsePayload[107] = 0;
            // fuelPressure (offset 107) = 45 PSI
            responsePayload[108] = 45;
            // oilPressure (offset 108) = 40 PSI
            responsePayload[109] = 40;
            // wmiPW (offset 109) = 0
            responsePayload[110] = 0;
            // status4 (offset 110) = fan off, no burn pending
            responsePayload[111] = 0;
            // vvt2Angle S16LE (offset 111-112) = 0
            responsePayload[112] = 0; responsePayload[113] = 0;
            // vvt2TargetAngle (113), vvt2Duty (114), outputsStatus (115) = 0
            // fuelTemp (offset 116) = 30°C → raw 70
            responsePayload[117] = 70;
            // fuelTempCorrection (offset 117) = 100
            responsePayload[118] = 100;
            // advance1 S08 (offset 118) = 15
            responsePayload[119] = 15;
            // advance2 S08 (offset 119) = 0
            responsePayload[120] = 0;
            // TS_SD_Status (offset 120) = 0
            responsePayload[121] = 0;
            // EMAP S16LE (offset 121-122) = 0
            responsePayload[122] = 0; responsePayload[123] = 0;
            // fanDuty (offset 123) = 0
            responsePayload[124] = 0;
            // airConStatus (offset 124) = 0
            responsePayload[125] = 0;
            // actualDwell U16LE (offset 125-126) = 3000 µs
            responsePayload[126] = static_cast<char>(0xB8); responsePayload[127] = static_cast<char>(0x0B);
            // status5 (offset 127) = 0
            responsePayload[128] = 0;
            // knockCount (offset 128) = 0
            responsePayload[129] = 0;
            // knockRetard (offset 129) = 0
            responsePayload[130] = 0;

            processResponse(responsePayload);

        } else if (cmdType == SpeeduinoCommands::CMD_SIGNATURE) { // 'S'
            responsePayload = "Speeduino 2025.01 (Sim)";
            processResponse(responsePayload);

        } else if (cmdType == SpeeduinoCommands::CMD_READ_PAGE) { // 'p'
            if (command.size() >= 7) {
                quint8 page = static_cast<quint8>(command[2]);
                quint16 offset = static_cast<quint8>(command[3]) |
                                 (static_cast<quint8>(command[4]) << 8);
                quint16 size = static_cast<quint8>(command[5]) |
                               (static_cast<quint8>(command[6]) << 8);

                if (page < 256) {
                    if (m_simulatedMemory[page].size() < offset + size) {
                        m_simulatedMemory[page].resize(offset + size + 100);
                    }
                    // Response: RC + data
                    responsePayload.append(static_cast<char>(SpeeduinoRC::RC_OK));
                    responsePayload.append(m_simulatedMemory[page].mid(offset, size));
                    processResponse(responsePayload);
                }
            }
        } else if (cmdType == SpeeduinoCommands::CMD_WRITE_PAGE) { // 'M'
            if (command.size() >= 7) {
                quint8 page = static_cast<quint8>(command[2]);
                quint16 offset = static_cast<quint8>(command[3]) |
                                 (static_cast<quint8>(command[4]) << 8);
                quint16 len = static_cast<quint8>(command[5]) |
                              (static_cast<quint8>(command[6]) << 8);

                if (command.size() >= 7 + len && page < 256) {
                    QByteArray dataToWrite = command.mid(7, len);
                    if (m_simulatedMemory[page].size() < offset + len) {
                        m_simulatedMemory[page].resize(offset + len + 100);
                    }
                    m_simulatedMemory[page].replace(offset, len, dataToWrite);
                    responsePayload.append(static_cast<char>(SpeeduinoRC::RC_OK));
                    processResponse(responsePayload);
                }
            }
        } else if (cmdType == SpeeduinoCommands::CMD_BURN_PAGE) { // 'b'
            Logger::info("Simulation: Burn command for page " +
                         QString::number(command.size() > 2 ? static_cast<quint8>(command[2]) : 0));
            responsePayload.append(static_cast<char>(SpeeduinoRC::RC_BURN_OK));
            processResponse(responsePayload);
        }

        m_packetsSent++;
        m_awaitingResponse = false;
        processCommandQueue();
        return;
    }

    // Real hardware
    if (m_serialPort->isOpen()) {
        QByteArray packet;
        if (m_currentCommand.isRaw) {
            packet = command; // Send raw byte
        } else {
            packet = SpeeduinoProtocol::wrapNewProtocol(command); // Wrap in CRC
        }

        m_serialPort->write(packet);
        m_serialPort->flush();
        
        // Force the bits out the wire immediately
        if (!m_serialPort->waitForBytesWritten(100)) {
            Logger::warning("Hardware write timeout on " + m_portName);
        }

        m_packetsSent++;
        
        // Log the first byte for debugging
        uint8_t opcode = command.isEmpty() ? 0 : static_cast<uint8_t>(command.at(0));
        Logger::info(QString("TX: Sent %1 command '%2' (0x%3) - Packet size: %4")
                     .arg(m_currentCommand.isRaw ? "Legacy" : "Wrapped")
                     .arg(QChar(opcode))
                     .arg(opcode, 2, 16, QChar('0'))
                     .arg(packet.size()));
    }
}

void SerialManager::queueCommand(const SerialCommand &command) {
    m_commandQueue.enqueue(command);
    processCommandQueue();
}

void SerialManager::onCommandTimeout() {
    Logger::warning("Command timeout (type: " + QString::number(static_cast<int>(m_currentCommand.type)) + ")");
    m_awaitingResponse = false;
    m_receiveBuffer.clear(); // Clear buffer to prevent stale data corruption

    if (m_currentCommand.retryCount > 0) {
        m_currentCommand.retryCount--;
        Logger::info("Retrying command...");
        m_commandQueue.prepend(m_currentCommand);
        processCommandQueue();
    } else {
        Logger::error("Command failed after retries");
        processCommandQueue();
    }
}

// ============================================================================
//  Response Processing — Routes by CommandType (BUG-007 fix)
// ============================================================================

void SerialManager::processResponse(const QByteArray &payload) {
    if (payload.isEmpty()) return;

    CommandType cmdType = m_currentCommand.type;

    switch (cmdType) {
        case CommandType::RealTimeData: {
            QByteArray dataBytes = m_currentCommand.isRaw ? payload : payload.mid(1);

            if (dataBytes.size() >= 120) {
                try {
                    RealTimeData rtData;
                    if (m_ecuDefinition.getOutputChannels().isEmpty()) {
                        rtData = m_protocol->parseRealTimeData(dataBytes);
                    } else {
                        SpeeduinoProtocol::DynamicRTData dyn = m_protocol->parseRealTimeDataDynamic(dataBytes, m_ecuDefinition);
                        rtData = dyn.legacy;
                    }
                    
                    if (rtData.secl < m_lastSecl && m_lastSecl > 10) {
                        Logger::warning("ECU restart detected (secl roll: " +
                                        QString::number(m_lastSecl) + " -> " +
                                        QString::number(rtData.secl) + ")");
                    }
                    m_lastSecl = rtData.secl;
                    emit dataReceived(rtData);
                } catch (const QString &e) {
                    Logger::error("RT parse error: " + e);
                }
            }
            break;
        }

        case CommandType::Signature: {
            // Signature is a string (may or may not have RC byte)
            QByteArray sigData = payload;
            // If first byte is RC_OK, skip it
            if (!sigData.isEmpty() && static_cast<uint8_t>(sigData.at(0)) == SpeeduinoRC::RC_OK) {
                sigData = sigData.mid(1);
            }

            ECUSignature sig = m_protocol->parseSignature(sigData);
            if (sig.isValid()) {
                // ===== SIGNATURE VALIDATION GATE (Phase 1) =====
                // Validate received signature against loaded definition
                auto validation = m_ecuDefinition.validateSignature(sig);
                
                if (!validation.isValid) {
                    // Signature mismatch — BLOCK connection
                    Logger::error("SIGNATURE VALIDATION FAILED: " + validation.message);
                    emit signatureValidationFailed(validation.message);
                    emit error("ECU signature validation failed: " + validation.message);
                    disconnectFromDevice();
                    return;
                }
                
                // Signature validated — store it, then query capabilities
                Logger::info("Signature validation passed: " + validation.message);
                m_signatureValidated = true;
                m_signature = sig;
                Logger::info("Signature OK: " + sig.toString() + " — querying capabilities...");

                // Queue 'f' capabilities request before marking connected
                SerialCommand capCmd;
                capCmd.data = m_protocol->createCapabilitiesRequest(); // 'f'
                capCmd.type = CommandType::Capabilities;
                capCmd.expectedResponse = -1;
                capCmd.timeoutMs = ProtocolTiming::TIMEOUT_MS;
                capCmd.retryCount = ProtocolTiming::RETRY_COUNT;
                queueCommand(capCmd);
            } else {
                Logger::error("Invalid ECU signature");
                disconnectFromDevice();
            }
            break;
        }

        case CommandType::ReadPage: {
            // Payload: [RC] + data bytes
            QByteArray pageData = payload;
            if (!pageData.isEmpty() && static_cast<uint8_t>(pageData.at(0)) == SpeeduinoRC::RC_OK) {
                pageData = pageData.mid(1);
            }
            emit tableResponseReceived(m_currentCommand.pageNum,
                                       m_currentCommand.pageOffset,
                                       pageData);
            break;
        }

        case CommandType::WritePage: {
            uint8_t rc = static_cast<uint8_t>(payload.at(0));
            if (rc == SpeeduinoRC::RC_OK) {
                Logger::info("Write to page " + QString::number(m_currentCommand.pageNum) + " OK");
            } else {
                Logger::error("Write failed, RC: 0x" + QString::number(rc, 16));
            }
            break;
        }

        case CommandType::BurnPage: {
            uint8_t rc = static_cast<uint8_t>(payload.at(0));
            if (rc == SpeeduinoRC::RC_BURN_OK || rc == SpeeduinoRC::RC_OK) {
                Logger::info("Burn page " + QString::number(m_currentCommand.pageNum) +
                             " acknowledged (RC: 0x" + QString::number(rc, 16) + ")");
            } else {
                Logger::error("Burn failed, RC: 0x" + QString::number(rc, 16));
            }
            break;
        }

        case CommandType::PageCRC: {
            // Payload: [RC] + CRC32 (4 bytes BE)
            if (payload.size() >= 5) {
                uint32_t pageCRC = (static_cast<uint8_t>(payload.at(1)) << 24) |
                                   (static_cast<uint8_t>(payload.at(2)) << 16) |
                                   (static_cast<uint8_t>(payload.at(3)) << 8)  |
                                   static_cast<uint8_t>(payload.at(4));
                Logger::info("Page " + QString::number(m_currentCommand.pageNum) +
                             " CRC: 0x" + QString::number(pageCRC, 16).toUpper());
                emit pageCRCReceived(m_currentCommand.pageNum, pageCRC);
            }
            break;
        }

        case CommandType::Capabilities: {
            // Payload: [RC_OK] [proto_ver 1B] [blockingFactor 2B BE] [tableBlockingFactor 2B BE]
            QByteArray capData = payload;
            if (!capData.isEmpty() && static_cast<uint8_t>(capData.at(0)) == SpeeduinoRC::RC_OK) {
                capData = capData.mid(1);
            }
            if (capData.size() >= 5) {
                m_signature.protocolVersion = static_cast<uint8_t>(capData.at(0));
                m_signature.blockingFactor = (static_cast<uint8_t>(capData.at(1)) << 8) |
                                              static_cast<uint8_t>(capData.at(2));
                m_signature.tableBlockingFactor = (static_cast<uint8_t>(capData.at(3)) << 8) |
                                                  static_cast<uint8_t>(capData.at(4));
                Logger::info("Capabilities: proto=" + QString::number(m_signature.protocolVersion) +
                             " blockFactor=" + QString::number(m_signature.blockingFactor) +
                             " tableBlockFactor=" + QString::number(m_signature.tableBlockingFactor));
            } else {
                Logger::warning("Capabilities response too short (" + QString::number(capData.size()) +
                                " bytes), using defaults");
                m_signature.protocolVersion = 2;
                m_signature.blockingFactor = 251;
                m_signature.tableBlockingFactor = 256;
            }

            // NOW mark connection as fully established
            m_status = ConnectionStatus::Connected;
            emit connectionStatusChanged(m_status);
            emit connected(m_signature);
            Logger::info("Connected to ECU: " + m_signature.toString());

            if (m_isPolling) {
                m_dataPollingTimer->start(m_pollingInterval);
            }
            m_heartbeatTimer->start();
            break;
        }

        case CommandType::TestComms: {
            // Heartbeat response: [RC_OK, 0xFF] — just acknowledge
            Logger::info("Heartbeat acknowledged");
            break;
        }

        default: {
            // Unknown command type — emit as generic table response for backward compat
            emit tableResponseReceived(0, 0, payload);
            break;
        }
    }
}

// ============================================================================
//  Heartbeat / Polling
// ============================================================================

void SerialManager::sendHeartbeat() {
    if (!m_isPolling) {
        if (m_commandQueue.isEmpty() && !m_awaitingResponse) {
            SerialCommand cmd;
            cmd.data = m_protocol->createTestCommsRequest(); // 'C' test comms
            cmd.type = CommandType::TestComms;
            cmd.expectedResponse = -1;
            cmd.timeoutMs = ProtocolTiming::TIMEOUT_MS;
            cmd.retryCount = 0;
            queueCommand(cmd);
        }
    }
}

void SerialManager::requestRealTimeData() {
    // Only poll if we are fully connected and validated
    if (m_status != ConnectionStatus::Connected) return;

    // If TX has been dead for too long (>200ms), force-clear the waiting flag
    if (m_awaitingResponse && m_lastCommandSent.msecsTo(QDateTime::currentDateTime()) > 200) {
        Logger::warning("TX Lock detected! Force-clearing awaitingResponse flag.");
        m_awaitingResponse = false;
    }

    if (m_commandQueue.size() < 2 && !m_awaitingResponse) {
        SerialCommand cmd;
        cmd.data = "A"; // Speeduino Real-time data request
        cmd.type = CommandType::RealTimeData;
        cmd.isRaw = true; // ALWAYS use raw 'A' for maximum compatibility (matches TunerStudio)
        cmd.timeoutMs = 120; // Fast timeout for 30Hz polling
        cmd.retryCount = 0;
        queueCommand(cmd);
        m_lastCommandSent = QDateTime::currentDateTime();
    }
}

// ============================================================================
//  Reconnection
// ============================================================================

void SerialManager::attemptReconnection() {
    if (m_reconnectAttempts >= m_maxReconnectAttempts) {
        m_reconnectTimer->stop();
        Logger::error("Max reconnection attempts reached");
        return;
    }

    m_reconnectAttempts++;
    Logger::info("Attempting reconnection (" +
                 QString::number(m_reconnectAttempts) + ")...");

    // FIX: ensure port is fully closed before trying to re-open it
    if (m_serialPort->isOpen()) {
        m_serialPort->clearError();
        m_serialPort->close();
    }

    QList<QSerialPortInfo> ports = detectDevices();
    bool portFound = false;
    for (const auto &port : ports) {
        if (port.portName() == m_portName) { portFound = true; break; }
    }

    if (portFound) {
        if (connectToDevice(m_portName, m_baudRate)) {
            m_reconnectTimer->stop();
        }
    }
}
