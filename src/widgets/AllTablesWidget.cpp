/**
 * @file AllTablesWidget.cpp
 * @brief All Tables Widget - Comprehensive tuning table editor
 *
 * Redesigned to match Figma V2.0 design with:
 * - VTEC dual-map support (Low/High cam profiles)
 * - Conditional UI elements based on settings
 * - Dark theme consistent with dashboard
 */

#include "AllTablesWidget.h"
#include "../utils/Logger.h"
#include "../core/TunerProColors.h"
#include <QDebug>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QSplitter>
#include <QVBoxLayout>
#include <QtMath>

#include "TunerMap3DWidget.h"

AllTablesWidget::AllTablesWidget(ECUSettingsManager *settingsManager, QWidget *parent)
    : QWidget(parent), m_vtecEnabled(false), m_liveTuningActive(true),
      m_gridMode(true), m_lightMode(true) // Light mode default for better visibility? No, dark theme.
{
  m_settingsManager = settingsManager;
  if (m_settingsManager) {
      connect(m_settingsManager, &ECUSettingsManager::tableDataReceived, 
              this, &AllTablesWidget::onTableDataReceived);
      connect(m_settingsManager, &ECUSettingsManager::definitionsLoaded, this,
              &AllTablesWidget::onDefinitionsLoaded);
  }
          
  setupUi();
}

AllTablesWidget::~AllTablesWidget() {}

void AllTablesWidget::setupUi() {
  // Main layout with dark theme background
  setStyleSheet(QString("background-color: %1;").arg(TunerProColors::BG_BASE));

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(0);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  // === HEADER SECTION ===
  QWidget *headerWidget = new QWidget(this);
  headerWidget->setStyleSheet(
      "background-color: #252525; border-bottom: 1px solid #333;");
  headerWidget->setFixedHeight(80);
  QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
  headerLayout->setContentsMargins(20, 10, 20, 10);

  // Left side - Title
  QVBoxLayout *titleLayout = new QVBoxLayout();
  titleLayout->setSpacing(2);

  m_titleLabel = new QLabel("All Tuning Tables", this);
  m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: "
                              "white; background: transparent;");

  m_subtitleLabel = new QLabel("Comprehensive table editor with conditional "
                               "visibility • 20 tables active",
                               this);
  m_subtitleLabel->setStyleSheet(
      "font-size: 11px; color: #AAA; background: transparent;");

  titleLayout->addWidget(m_titleLabel);
  titleLayout->addWidget(m_subtitleLabel);
  headerLayout->addLayout(titleLayout);
  headerLayout->addStretch();

  // Right side - Live Tuning Indicator
  m_liveTuningLabel = new QLabel("● LIVE TUNING", this);
  m_liveTuningLabel->setStyleSheet("background-color: rgba(76, 175, 80, 0.2); "
                                   "color: #4CAF50; "
                                   "font-weight: bold; "
                                   "padding: 8px 16px; "
                                   "border-radius: 4px; "
                                   "font-size: 12px;");
  headerLayout->addWidget(m_liveTuningLabel);

  mainLayout->addWidget(headerWidget);

  // === VTEC WARNING BANNER ===
  m_vtecWarningBanner = new QFrame(this);
  m_vtecWarningBanner->setStyleSheet(
      "background-color: rgba(233, 30, 99, 0.1); "
      "border: 1px solid rgba(233, 30, 99, 0.3); "
      "border-radius: 4px;");
  m_vtecWarningBanner->setFixedHeight(50);

  QHBoxLayout *vtecLayout = new QHBoxLayout(m_vtecWarningBanner);
  vtecLayout->setContentsMargins(20, 10, 20, 10);

  QLabel *vtecIcon = new QLabel("⚠", this);
  vtecIcon->setStyleSheet(
      "color: #FFC107; font-size: 16px; background: transparent;");

  QLabel *vtecTitle = new QLabel("VTEC enabled:", this);
  vtecTitle->setStyleSheet("color: #FFC107; font-weight: bold; font-size: "
                           "12px; background: transparent;");

  QLabel *vtecText = new QLabel(
      "Dual profiles active (Low Cam / High Cam). Check for discontinuity at "
      "engagement RPM to ensure smooth transitions.",
      this);
  vtecText->setStyleSheet(
      "color: #DDD; font-size: 11px; background: transparent;");

  vtecLayout->addWidget(vtecIcon);
  vtecLayout->addSpacing(10);
  vtecLayout->addWidget(vtecTitle);
  vtecLayout->addSpacing(10);
  vtecLayout->addWidget(vtecText);
  vtecLayout->addStretch();

  mainLayout->addWidget(m_vtecWarningBanner);

  // === TOOLBAR ===
  QWidget *toolbarWidget = new QWidget(this);
  toolbarWidget->setStyleSheet("background-color: #1E1E1E;");
  toolbarWidget->setFixedHeight(50);
  QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarWidget);
  toolbarLayout->setContentsMargins(20, 10, 20, 10);

  m_tableSelector = new QComboBox(this);
  m_tableSelector->setMinimumWidth(280);
  m_tableSelector->setStyleSheet("QComboBox { "
                                 "  background-color: #2B2B2B; "
                                 "  border: 1px solid #444; "
                                 "  border-radius: 4px; "
                                 "  padding: 8px 12px; "
                                 "  color: white; "
                                 "  font-size: 12px; "
                                 "} "
                                 "QComboBox::drop-down { border: none; } "
                                 "QComboBox::down-arrow { image: "
                                 "url(:/icons/dropdown.png); width: 12px; } "
                                 "QComboBox QAbstractItemView { "
                                 "  background-color: #2B2B2B; "
                                 "  color: white; "
                                 "  selection-background-color: #00BCD4; "
                                 "  selection-color: black; "
                                 "  border: 1px solid #444; "
                                 "}");

  connect(m_tableSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &AllTablesWidget::onTableSelectionChanged);

  // Search Box
  m_searchBox = new QLineEdit(this);
  m_searchBox->setPlaceholderText("🔍 Search tables...");
  m_searchBox->setMinimumWidth(200);
  m_searchBox->setStyleSheet("QLineEdit { "
                             "  background-color: #2B2B2B; "
                             "  border: 1px solid #444; "
                             "  border-radius: 4px; "
                             "  padding: 8px 12px; "
                             "  color: white; "
                             "  font-size: 12px; "
                             "}");
  connect(m_searchBox, &QLineEdit::textChanged, this,
          &AllTablesWidget::onSearchTextChanged);

  toolbarLayout->addWidget(m_tableSelector);
  toolbarLayout->addSpacing(15);
  toolbarLayout->addWidget(m_searchBox);
  toolbarLayout->addStretch();

  mainLayout->addWidget(toolbarWidget);

  // === CONTENT AREA ===
  QWidget *contentWidget = new QWidget(this);
  contentWidget->setStyleSheet("background-color: #1E1E1E;");
  QHBoxLayout *contentLayout = new QHBoxLayout(contentWidget);
  contentLayout->setContentsMargins(20, 10, 20, 20);
  contentLayout->setSpacing(20);

  // Left side - Table Container (Replacing QWidget with QSplitter for 45/55 visual split)
  QSplitter *leftSplitter = new QSplitter(Qt::Vertical, this);
  leftSplitter->setHandleWidth(6);
  leftSplitter->setStyleSheet(
      "QSplitter::handle:vertical { "
      "  background-color: #1A2640; "
      "  height: 6px; "
      "  image: url(\"data:image/svg+xml;utf8,<svg width='20' height='6' xmlns='http://www.w3.org/2000/svg'><rect x='2' y='1' width='16' height='1' fill='%233D5070'/><rect x='2' y='3' width='16' height='1' fill='%233D5070'/><rect x='2' y='5' width='16' height='1' fill='%233D5070'/></svg>\");"
      "}"
      "QSplitter::handle:hover { background-color: #00E5C840; }"
  );

  QWidget *topContainer = new QWidget(leftSplitter);
  topContainer->setMinimumHeight(280);
  topContainer->setStyleSheet("background-color: #252525; border-radius: 8px;");
  QVBoxLayout *topLayout = new QVBoxLayout(topContainer);
  topLayout->setContentsMargins(15, 15, 15, 15);
  topLayout->setSpacing(10);

  // Table Info Header
  QWidget *tableHeader = new QWidget(topContainer);
  tableHeader->setStyleSheet("background: transparent;");
  QVBoxLayout *tableHeaderLayout = new QVBoxLayout(tableHeader);
  tableHeaderLayout->setContentsMargins(0, 0, 0, 0);
  tableHeaderLayout->setSpacing(2);

  m_tableInfoLabel = new QLabel("VE Table (Volumetric Efficiency)", topContainer);
  m_tableInfoLabel->setStyleSheet("font-weight: bold; color: white; font-size: "
                                  "14px; background: transparent;");

  m_tableDescLabel = new QLabel(
      "Primary fuel calibration table - defines engine breathing efficiency",
      topContainer);
  m_tableDescLabel->setStyleSheet(
      "color: #AAA; font-size: 11px; background: transparent;");

  tableHeaderLayout->addWidget(m_tableInfoLabel);
  tableHeaderLayout->addWidget(m_tableDescLabel);
  topLayout->addWidget(tableHeader);

  // Cam Profile Tabs (VTEC mode)
  m_camProfileTabs = new QTabWidget(this);
  m_camProfileTabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_camProfileTabs->setStyleSheet(
      "QTabWidget::pane { border: none; background: transparent; } "
      "QTabBar::tab { "
      "  background-color: transparent; "
      "  color: #888; "
      "  padding: 10px 20px; "
      "  border: none; "
      "  border-bottom: 2px solid transparent; "
      "  font-weight: bold; "
      "} "
      "QTabBar::tab:selected { "
      "  color: white; "
      "  border-bottom: 2px solid #00BCD4; "
      "} "
      "QTabBar::tab:hover { color: #ccc; }");

  // Create editors for each cam profile
  m_lowCamEditor = new TableEditor(this);
  connect(m_lowCamEditor, &TableEditor::dataChanged, this,
          &AllTablesWidget::onTableDataChanged);

  m_highCamEditor = new TableEditor(this);
  connect(m_highCamEditor, &TableEditor::dataChanged, this,
          &AllTablesWidget::onTableDataChanged);

  m_camProfileTabs->addTab(m_lowCamEditor, "Low Cam Profile");
  m_camProfileTabs->addTab(m_highCamEditor, "High Cam Profile");

  topLayout->addWidget(m_camProfileTabs);

  // Main Table Editor (non-VTEC mode fallback)
  m_tableEditor = new TableEditor(topContainer);
  connect(m_tableEditor, &TableEditor::dataChanged, this,
          &AllTablesWidget::onTableDataChanged);
  m_tableEditor->setVisible(false); // Hidden when VTEC mode is active
  m_tableEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  topLayout->addWidget(m_tableEditor);

  m_colorLegendBar = new ColorLegendBar(topContainer);
  topLayout->addWidget(m_colorLegendBar);

  // Setup 3D Graph
  setup3DGraph();
  m_graphContainer->setParent(leftSplitter);
  
  leftSplitter->addWidget(topContainer);
  leftSplitter->addWidget(m_graphContainer);
  
  // 45 / 55 Split
  leftSplitter->setStretchFactor(0, 45);
  leftSplitter->setStretchFactor(1, 55);

  contentLayout->addWidget(leftSplitter, 1);

  // Right side - Control Panel
  QWidget *rightPanel = createRightControlPanel();
  rightPanel->setMaximumWidth(220);
  contentLayout->addWidget(rightPanel, 0);

  mainLayout->addWidget(contentWidget, 1);

  // Initial state
  updateVtecVisibility();
  setupTableDropdown();
}

void AllTablesWidget::setupTableDropdown() {
  onDefinitionsLoaded();
}

void AllTablesWidget::onDefinitionsLoaded() {
  m_tableSelector->blockSignals(true);
  m_tableSelector->clear();
  QStringList tables = m_settingsManager->getTableNames();
  if (tables.isEmpty()) {
    // Fallback if none loaded
    tables << "VE Table (Volumetric Efficiency)" << "Ignition Advance Table" << "AFR Target Table";
  }
  m_tableSelector->addItems(tables);
  m_tableSelector->blockSignals(false);
  
  if (m_tableSelector->count() > 0) {
      onTableSelectionChanged(0);
  }
}

QWidget *AllTablesWidget::createRightControlPanel() {
  QWidget *panel = new QWidget(this);
  panel->setStyleSheet("background-color: #252525; border-radius: 8px;");
  panel->setMinimumWidth(280);
  panel->setMaximumWidth(350);

  QVBoxLayout *layout = new QVBoxLayout(panel);
  layout->setContentsMargins(15, 15, 15, 15);
  layout->setSpacing(15);

  // --- Table Size Row ---
  QHBoxLayout *sizeRow = new QHBoxLayout();

  QLabel *sizeLabel = new QLabel("Table Size:", this);
  sizeLabel->setStyleSheet(
      "color: #AAA; font-size: 11px; background: transparent;");

  m_tableSizeLabel = new QLabel("12×17", this);
  m_tableSizeLabel->setStyleSheet("color: white; font-weight: bold; font-size: "
                                  "12px; background: transparent;");

  sizeRow->addWidget(sizeLabel);
  sizeRow->addWidget(m_tableSizeLabel);
  sizeRow->addStretch();

  // Control buttons
  m_resetBtn = new QPushButton("⟲ Reset", this);
  m_resetBtn->setStyleSheet(
      "QPushButton { background-color: #3B3B3B; color: #888; border: none; "
      "border-radius: 4px; padding: 6px 12px; font-size: 11px; } "
      "QPushButton:hover { background-color: #444; color: white; }");
  connect(m_resetBtn, &QPushButton::clicked, this, &AllTablesWidget::onReset);

  m_gridBtn = new QPushButton("⊞ Grid", this);
  m_gridBtn->setStyleSheet(
      "QPushButton { background-color: #00BCD4; color: white; border: none; "
      "border-radius: 4px; padding: 6px 12px; font-size: 11px; font-weight: "
      "bold; } "
      "QPushButton:hover { background-color: #00ACC1; }");
  connect(m_gridBtn, &QPushButton::clicked, this,
          &AllTablesWidget::onGridToggle);

  m_lightBtn = new QPushButton("☀ Light", this);
  m_lightBtn->setStyleSheet(
      "QPushButton { background-color: #3B3B3B; color: #888; border: none; "
      "border-radius: 4px; padding: 6px 12px; font-size: 11px; } "
      "QPushButton:hover { background-color: #444; color: white; }");
  connect(m_lightBtn, &QPushButton::clicked, this,
          &AllTablesWidget::onLightToggle);

  sizeRow->addWidget(m_resetBtn);
  sizeRow->addWidget(m_gridBtn);
  sizeRow->addWidget(m_lightBtn);

  layout->addLayout(sizeRow);

  // --- Zoom Slider ---
  QHBoxLayout *zoomRow = new QHBoxLayout();
  QLabel *zoomLabel = new QLabel("Zoom:", this);
  zoomLabel->setStyleSheet(
      "color: #AAA; font-size: 11px; background: transparent;");

  m_zoomSlider = new QSlider(Qt::Horizontal, this);
  m_zoomSlider->setRange(50, 200);
  m_zoomSlider->setValue(100);
  m_zoomSlider->setStyleSheet(
      "QSlider::groove:horizontal { background: #3B3B3B; height: 4px; "
      "border-radius: 2px; } "
      "QSlider::handle:horizontal { background: #00BCD4; width: 14px; margin: "
      "-5px 0; border-radius: 7px; }");

  QLabel *zoomValue = new QLabel("100%", this);
  zoomValue->setStyleSheet(
      "color: white; font-size: 11px; background: transparent;");

  zoomRow->addWidget(zoomLabel);
  zoomRow->addWidget(m_zoomSlider, 1);
  zoomRow->addWidget(zoomValue);

  // Hondata dropdown
  m_hondataCombo = new QComboBox(this);
  m_hondataCombo->addItems({"Hondata", "FlashPro", "KTuner"});
  m_hondataCombo->setStyleSheet(
      "QComboBox { background-color: #3B3B3B; border: 1px solid #444; "
      "border-radius: 4px; padding: 4px 8px; color: white; font-size: 11px; } "
      "QComboBox QAbstractItemView { "
      "  background-color: #2B2B2B; "
      "  color: white; "
      "  selection-background-color: #00BCD4; "
      "  selection-color: black; "
      "  border: 1px solid #444; "
      "}");

  zoomRow->addWidget(m_hondataCombo);
  layout->addLayout(zoomRow);

  // --- VE Table Visualization ---
  m_veTableViz = new QFrame(this);
  m_veTableViz->setStyleSheet(
      "background-color: #1E1E1E; border: 1px solid #444; border-radius: 6px;");
  m_veTableViz->setMinimumHeight(120);

  QVBoxLayout *vizLayout = new QVBoxLayout(m_veTableViz);
  vizLayout->setContentsMargins(10, 10, 10, 10);

  QLabel *vizTitle = new QLabel("VE Table (Volumetric Efficiency)", this);
  vizTitle->setStyleSheet("color: #00BCD4; font-weight: bold; font-size: 11px; "
                          "background: transparent;");

  QLabel *vizRange = new QLabel("Range: 40.0 - 95.0 %", this);
  vizRange->setStyleSheet(
      "color: #AAA; font-size: 10px; background: transparent;");

  m_mapIndicator = new QLabel("MAP ↑", this);
  m_mapIndicator->setStyleSheet(
      "color: #BBB; font-size: 11px; background: transparent;");
  m_mapIndicator->setAlignment(Qt::AlignRight);

  vizLayout->addWidget(vizTitle);
  vizLayout->addWidget(vizRange);
  vizLayout->addStretch();
  vizLayout->addWidget(m_mapIndicator);

  layout->addWidget(m_veTableViz);

  // --- Axis Selectors ---
  QLabel *axisLabel = new QLabel("Axes:", this);
  axisLabel->setStyleSheet("color: #AAA; font-size: 11px; background: "
                           "transparent; margin-top: 5px;");
  layout->addWidget(axisLabel);

  QString checkStyle =
      "QCheckBox { color: white; font-size: 11px; background: transparent; } "
      "QCheckBox::indicator { width: 16px; height: 16px; border-radius: 3px; "
      "border: 1px solid #444; background: #2B2B2B; } "
      "QCheckBox::indicator:checked { background: #00BCD4; border-color: "
      "#00BCD4; }";

  m_xAxisRpm = new QCheckBox("X: RPM (rpm)", this);
  m_xAxisRpm->setChecked(true);
  m_xAxisRpm->setStyleSheet(checkStyle);

  m_yAxisLoad = new QCheckBox("Y: Load (MAP) (kPa)", this);
  m_yAxisLoad->setChecked(true);
  m_yAxisLoad->setStyleSheet(checkStyle);

  m_zAxisVe = new QCheckBox("Z: VE (%)", this);
  m_zAxisVe->setChecked(true);
  m_zAxisVe->setStyleSheet(checkStyle);

  layout->addWidget(m_xAxisRpm);
  layout->addWidget(m_yAxisLoad);
  layout->addWidget(m_zAxisVe);

  // --- Adjust Selected ---
  QLabel *adjustLabel = new QLabel("Adjust Selected:", this);
  adjustLabel->setStyleSheet("color: #AAA; font-size: 11px; background: "
                             "transparent; margin-top: 10px;");
  layout->addWidget(adjustLabel);

  QHBoxLayout *adjustRow = new QHBoxLayout();

  m_adjustValue = new QDoubleSpinBox(this);
  m_adjustValue->setRange(-50, 50);
  m_adjustValue->setValue(0.5);
  m_adjustValue->setSingleStep(0.1);
  m_adjustValue->setStyleSheet(
      "QDoubleSpinBox { background-color: #2B2B2B; border: 1px solid #444; "
      "border-radius: 4px; padding: 6px; color: white; font-size: 12px; }");

  m_adjustAddBtn = new QPushButton("+ Add", this);
  m_adjustAddBtn->setStyleSheet(
      "QPushButton { background-color: rgba(0, 188, 212, 0.2); color: #00BCD4; "
      "border: 1px solid rgba(0, 188, 212, 0.3); border-radius: 4px; padding: "
      "6px 12px; font-size: 11px; } "
      "QPushButton:hover { background-color: rgba(0, 188, 212, 0.3); }");
  connect(m_adjustAddBtn, &QPushButton::clicked, this,
          &AllTablesWidget::onAdjustSelected);

  m_adjustSubBtn = new QPushButton("- Subtract", this);
  m_adjustSubBtn->setStyleSheet(
      "QPushButton { background-color: rgba(244, 67, 54, 0.2); color: #F44336; "
      "border: 1px solid rgba(244, 67, 54, 0.3); border-radius: 4px; padding: "
      "6px 12px; font-size: 11px; } "
      "QPushButton:hover { background-color: rgba(244, 67, 54, 0.3); }");
  connect(m_adjustSubBtn, &QPushButton::clicked, this,
          &AllTablesWidget::onAdjustSelected);

  adjustRow->addWidget(m_adjustValue, 1);
  adjustRow->addWidget(m_adjustAddBtn);
  adjustRow->addWidget(m_adjustSubBtn);
  layout->addLayout(adjustRow);

  // --- Quick Actions ---
  QLabel *actionsLabel = new QLabel("Quick Actions:", this);
  actionsLabel->setStyleSheet("color: #AAA; font-size: 11px; background: "
                              "transparent; margin-top: 10px;");
  layout->addWidget(actionsLabel);

  m_copyBtn = new QPushButton("📋 Copy Table", this);
  m_copyBtn->setStyleSheet(
      "QPushButton { background-color: #3B3B3B; color: white; border: none; "
      "border-radius: 4px; padding: 8px 12px; font-size: 11px; text-align: "
      "left; } "
      "QPushButton:hover { background-color: #444; }");
  connect(m_copyBtn, &QPushButton::clicked, this,
          &AllTablesWidget::onCopyTable);

  m_smoothBtn = new QPushButton("📊 Smooth Values", this);
  m_smoothBtn->setStyleSheet(
      "QPushButton { background-color: #3B3B3B; color: white; border: none; "
      "border-radius: 4px; padding: 8px 12px; font-size: 11px; text-align: "
      "left; } "
      "QPushButton:hover { background-color: #444; }");
  connect(m_smoothBtn, &QPushButton::clicked, this,
          &AllTablesWidget::onSmoothValues);

  layout->addWidget(m_copyBtn);
  layout->addWidget(m_smoothBtn);

  // Interpolate Toggle
  QHBoxLayout *interpRow = new QHBoxLayout();
  QLabel *interpLabel = new QLabel("Interpolate", this);
  interpLabel->setStyleSheet(
      "color: white; font-size: 11px; background: transparent;");

  m_interpolateBtn = new QPushButton(this);
  m_interpolateBtn->setCheckable(true);
  m_interpolateBtn->setFixedSize(44, 24);
  m_interpolateBtn->setStyleSheet(
      "QPushButton { background-color: #3B3B3B; border-radius: 12px; } "
      "QPushButton:checked { background-color: #00BCD4; }");
  connect(m_interpolateBtn, &QPushButton::clicked, this,
          &AllTablesWidget::onInterpolate);

  interpRow->addWidget(interpLabel);
  interpRow->addStretch();
  interpRow->addWidget(m_interpolateBtn);
  layout->addLayout(interpRow);

  layout->addStretch();

  return panel;
}

void AllTablesWidget::setup3DGraph() {
  m_graphContainer = new TunerMap3DWidget(this);
}

void AllTablesWidget::populateTable(const QString &tableName) {
  Logger::info("AllTablesWidget::populateTable called for: " + tableName);
  m_currentTableName = tableName;
  m_tableInfoLabel->setText(tableName);

  // Update description based on table
  if (tableName.contains("VE")) {
    m_tableDescLabel->setText(
        "Primary fuel calibration table - defines engine breathing efficiency");
  } else if (tableName.contains("Ignition")) {
    m_tableDescLabel->setText("Ignition timing advance table - degrees BTDC");
  } else if (tableName.contains("AFR")) {
    m_tableDescLabel->setText("Target Air-Fuel Ratio table for fuel control");
  } else {
    m_tableDescLabel->setText("Tuning calibration table");
  }

  m_lowCamEditor->setTableName(m_currentTableName);
  m_highCamEditor->setTableName(m_currentTableName);
  m_tableEditor->setTableName(m_currentTableName);

  // Clear existing display while waiting for real data
  QVector<QVector<double>> emptyData;
  QStringList emptyHeaders;
  m_lowCamEditor->setTableData(emptyData, emptyHeaders, emptyHeaders);
  m_highCamEditor->setTableData(emptyData, emptyHeaders, emptyHeaders);
  m_tableEditor->setTableData(emptyData, emptyHeaders, emptyHeaders);
  
  if (m_settingsManager) {
      Logger::info("AllTablesWidget: Requesting real table data from ECU for " + tableName);
      m_settingsManager->readTable(tableName);
  } else {
      Logger::warning("AllTablesWidget: Settings Manager is null, unable to read table");
  }
}




void AllTablesWidget::onTableDataReceived(const QString &tableName, const QVector<QVector<double>> &data) {
    if (tableName != m_currentTableName) return;
    
    QStringList xHeaders, yHeaders;
    int rows = data.size();
    int cols = rows > 0 ? data[0].size() : 0;
    
    // FIX-011: Use typical Speeduino default axis bins for 16x16 tables
    if (rows == 16 && cols == 16) {
        xHeaders = {"500","700","1000","1500","2000","2500","3000","3500",
                     "4000","4500","5000","5500","6000","6500","7000","8000"};
        yHeaders = {"20","26","32","38","44","50","58","66",
                     "74","82","90","98","106","114","130","160"};
    } else {
        for (int i = 0; i < cols; ++i) xHeaders << QString::number(i);
        for (int i = 0; i < rows; ++i) yHeaders << QString::number(i);
    }
    
    // Update editors
    if (m_vtecEnabled) {
        m_lowCamEditor->setTableData(data, xHeaders, yHeaders);
        m_highCamEditor->setTableData(data, xHeaders, yHeaders);
    } else {
        m_tableEditor->setTableData(data, xHeaders, yHeaders);
    }
    
    m_tableInfoLabel->setText(QString("%1 (%2x%3)").arg(tableName).arg(cols).arg(rows));
    
    double minV = 0, maxV = 100;
    if (m_vtecEnabled) {
        m_lowCamEditor->getRange(minV, maxV);
    } else {
        m_tableEditor->getRange(minV, maxV);
    }
    m_colorLegendBar->setRange(minV, maxV);
    
    update3DGraph();
    Logger::info("AllTablesWidget: Updated table " + tableName);
}

void AllTablesWidget::onTableDataChanged() { 
    double minV = 0, maxV = 100;
    if (m_vtecEnabled) {
        m_lowCamEditor->getRange(minV, maxV);
    } else {
        m_tableEditor->getRange(minV, maxV);
    }
    m_colorLegendBar->setRange(minV, maxV);

    update3DGraph();

    // FIX-005: Write changed cells back to ECU when live tuning is active
    if (m_settingsManager && m_liveTuningActive && !m_currentTableName.isEmpty()) {
        Logger::info("Live tuning: table " + m_currentTableName + " modified, saving to ECUSettingsManager");
        TableEditor *activeEditor = m_vtecEnabled ? 
            (m_camProfileTabs->currentIndex() == 0 ? m_lowCamEditor : m_highCamEditor) : 
            m_tableEditor;
            
        m_settingsManager->writeTable(m_currentTableName, activeEditor->getTableData());
    }
}

void AllTablesWidget::update3DGraph() {
  if (!m_graphContainer)
    return;

  TableEditor *activeEditor =
      m_vtecEnabled ? (m_camProfileTabs->currentIndex() == 0 ? m_lowCamEditor
                                                             : m_highCamEditor)
                    : m_tableEditor;

  QVector<QVector<double>> data = activeEditor->getTableData();
  if (data.isEmpty())
    return;

  double minVal, maxVal;
  activeEditor->getRange(minVal, maxVal);
  m_graphContainer->setData(data, minVal, maxVal);

  auto selected = activeEditor->selectedItems();
  if (!selected.isEmpty()) {
      QTableWidgetItem* item = selected.first();
      m_graphContainer->setSelectedIndex(item->row(), item->column());
  } else {
      m_graphContainer->setSelectedIndex(-1, -1);
  }
}

void AllTablesWidget::onTableSelectionChanged(int index) {
  populateTable(m_tableSelector->itemText(index));
}

void AllTablesWidget::onSearchTextChanged(const QString &text) {
  // Filter dropdown items based on search
  Q_UNUSED(text);
}

void AllTablesWidget::onCopyTable() {
  TableEditor *activeEditor =
      m_vtecEnabled ? (m_camProfileTabs->currentIndex() == 0 ? m_lowCamEditor
                                                             : m_highCamEditor)
                    : m_tableEditor;
  activeEditor->copyToClipboard();
}

void AllTablesWidget::onSmoothValues() {
  TableEditor *activeEditor =
      m_vtecEnabled ? (m_camProfileTabs->currentIndex() == 0 ? m_lowCamEditor
                                                             : m_highCamEditor)
                    : m_tableEditor;
  activeEditor->smoothSelected();
  
  if (m_liveTuningActive) {
      if (m_settingsManager) {
          m_settingsManager->writeTable(m_currentTableName, activeEditor->getTableData());
      }
      Logger::info("Live tuning: Smooth values applied");
  }
}

void AllTablesWidget::onInterpolate() {
  TableEditor *activeEditor =
      m_vtecEnabled ? (m_camProfileTabs->currentIndex() == 0 ? m_lowCamEditor
                                                             : m_highCamEditor)
                    : m_tableEditor;
  activeEditor->interpolateSelected();
  
  if (m_liveTuningActive) {
      if (m_settingsManager) {
          m_settingsManager->writeTable(m_currentTableName, activeEditor->getTableData());
      }
      Logger::info("Live tuning: Interpolate applied");
  }
}

void AllTablesWidget::onReset() { 
  if (m_settingsManager) {
      // Re-read from ECU if page map was available, for now reload local cache
      Logger::info("Table reset — reloading display for " + m_currentTableName);
  }
  populateTable(m_currentTableName); 
}

void AllTablesWidget::onAdjustSelected() {
  QPushButton *btn = qobject_cast<QPushButton *>(sender());
  double val = m_adjustValue->value();
  if (btn == m_adjustSubBtn)
    val = -val;

  TableEditor *activeEditor =
      m_vtecEnabled ? (m_camProfileTabs->currentIndex() == 0 ? m_lowCamEditor
                                                             : m_highCamEditor)
                    : m_tableEditor;
  activeEditor->adjustSelected(val);
}

void AllTablesWidget::onCamProfileChanged(int index) {
  Q_UNUSED(index);
  update3DGraph();
}

void AllTablesWidget::onGridToggle() {
  m_gridMode = !m_gridMode;
  m_gridBtn->setStyleSheet(
      m_gridMode
          ? "QPushButton { background-color: #00BCD4; color: white; border: "
            "none; "
            "border-radius: 4px; padding: 6px 12px; font-size: 11px; "
            "font-weight: bold; } "
            "QPushButton:hover { background-color: #00ACC1; }"
          : "QPushButton { background-color: #3B3B3B; color: #888; border: "
            "none; "
            "border-radius: 4px; padding: 6px 12px; font-size: 11px; } "
            "QPushButton:hover { background-color: #444; color: white; }");
}

void AllTablesWidget::onLightToggle() {
  m_lightMode = !m_lightMode;
  m_lightBtn->setStyleSheet(
      m_lightMode
          ? "QPushButton { background-color: #FFC107; color: black; border: "
            "none; "
            "border-radius: 4px; padding: 6px 12px; font-size: 11px; "
            "font-weight: bold; } "
            "QPushButton:hover { background-color: #FFD54F; }"
          : "QPushButton { background-color: #3B3B3B; color: #888; border: "
            "none; "
            "border-radius: 4px; padding: 6px 12px; font-size: 11px; } "
            "QPushButton:hover { background-color: #444; color: white; }");
}

void AllTablesWidget::setVtecEnabled(bool enabled) {
  m_vtecEnabled = enabled;
  updateVtecVisibility();
  populateTable(m_currentTableName);
}

void AllTablesWidget::setLiveTuningEnabled(bool enabled) {
  m_liveTuningActive = enabled;
  m_liveTuningLabel->setVisible(enabled);
  if (enabled) {
    m_liveTuningLabel->setStyleSheet(
        "background-color: rgba(76, 175, 80, 0.2); "
        "color: #4CAF50; "
        "font-weight: bold; "
        "padding: 8px 16px; "
        "border-radius: 4px; "
        "font-size: 12px;");
  }
}

void AllTablesWidget::updateVtecVisibility() {
  m_vtecWarningBanner->setVisible(m_vtecEnabled);
  m_camProfileTabs->setVisible(m_vtecEnabled);
  m_tableEditor->setVisible(!m_vtecEnabled);

  // Update subtitle
  if (m_vtecEnabled) {
    m_subtitleLabel->setText("Comprehensive table editor with conditional "
                             "visibility • 20 tables active • VTEC Mode");
  } else {
    m_subtitleLabel->setText("Comprehensive table editor with conditional "
                             "visibility • 20 tables active");
  }
}

void AllTablesWidget::setSerialManager(SerialManager *serial) {
    Logger::info("AllTablesWidget::setSerialManager called");
    if (m_settingsManager) {
        m_settingsManager->setSerialManager(serial);
    }
    // Initialize default table once we have serial manager
    populateTable("VE Table (Volumetric Efficiency)");
}
