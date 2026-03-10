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
#include "SpeeduinoProtocol.h"
#include <QObject>
#include <QQueue>
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

    SerialCommand() : type(CommandType::Unknown), expectedResponse(-1),
                      timeoutMs(1000), retryCount(3), pageNum(0), pageOffset(0) {}
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
    void queueCommand(const SerialCommand &command);

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
    void tryParseNewProtocolFrame();

    // === Member Variables ===
    QSerialPort *m_serialPort;
    ConnectionStatus m_status;
    ECUSignature m_signature;

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
    bool m_useNewProtocol;            ///< True = use new-protocol framing

    // Statistics
    int m_packetsSent;
    int m_packetsReceived;
    int m_crcErrors;
};

#endif // SERIALMANAGER_H
