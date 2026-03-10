#include "IOConfigWidget.h"
#include "../core/ECUSettingsManager.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>

IOConfigWidget::IOConfigWidget(QWidget *parent) : TuningWidgetBase(parent) {
  setupUi();
  // Trigger initial update (Manual)
  updateConfigFromPreset(0);
}

void IOConfigWidget::setupUi() {
  m_mainLayout->setSpacing(20);
  m_mainLayout->setContentsMargins(30, 30, 30, 30);

  // Global Style
  this->setStyleSheet("background-color: #121212; color: #E0E0E0; font-family: "
                      "'Segoe UI', sans-serif;");

  // --- Title Section ---
  QWidget *headerWidget = new QWidget(this);
  QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);
  headerLayout->setContentsMargins(0, 0, 0, 0);
  headerLayout->setSpacing(5);

  QLabel *title = new QLabel("ECU Configuration", this);
  title->setStyleSheet("font-size: 24px; font-weight: bold; color: #FFFFFF;");
  headerLayout->addWidget(title);

  QLabel *subtitle = new QLabel(
      "Select your ECU to automatically configure inputs and outputs", this);
  subtitle->setStyleSheet("font-size: 14px; color: #AAAAAA;");
  headerLayout->addWidget(subtitle);

  m_mainLayout->addWidget(headerWidget);

  // --- ECU Model Selection ---
  QWidget *selectionWidget = new QWidget(this);
  QHBoxLayout *selectionLayout = new QHBoxLayout(selectionWidget);
  selectionLayout->setContentsMargins(0, 0, 0, 0);

  QLabel *modelLabel = new QLabel("ECU Model", this);
  modelLabel->setStyleSheet(
      "font-size: 14px; font-weight: bold; color: #CCCCCC;");

  m_ecuModelCombo = new QComboBox(this);
  m_ecuModelCombo->addItem("Manual Configuration (No Preset)");         // 0
  m_ecuModelCombo->addItem("Speeduino v0.4.3c");                        // 1
  m_ecuModelCombo->addItem("Speeduino Dropbear V2");                    // 2
  m_ecuModelCombo->addItem("Speeduino UAC (Under Arduino 4 Channel)");  // 3
  m_ecuModelCombo->addItem("Speeduino NO2C (No Overhang Two Channel)"); // 4
  m_ecuModelCombo->addItem("Speeduino C2C (Compact 2 Channel)");        // 5
  m_ecuModelCombo->addItem("Megasquirt MS1 (Legacy)");                  // 6
  m_ecuModelCombo->addItem("Megasquirt MS2");                           // 7
  m_ecuModelCombo->addItem("Megasquirt MS3 / MS3X");                    // 8
  m_ecuModelCombo->addItem("Custom");                                   // 9

  m_ecuModelCombo->setStyleSheet(
      "QComboBox { "
      "  background-color: #1E1E1E; border: 1px solid #333; border-radius: "
      "4px; "
      "  padding: 8px; min-width: 300px; color: white; "
      "} "
      "QComboBox::drop-down { border: none; } "
      "QComboBox QAbstractItemView { background-color: #1E1E1E; color: white; "
      "selection-background-color: #00BCD4; selection-color: black; border: 1px solid #444; }");

  selectionLayout->addWidget(modelLabel);
  selectionLayout->addSpacing(10);
  selectionLayout->addWidget(m_ecuModelCombo);
  selectionLayout->addStretch();

  m_mainLayout->addWidget(selectionWidget);

  connect(m_ecuModelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &IOConfigWidget::updateConfigFromPreset);

  // --- Scroll Area for Tables ---
  QScrollArea *scrollArea = new QScrollArea(this);
  scrollArea->setWidgetResizable(true);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setStyleSheet("background: transparent;");

  QWidget *contentWidget = new QWidget();
  QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
  contentLayout->setSpacing(30);
  contentLayout->setContentsMargins(0, 0, 0, 0);

  // --- Analog Inputs ---
  QGroupBox *analogGroup = new QGroupBox("Analog Inputs", this);
  analogGroup->setStyleSheet(
      "QGroupBox { border: 1px solid #333; border-radius: 6px; margin-top: "
      "24px; font-weight: bold; font-size: 16px; color: #DDD; } "
      "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top "
      "left; padding: 0 10px; }");
  QVBoxLayout *analogLayout = new QVBoxLayout(analogGroup);

  m_analogTable =
      createStyledTable({"Channel", "Function", "Min Voltage", "Max Voltage",
                         "Current Value", "Show on Dash"});
  analogLayout->addWidget(m_analogTable);
  contentLayout->addWidget(analogGroup);

  // --- Digital Inputs ---
  QGroupBox *digitalGroup = new QGroupBox("Digital Inputs", this);
  digitalGroup->setStyleSheet(
      "QGroupBox { border: 1px solid #333; border-radius: 6px; margin-top: "
      "24px; font-weight: bold; font-size: 16px; color: #DDD; } "
      "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top "
      "left; padding: 0 10px; }");
  QVBoxLayout *digitalLayout = new QVBoxLayout(digitalGroup);

  m_digitalTable = createStyledTable(
      {"Channel", "Function", "Pull-up/Pull-down", "State", "Show on Dash"});
  digitalLayout->addWidget(m_digitalTable);
  contentLayout->addWidget(digitalGroup);

  // --- Digital Outputs ---
  QGroupBox *digitalOutGroup =
      new QGroupBox("Digital Outputs (16 Channels for V8 Support)", this);
  digitalOutGroup->setStyleSheet(
      "QGroupBox { border: 1px solid #333; border-radius: 6px; margin-top: "
      "24px; font-weight: bold; font-size: 16px; color: #DDD; } "
      "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top "
      "left; padding: 0 10px; }");
  QVBoxLayout *digitalOutLayout = new QVBoxLayout(digitalOutGroup);

  m_digitalOutputTable =
      createStyledTable({"Channel", "Function", "Mode", "State", "Test"});
  digitalOutLayout->addWidget(m_digitalOutputTable);
  contentLayout->addWidget(digitalOutGroup);

  scrollArea->setWidget(contentWidget);
  m_mainLayout->addWidget(scrollArea);
}

// Helper to auto-resize table height to avoid internal scrollbars
void resizeTableToContents(QTableWidget *table) {
  if (!table)
    return;
  int h = table->horizontalHeader()->height();
  for (int i = 0; i < table->rowCount(); ++i)
    h += table->rowHeight(i);
  // Add a small buffer for borders
  table->setFixedHeight(h + 5);
}

QTableWidget *IOConfigWidget::createStyledTable(const QStringList &headers) {
  QTableWidget *t = new QTableWidget();
  t->setColumnCount(headers.size());
  t->setHorizontalHeaderLabels(headers);
  t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  t->verticalHeader()->setVisible(false);
  t->setSelectionBehavior(QAbstractItemView::SelectRows);
  t->setAlternatingRowColors(true);
  t->setShowGrid(false); // Clean modern look, remove grid lines if preferred,
                         // or keep subtle
  t->setGridStyle(Qt::SolidLine);

  // Table Style
  t->setStyleSheet(
      "QTableWidget { "
      "  background-color: #121212; "
      "  alternate-background-color: #1A1A1A; " // Subtle alternating
      "  gridline-color: #222222; "
      "  border: none; "
      "  font-size: 13px; "
      "  color: #E0E0E0; "
      "} "
      "QTableWidget::item { padding: 8px; border-bottom: 1px solid #222; } "
      "QTableWidget::item:selected { background-color: #333333; color: white; "
      "} "
      "QHeaderView::section { "
      "  background-color: #1E1E1E; "
      "  color: #CCCCCC; "
      "  padding: 8px; "
      "  border: none; "
      "  border-bottom: 2px solid #333; "
      "  font-weight: bold; "
      "}");

  t->verticalHeader()->setDefaultSectionSize(45); // Slightly spacious
  return t;
}

void IOConfigWidget::updateConfigFromPreset(int index) {
  if (m_settingsManager) m_settingsManager->setValue("ecuModel", index);
  QList<QStringList> analogData;
  QList<QStringList> digitalInData;
  QList<QStringList> digitalOutData;

  // Default "Custom" or "Manual" size (8/8/16)
  int anCount = 8;
  int diCount = 6;
  int doCount = 16;

  // Prepare standardized lists based on prompt DB

  if (index == 1) { // Speeduino v0.4.3c
    analogData = {{"AN0", "None (Spare)", "0", "5", "1.88V"},
                  {"AN1", "None (Spare)", "0", "5", "0.66V"},
                  {"AN2", "None (Spare)", "0", "5", "2.81V"},
                  {"AN3", "None (Spare)", "0", "5", "2.34V"},
                  {"AN4", "None (Spare)", "0", "5", "3.89V"},
                  {"AN5", "None (Spare)", "0", "5", "4.54V"},
                  {"AN6", "None (Spare)", "0", "5", "0.77V"},
                  {"AN7", "None (Spare)", "0", "5", "0.00V"}};
    digitalInData = {{"DI0", "None (Spare)", "Pull-Up", "HIGH"},
                     {"DI1", "None (Spare)", "Pull-Up", "HIGH"},
                     {"DI2", "None (Spare)", "Pull-Up", "HIGH"},
                     {"DI3", "None (Spare)", "Pull-Up", "LOW"},
                     {"DI4", "None (Spare)", "Pull-Up", "HIGH"},
                     {"DI5", "None (Spare)", "Pull-Up", "LOW"}};
    digitalOutData = {{"DO0", "None (Spare)", "Push-Pull", "OFF"},
                      {"DO1", "None (Spare)", "Push-Pull", "OFF"},
                      {"DO2", "None (Spare)", "Push-Pull", "ON"},
                      {"DO3", "None (Spare)", "Push-Pull", "ON"},
                      {"DO4", "None (Spare)", "Push-Pull", "OFF"},
                      {"DO5", "None (Spare)", "Push-Pull", "ON"},
                      {"DO6", "None (Spare)", "Push-Pull", "ON"},
                      {"DO7", "None (Spare)", "Push-Pull", "OFF"},
                      {"DO8", "None (Spare)", "Push-Pull", "OFF"},
                      {"DO9", "None (Spare)", "Push-Pull", "OFF"},
                      {"DO10", "None (Spare)", "Push-Pull", "ON"},
                      {"DO11", "None (Spare)", "Push-Pull", "OFF"},
                      {"DO12", "None (Spare)", "Push-Pull", "OFF"},
                      {"DO13", "None (Spare)", "Push-Pull", "OFF"},
                      {"DO14", "None (Spare)", "Push-Pull", "OFF"},
                      {"DO15", "None (Spare)", "Push-Pull", "ON"}};
  } else if (index == 2) { // Speeduino Dropbear V2
    analogData = {{"AN0", "Spare Analog 1", "0", "5", "1.88V"},
                  {"AN1", "Spare Analog 2", "0", "5", "0.66V"},
                  {"AN2", "None", "0", "5", "2.81V"},
                  {"AN3", "None", "0", "5", "2.34V"},
                  {"AN4", "None", "0", "5", "3.89V"},
                  {"AN5", "None", "0", "5", "4.54V"},
                  {"AN6", "None", "0", "5", "0.77V"},
                  {"AN7", "None", "0", "5", "0.00V"}};
    digitalInData = {{"DI0", "Spare Digital In 2", "Pull-Up", "HIGH"},
                     {"DI1", "Spare Digital 1", "Pull-Up", "HIGH"},
                     {"DI2", "Clutch Input", "Pull-Up", "LOW"},
                     {"DI3", "Flex Sensor", "Pull-Up", "HIGH"},
                     {"DI4", "Crank Primary", "Pull-Up", "HIGH"},
                     {"DI5", "Cam Primary", "Pull-Up", "LOW"}};
    digitalOutData = {{"DO0", "Injector 1-8", "Push-Pull", "OFF"},
                      {"DO1", "Ignition 1-8", "Push-Pull", "OFF"},
                      {"DO2", "Boost", "Push-Pull", "ON"},
                      {"DO3", "Fan", "Push-Pull", "ON"},
                      {"DO4", "Spare 2/Stepper-B2", "Push-Pull", "OFF"},
                      {"DO5", "Idle/Stepper-B1", "Push-Pull", "ON"},
                      {"DO6", "VVT/Stepper-A1", "Push-Pull", "ON"},
                      {"DO7", "Spare 1/Idle 2", "Push-Pull", "OFF"},
                      {"DO8", "Fuel Pump", "Push-Pull", "OFF"},
                      {"DO9", "None", "Push-Pull", "OFF"},
                      {"DO10", "None", "Push-Pull", "ON"},
                      {"DO11", "None", "Push-Pull", "OFF"},
                      {"DO12", "None", "Push-Pull", "OFF"},
                      {"DO13", "None", "Push-Pull", "OFF"},
                      {"DO14", "None", "Push-Pull", "OFF"},
                      {"DO15", "None", "Push-Pull", "ON"}};
  } else if (index == 3) { // Speeduino UAC
    analogData = {{"AN0", "None (Aux)", "0", "5", "1.88V"},
                  {"AN1", "None (Aux)", "0", "5", "0.66V"},
                  {"AN2", "None (Aux)", "0", "5", "2.81V"},
                  {"AN3", "None (Aux)", "0", "5", "2.34V"},
                  {"AN4", "None", "0", "5", "3.89V"},
                  {"AN5", "None", "0", "5", "4.54V"},
                  {"AN6", "None", "0", "5", "0.77V"},
                  {"AN7", "None", "0", "5", "0.00V"}};
    digitalInData = {{"DI0", "None (Aux)", "Pull-Up", "HIGH"},
                     {"DI1", "None (Aux)", "Pull-Up", "HIGH"},
                     {"DI2", "None", "Pull-Up", "LOW"},
                     {"DI3", "None", "Pull-Up", "HIGH"},
                     {"DI4", "None", "Pull-Up", "HIGH"},
                     {"DI5", "None", "Pull-Up", "LOW"}};
    digitalOutData = {{"DO0", "Ignition 1-4", "Push-Pull", "OFF"},
                      {"DO1", "Injection 1-4", "Push-Pull", "OFF"},
                      {"DO2", "Medium/High Current", "Push-Pull", "ON"},
                      {"DO3", "None", "Push-Pull", "ON"},
                      {"DO4", "None", "Push-Pull", "OFF"},
                      {"DO5", "None", "Push-Pull", "ON"},
                      {"DO6", "None", "Push-Pull", "ON"},
                      {"DO7", "None", "Push-Pull", "OFF"},
                      {"DO8", "None", "Push-Pull", "OFF"},
                      {"DO9", "None", "Push-Pull", "OFF"},
                      {"DO10", "None", "Push-Pull", "ON"},
                      {"DO11", "None", "Push-Pull", "OFF"},
                      {"DO12", "None", "Push-Pull", "OFF"},
                      {"DO13", "None", "Push-Pull", "OFF"},
                      {"DO14", "None", "Push-Pull", "OFF"},
                      {"DO15", "None", "Push-Pull", "ON"}};
  } else if (index == 4 || index == 5) { // NO2C / C2C
    analogData = {{"AN0", "None (Protected)", "0", "5", "1.88V"},
                  {"AN1", "None", "0", "5", "0.66V"},
                  {"AN2", "None", "0", "5", "2.81V"},
                  {"AN3", "None", "0", "5", "2.34V"},
                  {"AN4", "None", "0", "5", "3.89V"},
                  {"AN5", "None", "0", "5", "4.54V"},
                  {"AN6", "None", "0", "5", "0.77V"},
                  {"AN7", "None", "0", "5", "0.00V"}};
    digitalInData = {{"DI0", "None (Protected)", "Pull-Up", "HIGH"},
                     {"DI1", "None (Protected)", "Pull-Up", "HIGH"},
                     {"DI2", "None", "Pull-Up", "LOW"},
                     {"DI3", "None", "Pull-Up", "HIGH"},
                     {"DI4", "None", "Pull-Up", "HIGH"},
                     {"DI5", "None", "Pull-Up", "LOW"}};
    digitalOutData = {{"DO0", "Coil Output Channels", "Push-Pull", "OFF"},
                      {"DO1", "Medium/High Current", "Push-Pull", "OFF"},
                      {"DO2", "Tachometer", "Push-Pull", "ON"},
                      {"DO3", "None", "Push-Pull", "ON"},
                      {"DO4", "None", "Push-Pull", "OFF"},
                      {"DO5", "None", "Push-Pull", "ON"},
                      {"DO6", "None", "Push-Pull", "ON"},
                      {"DO7", "None", "Push-Pull", "OFF"},
                      {"DO8", "None", "Push-Pull", "OFF"},
                      {"DO9", "None", "Push-Pull", "OFF"},
                      {"DO10", "None", "Push-Pull", "ON"},
                      {"DO11", "None", "Push-Pull", "OFF"},
                      {"DO12", "None", "Push-Pull", "OFF"},
                      {"DO13", "None", "Push-Pull", "OFF"},
                      {"DO14", "None", "Push-Pull", "OFF"},
                      {"DO15", "None", "Push-Pull", "ON"}};
  } else if (index == 6) { // MS1
    analogData = {{"AN0", "None (Spare)", "0", "5", "1.88V"},
                  {"AN1", "None (Spare)", "0", "5", "0.66V"},
                  {"AN2", "None (Spare)", "0", "5", "2.81V"},
                  {"AN3", "None (Spare)", "0", "5", "2.34V"},
                  {"AN4", "None", "0", "5", "3.89V"},
                  {"AN5", "None", "0", "5", "4.54V"},
                  {"AN6", "None", "0", "5", "0.77V"},
                  {"AN7", "None", "0", "5", "0.00V"}};
    digitalInData = {{"DI0", "None (Switch)", "Pull-Up", "HIGH"},
                     {"DI1", "None (Switch)", "Pull-Up", "HIGH"},
                     {"DI2", "Knock", "Pull-Up", "LOW"},
                     {"DI3", "Launch", "Pull-Up", "HIGH"},
                     {"DI4", "Map Switch", "Pull-Up", "HIGH"},
                     {"DI5", "None", "Pull-Up", "LOW"}};
    digitalOutData = {{"DO0", "Output 1 (Relay)", "Push-Pull", "OFF"},
                      {"DO1", "Output 2 (Relay)", "Push-Pull", "OFF"},
                      {"DO2", "Output 3", "Push-Pull", "ON"},
                      {"DO3", "Output 4", "Push-Pull", "ON"},
                      {"DO4", "Shift Light", "Push-Pull", "OFF"},
                      {"DO5", "Water Injection", "Push-Pull", "ON"},
                      {"DO6", "Nitrous", "Push-Pull", "ON"},
                      {"DO7", "Boost Control", "Push-Pull", "OFF"},
                      {"DO8", "Idle Valve", "Push-Pull", "OFF"},
                      {"DO9", "Tacho", "Push-Pull", "OFF"},
                      {"DO10", "None", "Push-Pull", "ON"},
                      {"DO11", "None", "Push-Pull", "OFF"},
                      {"DO12", "None", "Push-Pull", "OFF"},
                      {"DO13", "None", "Push-Pull", "OFF"},
                      {"DO14", "None", "Push-Pull", "OFF"},
                      {"DO15", "None", "Push-Pull", "ON"}};
  } else if (index == 7) { // MS2
    analogData = {{"AN0", "MAT", "0", "5", "1.88V"},
                  {"AN1", "CLT", "0", "5", "0.66V"},
                  {"AN2", "TPS", "0", "5", "2.81V"},
                  {"AN3", "O2", "0", "5", "2.34V"},
                  {"AN4", "MAP (Internal)", "0", "5", "3.89V"},
                  {"AN5", "MAF (Mod)", "0", "5", "4.54V"},
                  {"AN6", "Flex Fuel (Mod)", "0", "5", "0.77V"},
                  {"AN7", "Spare ADC", "0", "5", "0.00V"}};
    digitalInData = {{"DI0", "Crank/Cam Tach", "Pull-Up", "HIGH"},
                     {"DI1", "Switch (Nitrous)", "Pull-Up", "HIGH"},
                     {"DI2", "Knock", "Pull-Up", "LOW"},
                     {"DI3", "None (Spare)", "Pull-Up", "HIGH"},
                     {"DI4", "None (Spare)", "Pull-Up", "HIGH"},
                     {"DI5", "None (Spare)", "Pull-Up", "LOW"}};
    digitalOutData = {{"DO0", "Fuel Injector 1", "Push-Pull", "OFF"},
                      {"DO1", "Fuel Injector 2", "Push-Pull", "OFF"},
                      {"DO2", "Ignition A-C", "Push-Pull", "ON"},
                      {"DO3", "Fuel Pump", "Push-Pull", "ON"},
                      {"DO4", "Idle Valve", "Push-Pull", "OFF"},
                      {"DO5", "Tacho", "Push-Pull", "ON"},
                      {"DO6", "Boost Control", "Push-Pull", "ON"},
                      {"DO7", "Programmable", "Push-Pull", "OFF"},
                      {"DO8", "Nitrous", "Push-Pull", "OFF"},
                      {"DO9", "None", "Push-Pull", "OFF"},
                      {"DO10", "None", "Push-Pull", "ON"},
                      {"DO11", "None", "Push-Pull", "OFF"},
                      {"DO12", "None", "Push-Pull", "OFF"},
                      {"DO13", "None", "Push-Pull", "OFF"},
                      {"DO14", "None", "Push-Pull", "OFF"},
                      {"DO15", "None", "Push-Pull", "ON"}};
  } else if (index == 8) { // MS3
    analogData = {{"AN0", "SpareADC", "0", "5", "1.88V"},
                  {"AN1", "EGO2", "0", "5", "0.66V"},
                  {"AN2", "EXT_MAP", "0", "5", "2.81V"},
                  {"AN3", "None", "0", "5", "2.34V"},
                  {"AN4", "None", "0", "5", "3.89V"},
                  {"AN5", "None", "0", "5", "4.54V"},
                  {"AN6", "None", "0", "5", "0.77V"},
                  {"AN7", "None", "0", "5", "0.00V"}};
    digitalInData = {{"DI0", "Tableswitch In", "Pull-Down", "HIGH"},
                     {"DI1", "Datalog In", "Pull-Down", "HIGH"},
                     {"DI2", "Launch In", "Pull-Down", "LOW"},
                     {"DI3", "Nitrous In", "Pull-Up", "HIGH"},
                     {"DI4", "None", "Pull-Down", "HIGH"},
                     {"DI5", "None", "Pull-Down", "LOW"}};
    digitalOutData = {{"DO0", "Spark A-H", "Push-Pull", "OFF"},
                      {"DO1", "Boost", "Push-Pull", "OFF"},
                      {"DO2", "Idle", "Push-Pull", "ON"},
                      {"DO3", "Nitrous 1", "Push-Pull", "ON"},
                      {"DO4", "Nitrous 2", "Push-Pull", "OFF"},
                      {"DO5", "Tacho", "Push-Pull", "ON"},
                      {"DO6", "VVT", "Push-Pull", "ON"},
                      {"DO7", "Fuel Pump", "Push-Pull", "OFF"},
                      {"DO8", "Injector A-H", "Push-Pull", "OFF"},
                      {"DO9", "FIDLE", "Push-Pull", "OFF"},
                      {"DO10", "None", "Push-Pull", "ON"},
                      {"DO11", "None", "Push-Pull", "OFF"},
                      {"DO12", "None", "Push-Pull", "OFF"},
                      {"DO13", "None", "Push-Pull", "OFF"},
                      {"DO14", "None", "Push-Pull", "OFF"},
                      {"DO15", "None", "Push-Pull", "ON"}};
  } else { // 0 or 9 (Manual / Custom)
           // Default Fill
    for (int i = 0; i < anCount; i++) {
      analogData.append({QString("AN%1").arg(i), "None", "0", "5", "0.00V"});
    }
    for (int i = 0; i < diCount; i++) {
      digitalInData.append({QString("DI%1").arg(i), "None", "Pull-Up", "LOW"});
    }
    for (int i = 0; i < doCount; i++) {
      digitalOutData.append(
          {QString("DO%1").arg(i), "None", "Push-Pull", "OFF"});
    }
  }

  populateAnalogTable(analogData);
  populateDigitalInputTable(digitalInData);
  populateDigitalOutputTable(digitalOutData);

  // Auto-resize tables
  resizeTableToContents(m_analogTable);
  resizeTableToContents(m_digitalTable);
  resizeTableToContents(m_digitalOutputTable);
}

void IOConfigWidget::populateAnalogTable(const QList<QStringList> &data) {
  m_analogTable->setRowCount(0);
  for (const QStringList &rowItems : data) {
    int row = m_analogTable->rowCount();
    m_analogTable->insertRow(row);

    // Channel
    m_analogTable->setItem(row, 0, new QTableWidgetItem(rowItems[0]));

    // Function (ComboBox)
    QComboBox *func = new QComboBox();
    func->setStyleSheet("QComboBox { border: none; background: transparent; color: #BBB; } "
                        "QComboBox QAbstractItemView { background-color: #1E1E1E; color: white; selection-background-color: #00BCD4; selection-color: black; }");
    func->addItems(
        {"None", "TPS", "MAP", "CLT", "IAT", "O2", "Oil Press", "Fuel Press"});
    int idx =
        func->findText(rowItems[1], Qt::MatchContains); // Try to fuzzy match
    if (idx == -1) {
      func->addItem(rowItems[1]);
      func->setCurrentIndex(func->count() - 1);
    } else {
      func->setCurrentIndex(idx);
    }
    m_analogTable->setCellWidget(row, 1, func);

    // Min/Max V
    m_analogTable->setItem(row, 2, new QTableWidgetItem(rowItems[2]));
    m_analogTable->setItem(row, 3, new QTableWidgetItem(rowItems[3]));

    // Current Value
    QTableWidgetItem *valItem = new QTableWidgetItem(rowItems[4]);
    valItem->setForeground(QColor("#00FF00"));
    m_analogTable->setItem(row, 4, valItem);

    // Show on Dash
    QWidget *centered = new QWidget();
    QHBoxLayout *l = new QHBoxLayout(centered);
    l->addWidget(new QCheckBox());
    l->setAlignment(Qt::AlignCenter);
    l->setContentsMargins(0, 0, 0, 0);
    m_analogTable->setCellWidget(row, 5, centered);
  }
}

void IOConfigWidget::populateDigitalInputTable(const QList<QStringList> &data) {
  m_digitalTable->setRowCount(0);
  for (const QStringList &rowItems : data) {
    int row = m_digitalTable->rowCount();
    m_digitalTable->insertRow(row);

    // Channel
    m_digitalTable->setItem(row, 0, new QTableWidgetItem(rowItems[0]));

    // Function
    QComboBox *func = new QComboBox();
    func->setStyleSheet("QComboBox { border: none; background: transparent; color: #BBB; } "
                        "QComboBox QAbstractItemView { background-color: #1E1E1E; color: white; selection-background-color: #00BCD4; selection-color: black; }");
    func->addItems({"None", "Launch Control", "Flex Fuel", "Clutch", "Nitrous",
                    "Table Switch"});
    int idx = func->findText(rowItems[1], Qt::MatchContains);
    if (idx == -1) {
      func->addItem(rowItems[1]);
      func->setCurrentIndex(func->count() - 1);
    } else {
      func->setCurrentIndex(idx);
    }
    m_digitalTable->setCellWidget(row, 1, func);

    // Pull
    QComboBox *pull = new QComboBox();
    pull->setStyleSheet("QComboBox { border: none; background: transparent; color: #BBB; } "
                        "QComboBox QAbstractItemView { background-color: #1E1E1E; color: white; selection-background-color: #00BCD4; selection-color: black; }");
    pull->addItems({"Pull-Up", "Pull-Down"});
    pull->setCurrentText(rowItems[2]);
    m_digitalTable->setCellWidget(row, 2, pull);

    // State
    QLabel *state = new QLabel(rowItems[3]);
    bool isHigh = (rowItems[3] == "HIGH");
    state->setStyleSheet(isHigh
                             ? "background: #00AA00; color: white; padding: "
                               "2px 6px; border-radius: 4px; font-weight: bold;"
                             : "background: #444444; color: #AAA; padding: 2px "
                               "6px; border-radius: 4px;");
    state->setAlignment(Qt::AlignCenter);
    // Wrap label in widget to center it without taking full cell space
    QWidget *w = new QWidget();
    QHBoxLayout *l = new QHBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    l->setAlignment(Qt::AlignCenter);
    l->addWidget(state);
    m_digitalTable->setCellWidget(row, 3, w);

    // Show
    QWidget *centered = new QWidget();
    QHBoxLayout *l2 = new QHBoxLayout(centered);
    l2->addWidget(new QCheckBox());
    l2->setAlignment(Qt::AlignCenter);
    l2->setContentsMargins(0, 0, 0, 0);
    m_digitalTable->setCellWidget(row, 4, centered);
  }
}

void IOConfigWidget::populateDigitalOutputTable(
    const QList<QStringList> &data) {
  m_digitalOutputTable->setRowCount(0);
  for (const QStringList &rowItems : data) {
    int row = m_digitalOutputTable->rowCount();
    m_digitalOutputTable->insertRow(row);

    // Channel
    m_digitalOutputTable->setItem(row, 0, new QTableWidgetItem(rowItems[0]));

    // Function
    QComboBox *func = new QComboBox();
    func->setStyleSheet("QComboBox { border: none; background: transparent; color: #BBB; } "
                        "QComboBox QAbstractItemView { background-color: #1E1E1E; color: white; selection-background-color: #00BCD4; selection-color: black; }");
    func->addItems({"None", "Injector", "Ignition", "Fuel Pump", "Fan", "Boost",
                    "Idle", "VVT"});
    int idx = func->findText(rowItems[1], Qt::MatchContains);
    if (idx == -1) {
      func->addItem(rowItems[1]);
      func->setCurrentIndex(func->count() - 1);
    } else {
      func->setCurrentIndex(idx);
    }
    m_digitalOutputTable->setCellWidget(row, 1, func);

    // Mode
    QComboBox *mode = new QComboBox();
    mode->setStyleSheet("QComboBox { border: none; background: transparent; color: #BBB; } "
                        "QComboBox QAbstractItemView { background-color: #1E1E1E; color: white; selection-background-color: #00BCD4; selection-color: black; }");
    mode->addItems({"Push-Pull", "Open-Drain", "Inverted"});
    mode->setCurrentText(rowItems[2]);
    m_digitalOutputTable->setCellWidget(row, 2, mode);

    // State
    QLabel *state = new QLabel(rowItems[3]);
    bool isOn = (rowItems[3] == "ON");
    state->setStyleSheet(isOn ? "color: #00FF00; font-weight: bold;"
                              : "color: #777777; font-weight: normal;");
    state->setAlignment(Qt::AlignCenter);
    m_digitalOutputTable->setCellWidget(row, 3, state);

    // Test
    QPushButton *testBtn = new QPushButton("Test");
    testBtn->setCursor(Qt::PointingHandCursor);
    testBtn->setStyleSheet(
        "QPushButton { background-color: #333; border: 1px solid #555; "
        "border-radius: 3px; color: #DDD; padding: 4px 12px; }"
        "QPushButton:hover { background-color: #444; }"
        "QPushButton:pressed { background-color: #222; }");
    QWidget *centered = new QWidget();
    QHBoxLayout *l = new QHBoxLayout(centered);
    l->addWidget(testBtn);
    l->setAlignment(Qt::AlignCenter);
    l->setContentsMargins(0, 0, 0, 0);
    m_digitalOutputTable->setCellWidget(row, 4, centered);
  }
}

void IOConfigWidget::loadFromECU() {
  if (!m_settingsManager) return;
  // IO config is primarily preset-based, loads from ECU model selection
}
