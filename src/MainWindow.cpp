#include "MainWindow.h"
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDesktopServices>
#include <QUrl>
#include <QMouseEvent>
#include <QStyle>

// Include widget headers
#include "utils/Logger.h"
#include "utils/Settings.h"
#include "core/ECUSettingsManager.h"
#include "core/ProjectManager.h"
#include "core/UpdateChecker.h"
#include "widgets/AllTablesWidget.h"
#include "widgets/DashboardWidget.h"
#include "widgets/ECUSettingsWidget.h"
#include "widgets/LoggingWidget.h"
#include "widgets/SettingsDropdown.h"
#include "widgets/ToothLoggerWidget.h"
#include "dialogs/ConnectionDialog.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_serialManager(new SerialManager(this)),
      m_loggingManager(new LoggingManager(this)),
      m_settingsDropdown(nullptr),
      m_liveTuningEnabled(false) {
  // === Section 7.1: Fix window title ===
  setWindowTitle("TunerPro — Open-Source ECU Tuning");
  resize(1280, 800);
  
  // Set frameless window hint for custom titlebar
  setWindowFlags(Qt::FramelessWindowHint | Qt::Window);

  // Create centralized ECU Settings Manager before UI setup
  m_ecuSettingsManager = new ECUSettingsManager(this);
  m_ecuSettingsManager->setSerialManager(m_serialManager);
  
  // Create Project Manager
  m_projectManager = new ProjectManager(m_ecuSettingsManager, this);
  connect(m_projectManager, &ProjectManager::projectLoaded, [this](const QString &name) {
      m_projectButton->setText("Project: " + name);
      QMessageBox::information(this, "Project Loaded", "Successfully loaded project: " + name);
  });
  connect(m_projectManager, &ProjectManager::projectLoadFailed, [this](const QString &error) {
      QMessageBox::critical(this, "Project Load Error", "Failed to load project: " + error);
  });

  m_updateChecker = new UpdateChecker("1.0.0-alpha", this);
  connect(m_updateChecker, &UpdateChecker::updateAvailable, this, &MainWindow::onUpdateAvailable);
  connect(m_updateChecker, &UpdateChecker::upToDate, this, &MainWindow::onUpToDate);
  connect(m_updateChecker, &UpdateChecker::checkFailed, this, &MainWindow::onUpdateCheckFailed);

  setupUi();

  // Check for updates silently on startup
  m_updateChecker->checkForUpdates(true);

  // Create Settings Dropdown
  m_settingsDropdown = new SettingsDropdown(this);

  // Connect Settings to AllTables for conditional UI
  if (m_settingsDropdown && m_allTablesWidget) {
    connect(m_settingsDropdown, &SettingsDropdown::vtecSettingChanged,
            m_allTablesWidget, &AllTablesWidget::setVtecEnabled);
  }

  // Connect Serial Manager signals
  connect(m_serialManager, &SerialManager::dataReceived, this,
          &MainWindow::updateRealtimeData);
  connect(m_serialManager, &SerialManager::connectionStatusChanged, this,
          &MainWindow::onConnectionStatusChanged);
    connect(m_serialManager, &SerialManager::error, this,
      &MainWindow::onError);
    connect(m_serialManager, &SerialManager::disconnected, this,
      &MainWindow::onDisconnected);
    connect(m_serialManager, &SerialManager::signatureValidationFailed,
      this, [this](const QString &reason) {
        QMessageBox::critical(
      this,
      "ECU Signature Mismatch",
      "The connected ECU firmware signature does not match the loaded definition.\n\n"
      "Writes and burns were blocked for safety.\n\n"
      "Details: " + reason +
      "\n\nLoad the correct project/definition before reconnecting.");
      });

    connect(m_ecuSettingsManager, &ECUSettingsManager::errorOccurred,
      this, &MainWindow::onEcuSettingsError);

  // Connect Logging Manager
  connect(m_serialManager, &SerialManager::dataReceived, m_loggingManager,
          &LoggingManager::processData);

  // No clock timer — removed updateStatusBarClock (Section 2.5)
}

void MainWindow::postInit() {
  QString projPath = Settings::getLastEcuDefPath(); // We might repurpose this for project path
  bool loaded = false;

  if (!projPath.isEmpty() && QDir(projPath).exists()) {
    if (m_projectManager->loadProject(projPath)) {
      Logger::info("Project loaded from settings: " + projPath);
      loaded = true;
    } else {
      Logger::warning("Failed to load last used Project directory: " + projPath);
    }
  }

  if (!loaded) {
    showEcuDefError();
  }
}

void MainWindow::showEcuDefError() {
  QMessageBox::warning(
      this, "Project Required",
      "No valid TunerStudio Project is loaded.\n"
      "Please select a TunerStudio project directory.");

  onOpenProject();
}

void MainWindow::onOpenProject() {
  QString path = QFileDialog::getExistingDirectory(
      this, "Select TunerStudio Project Folder", QString(), QFileDialog::ShowDirsOnly);

  if (!path.isEmpty()) {
    if (m_projectManager->loadProject(path)) {
      Settings::setLastEcuDefPath(path);
    } else {
      showEcuDefError(); // Retry
    }
  }
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
  QWidget *centralWidget = new QWidget(this);
  QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  setCentralWidget(centralWidget);

  // 1. Custom Title Bar Area
  QWidget *titleBar = new QWidget(this);
  titleBar->setObjectName("TitleBar");
  QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
  titleLayout->setContentsMargins(16, 0, 16, 0);
  titleLayout->setSpacing(8);

  QLabel *appNameLabel = new QLabel("TunerPro", this);
  appNameLabel->setObjectName("TitleBarAppName");
  
  QLabel *projectNamePill = new QLabel("No Project", this);
  projectNamePill->setObjectName("ProjectNamePill");
  projectNamePill->setProperty("unsaved", false);
  
  QPushButton *closeBtn = new QPushButton(this);
  closeBtn->setObjectName("TitleBarClose");
  connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
  
  QPushButton *minBtn = new QPushButton(this);
  minBtn->setObjectName("TitleBarMinimize");
  connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
  
  QPushButton *maxBtn = new QPushButton(this);
  maxBtn->setObjectName("TitleBarMaximize");
  connect(maxBtn, &QPushButton::clicked, [this]() {
    isMaximized() ? showNormal() : showMaximized();
  });

  titleLayout->addWidget(appNameLabel);
  titleLayout->addStretch();
  titleLayout->addWidget(projectNamePill);
  titleLayout->addStretch();
  titleLayout->addWidget(minBtn);
  titleLayout->addWidget(maxBtn);
  titleLayout->addWidget(closeBtn);

  mainLayout->addWidget(titleBar);

  // 2. Toolbar
  QWidget *toolBar = new QWidget(this);
  toolBar->setObjectName("ToolBar");
  QHBoxLayout *toolbarLayout = new QHBoxLayout(toolBar);
  toolbarLayout->setContentsMargins(16, 0, 16, 0);
  toolbarLayout->setSpacing(12);

  m_readEcuCombo = new QComboBox(this);
  m_readEcuCombo->setObjectName("ComPortSelector"); // Need COM port selector eventually, repurposing readEcuCombo temporarily
  m_readEcuCombo->addItems({"Read/Write ECU", "Read ECU", "Write ECU"});

  m_ecuStatusLabel = new QLabel("OFFLINE", this);
  m_ecuStatusLabel->setObjectName("EcuStatusPill");
  m_ecuStatusLabel->setProperty("connected", false);

  m_liveTuningButton = new QPushButton("LIVE TUNING", this);
  m_liveTuningButton->setObjectName("LiveTuningToggle");
  m_liveTuningButton->setProperty("active", false);
  m_liveTuningButton->setCursor(Qt::PointingHandCursor);
  connect(m_liveTuningButton, &QPushButton::clicked, this, &MainWindow::onLiveTuningToggled);

  m_connectButton = new QPushButton("Connect", this);
  m_connectButton->setObjectName("ConnectButton");
  m_connectButton->setProperty("connected", false);
  connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);

  m_saveButton = new QPushButton("Save Details", this);
  m_saveButton->setObjectName("SaveButton");
  m_saveButton->setProperty("hasChanges", false);
  connect(m_saveButton, &QPushButton::clicked, this, &MainWindow::onSaveClicked);
  connect(m_readEcuCombo, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::onReadECUChanged);

  m_projectButton = new QPushButton("Open Project", this);
  m_projectButton->setObjectName("SaveButton"); // Borrowing style
  connect(m_projectButton, &QPushButton::clicked, this, &MainWindow::onOpenProject);

  m_settingsButton = new QPushButton(this);
  m_settingsButton->setObjectName("SettingsIconButton");
  m_settingsButton->setText("⚙");
  connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);

  toolbarLayout->addWidget(m_projectButton);
  toolbarLayout->addWidget(m_saveButton);
  toolbarLayout->addWidget(m_readEcuCombo);
  toolbarLayout->addStretch();
  toolbarLayout->addWidget(m_ecuStatusLabel);
  toolbarLayout->addWidget(m_liveTuningButton);
  toolbarLayout->addSpacing(16);
  toolbarLayout->addWidget(m_connectButton);
  toolbarLayout->addWidget(m_settingsButton);

  mainLayout->addWidget(toolBar);

  // 3. Navigation Tabs
  m_tabWidget = new QTabWidget(this);
  m_tabWidget->setObjectName("MainTabWidget");
  m_tabWidget->setTabPosition(QTabWidget::North);

  // === Section 1.1: Only create widgets that are actually displayed ===
  // The 9 tuning widgets are created inside ECUSettingsWidget — NOT here.
  m_dashboard = new DashboardWidget(this);
  m_allTablesWidget = new AllTablesWidget(m_ecuSettingsManager, this);
  m_allTablesWidget->setSerialManager(m_serialManager);
  m_ecuSettingsWidget = new ECUSettingsWidget(m_ecuSettingsManager, this);
  m_loggingWidget = new LoggingWidget(this);

  // Connect Logging Widget
  connect(m_loggingWidget, &LoggingWidget::startRequested, [this]() {
    if (m_loggingManager->startLogging()) {
      m_loggingWidget->setStatus(true, "Documents/TunerPro/Logs");
    }
  });
  connect(m_loggingWidget, &LoggingWidget::stopRequested, [this]() {
    m_loggingManager->stopLogging();
    m_loggingWidget->setStatus(false);
  });
  connect(m_loggingManager, &LoggingManager::statsUpdated, m_loggingWidget,
          &LoggingWidget::updateStats);
  connect(
      m_loggingManager, &LoggingManager::loggingStarted,
      [this](const QString &path) { m_loggingWidget->setStatus(true, path); });
  connect(m_loggingManager, &LoggingManager::loggingStopped,
          [this]() { m_loggingWidget->setStatus(false); });
  connect(m_loggingManager, &LoggingManager::bufferUsageUpdated,
          m_loggingWidget, &LoggingWidget::setBufferUsage);
  connect(m_loggingManager, &LoggingManager::errorOccurred,
          [this](const QString &err) {
            QMessageBox::warning(this, "Logging Error", err);
            m_loggingWidget->setStatus(false);
          });

  // Add Tabs - Consolidated layout
  m_tabWidget->addTab(m_dashboard, "Dashboard");
  m_tabWidget->addTab(m_allTablesWidget, "All Tables");
  m_tabWidget->addTab(m_ecuSettingsWidget, "ECU Settings");
  m_tabWidget->addTab(m_loggingWidget, "Logging");

  m_toothLoggerWidget = new ToothLoggerWidget(this);
  m_tabWidget->addTab(m_toothLoggerWidget, "Tooth Logger");

  mainLayout->addWidget(m_tabWidget);

  // 3. Status Bar
  createStatusBar();

  // 4. Menus
  QMenu *helpMenu = menuBar()->addMenu("Help");
  QAction *aboutAct = helpMenu->addAction("About TunerPro");
  connect(aboutAct, &QAction::triggered, this, &MainWindow::onAboutClicked);

  QAction *updateAct = helpMenu->addAction("Check for Updates...");
  connect(updateAct, &QAction::triggered, this, &MainWindow::onManualUpdateCheck);
}

void MainWindow::createStatusBar() {
  QStatusBar *bar = statusBar();
  bar->setObjectName("StatusBar");

  auto addStatusField = [&](QLabel** labelPtr, const QString& prefixName) {
    QWidget* container = new QWidget(this);
    QHBoxLayout* hLay = new QHBoxLayout(container);
    hLay->setContentsMargins(0, 0, 0, 0);
    hLay->setSpacing(4);
    
    QLabel* prefixLabel = new QLabel(prefixName + ":", this);
    prefixLabel->setProperty("class", "FieldLabel");
    
    *labelPtr = new QLabel("0", this);
    (*labelPtr)->setProperty("class", "FieldValue");
    (*labelPtr)->setProperty("alert", "safe");
    
    hLay->addWidget(prefixLabel);
    hLay->addWidget(*labelPtr);
    
    QLabel* div = new QLabel("|", this);
    div->setProperty("class", "FieldDivider");
    
    bar->addPermanentWidget(container);
    bar->addPermanentWidget(div);
  };

  addStatusField(&m_rpmLabel, "RPM");
  addStatusField(&m_mapLabel, "MAP");
  addStatusField(&m_afrLabel, "AFR");
  addStatusField(&m_ectLabel, "CLT");
  addStatusField(&m_boostLabel, "BOOST");
  addStatusField(&m_speedLabel, "SPEED");
  addStatusField(&m_gearLabel, "GEAR");
  addStatusField(&m_oilTLabel, "OIL T");
  addStatusField(&m_oilPLabel, "OIL P");
  addStatusField(&m_fuelPLabel, "FUEL P");

  m_ecuSavedLabel = new QLabel("ECU Saved: ---", this);
  m_ecuSavedLabel->setProperty("class", "FieldValue");
  bar->addPermanentWidget(m_ecuSavedLabel);
}

void MainWindow::setupDarkTheme() {
    // Redundant - theme is loaded from resources QSS globally now.
}

void MainWindow::onConnectionStatusChanged(ConnectionStatus status) {
  bool connected = (status == ConnectionStatus::Connected);
  updateConnectionUI(connected);

  if (connected) {
    // === Section 1.2: Wire SerialManager to ECUSettingsWidget on connect ===
    if (m_ecuSettingsWidget) {
      m_ecuSettingsWidget->setSerialManager(m_serialManager);
    }

    // Start data polling
    m_serialManager->startDataPolling();

    // Read all ECU pages after a brief delay to let connection stabilize
    QTimer::singleShot(500, this, [this]() {
      if (m_ecuSettingsWidget && m_serialManager && m_serialManager->isConnected()) {
        m_ecuSettingsWidget->readAllFromECU();
        Logger::info("Reading all ECU configuration pages after connect...");
      }
    });
  }
}

// === Section 2.2: updateConnectionUI helper ===
void MainWindow::updateConnectionUI(bool connected) {
  if (connected) {
    m_ecuStatusLabel->setText("ONLINE");
    m_ecuStatusLabel->setProperty("connected", true);
    m_ecuStatusLabel->style()->unpolish(m_ecuStatusLabel);
    m_ecuStatusLabel->style()->polish(m_ecuStatusLabel);

    m_connectButton->setText("Disconnect");
    m_connectButton->setProperty("connected", true);
    m_connectButton->style()->unpolish(m_connectButton);
    m_connectButton->style()->polish(m_connectButton);
  } else {
    m_ecuStatusLabel->setText("OFFLINE");
    m_ecuStatusLabel->setProperty("connected", false);
    m_ecuStatusLabel->style()->unpolish(m_ecuStatusLabel);
    m_ecuStatusLabel->style()->polish(m_ecuStatusLabel);

    m_connectButton->setText("Connect");
    m_connectButton->setProperty("connected", false);
    m_connectButton->style()->unpolish(m_connectButton);
    m_connectButton->style()->polish(m_connectButton);

    // Turn off live tuning on disconnect
    if (m_liveTuningEnabled) {
      onLiveTuningToggled();
    }

    if (m_ecuSavedLabel) {
      m_ecuSavedLabel->setText("ECU Saved: ---");
    }
  }
}

// === Section 2.1 + Section 3: Full data bridge from serial to UI ===
void MainWindow::updateRealtimeData(const RealTimeData &data) {
  // === Status Bar: ALL 11 labels (Section 3.1) ===
  if (m_rpmLabel)
    m_rpmLabel->setText(QString("RPM: %1").arg(data.getRPM()));
  if (m_mapLabel)
    m_mapLabel->setText(QString("MAP: %1 kPa").arg(data.getMAP(), 0, 'f', 1));
  if (m_afrLabel)
    m_afrLabel->setText(QString("AFR: %1").arg(data.getAFR(), 0, 'f', 1));
  if (m_ectLabel)
    m_ectLabel->setText(QString("ECT: %1°C").arg(data.getCoolant(), 0, 'f', 0));

  // Boost = MAP - atmospheric (101.3 kPa), converted to psi
  if (m_boostLabel) {
    double boostKpa = data.getMAP() - 101.3;
    double boostPsi = boostKpa * 0.14504;
    m_boostLabel->setText(QString("Boost: %1 psi").arg(boostPsi, 0, 'f', 1));
  }

  // Vehicle Speed (Not in standard Speeduino, estimate from RPM)
  if (m_speedLabel) {
    int speed = static_cast<int>(data.getRPM() / 45.0);
    m_speedLabel->setText(QString("Speed: %1 km/h (Est)").arg(speed));
  }

  // Gear estimate
  if (m_gearLabel) {
    int speed = static_cast<int>(data.getRPM() / 45.0);
    QString gear = "N";
    if (data.getRPM() > 500 && speed > 5) { // BUG-012 FIX: More robust speed threshold
      double ratio = static_cast<double>(data.getRPM()) / speed;
      if (ratio > 90) gear = "1";
      else if (ratio > 60) gear = "2";
      else if (ratio > 45) gear = "3";
      else if (ratio > 35) gear = "4";
      else gear = "5";
    }
    m_gearLabel->setText(QString("Gear: %1 (Est)").arg(gear));
  }

  // Oil Temperature - Not in standard Speeduino
  if (m_oilTLabel)
    m_oilTLabel->setText("Oil Temp: N/A");

  // Oil Pressure - Not in standard Speeduino
  if (m_oilPLabel)
    m_oilPLabel->setText("Oil P: N/A");

  // Fuel Pressure - Not in standard Speeduino
  if (m_fuelPLabel)
    m_fuelPLabel->setText("Fuel P: N/A");

  // === Section 2.1: Forward to Dashboard ===
  if (m_dashboard) {
    m_dashboard->updateData(data);
  }

  // === Section 5.3: Forward to Settings Widget ===
  if (m_ecuSettingsWidget) {
    m_ecuSettingsWidget->updateRealtimeData(data);
  }
}

void MainWindow::onConnectClicked() {
  if (m_serialManager->isConnected()) {
    m_serialManager->disconnectFromDevice();
    updateConnectionUI(false);
    return;
  }

  // Show port selection dialog
  auto ports = m_serialManager->detectDevices();
  if (!ports.isEmpty()) {
      ConnectionDialog dialog(this);
      if (dialog.exec() == QDialog::Accepted) {
          QString portName = dialog.getPortName();
          int baudRate = dialog.getBaudRate();
          if (!portName.isEmpty()) {
              m_serialManager->connectToDevice(portName, baudRate);
              // updateConnectionUI will be called via onConnectionStatusChanged signal
          }
      }
  } else {
      // No ports found - Suggest Simulation
      QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, "No Devices Found",
                                    "No Speeduino device detected.\nWould you like to enter Simulation Mode?",
                                    QMessageBox::Yes|QMessageBox::No);
      if (reply == QMessageBox::Yes) {
          m_serialManager->setSimulationMode(true);
          if (m_serialManager->connectToDevice("SIMULATOR")) {
             m_connectButton->setText("Disconnect (Sim)");
             m_connectButton->setStyleSheet(
                "QPushButton { background-color: #f57f17; color: white; border: none; padding: 6px; border-radius: 4px; }"
                "QPushButton:hover { background-color: #fbc02d; }");
          }
      }
  }
}

// === Section 4.1: Real Save Functionality ===
void MainWindow::onSaveClicked() {
  if (!m_serialManager->isConnected()) {
    QMessageBox::warning(this, "Not Connected",
        "Cannot save to ECU — no connection established.");
    return;
  }

  // FIX-007: Burn through the single canonical settings manager only
  if (m_ecuSettingsManager) {
    m_ecuSettingsManager->burnAllDirty();
  }
  // Do NOT also call m_ecuSettingsWidget->burnAllDirty() — they share the same manager

  // Update timestamp label with visual feedback
  if (m_ecuSavedLabel) {
    m_ecuSavedLabel->setText("ECU Saved: " +
        QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
  }

  Logger::info("Burn all dirty pages to flash — Save complete");
}

// === Section 4.4: Read/Write ECU Dropdown ===
void MainWindow::onReadECUChanged(int index) {
  if (index <= 0) return; // Index 0 is the placeholder label

  if (!m_serialManager->isConnected()) {
    QMessageBox::warning(this, "Not Connected",
        "Cannot read/write ECU — no connection established.");
    m_readEcuCombo->setCurrentIndex(0);
    return;
  }

  // FIX-007: Only use the canonical m_ecuSettingsManager, not also m_ecuSettingsWidget
  if (index == 1) {
    // READ ECU: Pull all config pages from ECU into local cache
    if (m_ecuSettingsManager) {
      m_ecuSettingsManager->readAllFromECU();
    }
    Logger::info("Reading all ECU pages...");
  } else if (index == 2) {
    // WRITE ECU: Push all dirty pages to ECU RAM, then burn to flash
    if (m_ecuSettingsManager) {
      m_ecuSettingsManager->burnAllDirty();
    }
    Logger::info("Writing all dirty pages and burning to flash...");
  }

  // Reset combo to default display
  m_readEcuCombo->setCurrentIndex(0);
}

void MainWindow::onSettingsClicked() {
  if (!m_settingsDropdown) {
    m_settingsDropdown = new SettingsDropdown(this);
  }

  if (m_settingsDropdown->isVisible()) {
    m_settingsDropdown->hide();
  } else {
    QPoint p =
        m_settingsButton->mapToGlobal(QPoint(0, m_settingsButton->height()));
    p.setX(p.x() + m_settingsButton->width() - m_settingsDropdown->width());
    m_settingsDropdown->move(p);
    m_settingsDropdown->show();
  }
}

// === Section 2.2: Disconnect ===
void MainWindow::onDisconnectClicked() {
  if (m_serialManager->isConnected()) {
    m_serialManager->disconnectFromDevice();
  }
  updateConnectionUI(false);
  Logger::info("Disconnected from ECU");
}

void MainWindow::onAboutClicked() {
  QMessageBox::about(this, "About TunerPro",
                     "TunerPro Version 2.0\n\nOpen-source tuning software for Speeduino ECU.");
}

// === Section 2.3: Demo Mode ===
void MainWindow::onEnableDemoMode() {
  m_serialManager->setSimulationMode(true);
  m_serialManager->connectToDevice("SIMULATOR");
  updateConnectionUI(true);
  Logger::info("Demo mode enabled");
}

void MainWindow::onDisableDemoMode() {
  if (m_serialManager->isConnected()) {
    m_serialManager->disconnectFromDevice();
  }
  m_serialManager->setSimulationMode(false);
  updateConnectionUI(false);
  Logger::info("Demo mode disabled");
}

// === Section 2.1: Data bridge ===
void MainWindow::onDataReceived(const RealTimeData &data) {
  updateRealtimeData(data);
}

// === Section 2.4: Error handling ===
void MainWindow::onError(const QString &error) {
  Logger::error("Serial error: " + error);
  if (statusBar()) {
    statusBar()->showMessage("Error: " + error, 5000);
  }
}

void MainWindow::onEcuSettingsError(const QString &error) {
  Logger::error("ECU settings error: " + error);

  if (statusBar()) {
    statusBar()->showMessage("ECU Settings Error: " + error, 6000);
  }

  if (error.contains("No ECU definition loaded", Qt::CaseInsensitive)) {
    QMessageBox::warning(
        this,
        "Definition Required",
        "ECU operations were blocked because no definition is loaded.\n\n"
        "Open a valid TunerStudio project before reading/writing ECU settings.\n\n"
        "Details: " + error);
    return;
  }

  if (error.contains("guardrail", Qt::CaseInsensitive) ||
      error.contains("blocked", Qt::CaseInsensitive)) {
    QMessageBox::critical(
        this,
        "Safety Guardrail Triggered",
        "A safety guardrail blocked this ECU operation to prevent invalid writes.\n\n"
        "Details: " + error);
    return;
  }

  QMessageBox::warning(this, "ECU Settings Error", error);
}

// === Section 2.4: Disconnection handling ===
void MainWindow::onDisconnected() {
  updateConnectionUI(false);
  if (statusBar()) {
    statusBar()->showMessage("ECU disconnected", 5000);
  }
  Logger::warning("ECU connection lost");
}

// === Section 5.3: Live Tuning Toggle ===
void MainWindow::onLiveTuningToggled() {
  m_liveTuningEnabled = !m_liveTuningEnabled;

  if (m_liveTuningEnabled) {
    m_liveTuningButton->setText("LIVE TUNING");
    m_liveTuningButton->setProperty("active", true);
    m_liveTuningButton->style()->unpolish(m_liveTuningButton);
    m_liveTuningButton->style()->polish(m_liveTuningButton);

    if (m_allTablesWidget)
      m_allTablesWidget->setLiveTuningEnabled(true);

  } else {
    m_liveTuningButton->setText("LIVE TUNING");
    m_liveTuningButton->setProperty("active", false);
    m_liveTuningButton->style()->unpolish(m_liveTuningButton);
    m_liveTuningButton->style()->polish(m_liveTuningButton);

      m_allTablesWidget->setLiveTuningEnabled(false);
  }
}

void MainWindow::onManualUpdateCheck() {
    m_updateChecker->checkForUpdates(false);
}

void MainWindow::onUpdateAvailable(const QString& latestVersion, const QString& downloadUrl, const QString& releaseNotesUrl, bool silentMode) {
    if (silentMode) {
        // Optional: show a non-intrusive notification, but for now we'll do standard popup
        // if we want to mimic a simple check
    }
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Update Available",
        QString("A new version of TunerPro (v%1) is available.\nWould you like to download it now?").arg(latestVersion),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        QDesktopServices::openUrl(QUrl(downloadUrl));
    }
}

void MainWindow::onUpToDate(bool silentMode) {
    if (!silentMode) {
        QMessageBox::information(this, "Up to Date", "You are running the latest version of TunerPro.");
    }
}

// === Custom Titlebar Window Dragging ===
void MainWindow::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    // Check if click is within the top titlebar area (assume height 36)
    QWidget* titleBar = findChild<QWidget*>("TitleBar");
    if (titleBar && titleBar->geometry().contains(event->pos())) {
      m_dragging = true;
      m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
      event->accept();
    }
  }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
  if (m_dragging && (event->buttons() & Qt::LeftButton)) {
    move(event->globalPosition().toPoint() - m_dragPosition);
    event->accept();
  }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    m_dragging = false;
    event->accept();
  }
}

void MainWindow::onUpdateCheckFailed(const QString& errorMessage, bool silentMode) {
    if (!silentMode) {
        QMessageBox::warning(this, "Update Check Failed", errorMessage);
    }
}
