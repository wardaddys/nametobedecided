/**
 * @file Settings.cpp
 * @brief Implementation of Settings
 *
 * @author Safeerullah Afridi (FA-22-151)
 * @author Muhammad Saeed Sajid (FA-22-143)
 * @supervisor Prof. Dr. Shariq Hussain
 * @date November 2025
 */

#include "Settings.h"
#include <QCoreApplication>

QSettings *Settings::m_settings = nullptr;

void Settings::initialize() {
  if (!m_settings) {
    // QSettings will use organization name and app name set in main.cpp
    m_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                               QCoreApplication::organizationName(),
                               QCoreApplication::applicationName());
  }
}

QString Settings::fileName() {
  if (m_settings) {
    return m_settings->fileName();
  }
  return QString();
}

QString Settings::getLastPort() {
  if (m_settings)
    return m_settings->value("Connection/LastPort", "").toString();
  return "";
}

void Settings::setLastPort(const QString &port) {
  if (m_settings)
    m_settings->setValue("Connection/LastPort", port);
}

int Settings::getLastBaudRate() {
  if (m_settings)
    return m_settings->value("Connection/LastBaudRate", 115200).toInt();
  return 115200;
}

void Settings::setLastBaudRate(int baud) {
  if (m_settings)
    m_settings->setValue("Connection/LastBaudRate", baud);
}

bool Settings::getDarkThemeEnabled() {
  if (m_settings)
    return m_settings->value("UI/DarkTheme", true).toBool();
  return true;
}

void Settings::setDarkThemeEnabled(bool enabled) {
  if (m_settings)
    m_settings->setValue("UI/DarkTheme", enabled);
}

QString Settings::getLastEcuDefPath() {
  if (m_settings)
    return m_settings->value("ECU/LastDefPath", "").toString();
  return "";
}

void Settings::setLastEcuDefPath(const QString &path) {
  if (m_settings)
    m_settings->setValue("ECU/LastDefPath", path);
}
