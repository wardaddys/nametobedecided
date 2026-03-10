/**
 * @file Logger.h
 * @brief Application Logging Utility
 * 
 * Provides static methods for logging messages to both the console and a log file.
 * Supports different log levels (Info, Warning, Error).
 * 
 * @author Safeerullah Afridi (FA-22-151)
 * @author Muhammad Saeed Sajid (FA-22-143)
 * @supervisor Prof. Dr. Shariq Hussain
 * @date November 2025
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

/**
 * @class Logger
 * @brief Static logging utility class
 */
class Logger {
public:
    /**
     * @brief Initialize the logger
     * @param filename Name of the log file (e.g., "TunerPro.log")
     */
    static void initialize(const QString &filename);
    
    /**
     * @brief Log an informational message
     * @param message Message to log
     */
    static void info(const QString &message);
    
    /**
     * @brief Log a warning message
     * @param message Message to log
     */
    static void warning(const QString &message);
    
    /**
     * @brief Log an error message
     * @param message Message to log
     */
    static void error(const QString &message);

private:
    static QFile m_logFile;
    static QMutex m_mutex;
    
    static void write(const QString &level, const QString &message);
};

#endif // LOGGER_H
