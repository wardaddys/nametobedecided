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
#include <QScreen>
#include <QGuiApplication>
#include <QWindow>
#ifdef Q_OS_WIN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

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
#include "core/LogPlayer.h"
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



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_serialManager(new SerialManager(this)),
      m_loggingManager(new LoggingManager(this)),

      m_liveTuningEnabled(false) {
  // === Section 7.1: Fix window title ===
  setWindowTitle("OS Tuner — Open-Source ECU Tuning");
  resize(1280, 800);
  
  // Set frameless window hint for custom titlebar
  setWindowFlags(Qt::FramelessWindowHint | Qt::Window);

  // Create centralized ECU Settings Manager before UI setup
  m_logPlayer = new LogPlayer(this);
  m_ecuSettingsManager = new ECUSettingsManager(this);
  m_ecuSettingsManager->setSerialManager(m_serialManager);
  
  // Create Project Manager
  m_projectManager = new ProjectManager(m_ecuSettingsManager, this);
  connect(m_projectManager, &ProjectManager::projectLoaded, [this](const QString &name) {
      m_projectButton->setText("Project: " + name);
      // Remove the annoying and potentially crash-causing synchronous QMessageBox
      
      if (!Settings::getFirstRunCompleted()) {
          // Defer the tour startup to avoid event loop conflicts immediately after load
          QTimer::singleShot(200, this, [this]() {
              startProductTour();
          });
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
  // Defer initialization to the next event loop cycle so the UI is fully constructed
  // and attached to the screen before MSQ signals hit the widgets.
  QTimer::singleShot(0, this, [this]() {
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
  });
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

  if (dialog.exec() == QDialog::Accepted) {
    QString path = dialog.selectedFiles().first();
    if (m_projectManager->loadProject(path)) {
      Settings::setLastEcuDefPath(path);
    } else {
      showEcuDefError(); // Retry
    }
  }
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
  appNameLabel->setStyleSheet(
      "QLabel {"
      "  font-family: 'Inter', sans-serif;"
      "  font-size: 22px;"
      "  font-weight: 900;"
      "  letter-spacing: 2px;"
      "  color: #00BCD4;"
      "  background: transparent;"
      "  border-bottom: 2px solid rgba(0, 188, 212, 0.3);"
      "}");

  headerLayout->addWidget(appNameLabel);
  headerLayout->addSpacing(20);

  // -- Center Section: Primary Actions --
  QString skeuoHeaderBtn = 
      "QPushButton { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4A4A4A, stop:0.4 #333333, stop:1 #1A1A1A); "
      "  color: #E0E0E0; "
      "  border: 1px solid #000000; "
      "  border-top: 1px solid #666666; "
      "  border-left: 1px solid #555555; "
      "  border-radius: 6px; "
      "  padding: 6px 16px; "
      "  font-family: 'Inter', sans-serif; "
      "  font-size: 13px; "
      "  font-weight: bold; "
      "  letter-spacing: 0.5px; "
      "} "
      "QPushButton:hover { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5A5A5A, stop:0.4 #404040, stop:1 #252525); "
      "  color: #FFFFFF; "
      "} "
      "QPushButton:pressed { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #111111, stop:1 #2B2B2B); "
      "  border-top: 1px solid #000000; "
      "  border-left: 1px solid #000000; "
      "  border-bottom: 1px solid #444444; "
      "  border-right: 1px solid #444444; "
      "}";

  m_newProjectButton = new QPushButton("New Project", this);
  m_newProjectButton->setObjectName("SaveButton");
  m_newProjectButton->setStyleSheet(skeuoHeaderBtn);
  connect(m_newProjectButton, &QPushButton::clicked, this, &MainWindow::onNewProjectClicked);

  m_projectButton = new QPushButton("Open Project", this);
  m_projectButton->setObjectName("SaveButton");
  m_projectButton->setStyleSheet(skeuoHeaderBtn);
  connect(m_projectButton, &QPushButton::clicked, this, &MainWindow::onOpenProject);

  m_saveButton = new QPushButton("Save", this);
  m_saveButton->setObjectName("SaveButton");
  m_saveButton->setProperty("hasChanges", false);
  m_saveButton->setStyleSheet(skeuoHeaderBtn);
  connect(m_saveButton, &QPushButton::clicked, this, &MainWindow::onSaveClicked);

  m_readEcuCombo = new QComboBox(this);
  m_readEcuCombo->setObjectName("ComPortSelector");
  m_readEcuCombo->setStyleSheet(
      "QComboBox { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #141414, stop:1 #2A2A2A); "
      "  border: 1px solid #111; "
      "  border-bottom: 1px solid #4A4A4A; "
      "  border-right: 1px solid #3A3A3A; "
      "  border-radius: 6px; "
      "  padding: 6px 12px; "
      "  color: #00BCD4; "
      "  font-size: 13px; "
      "  font-weight: bold; "
      "} "
      "QComboBox::drop-down { border: none; }");
  m_readEcuCombo->addItems({"Sync ECU", "Read All", "Write All"});
  connect(m_readEcuCombo, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::onReadECUChanged);

  headerLayout->addWidget(m_newProjectButton);
  headerLayout->addWidget(m_projectButton);
  headerLayout->addWidget(m_saveButton);
  headerLayout->addWidget(m_readEcuCombo);
  headerLayout->addStretch();

  // -- Right Section: Status, Help, Settings & Controls --
  m_ecuStatusLabel = new QLabel("🔴 OFFLINE", this);
  m_ecuStatusLabel->setObjectName("EcuStatusPill");
  m_ecuStatusLabel->setProperty("connected", false);
  m_ecuStatusLabel->setStyleSheet(
      "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2A0000, stop:1 #110000); "
      "border: 1px solid #000; "
      "border-bottom: 1px solid #5A0000; "
      "border-right: 1px solid #3A0000; "
      "border-radius: 12px; "
      "padding: 4px 12px; "
      "color: #FF5252; "
      "font-family: 'Inter', sans-serif; "
      "font-weight: 900; "
      "font-size: 12px;"
  );

  m_liveTuningButton = new QPushButton("LIVE", this);
  m_liveTuningButton->setObjectName("LiveTuningToggle");
  m_liveTuningButton->setProperty("active", false);
  m_liveTuningButton->setCursor(Qt::PointingHandCursor);
  m_liveTuningButton->setStyleSheet(
      "QPushButton { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1E1E1E, stop:1 #111111); "
      "  color: #666; "
      "  border: 1px solid #000; "
      "  border-bottom: 1px solid #333; "
      "  border-right: 1px solid #222; "
      "  border-radius: 12px; "
      "  padding: 4px 14px; "
      "  font-family: 'Inter', sans-serif; "
      "  font-weight: 900; "
      "  font-size: 12px; "
      "} "
      "QPushButton:checked { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFB300, stop:1 #F57C00); "
      "  color: #FFF; "
      "  border: 1px solid #BF360C; "
      "  border-top: 1px solid #FFE082; "
      "  border-left: 1px solid #FFCA28; "
      "}"
  );
  connect(m_liveTuningButton, &QPushButton::clicked, this, &MainWindow::onLiveTuningToggled);

  m_connectButton = new QPushButton("Connect", this);
  m_connectButton->setObjectName("ConnectButton");
  m_connectButton->setProperty("connected", false);
  m_connectButton->setStyleSheet(
      "QPushButton { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #00E676, stop:0.4 #00C853, stop:1 #00A300); "
      "  color: #FFFFFF; "
      "  border: 1px solid #003300; "
      "  border-top: 1px solid #69F0AE; "
      "  border-left: 1px solid #00E676; "
      "  border-radius: 6px; "
      "  padding: 6px 16px; "
      "  font-family: 'Inter', sans-serif; "
      "  font-weight: 900; "
      "  font-size: 13px; "
      "  letter-spacing: 1px; "
      "} "
      "QPushButton:hover { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #69F0AE, stop:0.4 #00E676, stop:1 #00C853); "
      "} "
      "QPushButton:pressed { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #004D00, stop:1 #00C853); "
      "  border-top: 1px solid #003300; "
      "  border-left: 1px solid #003300; "
      "  border-bottom: 1px solid #00E676; "
      "  border-right: 1px solid #00E676; "
      "}"
  );
  connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);

  m_helpButton = new QPushButton(this);
  m_helpButton->setObjectName("HelpIconButton");
  m_helpButton->setText("?");
  m_helpButton->setToolTip("Help & Updates");
  m_helpButton->setFixedSize(30, 30);
  m_helpButton->setStyleSheet(
      "QPushButton { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3A3A3A, stop:1 #1A1A1A); "
      "  color: #FFF; "
      "  border: 1px solid #000; "
      "  border-top: 1px solid #666; "
      "  border-radius: 15px; "
      "  font-family: 'Inter', sans-serif; "
      "  font-weight: 900; "
      "  font-size: 14px; "
      "} "
      "QPushButton:hover { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4A4A4A, stop:1 #2A2A2A); "
      "} "
      "QPushButton:pressed { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0A0A0A, stop:1 #1A1A1A); "
      "  border-top: 1px solid #000; "
      "  border-bottom: 1px solid #444; "
      "}"
  );
  connect(m_helpButton, &QPushButton::clicked, this, &MainWindow::onAboutClicked);



  // Window Controls
  QPushButton *minBtn = new QPushButton(this);
  minBtn->setObjectName("TitleBarMinimize");
  connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
  QPushButton *maxBtn = new QPushButton(this);
  maxBtn->setObjectName("TitleBarMaximize");
  connect(maxBtn, &QPushButton::clicked, [this]() {
#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(winId());

    if (property("osCustomMaximized").toBool()) {
      // --- RESTORE ---
      setProperty("osCustomMaximized", false);
      int l = property("osPhysL").toInt();
      int t = property("osPhysT").toInt();
      int w = property("osPhysW").toInt();
      int h = property("osPhysH").toInt();

      // Clamp the saved rect to the work area.
      // This handles the case where the app's initial window size (1400x900) is
      // larger than the screen (e.g. 1366x768 laptop), which would hide the status bar.
      HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
      MONITORINFO mi2; mi2.cbSize = sizeof(mi2);
      if (GetMonitorInfo(hMon, &mi2)) {
        const RECT &wa = mi2.rcWork;
        int waW = wa.right  - wa.left;
        int waH = wa.bottom - wa.top;
        if (w > waW) w = waW;          // cap width to available width
        if (h > waH) h = waH;          // cap height to available height
        if (l < wa.left) l = wa.left;  // don't go left of work area
        if (t < wa.top)  t = wa.top;   // don't go above work area
        if (l + w > wa.right)  l = wa.right  - w; // don't overflow right
        if (t + h > wa.bottom) t = wa.bottom - h; // don't overflow below taskbar
      }
      SetWindowPos(hwnd, nullptr, l, t, w, h, SWP_NOZORDER | SWP_NOACTIVATE);

    } else {
      // --- MAXIMIZE ---
      // Save the current physical window rect before touching anything.
      RECT wr;
      GetWindowRect(hwnd, &wr);
      setProperty("osPhysL", (int)wr.left);
      setProperty("osPhysT", (int)wr.top);
      setProperty("osPhysW", (int)(wr.right  - wr.left));
      setProperty("osPhysH", (int)(wr.bottom - wr.top));
      setProperty("osCustomMaximized", true);

      // Fill the monitor's work area (excludes taskbar, multi-monitor-aware).
      HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
      MONITORINFO mi; mi.cbSize = sizeof(mi);
      if (GetMonitorInfo(hMon, &mi)) {
        const RECT &wa = mi.rcWork;
        SetWindowPos(hwnd, nullptr,
                     wa.left, wa.top,
                     wa.right  - wa.left,
                     wa.bottom - wa.top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
      }
    }
#else
    // Non-Windows fallback
    if (property("osCustomMaximized").toBool()) {
      setProperty("osCustomMaximized", false);
      QRect saved = property("osSavedGeometry").value<QRect>();
      if (saved.isValid()) setGeometry(saved);
    } else {
      setProperty("osSavedGeometry", QVariant::fromValue(geometry()));
      setProperty("osCustomMaximized", true);
      QScreen *s = QGuiApplication::screenAt(geometry().center());
      if (!s) s = QGuiApplication::primaryScreen();
      setGeometry(s->availableGeometry());
    }
#endif
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
    m_loggingManager->startLogging();
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
          [this]() { 
              m_loggingWidget->setStatus(false); 
              m_loggingWidget->refreshLogList();
          });
  connect(m_loggingManager, &LoggingManager::bufferUsageUpdated,
          m_loggingWidget, &LoggingWidget::setBufferUsage);
  connect(m_loggingManager, &LoggingManager::errorOccurred,
          [this](const QString &err) {
            QMessageBox::warning(this, "Logging Error", err);
            m_loggingWidget->setStatus(false);
          });

  // Connect Log Player
  connect(m_loggingWidget, &LoggingWidget::playbackPlayRequested, [this](const QString &path) {
      if (!m_logPlayer->hasLogLoaded() || m_logPlayer->getLogName() != QFileInfo(path).fileName()) {
          m_logPlayer->loadLog(path);
      }
      m_logPlayer->play();
  });
  connect(m_loggingWidget, &LoggingWidget::playbackPauseRequested, m_logPlayer, &LogPlayer::pause);
  connect(m_loggingWidget, &LoggingWidget::playbackStopRequested, m_logPlayer, &LogPlayer::stop);
  connect(m_loggingWidget, &LoggingWidget::playbackSeekRequested, m_logPlayer, &LogPlayer::seek);
  
  connect(m_logPlayer, &LogPlayer::stateChanged, m_loggingWidget, &LoggingWidget::setPlaybackState);
  connect(m_logPlayer, &LogPlayer::playbackProgress, m_loggingWidget, &LoggingWidget::setPlaybackProgress);
  connect(m_logPlayer, &LogPlayer::dataFrameReady, this, &MainWindow::updateRealtimeData);

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
  bar->setStyleSheet(
        "QStatusBar { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #22262d, stop:1 #121519); "
        "  border-top: 2px solid #353b45; "
        "}"
  );

  QWidget* fieldsContainer = new QWidget(this);
  fieldsContainer->setStyleSheet("background: transparent; border: none;");
  QHBoxLayout* hLay = new QHBoxLayout(fieldsContainer);
  hLay->setContentsMargins(12, 4, 12, 4); 
  hLay->setSpacing(8);

  auto addStatusField = [&](QLabel** labelPtr, const QString& prefixName, int width) {
    QFrame* screen = new QFrame(this);
    screen->setFixedSize(width, 24);
    screen->setStyleSheet(
        "QFrame { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #080a0d, stop:1 #12161c); "
        "  border: 1px solid; "
        "  border-top-color: #000000; "
        "  border-left-color: #030405; "
        "  border-bottom-color: #3b424d; "
        "  border-right-color: #2b323c; "
        "  border-radius: 3px; "
        "}"
    );

    QHBoxLayout* sLay = new QHBoxLayout(screen);
    sLay->setContentsMargins(6, 0, 6, 0);
    sLay->setSpacing(4);

    QLabel* prefixLabel = new QLabel(prefixName, screen);
    prefixLabel->setStyleSheet("color: #5a6678; font-family: 'Inter', sans-serif; font-size: 10px; font-weight: 800; background: transparent; border: none;");
    
    *labelPtr = new QLabel("0", screen);
    (*labelPtr)->setStyleSheet("color: #00e5c8; font-family: 'JetBrains Mono', 'Consolas', monospace; font-size: 12px; font-weight: bold; background: transparent; border: none;");
    (*labelPtr)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    sLay->addWidget(prefixLabel);
    sLay->addWidget(*labelPtr, 1);
    
    hLay->addWidget(screen);
  };

  addStatusField(&m_rpmLabel, "RPM", 90);
  addStatusField(&m_mapLabel, "MAP", 105);
  addStatusField(&m_afrLabel, "AFR", 85);
  addStatusField(&m_ectLabel, "CLT", 90);
  addStatusField(&m_boostLabel, "BOOST", 115);
  addStatusField(&m_speedLabel, "SPEED", 120);
  addStatusField(&m_gearLabel, "GEAR", 80);
  addStatusField(&m_oilTLabel, "OIL T", 85);
  addStatusField(&m_oilPLabel, "OIL P", 85);
  addStatusField(&m_fuelPLabel, "FUEL P", 90);

  QFrame* ecuFrame = new QFrame(this);
  ecuFrame->setFixedSize(220, 24);
  ecuFrame->setStyleSheet(
      "QFrame { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1a1610, stop:1 #241a0b); "
      "  border: 1px solid; "
      "  border-top-color: #000000; "
      "  border-left-color: #050505; "
      "  border-bottom-color: #4a3a20; "
      "  border-right-color: #3b2e1a; "
      "  border-radius: 3px; "
      "}"
  );
  QHBoxLayout* ecuLay = new QHBoxLayout(ecuFrame);
  ecuLay->setContentsMargins(10, 0, 10, 0);
  m_ecuSavedLabel = new QLabel("ECU Saved: ---", ecuFrame);
  m_ecuSavedLabel->setStyleSheet("color: #ffb800; font-family: 'JetBrains Mono', 'Consolas', monospace; font-size: 11px; font-weight: bold; background: transparent; border: none;");
  ecuLay->addWidget(m_ecuSavedLabel, 0, Qt::AlignCenter);
  
  hLay->addWidget(ecuFrame);
  hLay->addStretch(); 

  bar->addWidget(fieldsContainer);
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
    m_ecuStatusLabel->setText("🟢 CONNECTED");
    m_ecuStatusLabel->setProperty("connected", true);
    m_ecuStatusLabel->style()->unpolish(m_ecuStatusLabel);
    m_ecuStatusLabel->style()->polish(m_ecuStatusLabel);

    m_connectButton->setText("Disconnect");
    m_connectButton->setProperty("connected", true);
    m_connectButton->style()->unpolish(m_connectButton);
    m_connectButton->style()->polish(m_connectButton);
  } else {
    m_ecuStatusLabel->setText("🔴 OFFLINE");
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

// === Native resize-from-edges/corners for frameless window ===
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result) {
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG") {
        MSG *msg = static_cast<MSG*>(message);
        if (msg->message == WM_NCHITTEST) {
            // Get the window's outer rect and cursor position, both in screen pixels.
            RECT wr;
            GetWindowRect(reinterpret_cast<HWND>(winId()), &wr);
            const int x = static_cast<int>(static_cast<short>(LOWORD(msg->lParam)));
            const int y = static_cast<int>(static_cast<short>(HIWORD(msg->lParam)));
            const int m = 8; // hit-test margin (px) — feel of the resize grab zone

            const bool onLeft   = (x < wr.left   + m);
            const bool onRight  = (x > wr.right  - m);
            const bool onTop    = (y < wr.top    + m);
            const bool onBottom = (y > wr.bottom - m);

            // Corners first (order matters — they take priority over edges)
            if (onTop    && onLeft)  { *result = HTTOPLEFT;     return true; }
            if (onTop    && onRight) { *result = HTTOPRIGHT;    return true; }
            if (onBottom && onLeft)  { *result = HTBOTTOMLEFT;  return true; }
            if (onBottom && onRight) { *result = HTBOTTOMRIGHT; return true; }
            // Edges
            if (onLeft)              { *result = HTLEFT;        return true; }
            if (onRight)             { *result = HTRIGHT;       return true; }
            if (onTop)               { *result = HTTOP;         return true; }
            if (onBottom)            { *result = HTBOTTOM;      return true; }
            // Everything else: let Qt handle it normally (title-bar drag, buttons, content)
        }
    }
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
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
