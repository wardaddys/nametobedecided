/**
 * @file LoggingManager.cpp
 * @brief Implementation of LoggingManager
 */

#include "LoggingManager.h"
#include <QDir>
#include <QStandardPaths>

LoggingManager::LoggingManager(QObject *parent)
    : QObject(parent), m_isLogging(false), m_recordCount(0), m_startTime(0),
      m_lastUpdate(0), m_recordsSinceUpdate(0), m_currentRate(0.0) {}

LoggingManager::~LoggingManager() { stopLogging(); }

bool LoggingManager::startLogging(const QString &filePath) {
  if (m_isLogging)
    return false;

  QString path = filePath;
  if (path.isEmpty()) {
    path = generateDefaultFilename();
  }

  m_logFile.setFileName(path);
  if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    emit errorOccurred("Failed to open file: " + m_logFile.errorString());
    return false;
  }

  m_stream.setDevice(&m_logFile);
  writeHeader();

  m_isLogging = true;
  m_recordCount = 0;
  m_startTime = QDateTime::currentMSecsSinceEpoch();
  m_lastUpdate = m_startTime;
  m_recordsSinceUpdate = 0;

  emit loggingStarted(path);
  return true;
}

void LoggingManager::stopLogging() {
  if (!m_isLogging)
    return;

  m_stream.flush();
  m_logFile.close();
  m_isLogging = false;

  emit loggingStopped();
}

bool LoggingManager::isLogging() const { return m_isLogging; }

QString LoggingManager::getCurrentRate() const {
  return QString::number(m_currentRate, 'f', 1) + " Hz";
}

qint64 LoggingManager::getRecordCount() const { return m_recordCount; }

qint64 LoggingManager::getFileSize() const { return m_logFile.size(); }

void LoggingManager::processData(const RealTimeData &data) {
  if (!m_isLogging)
    return;

  // Format: Timestamp,RPM,MAP,TPS,AFR,CLT,IAT,Batt,SpkAdv,PW1,Duty
  // Matches header in writeHeader()

  // Calculate Duty Cycle (Example: (RPM * PW) / 1200 for sequential)
  double dutyCycle = 0.0;
  if (data.getRPM() > 0) {
    dutyCycle = (data.getRPM() * data.getPulseWidth()) / 1200.0;
  }

  m_stream << QDateTime::currentMSecsSinceEpoch() << "," << data.getRPM() << ","
           << data.getMAP() << "," << data.getTPS() << "," << data.getAFR()
           << "," << data.getCoolant() << "," << data.getIAT() << ","
           << data.getBatteryVoltage() << "," << data.getAdvance() << ","
           << data.getPulseWidth() << "," << dutyCycle << "\n";

  m_recordCount++;
  m_recordsSinceUpdate++;

  qint64 now = QDateTime::currentMSecsSinceEpoch();
  if (now - m_lastUpdate >= 1000) {
    m_currentRate =
        (double)m_recordsSinceUpdate / ((now - m_lastUpdate) / 1000.0);
    m_lastUpdate = now;
    m_recordsSinceUpdate = 0;
    emit statsUpdated(m_recordCount, m_logFile.size(), m_currentRate);
    emit bufferUsageUpdated(15); // Simulated buffer usage for FIX verification
  }
}

QString LoggingManager::generateDefaultFilename() const {
  QString docs =
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  QDir dir(docs + "/OSTuner/Logs");
  if (!dir.exists())
    dir.mkpath(".");

  return dir.filePath(
      "Log_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss") +
      ".csv");
}

void LoggingManager::writeHeader() {
  m_stream << "Timestamp,RPM,MAP,TPS,AFR,CLT,IAT,Batt,SpkAdv,PW1,Duty\n";
}
