/**
 * @file SettingsDropdown.cpp
 * @brief Implementation of Settings Dropdown
 */

#include "SettingsDropdown.h"
#include <QApplication>
#include <QDebug>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QScreen>
#include <QTableWidget>

SettingsDropdown::SettingsDropdown(QWidget *parent) : QDialog(parent) {
  setupUi();

  // Set up window flags for popup behavior
  setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
  setAttribute(
      Qt::WA_DeleteOnClose,
      false); // Keep instance alive or manage externally? Better to just hide
  // For this implementation, we might want to keep it alive if the user wants
  // to toggle it. Actually standard QDialog behavior is hide() on close. A
  // 'Popup' usually closes on outside click.
}

SettingsDropdown::~SettingsDropdown() {}

void SettingsDropdown::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);

  // Position it relative to the parent (which should be the button)
  if (parentWidget()) {
    QPoint pos =
        parentWidget()->mapToGlobal(QPoint(0, parentWidget()->height()));
    // Adjust x to align right edge if needed, or simply place it.
    // The user said "top right side of the app".
    // If parent is the button, we align to it.
    // Let's assume the caller moves it, or we move it here if we assume parent
    // is the button. Ideally the caller (MainWindow) sets the position before
    // calling show() or exec(). But let's try to be smart if possible.

    // Correct approach: The MainWindow will handle specific positioning using
    // move().
  }
}

bool SettingsDropdown::eventFilter(QObject *obj, QEvent *event) {
  return QDialog::eventFilter(obj, event);
}

void SettingsDropdown::setupUi() {
  resize(900, 700); // Large enough for the content

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // Header styling
  setStyleSheet(R"(
        SettingsDropdown {
            background-color: #1f1f1f;
            border: 1px solid #444;
            border-radius: 8px;
        }
        QTabWidget::pane {
            border: none;
            border-top: 1px solid #333;
            background-color: #1f1f1f;
        }
        QTabBar::tab {
            background-color: transparent;
            color: #888;
            padding: 12px 20px;
            font-weight: bold;
            border-bottom: 2px solid transparent;
        }
        QTabBar::tab:selected {
            color: #fff;
            border-bottom: 2px solid #00BCD4;
        }
        QTabBar::tab:hover {
            color: #ddd;
        }
        QLabel { color: #fff; }
    )");

  // Title Bar
  QWidget *titleBar = new QWidget(this);
  titleBar->setFixedHeight(50);
  QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
  titleLayout->setContentsMargins(20, 0, 20, 0);

  QLabel *icon = new QLabel("⚙", this); // Unicode gear
  icon->setStyleSheet("color: #00BCD4; font-size: 18px;");

  QLabel *title = new QLabel("Engine Configuration & Technical Database", this);
  title->setStyleSheet("font-size: 14px; font-weight: bold;");

  QPushButton *closeBtn = new QPushButton("✕", this);
  closeBtn->setFlat(true);
  closeBtn->setStyleSheet("color: #888; font-size: 16px; border: none;");
  connect(closeBtn, &QPushButton::clicked, this, &SettingsDropdown::hide);

  titleLayout->addWidget(icon);
  titleLayout->addSpacing(10);
  titleLayout->addWidget(title);
  titleLayout->addStretch();
  titleLayout->addWidget(closeBtn);

  mainLayout->addWidget(titleBar);

  // Tabs
  m_tabWidget = new QTabWidget(this);
  m_tabWidget->addTab(createEngineConfigTab(), "Engine Configuration");
  m_tabWidget->addTab(createFuelInjectorsTab(), "Fuel & Injectors");
  m_tabWidget->addTab(createTechDatabaseTab(), "Technical Database");
  m_tabWidget->addTab(createControlStrategiesTab(), "Control Strategies");

  mainLayout->addWidget(m_tabWidget);
}

QLabel *SettingsDropdown::createBadge(const QString &text,
                                      const QString &color) {
  QLabel *badge = new QLabel(text, this);

  QString bgColor, textColor;
  if (color == "green") {
    bgColor = "rgba(76, 175, 80, 0.2)";
    textColor = "#4CAF50";
  } else if (color == "blue") {
    bgColor = "rgba(33, 150, 243, 0.2)";
    textColor = "#2196F3";
  } else if (color == "magenta") {
    bgColor = "rgba(233, 30, 99, 0.2)";
    textColor = "#E91E63";
  }

  badge->setStyleSheet(
      QString("background-color: %1; color: %2; font-weight: bold; "
              "padding: 4px 8px; border-radius: 4px; font-size: 10px;")
          .arg(bgColor, textColor));

  return badge;
}

QFrame *SettingsDropdown::createSectionHeader(const QString &icon,
                                              const QString &title,
                                              QLabel *badge) {
  QFrame *frame = new QFrame(this);
  QHBoxLayout *layout = new QHBoxLayout(frame);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(10);

  QLabel *iconLbl = new QLabel(icon, this);
  iconLbl->setStyleSheet("color: #00BCD4; font-size: 14px;");

  QLabel *titleLbl = new QLabel(title, this);
  titleLbl->setStyleSheet("font-weight: bold; font-size: 13px;");

  layout->addWidget(iconLbl);
  layout->addWidget(titleLbl);
  if (badge) {
    layout->addSpacing(10);
    layout->addWidget(badge);
  }
  layout->addStretch();

  return frame;
}

QWidget *SettingsDropdown::createEngineConfigTab() {
  QScrollArea *scroll = new QScrollArea(this);
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);

  QWidget *content = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(content);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(20);

  // 1. Engine Type
  QGroupBox *engineBox = new QGroupBox(this);
  engineBox->setStyleSheet("QGroupBox { background-color: #262626; border: 1px "
                           "solid #333; border-radius: 6px; }");
  QVBoxLayout *engineLayout = new QVBoxLayout(engineBox);

  engineLayout->addWidget(createSectionHeader(
      "⚙", "Engine Type", createBadge("Boost Control Active", "green")));

  m_engineTypeCombo = new QComboBox(this);
  m_engineTypeCombo->addItems(
      {"Naturally Aspirated", "Turbocharged", "Supercharged"});
  m_engineTypeCombo->setCurrentText("Turbocharged");
  m_engineTypeCombo->setStyleSheet(
      "QComboBox { padding: 8px; background-color: #1f1f1f; color: white; border: 1px solid #444; border-radius: 4px; }"
      "QComboBox QAbstractItemView { background-color: #1f1f1f; color: white; selection-background-color: #00BCD4; selection-color: black; border: 1px solid #444; }");
  connect(m_engineTypeCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &SettingsDropdown::onEngineTypeChanged);
  engineLayout->addWidget(m_engineTypeCombo);

  QLabel *engineDesc =
      new QLabel("✓ Boost Control enabled + Additional tables required", this);
  engineDesc->setStyleSheet("color: #888; font-size: 11px; margin-top: 5px;");
  engineLayout->addWidget(engineDesc);

  layout->addWidget(engineBox);

  // 2. Forced Induction
  QGroupBox *turboBox = new QGroupBox(this);
  turboBox->setStyleSheet("QGroupBox { background-color: #262626; border: 1px "
                          "solid #333; border-radius: 6px; }");
  QVBoxLayout *turboLayout = new QVBoxLayout(turboBox);

  turboLayout->addWidget(
      createSectionHeader("🚀", "Forced Induction Configuration",
                          createBadge("Advanced Setup", "blue")));

  QComboBox *turboCombo = new QComboBox(this);
  turboCombo->addItems({"Single Turbo", "Twin Turbo", "Compound Turbo"});
  turboCombo->setStyleSheet(
      "QComboBox { padding: 8px; background-color: #1f1f1f; color: white; border: 1px solid #444; border-radius: 4px; }"
      "QComboBox QAbstractItemView { background-color: #1f1f1f; color: white; selection-background-color: #00BCD4; selection-color: black; border: 1px solid #444; }");
  turboLayout->addWidget(turboCombo);

  QFrame *turboInfo = new QFrame(this);
  turboInfo->setStyleSheet("border: 1px solid #00BCD4; border-radius: 4px; "
                           "background-color: rgba(0, 188, 212, 0.05);");
  QVBoxLayout *infoLayout = new QVBoxLayout(turboInfo);
  QLabel *infoText =
      new QLabel("✓ One big fan pushing air—like a single strong push", this);
  infoText->setStyleSheet(
      "color: #00BCD4; font-weight: bold; font-size: 11px;");
  QLabel *infoSub = new QLabel("Visit the Boost tab to configure detailed "
                               "boost control parameters and tables.",
                               this);
  infoSub->setStyleSheet("color: #888; font-size: 11px;");
  infoLayout->addWidget(infoText);
  infoLayout->addWidget(infoSub);
  turboLayout->addWidget(turboInfo);

  layout->addWidget(turboBox);

  // 3. Valve Train
  QGroupBox *valveBox = new QGroupBox(this);
  valveBox->setStyleSheet("QGroupBox { background-color: #262626; border: 1px "
                          "solid #333; border-radius: 6px; }");
  QVBoxLayout *valveLayout = new QVBoxLayout(valveBox);

  valveLayout->addWidget(
      createSectionHeader("📈", "Valve Train System",
                          createBadge("Dual Maps Required", "magenta")));

  m_valveTrainCombo = new QComboBox(this);
  m_valveTrainCombo->addItems(
      {"Standard", "VTEC (Dual Profile) - Honda", "VVT - Continuous"});
  m_valveTrainCombo->setCurrentText("VTEC (Dual Profile) - Honda");
  m_valveTrainCombo->setStyleSheet(
      "QComboBox { padding: 8px; background-color: #1f1f1f; color: white; border: 1px solid #444; border-radius: 4px; }"
      "QComboBox QAbstractItemView { background-color: #1f1f1f; color: white; selection-background-color: #00BCD4; selection-color: black; border: 1px solid #444; }");
  connect(m_valveTrainCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &SettingsDropdown::onValveTrainChanged);
  valveLayout->addWidget(m_valveTrainCombo);

  QFrame *vtecWarn = new QFrame(this);
  vtecWarn->setStyleSheet("border: 1px solid #E91E63; border-radius: 4px; "
                          "background-color: rgba(233, 30, 99, 0.05);");
  QVBoxLayout *warnLayout = new QVBoxLayout(vtecWarn);
  QLabel *warnTitle = new QLabel("⚠ VTEC Configuration Active", this);
  warnTitle->setStyleSheet(
      "color: #E91E63; font-weight: bold; font-size: 11px;");
  QLabel *warnText = new QLabel(
      "You must configure separate tables for Low-Speed and High-Speed cam "
      "profiles. All fuel and ignition tables will be duplicated.",
      this);
  warnText->setStyleSheet("color: #ddd; font-size: 11px;");
  warnText->setWordWrap(true);
  warnLayout->addWidget(warnTitle);
  warnLayout->addWidget(warnText);
  valveLayout->addWidget(vtecWarn);

  layout->addWidget(valveBox);

  // 4. Fuel System
  QGroupBox *fuelBox = new QGroupBox(this);
  fuelBox->setStyleSheet("QGroupBox { background-color: #262626; border: 1px "
                         "solid #333; border-radius: 6px; }");
  QVBoxLayout *fuelLayout = new QVBoxLayout(fuelBox);

  fuelLayout->addWidget(createSectionHeader("⛽", "Fuel System"));

  QComboBox *fuelCombo = new QComboBox(this);
  fuelCombo->addItems({"Port Injection (Single Stage)", "Direct Injection",
                       "Staged Injection"});
  fuelCombo->setStyleSheet(
      "QComboBox { padding: 8px; background-color: #1f1f1f; color: white; border: 1px solid #444; border-radius: 4px; }"
      "QComboBox QAbstractItemView { background-color: #1f1f1f; color: white; selection-background-color: #00BCD4; selection-color: black; border: 1px solid #444; }");
  fuelLayout->addWidget(fuelCombo);

  layout->addWidget(fuelBox);

  // 5. Fuel Control Mode
  QGroupBox *modeBox = new QGroupBox(this);
  modeBox->setStyleSheet("QGroupBox { background-color: #262626; border: 1px "
                         "solid #333; border-radius: 6px; }");
  QVBoxLayout *modeLayout = new QVBoxLayout(modeBox);

  modeLayout->addWidget(createSectionHeader("🎛", "Fuel Control Mode"));

  QComboBox *modeCombo = new QComboBox(this);
  modeCombo->addItems({"Open Loop", "Closed Loop (O2 Feedback)"});
  modeCombo->setCurrentText("Closed Loop (O2 Feedback)");
  modeCombo->setStyleSheet(
      "QComboBox { padding: 8px; background-color: #1f1f1f; color: white; border: 1px solid #444; border-radius: 4px; }"
      "QComboBox QAbstractItemView { background-color: #1f1f1f; color: white; selection-background-color: #00BCD4; selection-color: black; border: 1px solid #444; }");
  modeLayout->addWidget(modeCombo);

  QLabel *modeDesc =
      new QLabel("✓ Automatic AFR correction using wideband O2 sensor", this);
  modeDesc->setStyleSheet("color: #888; font-size: 11px; margin-top: 5px;");
  modeLayout->addWidget(modeDesc);

  layout->addWidget(modeBox);
  layout->addStretch();

  scroll->setWidget(content);
  return scroll;
}

QWidget *SettingsDropdown::createFuelInjectorsTab() {
  QScrollArea *scroll = new QScrollArea(this);
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);

  QWidget *content = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(content);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(20);

  // Primary Injector Configuration
  QGroupBox *configBox = new QGroupBox(this);
  configBox->setStyleSheet("QGroupBox { background-color: #262626; border: 1px "
                           "solid #333; border-radius: 6px; }");
  QVBoxLayout *configLayout = new QVBoxLayout(configBox);

  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *headerTitle = new QLabel("Primary Injector Configuration", this);
  headerTitle->setStyleSheet(
      "color: #00BCD4; font-weight: bold; font-size: 13px;");
  headerLayout->addWidget(headerTitle);
  headerLayout->addStretch();
  QPushButton *showCalcBtn = new QPushButton("Show Calculator", this);
  showCalcBtn->setStyleSheet(
      "background-color: #333; border: 1px solid #444; border-radius: 4px; "
      "padding: 4px 8px; color: #ccc; font-size: 11px;");
  headerLayout->addWidget(showCalcBtn);
  configLayout->addLayout(headerLayout);

  QLabel *subTitle = new QLabel("● Primary Injectors", this);
  subTitle->setStyleSheet("color: #fff; font-weight: bold; margin-top: 10px;");
  configLayout->addWidget(subTitle);

  // Manufacturer and Model Dropdowns
  configLayout->addWidget(new QLabel("Manufacturer", this));
  QComboBox *manCombo = new QComboBox(this);
  manCombo->addItem("Denso");
  manCombo->setStyleSheet(
      "QComboBox { padding: 8px; background-color: #1f1f1f; color: white; border: 1px solid #444; border-radius: 4px; }"
      "QComboBox QAbstractItemView { background-color: #1f1f1f; color: white; selection-background-color: #00BCD4; selection-color: black; border: 1px solid #444; }");
  configLayout->addWidget(manCombo);

  configLayout->addWidget(new QLabel("Injector Model", this));
  QComboBox *modelCombo = new QComboBox(this);
  modelCombo->addItem("Denso 23250-66030 - 310cc (29.5lb/hr)");
  modelCombo->setStyleSheet(
      "QComboBox { padding: 8px; background-color: #1f1f1f; color: white; border: 1px solid #00BCD4; border-radius: 4px; }"
      "QComboBox QAbstractItemView { background-color: #1f1f1f; color: white; selection-background-color: #00BCD4; selection-color: black; border: 1px solid #444; }");
  configLayout->addWidget(modelCombo);

  // Info Panel
  QFrame *infoPanel = new QFrame(this);
  infoPanel->setStyleSheet(
      "background-color: #1f1f1f; border: 1px solid #333; border-radius: 4px; "
      "padding: 10px; margin-top: 5px;");
  QGridLayout *infoGrid = new QGridLayout(infoPanel);

  infoGrid->addWidget(new QLabel("Denso 23250-66030", this), 0, 0);
  QLabel *carModel = new QLabel("Toyota Supra 2JZ-GTE", this);
  carModel->setStyleSheet("color: #888;");
  infoGrid->addWidget(carModel, 1, 0);

  QLabel *streetBadge = new QLabel("STREET", this);
  streetBadge->setStyleSheet(
      "background-color: #4CAF50; color: #fff; border-radius: 4px; padding: "
      "2px 6px; font-weight: bold; font-size: 10px;");
  infoGrid->addWidget(streetBadge, 0, 1, 1, 1, Qt::AlignRight);

  // Grid of stats
  auto addStat = [infoGrid](const QString &label, const QString &val, int row,
                            int col) {
    QLabel *l = new QLabel(label, nullptr);
    l->setStyleSheet("color: #888; font-size: 10px;");
    QLabel *v = new QLabel(val, nullptr);
    v->setStyleSheet("color: #fff; font-weight: bold; font-size: 11px;");
    infoGrid->addWidget(l, row, col);
    infoGrid->addWidget(v, row + 1, col);
  };

  addStat("Flow Rate", "310 cc/min\n29.5 lb/hr", 2, 0);
  addStat("Impedance", "12 Ω", 2, 1);
  addStat("Dead Time @ 14V", "0.75 ms", 4, 0);
  addStat("Base Pressure", "3 bar (43.5 PSI)", 4, 1);

  configLayout->addWidget(infoPanel);

  // Actual Fuel Pressure
  configLayout->addWidget(new QLabel("Actual Fuel Pressure (bar)", this));
  QComboBox *pressureInput = new QComboBox(
      this); // Using combo to match styling, strictly it's an input
  pressureInput->addItem("3");
  pressureInput->setEditable(true);
  pressureInput->setStyleSheet(
      "QComboBox { padding: 8px; background-color: #1f1f1f; color: white; border: 1px solid #444; border-radius: 4px; }"
      "QComboBox QAbstractItemView { background-color: #1f1f1f; color: white; selection-background-color: #00BCD4; selection-color: black; border: 1px solid #444; }");
  configLayout->addWidget(pressureInput);

  // Estimated HP
  QFrame *hpFrame = new QFrame(this);
  hpFrame->setStyleSheet(
      "background-color: #1f1f1f; border: 1px solid #333; border-radius: 4px; "
      "padding: 10px; margin-top: 10px;");
  QVBoxLayout *hpLayout = new QVBoxLayout(hpFrame);
  QLabel *hpTitle = new QLabel("Estimated Max HP @ 85% Duty:", this);
  hpTitle->setStyleSheet("color: #888; font-size: 11px;");
  QLabel *hpVal = new QLabel("335 HP", this);
  hpVal->setStyleSheet("color: #00BCD4; font-weight: bold; font-size: 18px;");
  QLabel *hpDetail = new QLabel("4 Injectors x 310 cc/min x 85% duty", this);
  hpDetail->setStyleSheet("color: #666; font-size: 10px;");

  hpLayout->addWidget(hpTitle);
  hpLayout->addWidget(hpVal);
  hpLayout->addWidget(hpDetail);
  configLayout->addWidget(hpFrame);

  layout->addWidget(configBox);

  // Dead Time Table
  QGroupBox *deadTimeBox = new QGroupBox(this);
  deadTimeBox->setStyleSheet("QGroupBox { background-color: #262626; border: "
                             "1px solid #333; border-radius: 6px; }");
  QVBoxLayout *dtLayout = new QVBoxLayout(deadTimeBox);

  QHBoxLayout *dtHeader = new QHBoxLayout();
  QLabel *dtTitle = new QLabel("Primary Injector Dead Time Calibration", this);
  dtTitle->setStyleSheet("color: #00BCD4; font-weight: bold; font-size: 13px;");
  dtHeader->addWidget(dtTitle);
  dtHeader->addStretch();

  QPushButton *loadDensoBtn = new QPushButton("📥 Load from Denso", this);
  loadDensoBtn->setStyleSheet(
      "background-color: #2196F3; color: white; border: none; border-radius: "
      "4px; padding: 4px 8px; font-size: 11px;");
  QPushButton *importCsvBtn = new QPushButton("Import CSV", this);
  importCsvBtn->setStyleSheet(
      "background-color: #333; color: white; border: 1px solid #444; "
      "border-radius: 4px; padding: 4px 8px; font-size: 11px;");
  dtHeader->addWidget(loadDensoBtn);
  dtHeader->addWidget(importCsvBtn);
  dtLayout->addLayout(dtHeader);

  QLabel *dtDesc = new QLabel(
      "Dead Time: Time delay between ECU command and actual fuel spray.", this);
  dtDesc->setStyleSheet("color: #fff; font-size: 11px; margin-top: 5px;");
  dtLayout->addWidget(dtDesc);

  // Legend
  QHBoxLayout *legendLayout = new QHBoxLayout();
  auto addLegend = [legendLayout](const QString &color, const QString &text) {
    QLabel *lbl = new QLabel("■ " + text, nullptr);
    lbl->setStyleSheet("color: " + color + "; font-size: 10px;");
    legendLayout->addWidget(lbl);
  };
  addLegend("#4CAF50", "<0.5ms Excellent");
  addLegend("#FFC107", "0.8-1.0ms Moderate");
  addLegend("#F44336", ">1.5ms High");
  legendLayout->addStretch();
  dtLayout->addLayout(legendLayout);

  // Table
  QTableWidget *table = new QTableWidget(4, 9, this);
  table->setFixedHeight(160);
  table->horizontalHeader()->setVisible(false);
  table->verticalHeader()->setVisible(false);

  QStringList volts = {"7.0 V", "9.0 V", "11.0 V", "13.5 V", "14.5 V"};
  // Header Row implementation manually to match design style
  QStringList msHeaders = {"Voltage (V)", "0.5 ms", "1.0 ms",
                           "1.5 ms",      "2.0 ms", "2.5 ms",
                           "3.0 ms",      "3.5 ms", "4.0 ms"};

  // Actually standard header might be easier, but let's try to populate first
  // row
  table->setColumnCount(9);
  table->setRowCount(6); // 1 header + 5 data rows

  // Fill mockup data
  for (int col = 0; col < 9; col++) {
    QTableWidgetItem *item = new QTableWidgetItem(msHeaders[col]);
    item->setBackground(QColor("#2196F3")); // Blue header
    item->setForeground(Qt::white);
    item->setTextAlignment(Qt::AlignCenter);
    table->setItem(0, col, item);
  }

  struct RowData {
    QString v;
    QStringList vals;
  };
  QList<RowData> rows = {
      {"7.0 V",
       {"1.20", "1.15", "1.10", "1.05", "1.00", "0.95", "0.90", "0.85"}},
      {"9.0 V",
       {"1.05", "1.00", "0.95", "0.90", "0.85", "0.80", "0.75", "0.70"}},
      {"11.0 V",
       {"0.90", "0.85", "0.80", "0.75", "0.70", "0.65", "0.60", "0.55"}},
      {"13.5 V",
       {"0.75", "0.70", "0.65", "0.60", "0.55", "0.50", "0.45", "0.40"}},
      {"14.5 V",
       {"0.70", "0.65", "0.60", "0.55", "0.50", "0.45", "0.40", "0.35"}}};

  for (int i = 0; i < rows.size(); i++) {
    int rowIdx = i + 1;
    QTableWidgetItem *head = new QTableWidgetItem(rows[i].v);
    head->setBackground(QColor("#333"));
    head->setTextAlignment(Qt::AlignCenter);
    table->setItem(rowIdx, 0, head);

    for (int j = 0; j < rows[i].vals.size(); j++) {
      QTableWidgetItem *cell = new QTableWidgetItem(rows[i].vals[j]);
      cell->setTextAlignment(Qt::AlignCenter);
      // Color based on value
      double v = rows[i].vals[j].toDouble();
      if (v < 0.6)
        cell->setBackground(QColor("#4CAF50")); // Green
      else if (v < 1.0)
        cell->setBackground(QColor("#CDDC39")); // Lime/Yellow
      else
        cell->setBackground(QColor("#FF9800")); // Orange
      cell->setForeground(Qt::black);
      table->setItem(rowIdx, j + 1, cell);
    }
  }

  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  dtLayout->addWidget(table);

  layout->addWidget(deadTimeBox);
  layout->addStretch();

  scroll->setWidget(content);
  return scroll;
}

QWidget *SettingsDropdown::createTechDatabaseTab() {
  QWidget *tab = new QWidget();
  QVBoxLayout *l = new QVBoxLayout(tab);
  l->addWidget(new QLabel("Technical Database Content Placeholder", tab));
  return tab;
}

QWidget *SettingsDropdown::createControlStrategiesTab() {
  QWidget *tab = new QWidget();
  QVBoxLayout *l = new QVBoxLayout(tab);
  l->addWidget(new QLabel("Control Strategies Content Placeholder", tab));
  return tab;
}

void SettingsDropdown::onValveTrainChanged(int index) {
  // Index 0 = Standard, Index 1 = VTEC, Index 2 = VVT
  bool vtecEnabled = (index == 1);
  emit vtecSettingChanged(vtecEnabled);
}

void SettingsDropdown::onEngineTypeChanged(int index) {
  QString type = m_engineTypeCombo->itemText(index);
  emit engineTypeChanged(type);

  // Also emit boost control signal
  bool boostEnabled = (index == 1 || index == 2); // Turbo or Supercharged
  emit boostControlChanged(boostEnabled);
}
