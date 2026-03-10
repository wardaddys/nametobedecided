/**
 * @file LimitersWidget.cpp
 * @brief Implementation of Engine Limiters widget - Figma Design Match
 */

#include "LimitersWidget.h"
#include "../core/ECUSettingsManager.h"
#include <QHBoxLayout>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>

LimitersWidget::LimitersWidget(QWidget *parent) : TuningWidgetBase(parent) { setupUi(); }

LimitersWidget::~LimitersWidget() {}

void LimitersWidget::setupUi() {
  // Set dark background for this widget
  this->setStyleSheet("background-color: #2B2B2B;");

  m_mainLayout->setSpacing(15);
  m_mainLayout->setContentsMargins(20, 20, 20, 20);

  // Header
  QLabel *titleLabel = new QLabel("Engine Limiters & Protection", this);
  titleLabel->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: white;");
  m_mainLayout->addWidget(titleLabel);

  QLabel *subtitleLabel = new QLabel(
      "Configure RPM, speed limiters and launch control settings", this);
  subtitleLabel->setStyleSheet("font-size: 12px; color: #00BCD4;");
  m_mainLayout->addWidget(subtitleLabel);

  // Tab Widget
  m_tabWidget = new QTabWidget(this);
  m_tabWidget->setStyleSheet(R"(
        QTabWidget::pane { border: 1px solid #444; border-radius: 4px; background-color: #2B2B2B; }
        QTabBar::tab {
            background-color: #1E1E1E; color: #888;
            padding: 8px 16px; margin-right: 2px; border-radius: 4px 4px 0 0;
        }
        QTabBar::tab:selected { background-color: #2B2B2B; color: white; }
        QTabBar::tab:hover { background-color: #333; }
    )");

  m_tabWidget->addTab(createRevLimiterTab(), "Rev Limiter");
  m_tabWidget->addTab(createSpeedLimiterTab(), "Speed Limiter");
  m_tabWidget->addTab(createLaunchControlTab(), "Launch Control");

  m_mainLayout->addWidget(m_tabWidget, 1);
}

QWidget *LimitersWidget::createRevLimiterTab() {
  QScrollArea *scrollArea = new QScrollArea(this);
  scrollArea->setWidgetResizable(true);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setStyleSheet("QScrollArea { background-color: #2B2B2B; }");

  QWidget *scrollContent = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(scrollContent);
  layout->setSpacing(15);
  layout->setContentsMargins(10, 10, 10, 10);

  // Two column layout
  QHBoxLayout *columnsLayout = new QHBoxLayout();
  columnsLayout->setSpacing(20);
  columnsLayout->addWidget(createSoftLimiterSection(), 1);
  columnsLayout->addWidget(createHardLimiterSection(), 1);
  layout->addLayout(columnsLayout);

  // Rev Limit Zones
  layout->addWidget(createRevLimitZonesSection());
  layout->addStretch();

  scrollArea->setWidget(scrollContent);
  return scrollArea;
}

QWidget *LimitersWidget::createSpeedLimiterTab() {
  QWidget *tab = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout(tab);
  layout->setSpacing(15);
  layout->setContentsMargins(20, 20, 20, 20);

  QGroupBox *box = new QGroupBox("Speed Limiter Settings", this);
  box->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; color: white; border: 1px solid #444; border-radius: 8px; margin-top: 10px; padding-top: 10px; } "
                     "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }");
  QGridLayout *grid = new QGridLayout(box);
  grid->setSpacing(15);

  m_speedLimEnabled = new QCheckBox("Enable Speed Limiter", this);
  m_speedLimEnabled->setStyleSheet("color: white;");
  connect(m_speedLimEnabled, &QCheckBox::toggled, [this](bool checked) {
      if (m_settingsManager) m_settingsManager->setValue("speedLimEnabled", checked ? 1 : 0);
  });
  grid->addWidget(m_speedLimEnabled, 0, 0, 1, 2);

  grid->addWidget(new QLabel("Speed Limit (km/h):"), 1, 0);
  m_speedLimit = new QSpinBox(this);
  m_speedLimit->setRange(0, 300);
  m_speedLimit->setStyleSheet("background-color: #1E1E1E; color: white; padding: 4px; border: 1px solid #444; border-radius: 4px;");
  connect(m_speedLimit, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val) {
      if (m_settingsManager) m_settingsManager->setValue("speedLimit", val);
  });
  grid->addWidget(m_speedLimit, 1, 1);

  grid->addWidget(new QLabel("Action:"), 2, 0);
  m_speedLimAction = new QComboBox(this);
  m_speedLimAction->addItems({"Fuel Cut", "Spark Cut", "Both"});
  m_speedLimAction->setStyleSheet("QComboBox { background-color: #1E1E1E; color: white; padding: 4px; border: 1px solid #444; border-radius: 4px; }");
  connect(m_speedLimAction, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
      if (m_settingsManager) m_settingsManager->setValue("speedLimAction", idx);
  });
  grid->addWidget(m_speedLimAction, 2, 1);

  grid->addWidget(new QLabel("Hysteresis (km/h):"), 3, 0);
  m_speedLimHyst = new QSpinBox(this);
  m_speedLimHyst->setRange(0, 50);
  m_speedLimHyst->setStyleSheet("background-color: #1E1E1E; color: white; padding: 4px; border: 1px solid #444; border-radius: 4px;");
  connect(m_speedLimHyst, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val) {
      if (m_settingsManager) m_settingsManager->setValue("speedLimHyst", val);
  });
  grid->addWidget(m_speedLimHyst, 3, 1);

  layout->addWidget(box);
  layout->addStretch();
  return tab;
}

QWidget *LimitersWidget::createLaunchControlTab() {
  QWidget *tab = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout(tab);
  layout->setSpacing(15);
  layout->setContentsMargins(20, 20, 20, 20);

  QGroupBox *launchBox = new QGroupBox("Launch Control", this);
  launchBox->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; color: white; border: 1px solid #444; border-radius: 8px; margin-top: 10px; padding-top: 10px; } "
                           "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }");
  QGridLayout *launchGrid = new QGridLayout(launchBox);
  launchGrid->setSpacing(15);

  m_launchEnabled = new QCheckBox("Enable Launch Control", this);
  m_launchEnabled->setStyleSheet("color: white;");
  connect(m_launchEnabled, &QCheckBox::toggled, [this](bool checked) {
      if (m_settingsManager) m_settingsManager->setValue("launchEnabled", checked ? 1 : 0);
  });
  launchGrid->addWidget(m_launchEnabled, 0, 0, 1, 2);

  launchGrid->addWidget(new QLabel("Launch RPM:"), 1, 0);
  m_launchRpm = new QSpinBox(this);
  m_launchRpm->setRange(1000, 10000);
  m_launchRpm->setStyleSheet("background-color: #1E1E1E; color: white; padding: 4px; border: 1px solid #444; border-radius: 4px;");
  connect(m_launchRpm, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val) {
      if (m_settingsManager) m_settingsManager->setValue("launchRpm", val);
  });
  launchGrid->addWidget(m_launchRpm, 1, 1);

  launchGrid->addWidget(new QLabel("Timing Retard (°):"), 2, 0);
  m_launchRetard = new QSpinBox(this);
  m_launchRetard->setRange(0, 45);
  m_launchRetard->setStyleSheet("background-color: #1E1E1E; color: white; padding: 4px; border: 1px solid #444; border-radius: 4px;");
  connect(m_launchRetard, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val) {
      if (m_settingsManager) m_settingsManager->setValue("launchRetard", val);
  });
  launchGrid->addWidget(m_launchRetard, 2, 1);

  launchGrid->addWidget(new QLabel("TPS Threshold (%):"), 3, 0);
  m_launchTps = new QSpinBox(this);
  m_launchTps->setRange(0, 100);
  m_launchTps->setStyleSheet("background-color: #1E1E1E; color: white; padding: 4px; border: 1px solid #444; border-radius: 4px;");
  connect(m_launchTps, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val) {
      if (m_settingsManager) m_settingsManager->setValue("launchTps", val);
  });
  launchGrid->addWidget(m_launchTps, 3, 1);

  layout->addWidget(launchBox);

  // Flat Shift
  QGroupBox *flatBox = new QGroupBox("Flat Shift", this);
  flatBox->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; color: white; border: 1px solid #444; border-radius: 8px; margin-top: 10px; padding-top: 10px; } "
                         "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }");
  QGridLayout *flatGrid = new QGridLayout(flatBox);
  flatGrid->setSpacing(15);

  m_flatShiftEnabled = new QCheckBox("Enable Flat Shift", this);
  m_flatShiftEnabled->setStyleSheet("color: white;");
  connect(m_flatShiftEnabled, &QCheckBox::toggled, [this](bool checked) {
      if (m_settingsManager) m_settingsManager->setValue("flatShiftEnabled", checked ? 1 : 0);
  });
  flatGrid->addWidget(m_flatShiftEnabled, 0, 0, 1, 2);

  flatGrid->addWidget(new QLabel("RPM Drop:"), 1, 0);
  m_flatShiftRpmDrop = new QSpinBox(this);
  m_flatShiftRpmDrop->setRange(0, 5000);
  m_flatShiftRpmDrop->setStyleSheet("background-color: #1E1E1E; color: white; padding: 4px; border: 1px solid #444; border-radius: 4px;");
  connect(m_flatShiftRpmDrop, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val) {
      if (m_settingsManager) m_settingsManager->setValue("flatShiftRpmDrop", val);
  });
  flatGrid->addWidget(m_flatShiftRpmDrop, 1, 1);

  layout->addWidget(flatBox);
  layout->addStretch();
  return tab;
}

QWidget *LimitersWidget::createSoftLimiterSection() {
  QGroupBox *box = new QGroupBox("Soft Rev Limiter", this);
  box->setStyleSheet(R"(
        QGroupBox {
            font-size: 14px; font-weight: bold; color: white;
            border: 1px solid #444; border-radius: 8px;
            margin-top: 10px; padding-top: 10px;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }
    )");

  QVBoxLayout *layout = new QVBoxLayout(box);
  layout->setSpacing(12);

  // Soft Limit RPM
  QHBoxLayout *rpmRow = new QHBoxLayout();
  QLabel *rpmLabel = new QLabel("Soft Limit RPM", this);
  rpmLabel->setStyleSheet("color: #888;");
  m_softLimitValue = new QLabel("6000 rpm", this);
  m_softLimitValue->setStyleSheet("color: #FF9800; font-weight: bold;");
  rpmRow->addWidget(rpmLabel);
  rpmRow->addStretch();
  rpmRow->addWidget(m_softLimitValue);
  layout->addLayout(rpmRow);

  m_softLimitSlider = createStyledSlider(3000, 9000, 6000);
  m_softLimitSlider->setStyleSheet(R"(
        QSlider::groove:horizontal {
            border: 1px solid #444; height: 8px;
            background: #1E1E1E; border-radius: 4px;
        }
        QSlider::handle:horizontal {
            background: #FF9800; border: 1px solid #F57C00;
            width: 18px; margin: -5px 0; border-radius: 9px;
        }
        QSlider::sub-page:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FF9800, stop:1 #F57C00);
            border-radius: 4px;
        }
    )");
  connect(m_softLimitSlider, &QSlider::valueChanged, this,
          &LimitersWidget::onSoftLimitRpmChanged);
  layout->addWidget(m_softLimitSlider);

  QLabel *rpmInfo = new QLabel("RPM at which soft limiting begins", this);
  rpmInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(rpmInfo);

  // Soft Limit Method
  QLabel *methodLabel = new QLabel("Soft Limit Method", this);
  methodLabel->setStyleSheet("color: #888;");
  layout->addWidget(methodLabel);

  QHBoxLayout *methodRow = new QHBoxLayout();
  m_ignitionCutBtn = new QPushButton("Ignition Cut", this);
  m_ignitionCutBtn->setCheckable(true);
  m_ignitionCutBtn->setChecked(true);
  m_ignitionCutBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #00BCD4; color: white;
            padding: 10px 20px; border-radius: 4px; font-weight: bold;
        }
        QPushButton:!checked {
            background-color: #1E1E1E; color: #888;
        }
    )");
  connect(m_ignitionCutBtn, &QPushButton::clicked, this,
          &LimitersWidget::onIgnitionCutClicked);

  m_fuelCutBtn = new QPushButton("Fuel Cut", this);
  m_fuelCutBtn->setCheckable(true);
  m_fuelCutBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #00BCD4; color: white;
            padding: 10px 20px; border-radius: 4px; font-weight: bold;
        }
        QPushButton:!checked {
            background-color: #1E1E1E; color: #888;
        }
    )");
  connect(m_fuelCutBtn, &QPushButton::clicked, this,
          &LimitersWidget::onFuelCutClicked);

  methodRow->addWidget(m_ignitionCutBtn);
  methodRow->addWidget(m_fuelCutBtn);
  layout->addLayout(methodRow);

  QLabel *methodInfo =
      new QLabel("Cuts ignition - smoother, produces incredible flames", this);
  methodInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(methodInfo);

  // Tip
  QFrame *tipFrame = new QFrame(this);
  tipFrame->setStyleSheet(
      "background-color: rgba(0, 188, 212, 0.1); border: 1px solid #00BCD4; "
      "border-radius: 4px; padding: 8px;");
  QLabel *tipLabel = new QLabel(
      "TIP: Soft limiter allows brief excursions above the limit during shifts",
      this);
  tipLabel->setStyleSheet("color: #00BCD4; font-size: 10px;");
  tipLabel->setWordWrap(true);
  QVBoxLayout *tipLayout = new QVBoxLayout(tipFrame);
  tipLayout->addWidget(tipLabel);
  layout->addWidget(tipFrame);

  return box;
}

QWidget *LimitersWidget::createHardLimiterSection() {
  QGroupBox *box = new QGroupBox("Hard Rev Limiter", this);
  box->setStyleSheet(R"(
        QGroupBox {
            font-size: 14px; font-weight: bold; color: white;
            border: 1px solid #444; border-radius: 8px;
            margin-top: 10px; padding-top: 10px;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }
    )");

  QVBoxLayout *layout = new QVBoxLayout(box);
  layout->setSpacing(12);

  // Hard Limit RPM
  QHBoxLayout *rpmRow = new QHBoxLayout();
  QLabel *rpmLabel = new QLabel("Hard Limit RPM", this);
  rpmLabel->setStyleSheet("color: #888;");
  m_hardLimitValue = new QLabel("6885 rpm", this);
  m_hardLimitValue->setStyleSheet("color: #E53935; font-weight: bold;");
  rpmRow->addWidget(rpmLabel);
  rpmRow->addStretch();
  rpmRow->addWidget(m_hardLimitValue);
  layout->addLayout(rpmRow);

  m_hardLimitSlider = createStyledSlider(3000, 9000, 6885);
  m_hardLimitSlider->setStyleSheet(R"(
        QSlider::groove:horizontal {
            border: 1px solid #444; height: 8px;
            background: #1E1E1E; border-radius: 4px;
        }
        QSlider::handle:horizontal {
            background: #E53935; border: 1px solid #C62828;
            width: 18px; margin: -5px 0; border-radius: 9px;
        }
        QSlider::sub-page:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #E53935, stop:1 #C62828);
            border-radius: 4px;
        }
    )");
  connect(m_hardLimitSlider, &QSlider::valueChanged, this,
          &LimitersWidget::onHardLimitRpmChanged);
  layout->addWidget(m_hardLimitSlider);

  QLabel *rpmInfo = new QLabel(
      "Absolute maximum RPM - aggressive fuel and ignition cut", this);
  rpmInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(rpmInfo);

  // Warning
  QFrame *warnFrame = new QFrame(this);
  warnFrame->setStyleSheet(
      "background-color: rgba(229, 57, 53, 0.2); border: 1px solid #E53935; "
      "border-radius: 4px; padding: 8px;");
  QHBoxLayout *warnLayout = new QHBoxLayout(warnFrame);
  QLabel *warnIcon = new QLabel("⚠", this);
  warnIcon->setStyleSheet("color: #E53935; font-size: 16px;");
  QLabel *warnText = new QLabel(
      "WARNING: Hard limit should be 200-500 RPM above soft limit", this);
  warnText->setStyleSheet("color: #E53935; font-size: 11px;");
  warnLayout->addWidget(warnIcon);
  warnLayout->addWidget(warnText);
  warnLayout->addStretch();
  layout->addWidget(warnFrame);

  // Cut Type and Pattern
  QHBoxLayout *cutRow = new QHBoxLayout();

  QVBoxLayout *typeCol = new QVBoxLayout();
  QLabel *typeLabel = new QLabel("Cut Type", this);
  typeLabel->setStyleSheet("color: #888;");
  m_cutTypeCombo = new QComboBox(this);
  m_cutTypeCombo->addItems({"Fuel + Ign", "Fuel Only", "Ignition Only"});
  m_cutTypeCombo->setStyleSheet(
      "QComboBox { background-color: #1E1E1E; color: white; padding: 6px; border: 1px solid #444; border-radius: 4px; }"
      "QComboBox QAbstractItemView { background-color: #1E1E1E; color: white; selection-background-color: #00BCD4; selection-color: black; border: 1px solid #444; }");
  typeCol->addWidget(typeLabel);
  typeCol->addWidget(m_cutTypeCombo);
  cutRow->addLayout(typeCol);

  QVBoxLayout *patternCol = new QVBoxLayout();
  QLabel *patternLabel = new QLabel("Cut Pattern", this);
  patternLabel->setStyleSheet("color: #888;");
  m_cutPatternCombo = new QComboBox(this);
  m_cutPatternCombo->addItems({"All Cyl", "Sequential", "Random"});
  m_cutPatternCombo->setStyleSheet(
      "QComboBox { background-color: #1E1E1E; color: white; padding: 6px; border: 1px solid #444; border-radius: 4px; }"
      "QComboBox QAbstractItemView { background-color: #1E1E1E; color: white; selection-background-color: #00BCD4; selection-color: black; border: 1px solid #444; }");
  patternCol->addWidget(patternLabel);
  patternCol->addWidget(m_cutPatternCombo);
  cutRow->addLayout(patternCol);

  layout->addLayout(cutRow);

  return box;
}

QWidget *LimitersWidget::createRevLimitZonesSection() {
  QGroupBox *box = new QGroupBox("Rev Limit Zones", this);
  box->setStyleSheet(R"(
        QGroupBox {
            font-size: 14px; font-weight: bold; color: white;
            border: 1px solid #444; border-radius: 8px;
            margin-top: 10px; padding-top: 10px;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }
    )");

  QVBoxLayout *layout = new QVBoxLayout(box);
  layout->setSpacing(10);

  // Zones bar
  m_zonesBar = new QFrame(this);
  m_zonesBar->setFixedHeight(40);
  m_zonesBar->setStyleSheet("border-radius: 4px;");

  m_zonesBarLayout = new QHBoxLayout(m_zonesBar);
  m_zonesBarLayout->setContentsMargins(0, 0, 0, 0);
  m_zonesBarLayout->setSpacing(0);

  // Safe Zone (0 - soft limit) - Green
  m_safeZone = new QFrame(this);
  m_safeZone->setStyleSheet(
      "background-color: #4CAF50; border-radius: 4px 0 0 4px;");
  QHBoxLayout *safeLayout = new QHBoxLayout(m_safeZone);
  QLabel *safeStart = new QLabel("0 rpm", this);
  safeStart->setStyleSheet("color: white; font-size: 10px;");
  safeLayout->addWidget(safeStart);
  safeLayout->addStretch();

  // Soft Limit Zone (soft - hard limit) - Orange
  m_softZone = new QFrame(this);
  m_softZone->setStyleSheet("background-color: #FF9800;");
  QHBoxLayout *softLayout = new QHBoxLayout(m_softZone);
  m_softZoneLabel = new QLabel("6000 rpm", this);
  m_softZoneLabel->setStyleSheet("color: white; font-size: 10px;");
  softLayout->addWidget(m_softZoneLabel);
  softLayout->addStretch();

  // Hard Limit Zone (hard limit+) - Red
  m_hardZone = new QFrame(this);
  m_hardZone->setStyleSheet(
      "background-color: #E53935; border-radius: 0 4px 4px 0;");
  QHBoxLayout *hardLayout = new QHBoxLayout(m_hardZone);
  hardLayout->addStretch();
  m_hardZoneLabel = new QLabel("6885 rpm", this);
  m_hardZoneLabel->setStyleSheet("color: white; font-size: 10px;");
  hardLayout->addWidget(m_hardZoneLabel);

  // Initial proportions based on default values (soft=6000, hard=6885, max=9000)
  m_zonesBarLayout->addWidget(m_safeZone, 60);   // 6000 / 100
  m_zonesBarLayout->addWidget(m_softZone, 9);    // (6885-6000) / 100 = ~9
  m_zonesBarLayout->addWidget(m_hardZone, 21);   // (9000-6885) / 100 = ~21

  layout->addWidget(m_zonesBar);

  // Legend
  QHBoxLayout *legendRow = new QHBoxLayout();

  QHBoxLayout *safeLegend = new QHBoxLayout();
  QFrame *safeColor = new QFrame(this);
  safeColor->setFixedSize(12, 12);
  safeColor->setStyleSheet("background-color: #4CAF50; border-radius: 2px;");
  QLabel *safeText = new QLabel("Safe Zone", this);
  safeText->setStyleSheet("color: #888;");
  safeLegend->addWidget(safeColor);
  safeLegend->addWidget(safeText);
  legendRow->addLayout(safeLegend);

  legendRow->addSpacing(20);

  QHBoxLayout *softLegend = new QHBoxLayout();
  QFrame *softColor = new QFrame(this);
  softColor->setFixedSize(12, 12);
  softColor->setStyleSheet("background-color: #FF9800; border-radius: 2px;");
  QLabel *softText = new QLabel("Soft Limit", this);
  softText->setStyleSheet("color: #888;");
  softLegend->addWidget(softColor);
  softLegend->addWidget(softText);
  legendRow->addLayout(softLegend);

  legendRow->addSpacing(20);

  QHBoxLayout *hardLegend = new QHBoxLayout();
  QFrame *hardColor = new QFrame(this);
  hardColor->setFixedSize(12, 12);
  hardColor->setStyleSheet("background-color: #E53935; border-radius: 2px;");
  QLabel *hardText = new QLabel("Hard Limit", this);
  hardText->setStyleSheet("color: #888;");
  hardLegend->addWidget(hardColor);
  hardLegend->addWidget(hardText);
  legendRow->addLayout(hardLegend);

  legendRow->addStretch();
  layout->addLayout(legendRow);

  return box;
}

QSlider *LimitersWidget::createStyledSlider(int min, int max, int value) {
  QSlider *slider = new QSlider(Qt::Horizontal, this);
  slider->setRange(min, max);
  slider->setValue(value);
  slider->setStyleSheet(R"(
        QSlider::groove:horizontal {
            border: 1px solid #444; height: 8px;
            background: #1E1E1E; border-radius: 4px;
        }
        QSlider::handle:horizontal {
            background: #00BCD4; border: 1px solid #00ACC1;
            width: 18px; margin: -5px 0; border-radius: 9px;
        }
        QSlider::sub-page:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00BCD4, stop:1 #00ACC1);
            border-radius: 4px;
        }
    )");
  return slider;
}

void LimitersWidget::onSoftLimitRpmChanged(int value) {
  m_softLimitValue->setText(QString::number(value) + " rpm");
  updateZonesBar();
  if (m_settingsManager) {
    m_settingsManager->setValue("softRevLimit", value);
  }
}

void LimitersWidget::onHardLimitRpmChanged(int value) {
  m_hardLimitValue->setText(QString::number(value) + " rpm");
  updateZonesBar();
  if (m_settingsManager) {
    m_settingsManager->setValue("hardRevLimit", value);
  }
}

void LimitersWidget::updateZonesBar() {
  // Get current slider values
  int softLimit = m_softLimitSlider->value();
  int hardLimit = m_hardLimitSlider->value();
  int maxRpm = 9000;  // Max RPM range
  
  // Ensure soft limit is always less than hard limit
  if (softLimit >= hardLimit) {
    softLimit = hardLimit - 100;  // Keep at least 100 rpm gap
    if (softLimit < 3000) softLimit = 3000;
  }
  
  // Calculate proportions (as percentages of max RPM)
  int safeWidth = softLimit * 100 / maxRpm;
  int softWidth = (hardLimit - softLimit) * 100 / maxRpm;
  int hardWidth = (maxRpm - hardLimit) * 100 / maxRpm;
  
  // Ensure minimum widths for visibility
  if (softWidth < 5) softWidth = 5;
  if (hardWidth < 5) hardWidth = 5;
  
  // Update zone labels
  m_softZoneLabel->setText(QString::number(softLimit) + " rpm");
  m_hardZoneLabel->setText(QString::number(hardLimit) + " rpm");
  
  // Update layout stretch factors
  m_zonesBarLayout->setStretch(0, safeWidth);
  m_zonesBarLayout->setStretch(1, softWidth);
  m_zonesBarLayout->setStretch(2, hardWidth);
}

void LimitersWidget::onIgnitionCutClicked() {
  m_ignitionCutBtn->setChecked(true);
  m_fuelCutBtn->setChecked(false);
}

void LimitersWidget::onFuelCutClicked() {
  m_fuelCutBtn->setChecked(true);
  m_ignitionCutBtn->setChecked(false);
}

void LimitersWidget::loadFromECU() {
  if (!m_settingsManager) return;
  QVariant v;
  v = m_settingsManager->getValue("softRevLimit");
  if (v.isValid()) { m_softLimitSlider->setValue(v.toInt()); }
  v = m_settingsManager->getValue("hardRevLimit");
  if (v.isValid()) { m_hardLimitSlider->setValue(v.toInt()); }
}
