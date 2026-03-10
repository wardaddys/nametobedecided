#include "ECUSettingsWidget.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTableWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include "../utils/Logger.h"

// Include embedded widgets
#include "IdleControlWidget.h"
#include "ColdStartWidget.h"
#include "AccelEnrichmentWidget.h"
#include "TrimsWidget.h"
#include "VVTVTECWidget.h"
#include "BoostWidget.h"
#include "KnockWidget.h"
#include "LimitersWidget.h"
#include "IOConfigWidget.h"
#include "../core/ECUSettingsManager.h"
#include "../core/ReferenceDatabase.h"
#include "../core/TunerProColors.h"

// We use global QSS for combos and spin boxes as per Nuclear Specs, 
// so we don't need explicitly heavy styles here, but we will define a clean fallback/base just in case.
static const char* COMBO_STYLE = "";
static const char* SPIN_STYLE = "";

static const char* LABEL_STYLE = "font-family: 'Inter'; font-size: 13px; font-weight: 500;";

ECUSettingsWidget::ECUSettingsWidget(ECUSettingsManager *settingsManager, QWidget *parent) : QWidget(parent),
    m_idleWidget(nullptr), m_coldStartWidget(nullptr), m_accelWidget(nullptr),
    m_trimsWidget(nullptr), m_vvtVtecWidget(nullptr), m_boostWidget(nullptr),
    m_knockWidget(nullptr), m_limitersWidget(nullptr), m_ioConfigWidget(nullptr),
    m_serialManager(nullptr), m_settingsManager(settingsManager) {
  loadSensorCalibrationOptions();
  createEmbeddedWidgets();
  initializeSettingsManager();
  setupUi();
}

void ECUSettingsWidget::createEmbeddedWidgets() {
  // Create instances of the tuning control widgets
  m_idleWidget = new IdleControlWidget(this);
  m_coldStartWidget = new ColdStartWidget(this);
  m_accelWidget = new AccelEnrichmentWidget(this);
  m_trimsWidget = new TrimsWidget(this);
  m_vvtVtecWidget = new VVTVTECWidget(this);
  m_boostWidget = new BoostWidget(this);
  m_knockWidget = new KnockWidget(this);
  m_limitersWidget = new LimitersWidget(this);
  m_ioConfigWidget = new IOConfigWidget(this);
}

void ECUSettingsWidget::initializeSettingsManager() {
  if (!m_settingsManager) {
      Logger::error("ECUSettingsWidget: No settings manager provided!");
      return; 
  }
  
  // Connect to settings manager signals
  connect(m_settingsManager, &ECUSettingsManager::settingChanged,
          this, &ECUSettingsWidget::onSettingChanged);
  connect(m_settingsManager, &ECUSettingsManager::readComplete,
          this, &ECUSettingsWidget::onSettingsLoaded);
  
  // Pass settings manager to child tuning widgets
  if (m_idleWidget) m_idleWidget->setSettingsManager(m_settingsManager);
  if (m_coldStartWidget) m_coldStartWidget->setSettingsManager(m_settingsManager);
  if (m_accelWidget) m_accelWidget->setSettingsManager(m_settingsManager);
  if (m_trimsWidget) m_trimsWidget->setSettingsManager(m_settingsManager);
  if (m_vvtVtecWidget) m_vvtVtecWidget->setSettingsManager(m_settingsManager);
  if (m_boostWidget) m_boostWidget->setSettingsManager(m_settingsManager);
  if (m_knockWidget) m_knockWidget->setSettingsManager(m_settingsManager);
  if (m_limitersWidget) m_limitersWidget->setSettingsManager(m_settingsManager);
  if (m_ioConfigWidget) m_ioConfigWidget->setSettingsManager(m_settingsManager);
}

void ECUSettingsWidget::connectSettingsToECU() {
  if (m_serialManager && m_settingsManager) {
    m_settingsManager->setSerialManager(m_serialManager);
    Logger::info("ECU Settings connected to serial manager");
  }
}

void ECUSettingsWidget::setSerialManager(SerialManager *serialManager) {
  m_serialManager = serialManager;
  connectSettingsToECU();
}

void ECUSettingsWidget::readAllFromECU() {
  if (!m_settingsManager) return;

  // Read all configuration pages from the ECU
  m_settingsManager->readAllFromECU();

  // Trigger loadFromECU() on each embedded tuning widget
  if (m_idleWidget) m_idleWidget->loadFromECU();
  if (m_coldStartWidget) m_coldStartWidget->loadFromECU();
  if (m_accelWidget) m_accelWidget->loadFromECU();
  if (m_trimsWidget) m_trimsWidget->loadFromECU();
  if (m_vvtVtecWidget) m_vvtVtecWidget->loadFromECU();
  if (m_boostWidget) m_boostWidget->loadFromECU();
  if (m_knockWidget) m_knockWidget->loadFromECU();
  if (m_limitersWidget) m_limitersWidget->loadFromECU();
  if (m_ioConfigWidget) m_ioConfigWidget->loadFromECU();

  Logger::info("All tuning widgets loaded from ECU");
}

void ECUSettingsWidget::burnAllDirty() {
  if (m_settingsManager) {
    m_settingsManager->burnAllDirty();
  }
}

void ECUSettingsWidget::updateRealtimeData(const RealTimeData &data) {
  // Update embedded widgets
  if (m_idleWidget) m_idleWidget->updateRealtimeData(data);
  if (m_coldStartWidget) m_coldStartWidget->updateRealtimeData(data);
  if (m_accelWidget) m_accelWidget->updateRealtimeData(data);
  if (m_trimsWidget) m_trimsWidget->updateRealtimeData(data);
  if (m_vvtVtecWidget) m_vvtVtecWidget->updateRealtimeData(data);
  if (m_boostWidget) m_boostWidget->updateRealtimeData(data);
  if (m_knockWidget) m_knockWidget->updateRealtimeData(data);
  if (m_limitersWidget) m_limitersWidget->updateRealtimeData(data);
  if (m_ioConfigWidget) m_ioConfigWidget->updateRealtimeData(data);

  // BUG-010 FIX: Update IO status indicators from RealTimeData
  // Mapping defined in Speeduino protocol for engine bits
  auto setIndicator = [&](const QString &name, bool active) {
    if (m_ioStatusIndicators.contains(name)) {
      m_ioStatusIndicators[name]->setStyleSheet(
        QString("background: %1; border-radius: 10px;").arg(active ? "#00FF00" : "#333"));
    }
  };

  setIndicator("Fuel Pump", (data.engine & 0x01));
  setIndicator("Fan", (data.engine & 0x40));
  
  // NEW-001: Update EGO live readout
  if (m_egoLiveValue) {
    m_egoLiveValue->setText(QString("EGO Correction: %1 %").arg(data.egoCorrection));
  }
  setIndicator("Boost", (data.engine & 0x10)); // boostCut
  setIndicator("VTEC", (data.status1 & 0x01));
  setIndicator("Idle Valve", (data.engine & 0x04)); // idleStatus
}

void ECUSettingsWidget::onSettingChanged(const QString &name, const QVariant &value) {
  updateControlFromSetting(name);
}

void ECUSettingsWidget::onSettingsLoaded() {
  // Update all bound controls with loaded values
  for (const QString &name : m_settingCombos.keys()) {
    updateControlFromSetting(name);
  }
  for (const QString &name : m_settingSpins.keys()) {
    updateControlFromSetting(name);
  }
  for (const QString &name : m_settingDoubleSpins.keys()) {
    updateControlFromSetting(name);
  }
  for (const QString &name : m_settingChecks.keys()) {
    updateControlFromSetting(name);
  }
}

void ECUSettingsWidget::updateControlFromSetting(const QString &settingName) {
  if (!m_settingsManager) return;
  
  QVariant value = m_settingsManager->getValue(settingName);
  if (!value.isValid()) return;
  
  // Block signals while updating to prevent feedback loop
  if (m_settingCombos.contains(settingName)) {
    QComboBox *combo = m_settingCombos[settingName];
    combo->blockSignals(true);
    combo->setCurrentIndex(value.toInt());
    combo->blockSignals(false);
  }
  else if (m_settingSpins.contains(settingName)) {
    QSpinBox *spin = m_settingSpins[settingName];
    spin->blockSignals(true);
    spin->setValue(value.toInt());
    spin->blockSignals(false);
  }
  else if (m_settingDoubleSpins.contains(settingName)) {
    QDoubleSpinBox *spin = m_settingDoubleSpins[settingName];
    spin->blockSignals(true);
    spin->setValue(value.toDouble());
    spin->blockSignals(false);
  }
  else if (m_settingChecks.contains(settingName)) {
    QCheckBox *check = m_settingChecks[settingName];
    check->blockSignals(true);
    check->setChecked(value.toInt() != 0);
    check->blockSignals(false);
  }
}

// ===== Bound Control Helper Methods =====

QComboBox* ECUSettingsWidget::addBoundCombo(QGridLayout *grid, int row, int col,
                                             const QString &label, const QStringList &options,
                                             const QString &settingName) {
  QLabel *lbl = new QLabel(label);
  lbl->setStyleSheet(LABEL_STYLE);
  
  QComboBox *combo = new QComboBox();
  combo->addItems(options);
  combo->setStyleSheet(COMBO_STYLE);
  
  // Store mapping
  m_settingCombos[settingName] = combo;
  
  // Connect to settings manager
  connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          [this, settingName](int index) {
    if (m_settingsManager) {
      m_settingsManager->setValue(settingName, index);
    }
  });
  
  QVBoxLayout *v = new QVBoxLayout();
  v->setSpacing(6);
  v->addWidget(lbl);
  v->addWidget(combo);
  grid->addLayout(v, row, col);
  
  return combo;
}

QSpinBox* ECUSettingsWidget::addBoundSpin(QGridLayout *grid, int row, int col,
                                           const QString &label, const QString &settingName,
                                           const QString &suffix) {
  QLabel *lbl = new QLabel(label);
  lbl->setStyleSheet(LABEL_STYLE);
  
  QSpinBox *spin = new QSpinBox();
  
  // Get min/max from setting definition
  if (m_settingsManager && m_settingsManager->hasSetting(settingName)) {
    auto def = m_settingsManager->getDefinition(settingName);
    spin->setRange(static_cast<int>(def.min), static_cast<int>(def.max));
    if (!def.units.isEmpty() && suffix.isEmpty()) {
      spin->setSuffix(" " + def.units);
    }
  } else {
    spin->setRange(0, 65535);
  }
  
  if (!suffix.isEmpty()) {
    spin->setSuffix(suffix);
  }
  spin->setStyleSheet(SPIN_STYLE);
  
  // Store mapping
  m_settingSpins[settingName] = spin;
  
  // Connect to settings manager
  connect(spin, QOverload<int>::of(&QSpinBox::valueChanged),
          [this, settingName](int value) {
    if (m_settingsManager) {
      m_settingsManager->setValue(settingName, value);
    }
  });
  
  QVBoxLayout *v = new QVBoxLayout();
  v->setSpacing(6);
  v->addWidget(lbl);
  v->addWidget(spin);
  grid->addLayout(v, row, col);
  
  return spin;
}

QDoubleSpinBox* ECUSettingsWidget::addBoundDoubleSpin(QGridLayout *grid, int row, int col,
                                                       const QString &label, const QString &settingName,
                                                       const QString &suffix) {
  QLabel *lbl = new QLabel(label);
  lbl->setStyleSheet(LABEL_STYLE);
  
  QDoubleSpinBox *spin = new QDoubleSpinBox();
  
  // Get min/max from setting definition
  if (m_settingsManager && m_settingsManager->hasSetting(settingName)) {
    auto def = m_settingsManager->getDefinition(settingName);
    spin->setRange(def.min, def.max);
    spin->setDecimals(2);
    if (!def.units.isEmpty() && suffix.isEmpty()) {
      spin->setSuffix(" " + def.units);
    }
  } else {
    spin->setRange(0, 65535);
    spin->setDecimals(2);
  }
  
  if (!suffix.isEmpty()) {
    spin->setSuffix(suffix);
  }
  spin->setStyleSheet(SPIN_STYLE);
  
  // Store mapping
  m_settingDoubleSpins[settingName] = spin;
  
  // Connect to settings manager
  connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          [this, settingName](double value) {
    if (m_settingsManager) {
      m_settingsManager->setValue(settingName, value);
    }
  });
  
  QVBoxLayout *v = new QVBoxLayout();
  v->setSpacing(6);
  v->addWidget(lbl);
  v->addWidget(spin);
  grid->addLayout(v, row, col);
  
  return spin;
}

QCheckBox* ECUSettingsWidget::addBoundCheck(QGridLayout *grid, int row, int col,
                                             const QString &label, const QString &settingName) {
  QCheckBox *cb = new QCheckBox(label);
  cb->setStyleSheet(""); // Rely on global QSS
  
  // Store mapping
  m_settingChecks[settingName] = cb;
  
  // Connect to settings manager
  connect(cb, &QCheckBox::toggled, [this, settingName](bool checked) {
    if (m_settingsManager) {
      m_settingsManager->setValue(settingName, checked ? 1 : 0);
    }
  });
  
  grid->addWidget(cb, row, col);
  return cb;
}

void ECUSettingsWidget::loadSensorCalibrationOptions() {
  // CLT/IAT Sensors (Temperature)
  m_cltSensors << "GM" << "Ford" << "Toyota" << "Mazda" << "Mitsubishi" 
               << "Chrysler" << "Saab" << "BMW E30" << "Bosch FST7576" << "Custom";
  m_iatSensors = m_cltSensors;
  
  // MAP Sensors
  m_mapSensors << "MPX4115 (115 kPa)" << "MPX4250 (250 kPa)" 
               << "MPXH6300 (300 kPa)" << "MPXH6400 (400 kPa)" << "Custom";
  
  // O2 Sensors
  m_o2Sensors << "Narrowband" << "Wideband 0-5V Linear" << "AEM UEGO" 
              << "Innovate LC-1" << "Innovate 0-5V" << "Tech Edge" 
              << "DIY-WB" << "Zeitronix" << "Custom";
}

void ECUSettingsWidget::setupUi() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // --- Top Bar (Search) ---
  QWidget *topBar = new QWidget(this);
  topBar->setStyleSheet(QString("background: %1; border-bottom: 1px solid %2;")
                        .arg(TunerProColors::BG_BASE).arg(TunerProColors::BORDER_DEFAULT));
  QHBoxLayout *topLayout = new QHBoxLayout(topBar);
  m_searchBar = new QLineEdit(this);
  m_searchBar->setPlaceholderText("Search settings...");
  m_searchBar->setStyleSheet(QString("background: %1; border: 1px solid %2; "
                             "padding: 8px; border-radius: 4px; color: %3;")
                             .arg(TunerProColors::BG_RAISED)
                             .arg(TunerProColors::BORDER_SUBTLE)
                             .arg(TunerProColors::TEXT_PRIMARY));
  connect(m_searchBar, &QLineEdit::textChanged, this, &ECUSettingsWidget::onSearchTextChanged);
  topLayout->addWidget(m_searchBar);
  mainLayout->addWidget(topBar);

  // --- Main Content Area (Sidebar + Stack) ---
  QWidget *contentArea = new QWidget(this);
  QHBoxLayout *contentLayout = new QHBoxLayout(contentArea);
  contentLayout->setContentsMargins(0, 0, 0, 0);
  contentLayout->setSpacing(0);

  // Tree Sidebar
  m_sidebar = new QTreeWidget(this);
  m_sidebar->setHeaderHidden(true);
  m_sidebar->setFixedWidth(280);
  m_sidebar->setStyleSheet(QString(
      "QTreeWidget { background: %1; border-right: 1px solid %2; "
      "color: %3; outline: none; font-family: 'Inter'; font-size: 13px; }"
      "QTreeWidget::item { padding: 6px 4px; }"
      "QTreeWidget::item:selected { background: %4; border-left: 2px solid %6; color: %3; }"
      "QTreeWidget::item:hover { background: %5; }"
      "QTreeWidget::branch { background: %1; }")
      .arg(TunerProColors::BG_ELEVATED)
      .arg(TunerProColors::BORDER_DEFAULT)
      .arg(TunerProColors::TEXT_PRIMARY)
      .arg(TunerProColors::BG_INTERACTIVE)
      .arg(TunerProColors::BG_RAISED)
      .arg(TunerProColors::ACCENT));
  
  populateSidebar();
  connect(m_sidebar, &QTreeWidget::currentItemChanged, this, &ECUSettingsWidget::onSectionChanged);
  contentLayout->addWidget(m_sidebar);

  // Content Stack with Scroll Area
  m_contentStack = new QStackedWidget(this);
  m_contentStack->setStyleSheet(QString("background: %1;").arg(TunerProColors::BG_BASE));
  
  // Create all pages and add to stack
  int idx = 0;
  m_pageIndexMap["Engine and Sequential Settings"] = idx++;
  m_contentStack->addWidget(createEnginePage());
  
  m_pageIndexMap["Sequential Fuel Injection"] = idx++;
  m_contentStack->addWidget(createSequentialFuelPage());
  
  m_pageIndexMap["General Settings"] = idx++;
  m_contentStack->addWidget(createGeneralSettingsPage());
  
  m_pageIndexMap["Rev Limiter"] = idx++;
  m_contentStack->addWidget(createRevLimiterPage());
  
  m_pageIndexMap["Shift Light"] = idx++;
  m_contentStack->addWidget(createShiftLightPage());
  
  m_pageIndexMap["Engine State Settings"] = idx++;
  m_contentStack->addWidget(createEngineStatePage());
  
  m_pageIndexMap["Tacho Output"] = idx++;
  m_contentStack->addWidget(createTachoOutputPage());
  
  m_pageIndexMap["Fan Control"] = idx++;
  m_contentStack->addWidget(createFanControlPage());
  
  m_pageIndexMap["Torque Converter Lockup"] = idx++;
  m_contentStack->addWidget(createTorqueConverterPage());
  
  m_pageIndexMap["Alternator Control"] = idx++;
  m_contentStack->addWidget(createAlternatorControlPage());
  
  m_pageIndexMap["Barometric Correction"] = idx++;
  m_contentStack->addWidget(createBarometricPage());
  
  m_pageIndexMap["IAT/MAP Sample Settings"] = idx++;
  m_contentStack->addWidget(createIATMAPPage());
  
  m_pageIndexMap["ITB Load Settings"] = idx++;
  m_contentStack->addWidget(createTPSLoadPage());
  
  m_pageIndexMap["MAP Settings"] = idx++;
  m_contentStack->addWidget(createMAPSettingsPage());
  
  m_pageIndexMap["MAF Settings"] = idx++;
  m_contentStack->addWidget(createMAFSettingsPage());
  
  m_pageIndexMap["MAF Flow Curve"] = idx++;
  m_contentStack->addWidget(createMAFFlowCurvePage());
  
  m_pageIndexMap["MAF/MAT Correction Table"] = idx++;
  m_contentStack->addWidget(createMAFMATCorrectionPage());
  
  m_pageIndexMap["MAT Air Density Table"] = idx++;
  m_contentStack->addWidget(createMATAirDensityPage());
  
  m_pageIndexMap["MAT/CLT Correction"] = idx++;
  m_contentStack->addWidget(createMATCLTCorrectionPage());
  
  m_pageIndexMap["Realtime Display"] = idx++;
  m_contentStack->addWidget(createRealtimeDisplayPage());
  
  m_pageIndexMap["Feature List Showing IO pins"] = idx++;
  m_contentStack->addWidget(createIOPinsListPage());
  
  m_pageIndexMap["IO pins Usage & Status"] = idx++;
  m_contentStack->addWidget(createIOPinsUsagePage());
  
  m_pageIndexMap["CANloop Usage & Status"] = idx++;
  m_contentStack->addWidget(createCANbusPage());
  
  m_pageIndexMap["Expansion IO pins"] = idx++;
  m_contentStack->addWidget(createExpansionIOPage());
  
  m_pageIndexMap["Gauge and Settings Limits"] = idx++;
  m_contentStack->addWidget(createGaugeLimitsPage());
  
  // Tuning Controls - embedded widgets (formerly separate tabs)
  m_pageIndexMap["Idle Control"] = idx++;
  m_contentStack->addWidget(m_idleWidget);
  
  m_pageIndexMap["Cold Start & Warmup"] = idx++;
  m_contentStack->addWidget(m_coldStartWidget);
  
  m_pageIndexMap["Accel Enrichment"] = idx++;
  m_contentStack->addWidget(m_accelWidget);
  
  m_pageIndexMap["Fuel & Ignition Trims"] = idx++;
  m_contentStack->addWidget(m_trimsWidget);
  
  m_pageIndexMap["VVT/VTEC"] = idx++;
  m_contentStack->addWidget(m_vvtVtecWidget);
  
  m_pageIndexMap["Boost Control"] = idx++;
  m_contentStack->addWidget(m_boostWidget);
  
  m_pageIndexMap["Knock Control"] = idx++;
  m_contentStack->addWidget(m_knockWidget);
  
  m_pageIndexMap["Limiters"] = idx++;
  m_contentStack->addWidget(m_limitersWidget);
  
  m_pageIndexMap["I/O Configuration"] = idx++;
  m_contentStack->addWidget(m_ioConfigWidget);

  // NEW PAGES — TunerStudio Parity
  m_pageIndexMap["EGO / Closed-Loop"] = idx++;
  m_contentStack->addWidget(createEGOPage());
  
  m_pageIndexMap["Injector Dead-Time"] = idx++;
  m_contentStack->addWidget(createInjectorDeadTimePage());
  
  m_pageIndexMap["Sensor Calibration"] = idx++;
  m_contentStack->addWidget(createSensorCalibrationPage());
  
  m_pageIndexMap["Cranking Advance"] = idx++;
  m_contentStack->addWidget(createCrankingAdvancePage());

  m_pageIndexMap["Technical Database"] = idx++;
  m_contentStack->addWidget(createTechnicalReferencePage());

  m_pageIndexMap["Injector Catalog"] = idx++;
  m_contentStack->addWidget(createInjectorDatabasePage());

  contentLayout->addWidget(m_contentStack);
  mainLayout->addWidget(contentArea);
  
  // Select first item
  if (m_sidebar->topLevelItemCount() > 0 && m_sidebar->topLevelItem(0)->childCount() > 0) {
    m_sidebar->setCurrentItem(m_sidebar->topLevelItem(0)->child(0));
  }
}

void ECUSettingsWidget::populateSidebar() {
  // Engine & Basic Load Settings
  QTreeWidgetItem *engineCat = new QTreeWidgetItem(m_sidebar, QStringList() << "Engine & Basic Load Settings");
  engineCat->setForeground(0, QColor("#007BFF"));
  engineCat->setFont(0, QFont("Segoe UI", 9, QFont::Bold));
  engineCat->setExpanded(true);
  
  new QTreeWidgetItem(engineCat, QStringList() << "Engine and Sequential Settings");
  new QTreeWidgetItem(engineCat, QStringList() << "Sequential Fuel Injection");
  new QTreeWidgetItem(engineCat, QStringList() << "General Settings");
  new QTreeWidgetItem(engineCat, QStringList() << "Rev Limiter");
  new QTreeWidgetItem(engineCat, QStringList() << "Shift Light");
  new QTreeWidgetItem(engineCat, QStringList() << "Engine State Settings");
  new QTreeWidgetItem(engineCat, QStringList() << "Tacho Output");
  new QTreeWidgetItem(engineCat, QStringList() << "Fan Control");
  new QTreeWidgetItem(engineCat, QStringList() << "Torque Converter Lockup");
  new QTreeWidgetItem(engineCat, QStringList() << "Alternator Control");
  new QTreeWidgetItem(engineCat, QStringList() << "Barometric Correction");
  new QTreeWidgetItem(engineCat, QStringList() << "IAT/MAP Sample Settings");
  new QTreeWidgetItem(engineCat, QStringList() << "ITB Load Settings");
  new QTreeWidgetItem(engineCat, QStringList() << "MAP Settings");
  new QTreeWidgetItem(engineCat, QStringList() << "MAF Settings");
  new QTreeWidgetItem(engineCat, QStringList() << "MAF Flow Curve");
  new QTreeWidgetItem(engineCat, QStringList() << "MAF/MAT Correction Table");
  new QTreeWidgetItem(engineCat, QStringList() << "MAT Air Density Table");
  new QTreeWidgetItem(engineCat, QStringList() << "MAT/CLT Correction");
  new QTreeWidgetItem(engineCat, QStringList() << "Realtime Display");
  
  // IO Pins & Status
  QTreeWidgetItem *ioCat = new QTreeWidgetItem(m_sidebar, QStringList() << "IO Pins & Status");
  ioCat->setForeground(0, QColor("#007BFF"));
  ioCat->setFont(0, QFont("Segoe UI", 9, QFont::Bold));
  ioCat->setExpanded(true);
  
  new QTreeWidgetItem(ioCat, QStringList() << "Feature List Showing IO pins");
  new QTreeWidgetItem(ioCat, QStringList() << "IO pins Usage & Status");
  new QTreeWidgetItem(ioCat, QStringList() << "CANloop Usage & Status");
  new QTreeWidgetItem(ioCat, QStringList() << "Expansion IO pins");
  new QTreeWidgetItem(ioCat, QStringList() << "Gauge and Settings Limits");
  
  // Tuning Controls (formerly separate tabs)
  QTreeWidgetItem *tuningCat = new QTreeWidgetItem(m_sidebar, QStringList() << "Tuning Controls");
  tuningCat->setForeground(0, QColor("#00BCD4"));
  tuningCat->setFont(0, QFont("Segoe UI", 9, QFont::Bold));
  tuningCat->setExpanded(true);
  
  new QTreeWidgetItem(tuningCat, QStringList() << "Idle Control");
  new QTreeWidgetItem(tuningCat, QStringList() << "Cold Start & Warmup");
  new QTreeWidgetItem(tuningCat, QStringList() << "Accel Enrichment");
  new QTreeWidgetItem(tuningCat, QStringList() << "Fuel & Ignition Trims");
  new QTreeWidgetItem(tuningCat, QStringList() << "VVT/VTEC");
  new QTreeWidgetItem(tuningCat, QStringList() << "Boost Control");
  new QTreeWidgetItem(tuningCat, QStringList() << "Knock Control");
  new QTreeWidgetItem(tuningCat, QStringList() << "Limiters");
  new QTreeWidgetItem(tuningCat, QStringList() << "I/O Configuration");
  new QTreeWidgetItem(tuningCat, QStringList() << "EGO / Closed-Loop");
  new QTreeWidgetItem(tuningCat, QStringList() << "Injector Dead-Time");
  new QTreeWidgetItem(tuningCat, QStringList() << "Sensor Calibration");
  new QTreeWidgetItem(tuningCat, QStringList() << "Cranking Advance");

  // REFERENCE: Award-Winning Technical Database
  QTreeWidgetItem *refCat = new QTreeWidgetItem(m_sidebar, QStringList() << "Technical Reference & Manual");
  refCat->setForeground(0, QColor("#FFD700")); // Gold
  refCat->setFont(0, QFont("Segoe UI", 9, QFont::Bold));
  refCat->setExpanded(true);

  new QTreeWidgetItem(refCat, QStringList() << "Technical Database");
  new QTreeWidgetItem(refCat, QStringList() << "Injector Catalog");
}

void ECUSettingsWidget::onSectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
  Q_UNUSED(previous);
  if (!current || current->childCount() > 0) return; // Skip category headers
  
  QString name = current->text(0);
  if (m_pageIndexMap.contains(name)) {
    m_contentStack->setCurrentIndex(m_pageIndexMap[name]);
  }
}

void ECUSettingsWidget::onSearchTextChanged(const QString &text) {
  for (int i = 0; i < m_sidebar->topLevelItemCount(); ++i) {
    QTreeWidgetItem *cat = m_sidebar->topLevelItem(i);
    bool catVisible = false;
    for (int j = 0; j < cat->childCount(); ++j) {
      QTreeWidgetItem *item = cat->child(j);
      bool match = text.isEmpty() || item->text(0).contains(text, Qt::CaseInsensitive);
      item->setHidden(!match);
      if (match) catVisible = true;
    }
    cat->setHidden(!catVisible);
  }
}

// Helper to create a scrollable settings page
QWidget* ECUSettingsWidget::createSettingsPage(const QString &title) {
  QScrollArea *scroll = new QScrollArea();
  scroll->setWidgetResizable(true);
  scroll->setStyleSheet(QString("QScrollArea { border: none; background: %1; }").arg(TunerProColors::BG_BASE));
  
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(20);
  
  QLabel *header = new QLabel(title);
  header->setStyleSheet(QString("font-family: 'Inter'; font-size: 24px; font-weight: bold; color: %1; margin-bottom: 10px;").arg(TunerProColors::TEXT_PRIMARY));
  layout->addWidget(header);
  
  // Section divider 
  QFrame *divider = new QFrame();
  divider->setFrameShape(QFrame::HLine);
  divider->setFrameShadow(QFrame::Plain);
  divider->setStyleSheet(QString("background-color: %1; border: none;").arg(TunerProColors::BORDER_DEFAULT));
  divider->setFixedHeight(1);
  layout->addWidget(divider);
  layout->addSpacing(16);
  
  scroll->setWidget(page);
  return scroll;
}

void ECUSettingsWidget::addComboField(QGridLayout *grid, int row, int col, 
                                       const QString &label, const QStringList &options,
                                       const QString &defaultVal) {
  QLabel *lbl = new QLabel(label);
  lbl->setStyleSheet(LABEL_STYLE);
  
  QComboBox *combo = new QComboBox();
  combo->addItems(options);
  if (!defaultVal.isEmpty()) combo->setCurrentText(defaultVal);
  combo->setStyleSheet(COMBO_STYLE);
  
  QVBoxLayout *v = new QVBoxLayout();
  v->setSpacing(6);
  v->addWidget(lbl);
  v->addWidget(combo);
  grid->addLayout(v, row, col);
}

void ECUSettingsWidget::addSpinField(QGridLayout *grid, int row, int col,
                                      const QString &label, int min, int max, int defaultVal,
                                      const QString &suffix) {
  QLabel *lbl = new QLabel(label);
  lbl->setStyleSheet(LABEL_STYLE);
  
  QSpinBox *spin = new QSpinBox();
  spin->setRange(min, max);
  spin->setValue(defaultVal);
  spin->setSuffix(suffix);
  spin->setStyleSheet(SPIN_STYLE);
  
  QVBoxLayout *v = new QVBoxLayout();
  v->setSpacing(6);
  v->addWidget(lbl);
  v->addWidget(spin);
  grid->addLayout(v, row, col);
}

void ECUSettingsWidget::addDoubleSpinField(QGridLayout *grid, int row, int col,
                                            const QString &label, double min, double max, 
                                            double defaultVal, const QString &suffix) {
  QLabel *lbl = new QLabel(label);
  lbl->setStyleSheet(LABEL_STYLE);
  
  QDoubleSpinBox *spin = new QDoubleSpinBox();
  spin->setRange(min, max);
  spin->setValue(defaultVal);
  spin->setSuffix(suffix);
  spin->setDecimals(2);
  spin->setStyleSheet(SPIN_STYLE);
  
  QVBoxLayout *v = new QVBoxLayout();
  v->setSpacing(6);
  v->addWidget(lbl);
  v->addWidget(spin);
  grid->addLayout(v, row, col);
}

void ECUSettingsWidget::addCheckField(QGridLayout *grid, int row, int col,
                                       const QString &label, bool checked) {
  QCheckBox *cb = new QCheckBox(label);
  cb->setChecked(checked);
  cb->setStyleSheet("");
  grid->addWidget(cb, row, col);
}

void ECUSettingsWidget::addSensorCalibrationCombo(QGridLayout *grid, int row, int col,
                                                   const QString &label, const QString &sensorType) {
  QStringList options;
  if (sensorType == "CLT" || sensorType == "IAT") options = m_cltSensors;
  else if (sensorType == "MAP") options = m_mapSensors;
  else if (sensorType == "O2") options = m_o2Sensors;
  else options << "Custom";
  
  addComboField(grid, row, col, label, options);
}

// ==================== PAGE IMPLEMENTATIONS ====================

QWidget* ECUSettingsWidget::createEnginePage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Engine and Sequential Settings"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setColumnStretch(0, 1);
  grid->setColumnStretch(1, 1);
  grid->setSpacing(25);
  
  addBoundCombo(grid, 0, 0, "Control Algorithm", 
                {"Speed Density (MAP based)", "Alpha-N", "MAF", "Speed Density/Alpha-N Blend"},
                "algorithm");
  addBoundCombo(grid, 0, 1, "Squirts Per Engine Cycle", {"1", "2", "3", "4", "5", "6", "8"}, "squirtsPerCycle");
  
  addBoundCombo(grid, 1, 0, "Injector Staging", 
                {"Alternating (Bank-to-Bank)", "Simultaneous", "Semi-Sequential", "Sequential"},
                "injLayout");
  addBoundCombo(grid, 1, 1, "Engine Stroke/Rotary", 
                {"Four-stroke", "Two-stroke", "Rotary"}, "engineType");
  
  addBoundSpin(grid, 2, 0, "No. Cylinders/Rotors", "nCylinders");
  addBoundSpin(grid, 2, 1, "Number of Injectors", "nInjectors");
  
  addBoundCombo(grid, 3, 0, "Engine Type", {"Even Fire", "Odd Fire"}, "engineFireType");
  
  addBoundSpin(grid, 4, 0, "Engine Size (cc)", "engineCc", " cc");
  addBoundSpin(grid, 4, 1, "Injector Size Each (cc/min)", "injectorCc", " cc/min");
  
  layout->addLayout(grid);
  
  // Required Fuel Calculator Button
  QPushButton *btn = new QPushButton("Required Fuel Calculator");
  btn->setStyleSheet(
      "QPushButton { background: transparent; border: 1px solid #007BFF; "
      "color: #007BFF; padding: 10px 20px; border-radius: 4px; font-weight: bold; }"
      "QPushButton:hover { background: rgba(0, 123, 255, 0.1); }");
  layout->addWidget(btn, 0, Qt::AlignLeft);
  
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createSequentialFuelPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Sequential Fuel Injection"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setColumnStretch(0, 1);
  grid->setColumnStretch(1, 1);
  grid->setSpacing(25);
  
  addBoundCombo(grid, 0, 0, "Injection Mode", 
                {"Batch", "Semi-Sequential", "Sequential"}, "injMode");
  addBoundCombo(grid, 0, 1, "Injection Timing", 
                {"End of Injection", "Start of Injection"}, "injTiming");
  
  addBoundSpin(grid, 1, 0, "Injection Angle", "injAngle", "°");
  addBoundSpin(grid, 1, 1, "Injection Offset Cyl 1", "injAngleOffset", "°");
  
  addBoundCheck(grid, 2, 0, "Enable Staged Injection", "stagingEnabled");
  addBoundSpin(grid, 2, 1, "Staged Injection RPM", "stagingRpm", " RPM");
  
  addBoundSpin(grid, 3, 0, "Staged Injection TPS", "stagingTps", " %");
  addBoundSpin(grid, 3, 1, "Secondary Injector Size", "stagingInjSize", " cc/min");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createGeneralSettingsPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("General Settings"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setColumnStretch(0, 1);
  grid->setColumnStretch(1, 1);
  grid->setSpacing(25);
  
  addBoundCombo(grid, 0, 0, "Primary Trigger Type", 
                {"Missing Tooth", "Basic Distributor", "Dual Wheel", "VR", "Hall Effect"}, "triggerType");
  addBoundSpin(grid, 0, 1, "Trigger Wheel Teeth", "triggerWheel");
  addBoundSpin(grid, 1, 0, "Missing Teeth", "triggerMissing");
  addBoundSpin(grid, 1, 1, "Trigger Angle", "triggerAngle", "°");
  
  addBoundCombo(grid, 2, 0, "Spark Output Mode", 
                {"Wasted Spark", "Sequential", "Single Channel"}, "sparkMode");
  addBoundCombo(grid, 2, 1, "Spark Edge", {"Falling", "Rising"}, "sparkEdge");
  
  addBoundDoubleSpin(grid, 3, 0, "Fixed Dwell", "fixedDwell", " ms");
  addBoundCheck(grid, 3, 1, "Use Dwell Table", "useDwellTable");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createRevLimiterPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Rev Limiter"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setColumnStretch(0, 1);
  grid->setColumnStretch(1, 1);
  grid->setSpacing(25);
  
  // Use bound controls that connect to ECU settings
  addBoundCombo(grid, 0, 0, "Rev Limit Mode", 
                {"Hard Cut", "Soft Cut", "Fuel Only", "Spark Only"}, "revLimMode");
  addBoundSpin(grid, 0, 1, "Hard Rev Limit", "hardRevLim", " RPM");
  
  addBoundSpin(grid, 1, 0, "Soft Rev Limit", "softRevLim", " RPM");
  addBoundSpin(grid, 1, 1, "Soft Limit Retard", "softLimRetard", "°");
  
  addBoundSpin(grid, 2, 0, "Rev Limit Hysteresis", "revLimHyst", " RPM");
  addBoundDoubleSpin(grid, 2, 1, "Fuel Cut %", "fuelCutPct", " %");
  
  addBoundCheck(grid, 3, 0, "Enable Launch Control", "launchEnabled");
  addBoundSpin(grid, 3, 1, "Launch Control RPM", "launchRpm", " RPM");
  
  // Additional launch control settings
  addBoundSpin(grid, 4, 0, "Launch Retard", "launchRetard", "°");
  addBoundCheck(grid, 4, 1, "Enable Boost Cut", "boostCutEnable");
  addBoundDoubleSpin(grid, 5, 0, "Boost Cut Pressure", "boostCutPsi", " psi");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createShiftLightPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Shift Light"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addBoundCheck(grid, 0, 0, "Enable Shift Light", "shiftEnabled");
  addBoundSpin(grid, 0, 1, "Activation RPM", "shiftRpm", " RPM");
  
  addBoundCombo(grid, 1, 0, "Output Pin", 
                {"Spare 1", "Spare 2", "Spare 3", "Low Side 1", "Low Side 2", "Fan", "Tacho", "Fuel Pump"}, "shiftOutput");
  addBoundCheck(grid, 1, 1, "Invert Output", "shiftInvert");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createEngineStatePage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Engine State Settings"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addBoundSpin(grid, 0, 0, "Cranking RPM", "crankingRpm", " RPM");
  addBoundSpin(grid, 0, 1, "Flood Clear TPS", "floodClearTps", " %");
  
  addBoundSpin(grid, 1, 0, "Afterstart Enrichment Hold", "aseHold", " sec");
  addBoundDoubleSpin(grid, 1, 1, "Afterstart Decay Rate", "aseDecay", " %/sec");
  
  addBoundCheck(grid, 2, 0, "Enable Overrun Fuel Cut", "dfcoEnabled");
  addBoundSpin(grid, 2, 1, "Overrun Fuel Cut RPM", "dfcoRpm", " RPM");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createTachoOutputPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Tacho Output"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addBoundCheck(grid, 0, 0, "Enable Tacho Output", "tachoEnabled");
  addBoundCombo(grid, 0, 1, "Output Pin", 
                {"Tacho", "Spare 1", "Spare 2", "Low Side 1", "Low Side 2", "Fan", "Fuel Pump"}, "tachoOutput");
  
  addBoundSpin(grid, 1, 0, "Tacho Multiplier", "tachoMult");
  addBoundSpin(grid, 1, 1, "Tacho Divider", "tachoDiv");
  
  addBoundDoubleSpin(grid, 2, 0, "Pulse Width", "tachoPulseMs", " ms");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createFanControlPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Fan Control"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addBoundCheck(grid, 0, 0, "Enable Fan Control", "fanEnabled");
  addBoundCombo(grid, 0, 1, "Fan Output", 
                {"Fan", "Spare 1", "Spare 2", "Low Side 1", "Low Side 2", "Tacho", "Fuel Pump"}, "fanOutput");
  
  addBoundSpin(grid, 1, 0, "Fan On Temperature", "fanOnTemp", " °C");
  addBoundSpin(grid, 1, 1, "Fan Off Temperature", "fanOffTemp", " °C");
  
  addBoundSpin(grid, 2, 0, "Hysteresis", "fanHyst", " °C");
  addBoundCheck(grid, 2, 1, "Invert Output", "fanInvert");
  
  addBoundCheck(grid, 3, 0, "A/C Request Fan", "fanAcRequest");
  addBoundSpin(grid, 3, 1, "Min RPM for A/C", "fanMinRpmAc", " RPM");
  
  addBoundCheck(grid, 4, 0, "Fan Idle Up", "fanIdleUp");
  addBoundSpin(grid, 4, 1, "Fan Idle Up RPM", "fanIdleUpRpm", " RPM");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createTorqueConverterPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Torque Converter Lockup"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addBoundCheck(grid, 0, 0, "Enable TCC Lockup", "tccEnabled");
  addBoundCombo(grid, 0, 1, "Lockup Output", 
                {"Spare 1", "Spare 2", "Low Side 1", "Low Side 2", "Fan", "Tacho", "Fuel Pump"}, "tccOutput");
  
  addBoundSpin(grid, 1, 0, "Minimum Speed", "tccMinSpeed", " km/h");
  addBoundSpin(grid, 1, 1, "Minimum TPS", "tccMinTps", " %");
  
  addBoundSpin(grid, 2, 0, "Minimum RPM", "tccMinRpm", " RPM");
  addBoundSpin(grid, 2, 1, "Unlock TPS", "tccUnlockTps", " %");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createAlternatorControlPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Alternator Control"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addBoundCheck(grid, 0, 0, "Enable Alternator Control", "altEnabled");
  addBoundCombo(grid, 0, 1, "Alternator Output", 
                {"Spare 1", "Spare 2", "Low Side 1", "Low Side 2", "Fan", "Tacho", "Fuel Pump"}, "altOutput");
  
  addBoundDoubleSpin(grid, 1, 0, "Target Voltage", "altTargetV", " V");
  addBoundSpin(grid, 1, 1, "PWM Frequency", "altPwmFreq", " Hz");
  
  addBoundSpin(grid, 2, 0, "Min Duty", "altMinDuty", " %");
  addBoundSpin(grid, 2, 1, "Max Duty", "altMaxDuty", " %");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createBarometricPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Barometric Correction"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addBoundCombo(grid, 0, 0, "Baro Correction Mode", 
                {"Off", "Initial MAP Reading", "External Baro Sensor", "Key On MAP"}, "baroMode");
  addBoundCombo(grid, 0, 1, "Baro Sensor Type", 
                {"MPX4250", "MPX4115", "MPXH6300", "MPXH6400", "4 Bar", "GM 1 Bar", "GM 2 Bar", "Custom"}, "baroSensor");
  
  addBoundSpin(grid, 1, 0, "Baro Correction %", "baroCorrPct", " %");
  addBoundSpin(grid, 1, 1, "Update Interval", "baroUpdateSec", " sec");
  
  layout->addLayout(grid);
  
  // Baro Correction Table
  QLabel *tableLabel = new QLabel("Barometric Correction Table");
  tableLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #CCC; margin-top: 20px;");
  layout->addWidget(tableLabel);
  
  QTableWidget *table = new QTableWidget(4, 4);
  table->setStyleSheet("QTableWidget { background: #252525; color: #00FF00; gridline-color: #444; }"
                       "QHeaderView::section { background: #333; color: #CCC; padding: 4px; border: 1px solid #444; }");
  table->setFixedHeight(150);
  QStringList headers = {"60 kPa", "80 kPa", "100 kPa", "110 kPa"};
  table->setHorizontalHeaderLabels(headers);
  table->setVerticalHeaderLabels({"Fuel %", "Spark °", "Boost %", "IAT Corr"});
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  layout->addWidget(table);
  
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createIATMAPPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("IAT/MAP Sample Settings"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addBoundSpin(grid, 0, 0, "MAP Sample Count", "mapSampleCount");
  addBoundSpin(grid, 0, 1, "IAT Sample Count", "iatSampleCount");
  
  addBoundCombo(grid, 1, 0, "MAP Sample Method", 
                {"Average", "Minimum", "Maximum", "Cycle Average"}, "mapSampleMethod");
  addBoundCombo(grid, 1, 1, "IAT Sample Method", 
                {"Average", "Minimum", "Maximum", "Cycle Average"}, "iatSampleMethod");
  
  addBoundSpin(grid, 2, 0, "Sample Interval", "sampleIntervalMs", " ms");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createTPSLoadPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("TPS/ITB Load Settings"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addBoundDoubleSpin(grid, 0, 0, "TPS Closed Voltage", "tpsMin", " V");
  addBoundDoubleSpin(grid, 0, 1, "TPS Wide Open Voltage", "tpsMax", " V");
  
  addBoundSpin(grid, 1, 0, "TPS DOT Threshold", "tpsDotThresh", " %/sec");
  addBoundSpin(grid, 1, 1, "TPS Flood Clear", "tpsFloodClear", " %");
  
  addBoundCheck(grid, 2, 0, "Enable ITB Mode", "itbEnabled");
  addBoundSpin(grid, 2, 1, "Alpha-N Blend %", "itbBlend", " %");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createMAPSettingsPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("MAP Settings"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addBoundCombo(grid, 0, 0, "MAP Sensor Type", 
               {"MPX4250", "MPX4115", "MPXH6300", "MPXH6400", "4 Bar", "GM 1 Bar", "GM 2 Bar", "GM 3 Bar", "Custom"}, "mapSensorType");
  addBoundDoubleSpin(grid, 0, 1, "MAP at 0V", "mapAt0V", " kPa");
  
  addBoundDoubleSpin(grid, 1, 0, "MAP at 5V", "mapAt5V", " kPa");
  addBoundSpin(grid, 1, 1, "MAP Sample Window", "mapSampleWindow", "° BTDC");
  
  addBoundCheck(grid, 2, 0, "Use MAP for Timing", "mapForTiming");
  addBoundCheck(grid, 2, 1, "Launch Control MAP Limit", "launchMapLimit");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createMAFSettingsPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("MAF Settings"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addBoundCheck(grid, 0, 0, "Enable MAF", "mafEnabled");
  addBoundCombo(grid, 0, 1, "MAF Type", 
                {"Analog Voltage", "Frequency"}, "mafType");
  
  addBoundCombo(grid, 1, 0, "MAF Sensor", 
                {"GM LS1", "Ford Lightning", "Q45", "Custom"}, "mafSensor");
  addBoundSpin(grid, 1, 1, "MAF Frequency", "mafFreq", " Hz");
  
  addBoundDoubleSpin(grid, 2, 0, "MAF Flow at 0V", "mafFlow0V", " g/s");
  addBoundDoubleSpin(grid, 2, 1, "MAF Flow at 5V", "mafFlow5V", " g/s");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createMAFFlowCurvePage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("MAF Flow Curve"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QLabel *info = new QLabel("Define the MAF voltage to airflow relationship:");
  info->setStyleSheet("color: #888; margin-bottom: 10px;");
  layout->addWidget(info);
  
  // MAF Curve Table
  QTableWidget *table = new QTableWidget(2, 16);
  table->setStyleSheet("QTableWidget { background: #252525; color: #00FF00; gridline-color: #444; }"
                       "QHeaderView::section { background: #333; color: #CCC; padding: 4px; border: 1px solid #444; }");
  table->setFixedHeight(100);
  table->setVerticalHeaderLabels({"Voltage (V)", "Flow (g/s)"});
  table->horizontalHeader()->setVisible(false);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  
  // Populate with default values
  for (int i = 0; i < 16; ++i) {
    table->setItem(0, i, new QTableWidgetItem(QString::number(i * 0.3125, 'f', 2)));
    table->setItem(1, i, new QTableWidgetItem(QString::number(i * 25)));
  }
  
  layout->addWidget(table);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createMAFMATCorrectionPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("MAF/MAT Correction Table"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QLabel *info = new QLabel("Air temperature correction for MAF readings:");
  info->setStyleSheet("color: #888; margin-bottom: 10px;");
  layout->addWidget(info);
  
  QTableWidget *table = new QTableWidget(2, 8);
  table->setStyleSheet("QTableWidget { background: #252525; color: #00FF00; gridline-color: #444; }"
                       "QHeaderView::section { background: #333; color: #CCC; padding: 4px; }");
  table->setFixedHeight(100);
  table->setVerticalHeaderLabels({"Temp (°C)", "Corr (%)"});
  table->horizontalHeader()->setVisible(false);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  
  int temps[] = {-20, 0, 20, 40, 60, 80, 100, 120};
  int corrs[] = {120, 110, 100, 95, 90, 85, 80, 75};
  for (int i = 0; i < 8; ++i) {
    table->setItem(0, i, new QTableWidgetItem(QString::number(temps[i])));
    table->setItem(1, i, new QTableWidgetItem(QString::number(corrs[i])));
  }
  
  layout->addWidget(table);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createMATAirDensityPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("MAT Air Density Table"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QLabel *info = new QLabel("Intake Air Temperature correction for fuel calculation:");
  info->setStyleSheet("color: #888; margin-bottom: 10px;");
  layout->addWidget(info);
  
  QGridLayout *sensGrid = new QGridLayout();
  addSensorCalibrationCombo(sensGrid, 0, 0, "IAT Sensor Calibration", "IAT");
  layout->addLayout(sensGrid);
  
  QTableWidget *table = new QTableWidget(2, 10);
  table->setStyleSheet("QTableWidget { background: #252525; color: #00FF00; gridline-color: #444; }"
                       "QHeaderView::section { background: #333; color: #CCC; padding: 4px; }");
  table->setFixedHeight(100);
  table->setVerticalHeaderLabels({"Temp (°C)", "Density (%)"});
  table->horizontalHeader()->setVisible(false);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  
  layout->addWidget(table);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createMATCLTCorrectionPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("MAT/CLT Correction"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addSensorCalibrationCombo(grid, 0, 0, "CLT Sensor Calibration", "CLT");
  addSensorCalibrationCombo(grid, 0, 1, "IAT Sensor Calibration", "IAT");
  
  addDoubleSpinField(grid, 1, 0, "CLT Bias Resistor", 1000, 10000, 2490, " Ω");
  addDoubleSpinField(grid, 1, 1, "IAT Bias Resistor", 1000, 10000, 2490, " Ω");
  
  layout->addLayout(grid);
  
  QLabel *info = new QLabel("Sensor calibration tables will be loaded from .inc files");
  info->setStyleSheet("color: #666; font-style: italic; margin-top: 20px;");
  layout->addWidget(info);
  
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createRealtimeDisplayPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Realtime Display"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addBoundCombo(grid, 0, 0, "Temperature Display", {"Celsius", "Fahrenheit"}, "tempDisplay");
  addBoundCombo(grid, 0, 1, "Pressure Display", {"kPa", "PSI", "Bar"}, "pressDisplay");
  
  addBoundCombo(grid, 1, 0, "Lambda Display", {"Lambda", "AFR"}, "lambdaDisplay");
  addBoundCombo(grid, 1, 1, "Speed Display", {"km/h", "mph"}, "speedDisplay");
  
  addBoundSpin(grid, 2, 0, "Data Update Rate", "dataRate", " Hz");
  addBoundCheck(grid, 2, 1, "Show Raw ADC Values", "showRawAdc");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createIOPinsListPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Feature List Showing IO Pins"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  // IO Pins Table
  QTableWidget *table = new QTableWidget(16, 4);
  table->setStyleSheet("QTableWidget { background: #252525; color: white; gridline-color: #444; }"
                       "QHeaderView::section { background: #333; color: #CCC; padding: 6px; }");
  table->setHorizontalHeaderLabels({"Feature", "Pin", "Status", "Type"});
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->setAlternatingRowColors(true);
  
  QStringList features = {"Fuel Pump", "Idle Valve", "Boost Control", "Fan", "Tacho", "VTEC", 
                          "Injector 1", "Injector 2", "Injector 3", "Injector 4",
                          "Coil 1", "Coil 2", "Coil 3", "Coil 4", "VSS Input", "Cam Input"};
  QStringList pins = {"D2", "D3", "D4", "D5", "D6", "D7", "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "D8", "D9"};
  
  for (int i = 0; i < 16; ++i) {
    table->setItem(i, 0, new QTableWidgetItem(features[i]));
    table->setItem(i, 1, new QTableWidgetItem(pins[i]));
    table->setItem(i, 2, new QTableWidgetItem(i < 10 ? "Active" : "Inactive"));
    table->setItem(i, 3, new QTableWidgetItem(i < 10 ? "Output" : "Input"));
  }
  
  layout->addWidget(table);
  return scroll;
}

QWidget* ECUSettingsWidget::createIOPinsUsagePage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("IO Pins Usage & Status"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  // Status indicators
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(15);
  
  QStringList ioNames = {"Injector 1", "Injector 2", "Injector 3", "Injector 4",
                         "Coil 1", "Coil 2", "Fuel Pump", "Idle Valve",
                         "Fan", "VTEC", "Boost", "Tacho"};
  
  for (int i = 0; i < 12; ++i) {
    QFrame *indicator = new QFrame();
    indicator->setFixedSize(20, 20);
    indicator->setStyleSheet("background: #333; border-radius: 10px;");
    
    // Store in map for BUG-010 updates
    m_ioStatusIndicators[ioNames[i]] = indicator;

    QLabel *label = new QLabel(ioNames[i]);
    label->setStyleSheet("color: #CCC;");
    
    QHBoxLayout *row = new QHBoxLayout();
    row->addWidget(indicator);
    row->addWidget(label);
    row->addStretch();
    
    grid->addLayout(row, i / 4, i % 4);
  }
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createCANbusPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("CANbus Usage & Status"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addBoundCheck(grid, 0, 0, "Enable CAN Bus", "canEnabled");
  addBoundCombo(grid, 0, 1, "CAN Speed", {"125 kbps", "250 kbps", "500 kbps", "1 Mbps"}, "canSpeed");
  
  addBoundSpin(grid, 1, 0, "Base CAN ID", "canBaseId");
  addBoundCombo(grid, 1, 1, "CAN Protocol", {"Standard", "OBD2", "Custom"}, "canProtocol");
  
  addBoundCheck(grid, 2, 0, "Broadcast Realtime Data", "canBroadcast");
  addBoundSpin(grid, 2, 1, "Broadcast Interval", "canInterval", " ms");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createExpansionIOPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Expansion IO Pins"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QLabel *info = new QLabel("Configure expansion board IO pins:");
  info->setStyleSheet("color: #888; margin-bottom: 15px;");
  layout->addWidget(info);
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(20);
  
  for (int i = 0; i < 8; ++i) {
    addBoundCombo(grid, i, 0, QString("Expansion Pin %1").arg(i + 1),
                  {"Unused", "Generic Output", "Generic Input", "PWM Output", 
                   "Analog Input", "Fuel Pump", "Fan", "Boost Control"},
                  QString("expansionPin%1").arg(i + 1));
  }
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

QWidget* ECUSettingsWidget::createGaugeLimitsPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Gauge and Settings Limits"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addSpinField(grid, 0, 0, "RPM Gauge Max", 5000, 20000, 8000, " RPM");
  addSpinField(grid, 0, 1, "MAP Gauge Max", 100, 400, 250, " kPa");
  
  addSpinField(grid, 1, 0, "CLT Gauge Max", 100, 150, 120, " °C");
  addSpinField(grid, 1, 1, "IAT Gauge Max", 50, 100, 80, " °C");
  
  addDoubleSpinField(grid, 2, 0, "AFR Gauge Min", 8, 12, 10, " AFR");
  addDoubleSpinField(grid, 2, 1, "AFR Gauge Max", 16, 22, 18, " AFR");
  
  addSpinField(grid, 3, 0, "TPS Gauge Max", 100, 100, 100, " %");
  addSpinField(grid, 3, 1, "Duty Cycle Max", 100, 100, 100, " %");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

// ========================================================================
// NEW-001: EGO / Closed-Loop O2 Control Settings
// ========================================================================
QWidget* ECUSettingsWidget::createEGOPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("EGO / Closed-Loop O2 Control"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  // Info banner
  QLabel *infoBanner = new QLabel(
      "⚡ Configure the closed-loop O2 correction system. When enabled, the ECU "
      "adjusts fuel delivery based on the O2 sensor reading to maintain the target AFR.");
  infoBanner->setWordWrap(true);
  infoBanner->setStyleSheet("background: rgba(0,188,212,0.1); border: 1px solid rgba(0,188,212,0.3); "
                            "border-radius: 4px; padding: 10px; color: #00BCD4; font-size: 12px;");
  layout->addWidget(infoBanner);
  
  // Main controls
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addBoundCombo(grid, 0, 0, "EGO Algorithm", {"Disabled", "Simple On/Off", "PID"}, "egoType");
  addBoundCombo(grid, 0, 1, "O2 Sensor Type", {"Narrowband", "Wideband Generic", "AEM UEGO", "Innovate LC-1"}, "egoSensor");
  
  addBoundSpin(grid, 1, 0, "Authority", "egoAuthority", " %");
  addBoundDoubleSpin(grid, 1, 1, "Step Size", "egoStepSize", " %");
  
  addBoundSpin(grid, 2, 0, "Update Rate", "egoUpdateRate", " ms");
  addBoundSpin(grid, 2, 1, "Min CLT", "egoMinClt", " °C");
  
  addBoundSpin(grid, 3, 0, "Min RPM", "egoMinRpm", " RPM");
  addBoundSpin(grid, 3, 1, "Min TPS", "egoMinTps", " %");
  addBoundSpin(grid, 4, 0, "Max MAP", "egoMaxMap", " kPa");
  
  addBoundSpin(grid, 5, 0, "Min Correction", "egoCorrMin", " %");
  addBoundSpin(grid, 5, 1, "Max Correction", "egoCorrMax", " %");
  
  layout->addLayout(grid);
  
  // PID tuning group
  QGroupBox *pidGroup = new QGroupBox("PID Tuning (when Algorithm = PID)");
  pidGroup->setStyleSheet("QGroupBox { color: #00BCD4; border: 1px solid #444; "
                          "border-radius: 4px; margin-top: 15px; padding-top: 15px; } "
                          "QGroupBox::title { subcontrol-position: top left; padding: 0 8px; }");
  QGridLayout *pidGrid = new QGridLayout(pidGroup);
  pidGrid->setSpacing(25);
  
  addBoundDoubleSpin(pidGrid, 0, 0, "Kp (Proportional)", "egoKP");
  addBoundDoubleSpin(pidGrid, 0, 1, "Ki (Integral)", "egoKI");
  addBoundDoubleSpin(pidGrid, 0, 2, "Kd (Derivative)", "egoKD");
  
  layout->addWidget(pidGroup);
  
  // Live readout
  QGroupBox *liveGroup = new QGroupBox("Live Status");
  liveGroup->setStyleSheet("QGroupBox { color: #4CAF50; border: 1px solid #444; "
                           "border-radius: 4px; margin-top: 15px; padding-top: 15px; } "
                           "QGroupBox::title { subcontrol-position: top left; padding: 0 8px; }");
  QHBoxLayout *liveLayout = new QHBoxLayout(liveGroup);
  
  m_egoLiveValue = new QLabel("EGO Correction: --- %");
  m_egoLiveValue->setStyleSheet("color: #00FF00; font-size: 18px; font-weight: bold;");
  liveLayout->addWidget(m_egoLiveValue);
  liveLayout->addStretch();
  
  layout->addWidget(liveGroup);
  layout->addStretch();
  return scroll;
}

// ========================================================================
// NEW-002: Injector Dead-Time vs Battery Voltage
// ========================================================================
QWidget* ECUSettingsWidget::createInjectorDeadTimePage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Injector Dead-Time vs Battery Voltage"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QLabel *infoBanner = new QLabel(
      "🔧 Define injector opening dead-time across battery voltage. Lower voltages "
      "require longer dead-times to account for slower injector response.");
  infoBanner->setWordWrap(true);
  infoBanner->setStyleSheet("background: rgba(255,152,0,0.1); border: 1px solid rgba(255,152,0,0.3); "
                            "border-radius: 4px; padding: 10px; color: #FF9800; font-size: 12px;");
  layout->addWidget(infoBanner);
  
  // 6-point table
  QTableWidget *table = new QTableWidget(6, 2);
  table->setHorizontalHeaderLabels({"Battery Voltage (V)", "Dead-Time (ms)"});
  table->horizontalHeader()->setStretchLastSection(true);
  table->setStyleSheet(
      "QTableWidget { background: #252525; border: 1px solid #444; color: white; gridline-color: #333; } "
      "QHeaderView::section { background: #333; color: #00BCD4; padding: 6px; border: 1px solid #444; font-weight: bold; } "
      "QTableWidget::item { padding: 4px; }");
  table->setFixedHeight(240);
  
  // Populate default values (typical injector data)
  double defaultVoltage[] = {6.0, 8.0, 10.0, 12.0, 14.0, 16.0};
  double defaultDead[]    = {3.0, 2.0, 1.2, 1.0, 0.8, 0.7};
  
  for (int i = 0; i < 6; ++i) {
    // Voltage bins
    QDoubleSpinBox *vSpin = new QDoubleSpinBox();
    vSpin->setRange(6.0, 20.0);
    vSpin->setDecimals(1);
    vSpin->setValue(defaultVoltage[i]);
    vSpin->setSuffix(" V");
    vSpin->setStyleSheet(SPIN_STYLE);
    QString vName = QString("injDeadBin%1").arg(i + 1);
    m_settingDoubleSpins[vName] = vSpin;
    table->setCellWidget(i, 0, vSpin);
    
    connect(vSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this, vName](double val) {
      if (m_settingsManager) m_settingsManager->setValue(vName, val);
    });
    
    // Dead-time values
    QDoubleSpinBox *dSpin = new QDoubleSpinBox();
    dSpin->setRange(0.0, 5.0);
    dSpin->setDecimals(2);
    dSpin->setValue(defaultDead[i]);
    dSpin->setSuffix(" ms");
    dSpin->setStyleSheet(SPIN_STYLE);
    QString dName = QString("injDeadVal%1").arg(i + 1);
    m_settingDoubleSpins[dName] = dSpin;
    table->setCellWidget(i, 1, dSpin);
    
    connect(dSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this, dName](double val) {
      if (m_settingsManager) m_settingsManager->setValue(dName, val);
    });
  }
  
  layout->addWidget(table);
  layout->addStretch();
  return scroll;
}

// ========================================================================
// NEW-003: Sensor Calibration (CLT, IAT, O2)
// ========================================================================
QWidget* ECUSettingsWidget::createSensorCalibrationPage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Sensor Calibration"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QLabel *infoBanner = new QLabel(
      "🌡️ Select predefined calibration profiles for temperature and O2 sensors, "
      "or define custom 3-point lookup tables.");
  infoBanner->setWordWrap(true);
  infoBanner->setStyleSheet("background: rgba(76,175,80,0.1); border: 1px solid rgba(76,175,80,0.3); "
                            "border-radius: 4px; padding: 10px; color: #4CAF50; font-size: 12px;");
  layout->addWidget(infoBanner);
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  // Sensor type selectors
  addSensorCalibrationCombo(grid, 0, 0, "CLT Sensor Type", "CLT");
  addSensorCalibrationCombo(grid, 0, 1, "IAT Sensor Type", "IAT");
  addSensorCalibrationCombo(grid, 1, 0, "MAP Sensor Type", "MAP");
  addSensorCalibrationCombo(grid, 1, 1, "O2 Sensor Type", "O2");
  
  layout->addLayout(grid);
  
  // Custom CLT 3-point mapping
  QGroupBox *cltGroup = new QGroupBox("Custom CLT Calibration (3-point lookup)");
  cltGroup->setStyleSheet("QGroupBox { color: #FF9800; border: 1px solid #444; "
                          "border-radius: 4px; margin-top: 15px; padding-top: 15px; } "
                          "QGroupBox::title { subcontrol-position: top left; padding: 0 8px; }");
  QGridLayout *cltGrid = new QGridLayout(cltGroup);
  cltGrid->setSpacing(20);
  
  QLabel *hdrAdc = new QLabel("ADC Value"); hdrAdc->setStyleSheet(LABEL_STYLE);
  QLabel *hdrTemp = new QLabel("Temperature (°C)"); hdrTemp->setStyleSheet(LABEL_STYLE);
  cltGrid->addWidget(hdrAdc, 0, 1);
  cltGrid->addWidget(hdrTemp, 0, 2);
  
  for (int i = 0; i < 3; ++i) {
    QLabel *lbl = new QLabel(QString("Point %1:").arg(i + 1));
    lbl->setStyleSheet(LABEL_STYLE);
    
    QSpinBox *adc = new QSpinBox();
    adc->setRange(0, 1023);
    adc->setStyleSheet(SPIN_STYLE);
    
    QDoubleSpinBox *temp = new QDoubleSpinBox();
    temp->setRange(-40, 200);
    temp->setDecimals(1);
    temp->setSuffix(" °C");
    temp->setStyleSheet(SPIN_STYLE);
    
    cltGrid->addWidget(lbl, i + 1, 0);
    cltGrid->addWidget(adc, i + 1, 1);
    cltGrid->addWidget(temp, i + 1, 2);
  }
  
  layout->addWidget(cltGroup);
  
  // Upload button
  QPushButton *uploadBtn = new QPushButton("📤 Upload Calibration to ECU");
  uploadBtn->setStyleSheet(
      "QPushButton { background: rgba(0,188,212,0.2); color: #00BCD4; "
      "border: 1px solid rgba(0,188,212,0.3); border-radius: 4px; "
      "padding: 10px 20px; font-weight: bold; font-size: 13px; } "
      "QPushButton:hover { background: rgba(0,188,212,0.3); }");
  connect(uploadBtn, &QPushButton::clicked, [this]() {
    Logger::info("Sensor calibration upload requested (requires 't' command support)");
  });
  layout->addWidget(uploadBtn);
  
  layout->addStretch();
  return scroll;
}

// ========================================================================
// NEW-005: Cranking Advance Settings
// ========================================================================
QWidget* ECUSettingsWidget::createCrankingAdvancePage() {
  QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Cranking Advance Settings"));
  QWidget *page = scroll->widget();
  QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
  
  QLabel *infoBanner = new QLabel(
      "🔑 Configure the ignition timing advance used during engine cranking. "
      "A fixed timing is safer during start-up to prevent kickback.");
  infoBanner->setWordWrap(true);
  infoBanner->setStyleSheet("background: rgba(156,39,176,0.1); border: 1px solid rgba(156,39,176,0.3); "
                            "border-radius: 4px; padding: 10px; color: #9C27B0; font-size: 12px;");
  layout->addWidget(infoBanner);
  
  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(25);
  
  addBoundSpin(grid, 0, 0, "Cranking Advance", "crankingAdvance", " °BTDC");
  addBoundCheck(grid, 0, 1, "Use Fixed Cranking Timing", "fixedCrankTiming");
  
  layout->addLayout(grid);
  layout->addStretch();
  return scroll;
}

// ========================================================================
// AWARD-WINNING: Technical Reference Database
// ========================================================================
QWidget* ECUSettingsWidget::createTechnicalReferencePage() {
    QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Technical Knowledge Database"));
    QWidget *page = scroll->widget();
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
    layout->setSpacing(25);

    QLabel *intro = new QLabel("📚 Technical Reference: Detailed information on ECU tuning concepts and Speeduino logic.");
    intro->setStyleSheet("color: #FFD700; font-style: italic; font-size: 13px;");
    layout->addWidget(intro);

    // Search bar for DB
    QLineEdit *dbSearch = new QLineEdit();
    dbSearch->setPlaceholderText("Search technical topics (e.g. VTEC, Turbo)...");
    dbSearch->setStyleSheet("padding: 10px; background: #222; color: white; border: 1px solid #444; border-radius: 4px;");
    layout->addWidget(dbSearch);

    QVBoxLayout *entriesLayout = new QVBoxLayout();
    layout->addLayout(entriesLayout);

    auto renderTerm = [entriesLayout](const TechnicalTerm &term) {
        QGroupBox *group = new QGroupBox(term.title);
        group->setStyleSheet("QGroupBox { border: 2px solid #333; border-radius: 8px; margin-top: 20px; padding-top: 20px; color: #00BCD4; font-weight: bold; font-size: 15px; }");
        QVBoxLayout *gl = new QVBoxLayout(group);

        QLabel *catBadge = new QLabel(term.category);
        catBadge->setStyleSheet("background: rgba(0, 188, 212, 0.2); color: #00BCD4; padding: 4px 8px; border-radius: 4px; font-size: 10px; font-weight: bold; font-family: monospace;");
        gl->addWidget(catBadge, 0, Qt::AlignLeft);

        QLabel *desc = new QLabel(term.fullDescription);
        desc->setWordWrap(true);
        desc->setStyleSheet("color: #CCC; font-size: 13px; margin: 10px 0;");
        gl->addWidget(desc);

        if (!term.characteristics.isEmpty()) {
            QLabel *h = new QLabel("Characteristics:"); h->setStyleSheet("color: #AAA; font-weight: bold; font-size: 11px;");
            gl->addWidget(h);
            QLabel *list = new QLabel(" • " + term.characteristics.join("\n • "));
            list->setStyleSheet("color: #DDD; font-size: 11px; margin-bottom: 10px;");
            gl->addWidget(list);
        }

        if (!term.tuningFocus.isEmpty()) {
            QLabel *h = new QLabel("Tuning Focus:"); h->setStyleSheet("color: #4CAF50; font-weight: bold; font-size: 11px;");
            gl->addWidget(h);
            QLabel *list = new QLabel(" ✓ " + term.tuningFocus.join("\n ✓ "));
            list->setStyleSheet("color: #A5D6A7; font-size: 11px; margin-bottom: 10px;");
            gl->addWidget(list);
        }

        if (!term.typicalValues.isEmpty()) {
            QFrame *stats = new QFrame();
            stats->setStyleSheet("background: #111; border-radius: 4px; padding: 10px;");
            QGridLayout *sl = new QGridLayout(stats);
            int row = 0;
            for (auto it = term.typicalValues.begin(); it != term.typicalValues.end(); ++it) {
                QLabel *kl = new QLabel(it.key() + ":"); kl->setStyleSheet("color: #888; font-size: 10px;");
                QLabel *vl = new QLabel(it.value()); vl->setStyleSheet("color: #00BCD4; font-weight: bold; font-size: 11px;");
                sl->addWidget(kl, row, 0);
                sl->addWidget(vl, row, 1);
                row++;
            }
            gl->addWidget(stats);
        }

        entriesLayout->addWidget(group);
    };

    // Render all terms initially
    for (const auto &category : ReferenceDatabase::instance().getCategories()) {
        for (const auto &term : ReferenceDatabase::instance().getTermsByCategory(category)) {
            renderTerm(term);
        }
    }

    layout->addStretch();
    return scroll;
}

QWidget* ECUSettingsWidget::createInjectorDatabasePage() {
    QScrollArea *scroll = qobject_cast<QScrollArea*>(createSettingsPage("Comprehensive Injector Catalog"));
    QWidget *page = scroll->widget();
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(page->layout());
    layout->setSpacing(20);

    QLabel *intro = new QLabel("⛽ Comprehensive Injector Reference: Specs, Latency (Dead-time), and Flow Rates.");
    intro->setStyleSheet("color: #00BCD4; font-style: italic; font-size: 13px;");
    layout->addWidget(intro);

    // Table view for injectors
    QTableWidget *table = new QTableWidget(0, 6);
    table->setHorizontalHeaderLabels({"Brand", "Model", "Flow (cc/min)", "Impedance", "Dead-time @14V", "Manufacturer"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setStyleSheet("QTableWidget { background: #252525; color: white; gridline-color: #444; } "
                         "QHeaderView::section { background: #333; color: #00BCD4; padding: 6px; } "
                         "QTableWidget::item { padding: 4px; }");
    table->verticalHeader()->setVisible(false);
    
    auto injectors = ReferenceDatabase::instance().getAllInjectors();
    table->setRowCount(injectors.size());
    for (int i = 0; i < injectors.size(); ++i) {
        const auto &inj = injectors[i];
        table->setItem(i, 0, new QTableWidgetItem(inj.brand));
        table->setItem(i, 1, new QTableWidgetItem(inj.model));
        table->setItem(i, 2, new QTableWidgetItem(QString::number(inj.flowRateCCMin)));
        table->setItem(i, 3, new QTableWidgetItem(QString::number(inj.impedance) + " Ω"));
        table->setItem(i, 4, new QTableWidgetItem(QString::number(inj.deadTime14V) + " ms"));
        table->setItem(i, 5, new QTableWidgetItem(inj.manufacturer));
        
        // Color coding for impedance
        if (inj.impedance < 4) table->item(i, 3)->setForeground(QColor("#FF5252")); // Low Z
        else table->item(i, 3)->setForeground(QColor("#4CAF50")); // High Z
    }
    
    layout->addWidget(table);

    // HP Calculator Area
    QGroupBox *calcGroup = new QGroupBox("Estimated Max HP @ 85% Duty");
    calcGroup->setStyleSheet("QGroupBox { color: #4CAF50; border: 1px solid #444; border-radius: 8px; margin-top: 10px; padding-top: 15px; }");
    QHBoxLayout *cl = new QHBoxLayout(calcGroup);
    
    QLabel *calcIcon = new QLabel("🐎"); calcIcon->setStyleSheet("font-size: 24px;");
    QVBoxLayout *ct = new QVBoxLayout();
    QLabel *hpVal = new QLabel("Select an injector to calculate potential HP...");
    hpVal->setStyleSheet("color: #00FF00; font-size: 16px; font-weight: bold;");
    ct->addWidget(hpVal);
    
    cl->addWidget(calcIcon);
    cl->addLayout(ct);
    cl->addStretch();
    layout->addWidget(calcGroup);

    connect(table, &QTableWidget::itemSelectionChanged, [table, hpVal, injectors]() {
        int row = table->currentRow();
        if (row >= 0 && row < injectors.size()) {
            const auto &inj = injectors[row];
            // Simple calculation: cc / 5 = approx HP per injector for many engines
            double hpPerInj = inj.flowRateCCMin / 5.0; 
            double totalHp = hpPerInj * 4; // Assume 4 cylinders for demo
            hpVal->setText(QString("%1 HP estimate (4 cyl, 85% Duty)").arg(static_cast<int>(totalHp)));
        }
    });

    layout->addStretch();
    return scroll;
}
