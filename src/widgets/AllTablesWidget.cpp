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

  // === TOOLBAR (Combined Header & Toolbar) ===
  QWidget *toolbarWidget = new QWidget(this);
  toolbarWidget->setStyleSheet("background-color: #252525; border-bottom: 1px solid #333;");
  toolbarWidget->setMinimumHeight(60);
  QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarWidget);
  toolbarLayout->setContentsMargins(20, 10, 20, 10);

  // Left side - Title
  QVBoxLayout *titleLayout = new QVBoxLayout();
  titleLayout->setSpacing(2);

  m_titleLabel = new QLabel("All Tuning Tables", this);
  m_titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: white; background: transparent;");

  m_subtitleLabel = new QLabel("Comprehensive table editor with conditional visibility", this);
  m_subtitleLabel->setStyleSheet("font-size: 10px; color: #AAA; background: transparent;");

  titleLayout->addWidget(m_titleLabel);
  titleLayout->addWidget(m_subtitleLabel);
  toolbarLayout->addLayout(titleLayout);

  toolbarLayout->addSpacing(30);

  m_tableSelector = new QComboBox(this);
  m_tableSelector->setMinimumWidth(220);
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
  m_searchBox->setMinimumWidth(180);
  m_searchBox->setStyleSheet("QLineEdit { "
                             "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #141414, stop:1 #2A2A2A); "
                             "  border: 1px solid #111; "
                             "  border-bottom: 1px solid #4A4A4A; "
                             "  border-right: 1px solid #3A3A3A; "
                             "  border-radius: 4px; "
                             "  padding: 8px 12px; "
                             "  color: #E0E0E0; "
                             "  font-size: 12px; "
                             "}"
                             "QLineEdit:focus { border: 1px solid #00BCD4; }");
  connect(m_searchBox, &QLineEdit::textChanged, this,
          &AllTablesWidget::onSearchTextChanged);

  toolbarLayout->addWidget(m_tableSelector);
  toolbarLayout->addSpacing(15);
  toolbarLayout->addWidget(m_searchBox);

  // -- Moved from Right Panel --
  toolbarLayout->addSpacing(20);

  // Quick Actions
  QString skeuoButtonBase = 
      "QPushButton { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5A5A5A, stop:0.4 #3D3D3D, stop:1 #2B2B2B); "
      "  color: #F0F0F0; "
      "  border: 1px solid #1A1A1A; "
      "  border-top: 1px solid #777; "
      "  border-left: 1px solid #555; "
      "  border-radius: 5px; "
      "  padding: 6px 14px; "
      "  font-size: 11px; "
      "  font-weight: bold; "
      "} "
      "QPushButton:hover { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6A6A6A, stop:0.4 #4D4D4D, stop:1 #3B3B3B); "
      "} "
      "QPushButton:pressed { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #222, stop:1 #3D3D3D); "
      "  border-top: 1px solid #111; "
      "  border-left: 1px solid #111; "
      "  border-bottom: 1px solid #555; "
      "  border-right: 1px solid #555; "
      "}";

  m_copyBtn = new QPushButton("📋 Copy", this);
  m_copyBtn->setStyleSheet(skeuoButtonBase);
  connect(m_copyBtn, &QPushButton::clicked, this, &AllTablesWidget::onCopyTable);

  m_smoothBtn = new QPushButton("📊 Smooth", this);
  m_smoothBtn->setStyleSheet(skeuoButtonBase);
  connect(m_smoothBtn, &QPushButton::clicked, this, &AllTablesWidget::onSmoothValues);

  toolbarLayout->addWidget(m_copyBtn);
  toolbarLayout->addWidget(m_smoothBtn);

  toolbarLayout->addSpacing(20);

  // Adjust Selected
  QLabel *adjustLabel = new QLabel("Adjust:", this);
  adjustLabel->setStyleSheet("color: #AAA; font-size: 11px; font-weight: bold; background: transparent;");

  m_adjustValue = new QDoubleSpinBox(this);
  m_adjustValue->setRange(-50, 50);
  m_adjustValue->setValue(0.5);
  m_adjustValue->setSingleStep(0.1);
  m_adjustValue->setStyleSheet(
      "QDoubleSpinBox { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #141414, stop:1 #2A2A2A); "
      "  border: 1px solid #111; "
      "  border-bottom: 1px solid #4A4A4A; "
      "  border-right: 1px solid #3A3A3A; "
      "  border-radius: 4px; "
      "  padding: 4px; "
      "  color: #E0E0E0; "
      "  font-size: 12px; "
      "  min-width: 50px; "
      "}");

  m_adjustAddBtn = new QPushButton("+ Add", this);
  m_adjustAddBtn->setStyleSheet(
      "QPushButton { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #00E5FF, stop:0.4 #00ACC1, stop:1 #00838F); "
      "  color: #FFFFFF; "
      "  border: 1px solid #004D40; "
      "  border-top: 1px solid #84FFFF; "
      "  border-left: 1px solid #18FFFF; "
      "  border-radius: 5px; "
      "  padding: 6px 12px; "
      "  font-size: 11px; "
      "  font-weight: bold; "
      "} "
      "QPushButton:hover { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #18FFFF, stop:0.4 #00BCD4, stop:1 #0097A7); "
      "} "
      "QPushButton:pressed { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #006064, stop:1 #0097A7); "
      "  border-top: 1px solid #004D40; "
      "  border-left: 1px solid #004D40; "
      "  border-bottom: 1px solid #00ACC1; "
      "  border-right: 1px solid #00ACC1; "
      "}");
  connect(m_adjustAddBtn, &QPushButton::clicked, this, &AllTablesWidget::onAdjustSelected);

  m_adjustSubBtn = new QPushButton("- Sub", this);
  m_adjustSubBtn->setStyleSheet(
      "QPushButton { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FF5252, stop:0.4 #E53935, stop:1 #C62828); "
      "  color: #FFFFFF; "
      "  border: 1px solid #880E4F; "
      "  border-top: 1px solid #FF8A80; "
      "  border-left: 1px solid #FF5252; "
      "  border-radius: 5px; "
      "  padding: 6px 12px; "
      "  font-size: 11px; "
      "  font-weight: bold; "
      "} "
      "QPushButton:hover { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FF8A80, stop:0.4 #EF5350, stop:1 #D32F2F); "
      "} "
      "QPushButton:pressed { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B71C1C, stop:1 #D32F2F); "
      "  border-top: 1px solid #880E4F; "
      "  border-left: 1px solid #880E4F; "
      "  border-bottom: 1px solid #EF5350; "
      "  border-right: 1px solid #EF5350; "
      "}");
  connect(m_adjustSubBtn, &QPushButton::clicked, this, &AllTablesWidget::onAdjustSelected);

  toolbarLayout->addWidget(adjustLabel);
  toolbarLayout->addWidget(m_adjustValue);
  toolbarLayout->addWidget(m_adjustAddBtn);
  toolbarLayout->addWidget(m_adjustSubBtn);

  toolbarLayout->addSpacing(20);

  // Interpolate Toggle
  QLabel *interpLabel = new QLabel("Interpolate:", this);
  interpLabel->setStyleSheet("color: white; font-size: 11px; font-weight: bold; background: transparent;");

  m_interpolateBtn = new QPushButton(this);
  m_interpolateBtn->setCheckable(true);
  m_interpolateBtn->setFixedSize(44, 22);
  m_interpolateBtn->setStyleSheet(
      "QPushButton { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #141414, stop:1 #2A2A2A); "
      "  border: 1px solid #111; "
      "  border-bottom: 1px solid #4A4A4A; "
      "  border-right: 1px solid #3A3A3A; "
      "  border-radius: 11px; " // Pill shaped track
      "} "
      "QPushButton:checked { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #006064, stop:1 #00BCD4); "
      "  border-bottom: 1px solid #84FFFF; "
      "}");
  connect(m_interpolateBtn, &QPushButton::clicked, this, &AllTablesWidget::onInterpolate);

  toolbarLayout->addWidget(interpLabel);
  toolbarLayout->addWidget(m_interpolateBtn);

  toolbarLayout->addStretch();

  mainLayout->addWidget(toolbarWidget);

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

  // === CONTENT AREA ===
  QWidget *contentWidget = new QWidget(this);
  contentWidget->setStyleSheet("background-color: #1E1E1E;");
  QHBoxLayout *contentLayout = new QHBoxLayout(contentWidget);
  contentLayout->setContentsMargins(20, 10, 20, 20);
  contentLayout->setSpacing(20);

  // Left side: QTabWidget — Table tab gets full height, 3D Surface tab gets full height
  m_mainViewTabs = new QTabWidget(this);
  m_mainViewTabs->setDocumentMode(false);
  m_mainViewTabs->setStyleSheet(
      "QTabWidget::pane { "
      "  border: none; "
      "  background-color: #252525; "
      "  border-radius: 0 8px 8px 8px; "
      "} "
      "QTabBar { "
      "  background: #1E1E1E; "
      "} "
      "QTabBar::tab { "
      "  background-color: #1E1E1E; "
      "  color: #888; "
      "  padding: 9px 22px; "
      "  border: 1px solid #333; "
      "  border-bottom: none; "
      "  border-top-left-radius: 6px; "
      "  border-top-right-radius: 6px; "
      "  font-weight: bold; "
      "  font-size: 12px; "
      "  margin-right: 2px; "
      "} "
      "QTabBar::tab:selected { "
      "  color: white; "
      "  background-color: #252525; "
      "  border-color: #444; "
      "  border-bottom: 2px solid #00BCD4; "
      "} "
      "QTabBar::tab:hover:!selected { "
      "  color: #ccc; "
      "  background-color: #2B2B2B; "
      "}");

  // ── TABLE TAB ─────────────────────────────────────────────────────────────
  m_tableTab = new QWidget(this);
  m_tableTab->setStyleSheet("background-color: #252525;");
  QVBoxLayout *topLayout = new QVBoxLayout(m_tableTab);
  topLayout->setContentsMargins(15, 12, 15, 10);
  topLayout->setSpacing(8);

  // Table Info Header
  QWidget *tableHeader = new QWidget(m_tableTab);
  tableHeader->setStyleSheet("background: transparent;");
  QVBoxLayout *tableHeaderLayout = new QVBoxLayout(tableHeader);
  tableHeaderLayout->setContentsMargins(0, 0, 0, 0);
  tableHeaderLayout->setSpacing(2);

  m_tableInfoLabel = new QLabel("VE Table (Volumetric Efficiency)", m_tableTab);
  m_tableInfoLabel->setStyleSheet(
      "font-weight: bold; color: white; font-size: 14px; background: transparent;");

  m_tableDescLabel = new QLabel(
      "Primary fuel calibration table - defines engine breathing efficiency", m_tableTab);
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
      "  background-color: transparent; color: #888; "
      "  padding: 8px 18px; border: none; "
      "  border-bottom: 2px solid transparent; font-weight: bold; "
      "} "
      "QTabBar::tab:selected { color: white; border-bottom: 2px solid #00BCD4; } "
      "QTabBar::tab:hover { color: #ccc; }");

  m_lowCamEditor = new TableEditor(this);
  connect(m_lowCamEditor, &TableEditor::dataChanged, this,
          &AllTablesWidget::onTableDataChanged);

  m_highCamEditor = new TableEditor(this);
  connect(m_highCamEditor, &TableEditor::dataChanged, this,
          &AllTablesWidget::onTableDataChanged);

  m_camProfileTabs->addTab(m_lowCamEditor, "Low Cam Profile");
  m_camProfileTabs->addTab(m_highCamEditor, "High Cam Profile");
  topLayout->addWidget(m_camProfileTabs);

  // Main Table Editor (non-VTEC mode)
  m_tableEditor = new TableEditor(m_tableTab);
  connect(m_tableEditor, &TableEditor::dataChanged, this,
          &AllTablesWidget::onTableDataChanged);
  m_tableEditor->setVisible(false);
  m_tableEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  topLayout->addWidget(m_tableEditor);

  m_colorLegendBar = new ColorLegendBar(m_tableTab);
  topLayout->addWidget(m_colorLegendBar);

  // ── 3D SURFACE TAB ────────────────────────────────────────────────────────
  setup3DGraph();

  // Assemble tabs
  m_dummyTable = new QWidget(this);
  m_dummy3D = new QWidget(this);
  m_dummySplit = new QWidget(this);
  m_splitter = nullptr;

  m_mainViewTabs->addTab(m_dummyTable, "  Table");
  m_mainViewTabs->addTab(m_dummy3D, "  3D Surface");
  m_mainViewTabs->addTab(m_dummySplit, "  Split View");

  connect(m_mainViewTabs, &QTabWidget::currentChanged, this, &AllTablesWidget::onMainTabChanged);

  contentLayout->addWidget(m_mainViewTabs, 1);
  
  // Initialize view
  onMainTabChanged(0);

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
    
    // T3: Provide real-world axis bin labels for common Speeduino table sizes
    if (rows == 16 && cols == 16) {
        xHeaders = {"500","700","1000","1500","2000","2500","3000","3500",
                     "4000","4500","5000","5500","6000","6500","7000","8000"};
        yHeaders = {"20","26","32","38","44","50","58","66",
                     "74","82","90","98","106","114","130","160"};
    } else if (rows == 12 && cols == 12) {
        xHeaders = {"500","1000","1500","2000","2500","3000",
                     "3500","4000","5000","6000","7000","8000"};
        yHeaders = {"20","30","40","50","60","70",
                     "80","90","100","110","130","160"};
    } else if (rows == 8 && cols == 8) {
        xHeaders = {"500","1000","2000","3000","4000","5000","6000","7000"};
        yHeaders = {"20","40","60","80","100","120","140","160"};
    } else {
        // Fallback: generic numeric indices
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

    // Pass axis labels to 3D graph for tick marks
    m_graphContainer->setAxisLabels(xHeaders, yHeaders);
    
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
  // T5: filter the table selector combo in real-time
  // Save full list on first call
  if (m_allTableNames.isEmpty()) {
    for (int i = 0; i < m_tableSelector->count(); ++i)
      m_allTableNames << m_tableSelector->itemText(i);
  }

  m_tableSelector->blockSignals(true);
  m_tableSelector->clear();

  if (text.trimmed().isEmpty()) {
    m_tableSelector->addItems(m_allTableNames);
  } else {
    for (const QString &name : m_allTableNames) {
      if (name.contains(text, Qt::CaseInsensitive))
        m_tableSelector->addItem(name);
    }
  }

  m_tableSelector->blockSignals(false);
  if (m_tableSelector->count() > 0)
    onTableSelectionChanged(0);
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


void AllTablesWidget::setVtecEnabled(bool enabled) {
  m_vtecEnabled = enabled;
  updateVtecVisibility();
  populateTable(m_currentTableName);
}

void AllTablesWidget::setLiveTuningEnabled(bool enabled) {
  m_liveTuningActive = enabled;
  // Live tuning label was removed to save space
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

void AllTablesWidget::onMainTabChanged(int index) {
  if (index == 0) {
    // Table Only
    if (!m_dummyTable->layout()) {
      QVBoxLayout *l = new QVBoxLayout(m_dummyTable);
      l->setContentsMargins(0, 0, 0, 0);
    }
    m_dummyTable->layout()->addWidget(m_tableTab);
    m_tableTab->show();
    m_graphContainer->hide();
    m_graphContainer->setParent(this);
  } else if (index == 1) {
    // 3D Surface Only
    if (!m_dummy3D->layout()) {
      QVBoxLayout *l = new QVBoxLayout(m_dummy3D);
      l->setContentsMargins(0, 0, 0, 0);
    }
    m_dummy3D->layout()->addWidget(m_graphContainer);
    m_graphContainer->show();
    m_tableTab->hide();
    m_tableTab->setParent(this);
  } else if (index == 2) {
    // Split View
    if (!m_dummySplit->layout()) {
      QHBoxLayout *l = new QHBoxLayout(m_dummySplit);
      l->setContentsMargins(10, 10, 10, 10);
      l->setSpacing(10); // Spacing between items inside the layout (if any, but splitter handles its own)
      
      m_splitter = new QSplitter(Qt::Horizontal, m_dummySplit);
      m_splitter->setHandleWidth(8);
      m_splitter->setStyleSheet(
          "QSplitter::handle { "
          "  background-color: #3A3A3A; "
          "  margin: 15px 2px; "
          "  border-radius: 3px; "
          "} "
          "QSplitter::handle:hover { "
          "  background-color: #00BCD4; "
          "} "
          "QSplitter::handle:pressed { "
          "  background-color: #008BA3; "
          "}");
      l->addWidget(m_splitter);
    }
    m_splitter->addWidget(m_tableTab);
    m_splitter->addWidget(m_graphContainer);
    m_tableTab->show();
    m_graphContainer->show();
    m_splitter->setSizes(QList<int>() << 650 << 350); // Balanced default sizes
  }
}

