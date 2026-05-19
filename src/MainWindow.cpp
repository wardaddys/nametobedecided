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
#include <QPointer>
#include <QStyle>

// Include widget headers
#include "utils/Logger.h"
#include "utils/Settings.h"
#include "core/ECUSettingsManager.h"
#include "core/version.h"
#include "core/ProjectManager.h"
#include "core/UpdateChecker.h"
// D2: Workspaces preview tab.
#include "widgets/workspaces/WorkspaceContainer.h"
#include "widgets/workspaces/FuelingWorkspace.h"
#include "widgets/workspaces/IgnitionWorkspace.h"
#include "widgets/AllTablesWidget.h"
#include "widgets/DashboardWidget.h"
#include "widgets/ECUSettingsWidget.h"
#include "widgets/LoggingWidget.h"
#include "widgets/SettingsDropdown.h"
#include "widgets/ToothLoggerWidget.h"
#include "widgets/ProjectWizardOverlay.h"
#include "dialogs/ConnectionDialog.h"
#include "widgets/onboarding/ProductTourOverlay.h"
#include "widgets/onboarding/TourStep.h"
#include <QFileDialog>
#include <QFileIconProvider>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

class ProjectIconProvider : public QFileIconProvider {
public:
    ProjectIconProvider() {
        m_projectIcon = QIcon(":/icons/app_icon.png");
    }
    
    QIcon icon(const QFileInfo &info) const override {
        if (info.isDir()) {
            QDir dir(info.absoluteFilePath());
            if (dir.exists("project.properties") || dir.exists("projectCfg")) {
                return m_projectIcon;
            }
        }
        return QFileIconProvider::icon(info);
    }
private:
    QIcon m_projectIcon;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_serialManager(new SerialManager(this)),
      m_loggingManager(new LoggingManager(this)),
      m_settingsDropdown(nullptr),
      m_liveTuningEnabled(false) {
  // === Section 7.1: Fix window title ===
  setWindowTitle("OS Tuner — Open-Source ECU Tuning");
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
      if (!Settings::getFirstRunCompleted()) {
          startProductTour();
      }
  });
  connect(m_projectManager, &ProjectManager::projectLoadFailed, [this](const QString &error) {
      QMessageBox::critical(this, "Project Load Error", "Failed to load project: " + error);
  });

  m_updateChecker = new UpdateChecker(OSTUNER_VERSION_STRING, this);
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
  // Auto-restore the last opened project.
  //
  // Bug fix: previously this method always ran showEcuDefError() on failure,
  // even when ProjectManager::projectLoadFailed had *already* shown a
  // critical-error QMessageBox (MainWindow ctor wires that signal at line 59).
  // That stacked two dialogs on top of each other every time the saved path
  // pointed at a folder that no longer had a valid INI — which is exactly
  // the "Failed to load project" + "No valid project is loaded" pair the
  // user reported. We now stay silent on a failed auto-restore (the ctor's
  // signal handler covers user feedback) and only prompt the user to pick a
  // project when there was no saved path at all.
  const QString projPath = Settings::getLastEcuDefPath();
  if (projPath.isEmpty()) {
    showEcuDefError();
    return;
  }
  if (!QDir(projPath).exists()) {
    Logger::warning("Last project path no longer exists: " + projPath);
    Settings::setLastEcuDefPath("");
    showEcuDefError();
    return;
  }
  if (m_projectManager->loadProject(projPath)) {
    Logger::info("Project loaded from settings: " + projPath);
  } else {
    // ProjectManager already emitted projectLoadFailed → critical dialog.
    // Clear the stale setting so the next launch starts clean.
    Logger::warning("Failed to load last used Project directory: " + projPath);
    Settings::setLastEcuDefPath("");
  }
}

void MainWindow::showEcuDefError() {
  QMessageBox::warning(
      this, "Project Required",
      "No valid project is loaded.\n"
      "Please select an ECU project directory.");

  onOpenProject();
}

void MainWindow::onOpenProject() {
  QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/TunerStudioProjects";
  if (!QDir(defaultPath).exists()) {
      defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  }

  QFileDialog dialog(this, "Select ECU Project Folder", defaultPath);
  dialog.setFileMode(QFileDialog::Directory);
  dialog.setOption(QFileDialog::ShowDirsOnly, true);
  dialog.setOption(QFileDialog::DontUseNativeDialog, true);
  
  ProjectIconProvider *iconProvider = new ProjectIconProvider;
  dialog.setIconProvider(iconProvider);

  if (dialog.exec() == QDialog::Accepted) {
    QString path = dialog.selectedFiles().first();
    if (m_projectManager->loadProject(path)) {
      Settings::setLastEcuDefPath(path);
    } else {
      showEcuDefError(); // Retry
    }
  }
  delete iconProvider;
}

MainWindow::~MainWindow() {
  // [BUG-SHUTDOWN] Sever ALL signal connections from SerialManager to this
  // MainWindow BEFORE any child-widget destructors run. Without this,
  // SerialManager::~SerialManager() → disconnectFromDevice() emits
  // connectionStatusChanged / disconnected into a half-destroyed MainWindow,
  // triggering Qt's "Called object is not of the correct type" assert.
  if (m_serialManager) {
    disconnect(m_serialManager, nullptr, this, nullptr);
    m_serialManager->disconnectFromDevice();
  }
}


void MainWindow::setupUi() {
  QWidget *centralWidget = new QWidget(this);
  QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  setCentralWidget(centralWidget);
  
  // Create Wizard Overlay (covers central widget)
  m_projectWizard = new ProjectWizardOverlay(centralWidget);
  m_projectWizard->setSerialManager(m_serialManager);
  connect(m_projectWizard, &ProjectWizardOverlay::projectCreated, this, &MainWindow::onProjectCreated);
  m_projectWizard->raise(); // ensure it's on top

  // 1. Consolidated Header Bar (Title + Tools)
  QWidget *headerBar = new QWidget(this);
  headerBar->setObjectName("HeaderBar");
  QHBoxLayout *headerLayout = new QHBoxLayout(headerBar);
  headerLayout->setContentsMargins(16, 0, 16, 0);
  headerLayout->setSpacing(12);

  // -- Left Section: Brand & Project --
  QLabel *appNameLabel = new QLabel("OS TUNER", this);
  appNameLabel->setObjectName("TitleBarAppName");
  
  QLabel *projectNamePill = new QLabel("No Project", this);
  projectNamePill->setObjectName("ProjectNamePill");
  projectNamePill->setProperty("unsaved", false);

  headerLayout->addWidget(appNameLabel);
  headerLayout->addWidget(projectNamePill);
  headerLayout->addSpacing(20);

  // -- Center Section: Primary Actions --
  m_newProjectButton = new QPushButton("New Project", this);
  m_newProjectButton->setObjectName("SaveButton");
  connect(m_newProjectButton, &QPushButton::clicked, this, &MainWindow::onNewProjectClicked);

  m_projectButton = new QPushButton("Open Project", this);
  m_projectButton->setObjectName("SaveButton");
  connect(m_projectButton, &QPushButton::clicked, this, &MainWindow::onOpenProject);

  m_saveButton = new QPushButton("Save", this);
  m_saveButton->setObjectName("SaveButton");
  m_saveButton->setProperty("hasChanges", false);
  connect(m_saveButton, &QPushButton::clicked, this, &MainWindow::onSaveClicked);

  m_readEcuCombo = new QComboBox(this);
  m_readEcuCombo->setObjectName("ComPortSelector");
  m_readEcuCombo->addItems({"Sync ECU", "Read All", "Write All"});
  connect(m_readEcuCombo, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::onReadECUChanged);

  headerLayout->addWidget(m_newProjectButton);
  headerLayout->addWidget(m_projectButton);
  headerLayout->addWidget(m_saveButton);
  headerLayout->addWidget(m_readEcuCombo);
  headerLayout->addStretch();

  // -- Right Section: Status, Help, Settings & Controls --
  m_ecuStatusLabel = new QLabel("OFFLINE", this);
  m_ecuStatusLabel->setObjectName("EcuStatusPill");
  m_ecuStatusLabel->setProperty("connected", false);

  m_liveTuningButton = new QPushButton("LIVE", this);
  m_liveTuningButton->setObjectName("LiveTuningToggle");
  m_liveTuningButton->setProperty("active", false);
  m_liveTuningButton->setCursor(Qt::PointingHandCursor);
  connect(m_liveTuningButton, &QPushButton::clicked, this, &MainWindow::onLiveTuningToggled);

  m_connectButton = new QPushButton("Connect", this);
  m_connectButton->setObjectName("ConnectButton");
  m_connectButton->setProperty("connected", false);
  connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);

  m_helpButton = new QPushButton(this);
  m_helpButton->setObjectName("HelpIconButton"); // Use custom objectName
  m_helpButton->setText("?");
  m_helpButton->setToolTip("Help & Updates");
  connect(m_helpButton, &QPushButton::clicked, this, &MainWindow::onAboutClicked);

  m_settingsButton = new QPushButton(this);
  m_settingsButton->setObjectName("SettingsIconButton");
  m_settingsButton->setText("⚙");
  connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);

  // Window Controls
  QPushButton *minBtn = new QPushButton(this);
  minBtn->setObjectName("TitleBarMinimize");
  connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
  
  QPushButton *maxBtn = new QPushButton(this);
  maxBtn->setObjectName("TitleBarMaximize");
  connect(maxBtn, &QPushButton::clicked, [this]() {
    isMaximized() ? showNormal() : showMaximized();
  });

  QPushButton *closeBtn = new QPushButton(this);
  closeBtn->setObjectName("TitleBarClose");
  connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);

  headerLayout->addWidget(m_ecuStatusLabel);
  headerLayout->addWidget(m_liveTuningButton);
  headerLayout->addSpacing(8);
  headerLayout->addWidget(m_connectButton);
  headerLayout->addSpacing(8);
  headerLayout->addWidget(m_helpButton);
  headerLayout->addWidget(m_settingsButton);
  headerLayout->addSpacing(8);
  headerLayout->addWidget(minBtn);
  headerLayout->addWidget(maxBtn);
  headerLayout->addWidget(closeBtn);

  mainLayout->addWidget(headerBar);

  // 3. Navigation Tabs
  m_tabWidget = new QTabWidget(this);
  m_tabWidget->setObjectName("MainTabWidget");
  m_tabWidget->setTabPosition(QTabWidget::North);

  // === Section 1.1: Only create widgets that are actually displayed ===
  // The 9 tuning widgets are created inside ECUSettingsWidget — NOT here.
  m_dashboard = new DashboardWidget(this);
  m_dashboard->setSettingsManager(m_ecuSettingsManager);  // Fix 2: wire offline scalar updates
  m_dashboard->setSerialManager(m_serialManager);         // Wire serial for calibration uploads

  m_allTablesWidget = new AllTablesWidget(m_ecuSettingsManager, this);
  m_allTablesWidget->setSerialManager(m_serialManager);
  m_ecuSettingsWidget = new ECUSettingsWidget(m_ecuSettingsManager, this);
  m_loggingWidget = new LoggingWidget(this);

  // Connect Logging Widget
  connect(m_loggingWidget, &LoggingWidget::startRequested, [this]() {
    if (m_loggingManager->startLogging()) {
      m_loggingWidget->setStatus(true, "Documents/OSTuner/Logs");
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

  // D2: Workspaces (Preview) tab. Opt-in via Settings flag — default off so
  // the existing ECU Settings UI remains the canonical edit surface.
  if (Settings::getWorkspacesPreviewEnabled()) {
    auto* workspaces = new WorkspaceContainer(this);
    auto* fueling   = new FuelingWorkspace(this);
    auto* ignition  = new IgnitionWorkspace(this);
    fueling->setSettingsManager(m_ecuSettingsManager);
    ignition->setSettingsManager(m_ecuSettingsManager);
    workspaces->registerWorkspace(fueling);
    workspaces->registerWorkspace(ignition);
    m_tabWidget->addTab(workspaces, "Workspaces (Preview)");
  }

  mainLayout->addWidget(m_tabWidget);

  // 3. Status Bar
  createStatusBar();

  // 4. Removed separate QMenuBar to prevent extra top bar
  // Help and Updates are now accessible via the '?' icon button in the header.
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
    if (m_toothLoggerWidget) {
      m_toothLoggerWidget->setSerialManager(m_serialManager);
    }

    // Start data polling
    m_serialManager->startDataPolling(33); // 30Hz for responsive movement

    // [FIX-QUEUE] Do NOT auto-read pages on connect — they flood the command
    // queue and starve RT polling. User can trigger manually via toolbar.
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
  // 1. Update Dashboard Widget
  if (m_dashboard) {
    m_dashboard->updateData(data);
  }

  // 2. Update Status Bar Labels (prefix is already in the separate QLabel from addStatusField)
  if (m_rpmLabel)
    m_rpmLabel->setText(QString::number(data.getRPM()));
  if (m_mapLabel)
    m_mapLabel->setText(QString("%1 kPa").arg(data.getMAP(), 0, 'f', 1));
  if (m_afrLabel)
    m_afrLabel->setText(QString::number(data.getAFR(), 'f', 1));
  if (m_ectLabel)
    m_ectLabel->setText(QString("%1°C").arg(data.getCoolant(), 0, 'f', 0));

  // Boost = MAP - atmospheric (101.3 kPa), converted to psi
  if (m_boostLabel) {
    double boostKpa = data.getMAP() - 101.3;
    double boostPsi = boostKpa * 0.14504;
    m_boostLabel->setText(QString("%1 psi").arg(boostPsi, 0, 'f', 1));
  }

  // Vehicle Speed
  if (m_speedLabel) {
    m_speedLabel->setText(QString("%1 km/h").arg(data.getVSS()));
  }

  // Gear
  if (m_gearLabel) {
    m_gearLabel->setText(data.gear == 0 ? "N" : QString::number(data.gear));
  }

  // Oil Temperature - Not in standard Speeduino
  if (m_oilTLabel)
    m_oilTLabel->setText("N/A");

  // Oil Pressure - Not in standard Speeduino
  if (m_oilPLabel)
    m_oilPLabel->setText("N/A");

  // Fuel Pressure - Not in standard Speeduino
  if (m_fuelPLabel)
    m_fuelPLabel->setText("N/A");

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
  QMenu helpMenu(this);
  helpMenu.addAction("About OS Tuner", this, [this]() {
      QMessageBox::about(this, "About OS Tuner",
                       "OS Tuner Version 0.5.0-alpha\n\nOpen-source tuning software for Speeduino ECU.");
  });
  helpMenu.addAction("Replay Onboarding Tour", this, &MainWindow::startProductTour);
  helpMenu.addAction("Check for Updates...", this, &MainWindow::onManualUpdateCheck);
  
  helpMenu.exec(QCursor::pos());
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
// BUG-B fix: both branches null-check m_allTablesWidget before calling
// setLiveTuningEnabled. The original code only guarded the enable path; the
// disable path would crash on shutdown teardown when the widget had been freed.
void MainWindow::onLiveTuningToggled() {
  m_liveTuningEnabled = !m_liveTuningEnabled;

  if (m_liveTuningEnabled) {
    m_liveTuningButton->setText("LIVE TUNING");
    m_liveTuningButton->setProperty("active", true);
    m_liveTuningButton->style()->unpolish(m_liveTuningButton);
    m_liveTuningButton->style()->polish(m_liveTuningButton);

    if (m_allTablesWidget) // BUG-B fix: guard
      m_allTablesWidget->setLiveTuningEnabled(true);

  } else {
    m_liveTuningButton->setText("LIVE TUNING");
    m_liveTuningButton->setProperty("active", false);
    m_liveTuningButton->style()->unpolish(m_liveTuningButton);
    m_liveTuningButton->style()->polish(m_liveTuningButton);

    if (m_allTablesWidget) // BUG-B fix: guard (this branch was missing pre-Phase 0)
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
        QString("A new version of OS Tuner (v%1) is available.\nWould you like to download it now?").arg(latestVersion),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        QDesktopServices::openUrl(QUrl(downloadUrl));
    }
}

void MainWindow::onUpToDate(bool silentMode) {
    if (!silentMode) {
        QMessageBox::information(this, "Up to Date", "You are running the latest version of OS Tuner.");
    }
}

// === Custom Titlebar Window Dragging ===
void MainWindow::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    // Check if click is within the top header area (assume height 48)
    QWidget* headerBar = findChild<QWidget*>("HeaderBar");
    if (headerBar && headerBar->geometry().contains(event->pos())) {
      m_dragging = true;
      m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
      event->accept();
    }
  }
}

void MainWindow::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
}

void MainWindow::onNewProjectClicked() {
    if (m_projectWizard) {
        m_projectWizard->resize(centralWidget()->size());
        m_projectWizard->startWizard();
    }
}

void MainWindow::onProjectCreated(const QString &name, const QString &path) {
    // Save to settings
    Settings::setLastEcuDefPath(path);
    
    // Actually load the project (which triggers the projectLoaded signal to update UI)
    if (m_projectManager->loadProject(path)) {
        Logger::info("Created and loaded new project: " + name + " at " + path);
    } else {
        QMessageBox::warning(this, "Project Error", "Project was created but failed to load.");
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

void MainWindow::startProductTour() {
    if (!m_tourOverlay) {
        m_tourOverlay = new ProductTourOverlay(this->centralWidget());
        connect(m_tourOverlay, &ProductTourOverlay::tourCompleted, this, []() {
            Settings::setFirstRunCompleted(true);
            QMessageBox::information(nullptr, "Tour Completed", 
                "Want the full guide on tuning? Click Help (?) -> Learn to Tune. Otherwise, you're ready!");
        });
        connect(m_tourOverlay, &ProductTourOverlay::tourSkipped, this, []() {
            Settings::setFirstRunCompleted(true);
        });
    }

    // Step 1: Dashboard gauges row (RPM Gauge)
    TourStep s1;
    s1.anchorWidget = m_dashboard->getRpmGauge();
    s1.title = "Live Engine Sensors";
    s1.body = "These are your live engine sensors. Once your ECU is connected, they update 100× a second.";
    s1.padding = 12;
    s1.onShow = [this]() {
        m_tabWidget->setCurrentIndex(0); // Switch to Dashboard tab
    };
    m_tourOverlay->addStep(s1);

    // Step 2: Deliberately-not-calibrated gauge (MAP Gauge)
    TourStep s2;
    s2.anchorWidget = m_dashboard->getMapGauge();
    s2.title = "Sensor Calibration Alert";
    s2.body = "If a gauge glows orange, that sensor isn't calibrated yet. Click it to set it up (e.g. TPS, MAP, Temp).";
    s2.padding = 12;
    s2.onShow = [this]() {
        m_tabWidget->setCurrentIndex(0); // Switch to Dashboard tab
    };
    m_tourOverlay->addStep(s2);

    // Step 3: Workspace tabs
    TourStep s3;
    QWidget *tabBar = m_tabWidget->findChild<QWidget*>("qt_tabwidget_tabbar");
    s3.anchorWidget = tabBar ? tabBar : m_tabWidget;
    s3.title = "Workspace Navigation";
    s3.body = "Fueling, Ignition, Sensors — this is where you tune. Most of your time will be in Fueling.";
    s3.padding = 8;
    s3.onShow = [this]() {
        m_tabWidget->setCurrentIndex(0); // Switch to Dashboard tab
    };
    m_tourOverlay->addStep(s3);

    // Step 4: The 3D fuel map
    TourStep s4;
    s4.anchorWidget = m_allTablesWidget->getGraphContainer();
    s4.title = "3D VE Table Visualization";
    s4.body = "This is a VE table. Each cell tells the ECU how much air the engine is breathing at that RPM and load.";
    s4.padding = 12;
    s4.onShow = [this]() {
        m_tabWidget->setCurrentIndex(1); // Switch to All Tables tab
        if (m_allTablesWidget->getMainViewTabs()) {
            m_allTablesWidget->getMainViewTabs()->setCurrentIndex(1); // Switch to 3D tab within All Tables
        }
    };
    m_tourOverlay->addStep(s4);

    // Step 5: Burn/Save Button
    TourStep s5;
    s5.anchorWidget = m_saveButton;
    s5.title = "Commit Tune changes";
    s5.body = "Edits live in memory until you Save/Burn. Save writes them to the ECU permanently. Always Burn before unplugging.";
    s5.padding = 8;
    s5.onShow = [this]() {
        // Nothing special to switch
    };
    m_tourOverlay->addStep(s5);

    // Step 6: Help Menu
    TourStep s6;
    s6.anchorWidget = m_helpButton;
    s6.title = "Demo & Exploration Mode";
    s6.body = "No ECU yet? Click '?' or Help -> Demo Mode to explore with simulated data — nothing you do here can hurt anything.";
    s6.padding = 8;
    s6.onShow = [this]() {
        // Highlight Help Button
    };
    m_tourOverlay->addStep(s6);

    // Start the tour!
    m_tourOverlay->startTour();
}
