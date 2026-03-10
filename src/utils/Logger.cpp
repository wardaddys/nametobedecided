/**
 * @file Logger.cpp
 * @brief Implementation of Logger
 * 
 * @author Safeerullah Afridi (FA-22-151)
 * @author Muhammad Saeed Sajid (FA-22-143)
 * @supervisor Prof. Dr. Shariq Hussain
 * @date November 2025
 */

#include "Logger.h"
#include <QDebug>
#include <iostream>

QFile Logger::m_logFile;
QMutex Logger::m_mutex;

void Logger::initialize(const QString &filename) {
    QMutexLocker locker(&m_mutex);
    m_logFile.setFileName(filename);
    if (!m_logFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Failed to open log file:" << filename;
    }
}

void Logger::info(const QString &message) {
    write("INFO", message);
}

void Logger::warning(const QString &message) {
    write("WARN", message);
}

void Logger::error(const QString &message) {
    write("ERROR", message);
}

void Logger::write(const QString &level, const QString &message) {
    QMutexLocker locker(&m_mutex);
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString logEntry = QString("[%1] [%2] %3").arg(timestamp, level, message);
    
    // Write to console
    if (level == "ERROR") {
        qCritical().noquote() << logEntry;
    } else if (level == "WARN") {
        qWarning().noquote() << logEntry;
    } else {
        qInfo().noquote() << logEntry;
    }
    
    // Write to file
    if (m_logFile.isOpen()) {
        QTextStream stream(&m_logFile);
        stream << logEntry << "\n";
        m_logFile.flush();
    }
}
