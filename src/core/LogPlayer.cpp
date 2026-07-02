#include "LogPlayer.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

LogPlayer::LogPlayer(QObject *parent)
    : QObject(parent), m_currentIndex(0), m_isPlaying(false) {
    
    // Default playback rate (approx 30Hz, matches SerialManager polling)
    m_timer.setInterval(33);
    connect(&m_timer, &QTimer::timeout, this, &LogPlayer::onTick);
}

LogPlayer::~LogPlayer() {
    stop();
}

bool LogPlayer::loadLog(const QString &filePath) {
    stop();
    m_logData.clear();
    m_logName.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "LogPlayer: Failed to open log file" << filePath;
        emit logLoaded(false);
        return false;
    }

    QFileInfo fi(filePath);
    m_logName = fi.fileName();

    QTextStream in(&file);
    bool headerSkipped = false;
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!headerSkipped) {
            headerSkipped = true; // Skip header
            continue;
        }
        parseCsvLine(line);
    }
    
    file.close();
    m_currentIndex = 0;
    
    emit logLoaded(true);
    emit playbackProgress(0, getTotalDurationMs());
    return true;
}

void LogPlayer::parseCsvLine(const QString &line) {
    // Format: Timestamp,RPM,MAP,TPS,AFR,CLT,IAT,Batt,SpkAdv,PW1,Duty
    QStringList parts = line.split(',');
    if (parts.size() < 11) return;

    RealTimeData data;
    
    // Note: We don't really use the timestamp for pacing in this simple implementation,
    // but we store it in RealTimeData's timestamp if needed.
    qint64 ts = parts[0].toLongLong();
    data.timestamp = QDateTime::fromMSecsSinceEpoch(ts);

    data.rpm = static_cast<uint16_t>(parts[1].toInt());
    data.map = static_cast<uint16_t>(parts[2].toDouble());
    data.tps = static_cast<uint8_t>(parts[3].toDouble() * 2.0); // tps is stored as raw (val * 0.5)
    data.o2 = static_cast<uint8_t>(parts[4].toDouble() * 10.0); // afr is stored as raw (val * 0.1)
    data.coolant = static_cast<uint8_t>(parts[5].toDouble() + 40.0); // clt is (raw - 40)
    data.iat = static_cast<uint8_t>(parts[6].toDouble() + 40.0); // iat is (raw - 40)
    data.battery10 = static_cast<uint8_t>(parts[7].toDouble() * 10.0); // bat is (raw * 0.1)
    data.advance = static_cast<int8_t>(parts[8].toInt());
    data.pw1 = static_cast<uint16_t>(parts[9].toDouble() * 1000.0); // pw1 is ms, stored as us

    // Duty cycle is a computed value in ECUData, not stored directly, 
    // but the gauge will calculate it if needed, or we just rely on standard values.

    m_logData.append(data);
}

void LogPlayer::play() {
    if (m_logData.isEmpty()) return;
    if (m_currentIndex >= m_logData.size()) {
        m_currentIndex = 0; // Loop back to start if at end
    }
    m_isPlaying = true;
    m_timer.start();
    emit stateChanged(true);
}

void LogPlayer::pause() {
    m_isPlaying = false;
    m_timer.stop();
    emit stateChanged(false);
}

void LogPlayer::stop() {
    pause();
    m_currentIndex = 0;
    emit playbackProgress(0, getTotalDurationMs());
}

void LogPlayer::seek(int percentage) {
    if (m_logData.isEmpty()) return;
    
    percentage = qBound(0, percentage, 100);
    m_currentIndex = (percentage * (m_logData.size() - 1)) / 100;
    
    emit playbackProgress(getCurrentTimeMs(), getTotalDurationMs());
    
    // Emit the frame at the new position immediately
    if (m_currentIndex < m_logData.size()) {
        emit dataFrameReady(m_logData[m_currentIndex]);
    }
}

bool LogPlayer::isPlaying() const {
    return m_isPlaying;
}

bool LogPlayer::hasLogLoaded() const {
    return !m_logData.isEmpty();
}

int LogPlayer::getTotalDurationMs() const {
    if (m_logData.size() < 2) return 0;
    qint64 start = m_logData.first().timestamp.toMSecsSinceEpoch();
    qint64 end = m_logData.last().timestamp.toMSecsSinceEpoch();
    return static_cast<int>(end - start);
}

int LogPlayer::getCurrentTimeMs() const {
    if (m_logData.isEmpty() || m_currentIndex >= m_logData.size()) return 0;
    qint64 start = m_logData.first().timestamp.toMSecsSinceEpoch();
    qint64 current = m_logData[m_currentIndex].timestamp.toMSecsSinceEpoch();
    return static_cast<int>(current - start);
}

QString LogPlayer::getLogName() const {
    return m_logName;
}

void LogPlayer::onTick() {
    if (m_currentIndex < m_logData.size()) {
        emit dataFrameReady(m_logData[m_currentIndex]);
        emit playbackProgress(getCurrentTimeMs(), getTotalDurationMs());
        m_currentIndex++;
    } else {
        stop(); // Reached the end
    }
}
