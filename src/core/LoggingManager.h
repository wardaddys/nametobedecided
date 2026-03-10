/**
 * @file LoggingManager.h
 * @brief Manages data logging to CSV files
 */

#ifndef LOGGINGMANAGER_H
#define LOGGINGMANAGER_H

#include "ECUData.h"
#include <QDateTime>
#include <QFile>
#include <QObject>
#include <QTextStream>
#include <QTimer>


class LoggingManager : public QObject {
  Q_OBJECT

public:
  explicit LoggingManager(QObject *parent = nullptr);
  ~LoggingManager();

  bool startLogging(const QString &filePath = QString());
  void stopLogging();
  bool isLogging() const;
  QString getCurrentRate() const;
  qint64 getRecordCount() const;
  qint64 getFileSize() const;

public slots:
  void processData(const RealTimeData &data);

signals:
  void loggingStarted(const QString &filePath);
  void loggingStopped();
  void errorOccurred(const QString &error);
  void statsUpdated(qint64 records, qint64 bytes, double rateHz);
  void bufferUsageUpdated(int percentage);

private:
  QFile m_logFile;
  QTextStream m_stream;
  bool m_isLogging;
  qint64 m_recordCount;
  qint64 m_startTime;

  // Rate calculation
  qint64 m_lastUpdate;
  int m_recordsSinceUpdate;
  double m_currentRate;

  QString generateDefaultFilename() const;
  void writeHeader();
};

#endif // LOGGINGMANAGER_H
