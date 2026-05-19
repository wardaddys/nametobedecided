/**
 * @file Settings.h
 * @brief Application Settings Manager
 *
 * Manages persistent application settings using QSettings.
 * Stores preferences like last connected port, baud rate, and theme options.
 *
 * @author Safeerullah Afridi (FA-22-151)
 * @author Muhammad Saeed Sajid (FA-22-143)
 * @supervisor Prof. Dr. Shariq Hussain
 * @date November 2025
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QColor>
#include <QObject>
#include <QSettings>
#include <QString>

/**
 * @class Settings
 * @brief Static settings manager class
 */
class Settings {
public:
  /**
   * @brief Initialize settings
   */
  static void initialize();

  /**
   * @brief Get the path to the settings file
   * @return Absolute path to settings file
   */
  static QString fileName();

  // === Connection Settings ===
  static QString getLastPort();
  static void setLastPort(const QString &port);

  static int getLastBaudRate();
  static void setLastBaudRate(int baud);

  // === UI Settings ===
  static bool getDarkThemeEnabled();
  static void setDarkThemeEnabled(bool enabled);

  // === ECU Settings ===
  static QString getLastEcuDefPath();
  static void setLastEcuDefPath(const QString &path);

  // === Workspaces Preview (D2) ===
  // Off by default. When true, MainWindow shows a "Workspaces (Preview)" tab
  // that hosts the unified-settings WorkspaceContainer alongside the existing
  // ECU Settings UI.
  static bool getWorkspacesPreviewEnabled();
  static void setWorkspacesPreviewEnabled(bool enabled);

  // === Onboarding (Phase A) ===
  static bool getFirstRunCompleted();
  static void setFirstRunCompleted(bool completed);

private:
  static QSettings *m_settings;
};

#endif // SETTINGS_H
