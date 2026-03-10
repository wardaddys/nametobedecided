/**
 * @file main.cpp
 * @brief TunerPro ECU Tuning Application - Main Entry Point
 *
 * This is the starting point of the TunerPro application. It initializes
 * the Qt application framework, sets up the main window, applies the dark
 * theme, and starts the event loop.
 *
 * @author Safeerullah Afridi (FA-22-151)
 * @author Muhammad Saeed Sajid (FA-22-143)
 * @supervisor Prof. Dr. Shariq Hussain
 * @date November 2025
 * @version 1.0.0
 */

#include "MainWindow.h"
#include "utils/Logger.h"
#include "utils/Settings.h"
#include "widgets/TunerProSplashScreen.h"
#include <QApplication>
#include <QEventLoop>
#include <QTimer>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QFontDatabase>

/**
 * @brief Load application fonts
 */
void loadFonts() {
  QStringList fontFiles = {
    ":/fonts/JetBrainsMono-Regular.ttf",
    ":/fonts/JetBrainsMono-Light.ttf",
    ":/fonts/BarlowCondensed-Regular.ttf",
    ":/fonts/BarlowCondensed-Medium.ttf",
    ":/fonts/BarlowCondensed-SemiBold.ttf"
  };

  for (const QString& fontFile : fontFiles) {
    int fontId = QFontDatabase::addApplicationFont(fontFile);
    if (fontId == -1) {
      Logger::error("Failed to load font: " + fontFile);
    }
  }
}

/**
 * @brief Load and apply the application stylesheet
 *
 * This function loads the dark theme QSS file from resources and applies
 * it to the entire application. The dark theme provides better visibility
 * for gauges and matches automotive tuning software conventions.
 *
 * @param app Pointer to QApplication instance
 * @return true if stylesheet loaded successfully, false otherwise
 */
bool loadStyleSheet(QApplication *app) {
  QFile styleFile(":/styles/darktheme.qss");

  if (!styleFile.open(QFile::ReadOnly | QFile::Text)) {
    Logger::error("Failed to load stylesheet: " + styleFile.errorString());
    return false;
  }

  QTextStream stream(&styleFile);
  QString styleSheet = stream.readAll();
  app->setStyleSheet(styleSheet);
  styleFile.close();

  Logger::info("Dark theme stylesheet loaded successfully");
  return true;
}

/**
 * @brief Main application entry point
 *
 * Initializes the Qt application, configures application metadata,
 * loads settings and stylesheet, creates the main window, and starts
 * the event loop.
 *
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 * @return Application exit code (0 for success)
 */
int main(int argc, char *argv[]) {
  // Create Qt application instance
  QApplication app(argc, argv);

  // Set application metadata (used for QSettings organization)
  QApplication::setOrganizationName("TunerProTeam");
  QApplication::setOrganizationDomain("tunerpro.org");
  QApplication::setApplicationName("TunerPro");
  QApplication::setApplicationVersion("1.0.0-alpha");

  // Initialize logging system
  Logger::initialize("TunerPro.log");
  Logger::info("=== TunerPro ECU Tuning Application Starting ===");
  Logger::info("Version: 1.0.0-alpha");
  Logger::info("Build Date: " + QString(__DATE__) + " " + QString(__TIME__));

  // Initialize settings system
  Settings::initialize();
  Logger::info("Settings initialized from: " + Settings::fileName());

  // Load fonts
  loadFonts();

  // Load and apply dark theme stylesheet
  if (!loadStyleSheet(&app)) {
    QMessageBox::warning(nullptr, "TunerPro - Warning",
                         "Failed to load stylesheet. Using default theme.");
  }

  // Show Splash Screen
  TunerProSplashScreen splash;
  splash.show();
  splash.startAnimations();
  
  QEventLoop splashLoop;
  TunerProSplashScreen::Action action = TunerProSplashScreen::Exit;
  QObject::connect(&splash, &TunerProSplashScreen::actionSelected, [&](TunerProSplashScreen::Action a){
      action = a;
      splashLoop.quit();
  });
  
  // Wait to let animation run to completion. 2.8 sec min, plus user click time.
  // Actually we need the user to click a button, which fades in.
  splashLoop.exec();

  if (action == TunerProSplashScreen::Exit) {
    Logger::info("Startup cancelled by user exiting");
    return 0;
  }

  // Handle Action Pre-Main Window
  if (action == TunerProSplashScreen::Browse) {
    // Clear last path so postInit prompts for new file
    Settings::setLastEcuDefPath("");
  }

  // Create and show main window
  MainWindow mainWindow;
  mainWindow.setWindowTitle("TunerPro - Open-Source ECU Tuning");
  mainWindow.resize(1400, 900); // Initial window size
  mainWindow.show();

  // The splash should theoretically close after 2.8s AND main window ready, 
  // but we blocked main on user input, so main window is ready now. We can finish splash.
  splash.finish(&mainWindow);

  // Post-initialization
  mainWindow.postInit();

  Logger::info("Main window created and displayed");

  // Start Qt event loop
  int exitCode = app.exec();

  // Cleanup before exit
  Logger::info("Application shutting down with exit code: " +
               QString::number(exitCode));
  Logger::info("=== TunerPro Terminated ===");

  return exitCode;
}
