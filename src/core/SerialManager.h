/**
 * @file SerialManager.h
 * @brief Serial communication manager for Speeduino ECU
 *
 * Implements new-protocol framing with CRC32, command tracking,
 * DTR reset handling, and proper response routing.
 */

#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include "ECUData.h"
#include "ECUDefinition.h"
#include "SpeeduinoProtocol.h"
#include <QObject>
#include <QQueue>
#include <QDateTime>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

/**
 * @brief Command queue entry for serial transmission
 */
struct SerialCommand {
    QByteArray data;          ///< Raw payload (before protocol wrapping)
    CommandType type;         ///< What kind of command this is
    int expectedResponse;     ///< Expected response size (-1 for variable)
    int timeoutMs;            ///< Timeout in milliseconds
    int retryCount;           ///< Number of retry attempts remaining
    quint8 pageNum;           ///< Page number (for read/write/burn tracking)
    quint16 pageOffset;       ///< Offset (for page read tracking)
    bool isRaw;               ///< True if this command should be sent WITHOUT protocol wrapping

    SerialCommand() : type(CommandType::Unknown), expectedResponse(-1),
                      timeoutMs(1000), retryCount(3), pageNum(0), pageOffset(0),
                      isRaw(true) {}  // Default RAW: handshake uses raw 'S', so ECU is in legacy mode
};

/**
 * @brief Manages serial communication with Speeduino ECU
 */
class SerialManager : public QObject {
    Q_OBJECT

public:
    explicit SerialManager(QObject *parent = nullptr);
    ~SerialManager();

    // === Connection Management ===
    QList<QSerialPortInfo> detectDevices();
    bool loadEcuDefinition(const QString &filePath);
    void setEcuDefinition(const ECUDefinition &def);
    bool connectToDevice(const QString &portName, qint32 baudRate = 115200);
    void disconnectFromDevice();
    bool isConnected() const;
    bool isSimulationConnected() const;
    ConnectionStatus getStatus() const;
    ECUSignature getSignature() const;

    // === Data Polling ===
    void startDataPolling(int intervalMs = 50);
    void stopDataPolling();
    void setPollingRate(int intervalMs);

    // === Simulation Mode ===
    void setSimulationMode(bool enabled);
    bool isSimulation() const { return m_isSimulation; }

    // === Table Operations ===
    void readTable(quint8 table, quint16 offset, quint16 size);
    void writeTable(quint8 table, quint16 offset, const QByteArray &data);
    void sendBurnCommand(quint8 page);
    void sendCalibrationTable(quint8 tableIndex, const QByteArray &tableData);
    void queueCommand(const SerialCommand &command);

    /**
     * @brief Returns true if the connected ECU's signature has been validated
     * against the loaded definition. Writes, burns, and calibration uploads
     * are blocked when this is false (unless running in simulation mode).
     */
    bool isSignatureValidated() const { return m_signatureValidated; }

    SpeeduinoProtocol *m_protocol;

signals:
    void connectionStatusChanged(ConnectionStatus status);
    void connected(const ECUSignature &signature);
    void disconnected();
    void error(const QString &error);
    void dataReceived(const RealTimeData &data);
    void pollingRateChanged(double hz);
    void tableResponseReceived(quint8 table, quint16 offset, const QByteArray &data);
    void pageCRCReceived(quint8 page, uint32_t crc);
    void toothDataReceived(const QByteArray &data);
    
    /**
     * @brief Emitted when ECU signature fails validation against loaded definition
     * 
     * This is a CRITICAL safety event. Writes must be blocked when this signal
     * is emitted, as the ECU may not be compatible with the tuning definition.
     */
    void signatureValidationFailed(const QString &reason);

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);
    void processCommandQueue();
    void onCommandTimeout();
    void sendHeartbeat();
    void requestRealTimeData();

private:
    void sendCommand(const QByteArray &command);
    void processResponse(const QByteArray &payload);
    void attemptReconnection();
    bool tryParseNewProtocolFrame();

    // === Member Variables ===
    QSerialPort *m_serialPort;
    ConnectionStatus m_status;
    ECUSignature m_signature;
    ECUDefinition m_ecuDefinition;          ///< Loaded ECU definition for signature validation
    bool m_signatureValidated = false;      ///< True if signature matches loaded definition

    // Simulation
    bool m_isSimulation = false;
    QByteArray m_simulatedMemory[256];

    // Receive buffer and command tracking
    QByteArray m_receiveBuffer;
    QQueue<SerialCommand> m_commandQueue;
    SerialCommand m_currentCommand;
    bool m_awaitingResponse;          ///< True when waiting for a response

    // Timers
    QTimer *m_heartbeatTimer;
    QTimer *m_dataPollingTimer;
    QTimer *m_commandTimeoutTimer;
    QTimer *m_reconnectTimer;

    // Connection info
    QString m_portName;
    qint32 m_baudRate;
    int m_reconnectAttempts;
    int m_maxReconnectAttempts;
    bool m_isPolling;
    int m_pollingInterval;

    // Protocol state
    uint8_t m_lastSecl;               ///< Last secl value (for restart detection)
    QDateTime m_lastCommandSent;      ///< Watchdog for TX activity
    bool m_waitingForRT = false;      ///< True when direct 'A' command was sent (bypasses queue)

    // Statistics
    int m_packetsSent;
    int m_packetsReceived;
    int m_crcErrors;
};

#endif // SERIALMANAGER_H
