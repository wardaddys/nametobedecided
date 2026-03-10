/**
 * @file IdleControlWidget.cpp
 * @brief Implementation of Idle Control widget - Figma Design Match
 *
 * @author Safeerullah Afridi (FA-22-151)
 * @author Muhammad Saeed Sajid (FA-22-143)
 * @supervisor Prof. Dr. Shariq Hussain
 * @date December 2025
 */

#include "IdleControlWidget.h"
#include "../core/ECUSettingsManager.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QScrollArea>
#include <QVBoxLayout>


IdleControlWidget::IdleControlWidget(QWidget *parent) : TuningWidgetBase(parent) {
  setupUi();
}

IdleControlWidget::~IdleControlWidget() {}

void IdleControlWidget::setupUi() {
  QScrollArea *scrollArea = new QScrollArea(this);
  scrollArea->setWidgetResizable(true);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setStyleSheet(
      "QScrollArea { background-color: #2B2B2B; border: none; }");

  QWidget *scrollContent = new QWidget();
  scrollContent->setStyleSheet("background-color: #2B2B2B;");
  QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
  scrollLayout->setSpacing(15);
  scrollLayout->setContentsMargins(20, 20, 20, 20);

  // --- HEADER ---
  QLabel *titleLabel = new QLabel("Idle Control", this);
  titleLabel->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: white;");
  scrollLayout->addWidget(titleLabel);

  QLabel *subtitleLabel = new QLabel(
      "Configure closed-loop idle speed control and idle-up conditions", this);
  subtitleLabel->setStyleSheet("font-size: 12px; color: #00BCD4;");
  scrollLayout->addWidget(subtitleLabel);

  // --- TWO COLUMN LAYOUT ---
  QHBoxLayout *columnsLayout = new QHBoxLayout();
  columnsLayout->setSpacing(20);

  // LEFT COLUMN
  QVBoxLayout *leftCol = new QVBoxLayout();
  leftCol->setSpacing(15);
  leftCol->addWidget(createTargetIdleSection());
  leftCol->addWidget(createPIDSection());
  leftCol->addWidget(createDashpotSection());
  leftCol->addStretch();
  columnsLayout->addLayout(leftCol, 1);

  // RIGHT COLUMN
  QVBoxLayout *rightCol = new QVBoxLayout();
  rightCol->setSpacing(15);
  rightCol->addWidget(createWarmupSection());
  rightCol->addWidget(createAcIdleUpSection());
  rightCol->addWidget(createOutputLimitsSection());
  rightCol->addWidget(createLiveStatusSection());
  rightCol->addStretch();
  columnsLayout->addLayout(rightCol, 1);

  scrollLayout->addLayout(columnsLayout);
  scrollLayout->addStretch();

  scrollArea->setWidget(scrollContent);

  m_mainLayout->addWidget(scrollArea);
}

QWidget *IdleControlWidget::createTargetIdleSection() {
  QGroupBox *box = new QGroupBox("Target Idle Speed", this);
  box->setStyleSheet(R"(
        QGroupBox {
            font-size: 14px;
            font-weight: bold;
            color: white;
            border: 1px solid #444;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
    )");

  QVBoxLayout *layout = new QVBoxLayout(box);
  layout->setSpacing(10);

  // Target RPM slider
  QHBoxLayout *rpmRow = new QHBoxLayout();
  QLabel *label = new QLabel("Target RPM", this);
  label->setStyleSheet("color: #888;");
  m_targetRpmValue = new QLabel("850 rpm", this);
  m_targetRpmValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  rpmRow->addWidget(label);
  rpmRow->addStretch();
  rpmRow->addWidget(m_targetRpmValue);
  layout->addLayout(rpmRow);

  m_targetRpmSlider = createStyledSlider(500, 1500, 850);
  connect(m_targetRpmSlider, &QSlider::valueChanged, this,
          &IdleControlWidget::onTargetRpmChanged);
  layout->addWidget(m_targetRpmSlider);

  return box;
}

QWidget *IdleControlWidget::createPIDSection() {
  QGroupBox *box = new QGroupBox("PID Parameters", this);
  box->setStyleSheet(R"(
        QGroupBox {
            font-size: 14px;
            font-weight: bold;
            color: white;
            border: 1px solid #444;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
    )");

  QVBoxLayout *layout = new QVBoxLayout(box);
  layout->setSpacing(12);

  // P Gain
  QHBoxLayout *pRow = new QHBoxLayout();
  QLabel *pLabel = new QLabel("P Gain (Kp)", this);
  pLabel->setStyleSheet("color: #888;");
  m_pGainSpin = new QDoubleSpinBox(this);
  m_pGainSpin->setRange(0, 10);
  m_pGainSpin->setValue(0.150);
  m_pGainSpin->setSingleStep(0.01);
  m_pGainSpin->setDecimals(3);
  m_pGainSpin->setStyleSheet(
      "background-color: #1E1E1E; color: white; padding: 4px;");
  pRow->addWidget(pLabel);
  pRow->addStretch();
  pRow->addWidget(m_pGainSpin);
  layout->addLayout(pRow);

  m_pGainSlider = createStyledSlider(0, 1000, 150);
  connect(m_pGainSlider, &QSlider::valueChanged,
          [this](int val) { m_pGainSpin->setValue(val / 1000.0); });
  connect(m_pGainSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &IdleControlWidget::onPGainChanged);
  layout->addWidget(m_pGainSlider);

  // I Gain
  QHBoxLayout *iRow = new QHBoxLayout();
  QLabel *iLabel = new QLabel("I Gain (Ki)", this);
  iLabel->setStyleSheet("color: #888;");
  m_iGainSpin = new QDoubleSpinBox(this);
  m_iGainSpin->setRange(0, 10);
  m_iGainSpin->setValue(0.080);
  m_iGainSpin->setSingleStep(0.01);
  m_iGainSpin->setDecimals(3);
  m_iGainSpin->setStyleSheet(
      "background-color: #1E1E1E; color: white; padding: 4px;");
  iRow->addWidget(iLabel);
  iRow->addStretch();
  iRow->addWidget(m_iGainSpin);
  layout->addLayout(iRow);

  m_iGainSlider = createStyledSlider(0, 1000, 80);
  connect(m_iGainSlider, &QSlider::valueChanged,
          [this](int val) { m_iGainSpin->setValue(val / 1000.0); });
  connect(m_iGainSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &IdleControlWidget::onIGainChanged);
  layout->addWidget(m_iGainSlider);

  // D Gain
  QHBoxLayout *dRow = new QHBoxLayout();
  QLabel *dLabel = new QLabel("D Gain (Kd)", this);
  dLabel->setStyleSheet("color: #888;");
  m_dGainSpin = new QDoubleSpinBox(this);
  m_dGainSpin->setRange(0, 10);
  m_dGainSpin->setValue(0.050);
  m_dGainSpin->setSingleStep(0.01);
  m_dGainSpin->setDecimals(3);
  m_dGainSpin->setStyleSheet(
      "background-color: #1E1E1E; color: white; padding: 4px;");
  dRow->addWidget(dLabel);
  dRow->addStretch();
  dRow->addWidget(m_dGainSpin);
  layout->addLayout(dRow);

  m_dGainSlider = createStyledSlider(0, 1000, 50);
  connect(m_dGainSlider, &QSlider::valueChanged,
          [this](int val) { m_dGainSpin->setValue(val / 1000.0); });
  connect(m_dGainSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &IdleControlWidget::onDGainChanged);
  layout->addWidget(m_dGainSlider);

  // Tip label
  QLabel *tipLabel = new QLabel("TIP: Start with P=0.15, I=0.08, D=0.05 and "
                                "adjust based on idle stability",
                                this);
  tipLabel->setStyleSheet("color: #666; font-size: 10px; font-style: italic;");
  tipLabel->setWordWrap(true);
  layout->addWidget(tipLabel);

  return box;
}

QWidget *IdleControlWidget::createWarmupSection() {
  QGroupBox *box = new QGroupBox("Warmup Idle-Up", this);
  box->setStyleSheet(R"(
        QGroupBox {
            font-size: 14px;
            font-weight: bold;
            color: white;
            border: 1px solid #444;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
    )");

  QVBoxLayout *layout = new QVBoxLayout(box);
  layout->setSpacing(10);

  // Cold Start Adder
  QHBoxLayout *adderRow = new QHBoxLayout();
  QLabel *adderLabel = new QLabel("Cold Start Adder", this);
  adderLabel->setStyleSheet("color: #888;");
  m_coldStartAdderValue = new QLabel("+200 rpm", this);
  m_coldStartAdderValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  adderRow->addWidget(adderLabel);
  adderRow->addStretch();
  adderRow->addWidget(m_coldStartAdderValue);
  layout->addLayout(adderRow);

  m_coldStartAdderSlider = createStyledSlider(0, 500, 200);
  connect(m_coldStartAdderSlider, &QSlider::valueChanged, this,
          &IdleControlWidget::onColdStartAdderChanged);
  layout->addWidget(m_coldStartAdderSlider);

  QLabel *adderInfo =
      new QLabel("Additional RPM when coolant temp is below 80°C", this);
  adderInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(adderInfo);

  // Warmup Curve Preview
  QFrame *curveFrame = new QFrame(this);
  curveFrame->setFixedHeight(80);
  curveFrame->setStyleSheet(
      "background-color: #1E1E1E; border: 1px solid #444; border-radius: 4px;");

  QVBoxLayout *curveLayout = new QVBoxLayout(curveFrame);
  QLabel *curveTitle = new QLabel("Warmup Curve Preview", this);
  curveTitle->setStyleSheet("color: #888; font-size: 11px;");
  curveLayout->addWidget(curveTitle);

  QHBoxLayout *curvePoints = new QHBoxLayout();
  QStringList temps = {"-10°C", "20°C", "40°C", "80°C"};
  QStringList rpms = {"450 rpm", "372 rpm", "200 rpm", "0 rpm"};
  for (int i = 0; i < temps.size(); i++) {
    QVBoxLayout *pointLayout = new QVBoxLayout();
    QLabel *tempLabel = new QLabel(temps[i], this);
    tempLabel->setStyleSheet("color: #666; font-size: 10px;");
    tempLabel->setAlignment(Qt::AlignCenter);
    QLabel *rpmLabel = new QLabel(rpms[i], this);
    rpmLabel->setStyleSheet("color: #00BCD4; font-size: 11px;");
    rpmLabel->setAlignment(Qt::AlignCenter);
    pointLayout->addWidget(tempLabel);
    pointLayout->addWidget(rpmLabel);
    curvePoints->addLayout(pointLayout);
  }
  curveLayout->addLayout(curvePoints);

  layout->addWidget(curveFrame);

  return box;
}

QWidget *IdleControlWidget::createAcIdleUpSection() {
  QGroupBox *box = new QGroupBox("A/C Idle-Up", this);
  box->setStyleSheet(R"(
        QGroupBox {
            font-size: 14px;
            font-weight: bold;
            color: white;
            border: 1px solid #444;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
    )");

  QVBoxLayout *layout = new QVBoxLayout(box);
  layout->setSpacing(10);

  // A/C Compressor Adder
  QHBoxLayout *acRow = new QHBoxLayout();
  QLabel *acLabel = new QLabel("A/C Compressor Adder", this);
  acLabel->setStyleSheet("color: #888;");
  m_acAdderValue = new QLabel("+100 rpm", this);
  m_acAdderValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  acRow->addWidget(acLabel);
  acRow->addStretch();
  acRow->addWidget(m_acAdderValue);
  layout->addLayout(acRow);

  m_acAdderSlider = createStyledSlider(0, 300, 100);
  connect(m_acAdderSlider, &QSlider::valueChanged, this,
          &IdleControlWidget::onAcAdderChanged);
  layout->addWidget(m_acAdderSlider);

  QLabel *acInfo =
      new QLabel("Additional RPM when A/C compressor clutch is engaged", this);
  acInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(acInfo);

  return box;
}

QWidget *IdleControlWidget::createDashpotSection() {
  QGroupBox *box = new QGroupBox("Dashpot (Throttle Close Delay)", this);
  box->setStyleSheet(R"(
        QGroupBox {
            font-size: 14px;
            font-weight: bold;
            color: white;
            border: 1px solid #444;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
    )");

  QVBoxLayout *layout = new QVBoxLayout(box);
  layout->setSpacing(10);

  // Toggle
  QHBoxLayout *toggleRow = new QHBoxLayout();
  m_dashpotToggle = new QCheckBox("Enable", this);
  m_dashpotToggle->setChecked(true);
  m_dashpotToggle->setStyleSheet(
      "QCheckBox { color: white; } QCheckBox::indicator:checked { "
      "background-color: #00BCD4; }");
  connect(m_dashpotToggle, &QCheckBox::toggled, this,
          &IdleControlWidget::onDashpotToggled);
  toggleRow->addWidget(m_dashpotToggle);
  toggleRow->addStretch();
  layout->addLayout(toggleRow);

  // Decay Rate
  QHBoxLayout *decayRow = new QHBoxLayout();
  QLabel *decayLabel = new QLabel("Decay Rate", this);
  decayLabel->setStyleSheet("color: #888;");
  m_decayRateValue = new QLabel("5%/s", this);
  m_decayRateValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  decayRow->addWidget(decayLabel);
  decayRow->addStretch();
  decayRow->addWidget(m_decayRateValue);
  layout->addLayout(decayRow);

  m_decayRateSlider = createStyledSlider(1, 20, 5);
  connect(m_decayRateSlider, &QSlider::valueChanged, this,
          &IdleControlWidget::onDecayRateChanged);
  layout->addWidget(m_decayRateSlider);

  QLabel *dashpotInfo = new QLabel(
      "Controls how quickly idle valve closes when throttle is released", this);
  dashpotInfo->setStyleSheet("color: #666; font-size: 10px;");
  dashpotInfo->setWordWrap(true);
  layout->addWidget(dashpotInfo);

  return box;
}

QWidget *IdleControlWidget::createOutputLimitsSection() {
  QGroupBox *box = new QGroupBox("Output Limits", this);
  box->setStyleSheet(R"(
        QGroupBox {
            font-size: 14px;
            font-weight: bold;
            color: white;
            border: 1px solid #444;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
    )");

  QHBoxLayout *layout = new QHBoxLayout(box);
  layout->setSpacing(20);

  // Min Duty
  QVBoxLayout *minCol = new QVBoxLayout();
  QLabel *minLabel = new QLabel("Min Duty", this);
  minLabel->setStyleSheet("color: #888;");
  m_minDutySpin = new QSpinBox(this);
  m_minDutySpin->setRange(0, 100);
  m_minDutySpin->setValue(5);
  m_minDutySpin->setSuffix("%");
  m_minDutySpin->setStyleSheet("background-color: #1E1E1E; color: white; "
                               "padding: 8px; font-size: 14px;");
  minCol->addWidget(minLabel);
  minCol->addWidget(m_minDutySpin);
  layout->addLayout(minCol);

  // Max Duty
  QVBoxLayout *maxCol = new QVBoxLayout();
  QLabel *maxLabel = new QLabel("Max Duty", this);
  maxLabel->setStyleSheet("color: #888;");
  m_maxDutySpin = new QSpinBox(this);
  m_maxDutySpin->setRange(0, 100);
  m_maxDutySpin->setValue(95);
  m_maxDutySpin->setSuffix("%");
  m_maxDutySpin->setStyleSheet("background-color: #1E1E1E; color: white; "
                               "padding: 8px; font-size: 14px;");
  maxCol->addWidget(maxLabel);
  maxCol->addWidget(m_maxDutySpin);
  layout->addLayout(maxCol);

  layout->addStretch();

  return box;
}

QWidget *IdleControlWidget::createLiveStatusSection() {
  QFrame *box = new QFrame(this);
  box->setStyleSheet(
      "QFrame { background-color: rgba(0, 188, 212, 0.1); border: 1px solid "
      "#00BCD4; border-radius: 8px; padding: 10px; }");

  QVBoxLayout *layout = new QVBoxLayout(box);
  layout->setSpacing(8);

  QLabel *titleLabel = new QLabel("Live Status", this);
  titleLabel->setStyleSheet(
      "color: #00BCD4; font-size: 14px; font-weight: bold;");
  layout->addWidget(titleLabel);

  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(10);

  QLabel *currentRpmTitle = new QLabel("Current RPM", this);
  currentRpmTitle->setStyleSheet("color: #888;");
  m_currentRpmLabel = new QLabel("847 rpm", this);
  m_currentRpmLabel->setStyleSheet("color: #00BCD4; font-weight: bold;");
  grid->addWidget(currentRpmTitle, 0, 0);
  grid->addWidget(m_currentRpmLabel, 0, 1);

  QLabel *targetRpmTitle = new QLabel("Target RPM", this);
  targetRpmTitle->setStyleSheet("color: #888;");
  m_targetRpmLabel = new QLabel("850 rpm", this);
  m_targetRpmLabel->setStyleSheet("color: #00BCD4; font-weight: bold;");
  grid->addWidget(targetRpmTitle, 0, 2);
  grid->addWidget(m_targetRpmLabel, 0, 3);

  QLabel *iacDutyTitle = new QLabel("IAC Duty", this);
  iacDutyTitle->setStyleSheet("color: #888;");
  m_iacDutyLabel = new QLabel("--", this);
  m_iacDutyLabel->setStyleSheet("color: white;");
  grid->addWidget(iacDutyTitle, 1, 0);
  grid->addWidget(m_iacDutyLabel, 1, 1);

  QLabel *errorTitle = new QLabel("Error", this);
  errorTitle->setStyleSheet("color: #888;");
  m_errorLabel = new QLabel("--", this);
  m_errorLabel->setStyleSheet("color: white;");
  grid->addWidget(errorTitle, 1, 2);
  grid->addWidget(m_errorLabel, 1, 3);

  layout->addLayout(grid);

  return box;
}

QSlider *IdleControlWidget::createStyledSlider(int min, int max, int value) {
  QSlider *slider = new QSlider(Qt::Horizontal, this);
  slider->setRange(min, max);
  slider->setValue(value);
  slider->setStyleSheet(R"(
        QSlider::groove:horizontal {
            border: 1px solid #444;
            height: 8px;
            background: #1E1E1E;
            border-radius: 4px;
        }
        QSlider::handle:horizontal {
            background: #00BCD4;
            border: 1px solid #00ACC1;
            width: 18px;
            margin: -5px 0;
            border-radius: 9px;
        }
        QSlider::sub-page:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #00BCD4, stop:1 #00ACC1);
            border-radius: 4px;
        }
    )");
  return slider;
}

// Slot implementations
void IdleControlWidget::onTargetRpmChanged(int value) {
  m_targetRpmValue->setText(QString::number(value) + " rpm");
  m_targetRpmLabel->setText(QString::number(value) + " rpm");
  if (m_settingsManager) m_settingsManager->setValue("idleRPM", value);
}

void IdleControlWidget::onPGainChanged(double value) {
  m_pGainSlider->blockSignals(true);
  m_pGainSlider->setValue(static_cast<int>(value * 1000));
  m_pGainSlider->blockSignals(false);
  if (m_settingsManager) m_settingsManager->setValue("idlePGain", value);
}

void IdleControlWidget::onIGainChanged(double value) {
  m_iGainSlider->blockSignals(true);
  m_iGainSlider->setValue(static_cast<int>(value * 1000));
  m_iGainSlider->blockSignals(false);
  if (m_settingsManager) m_settingsManager->setValue("idleIGain", value);
}

void IdleControlWidget::onDGainChanged(double value) {
  m_dGainSlider->blockSignals(true);
  m_dGainSlider->setValue(static_cast<int>(value * 1000));
  m_dGainSlider->blockSignals(false);
  if (m_settingsManager) m_settingsManager->setValue("idleDGain", value);
}

void IdleControlWidget::onColdStartAdderChanged(int value) {
  m_coldStartAdderValue->setText("+" + QString::number(value) + " rpm");
  if (m_settingsManager) m_settingsManager->setValue("idleColdAdder", value);
}

void IdleControlWidget::onAcAdderChanged(int value) {
  m_acAdderValue->setText("+" + QString::number(value) + " rpm");
  if (m_settingsManager) m_settingsManager->setValue("idleAcAdder", value);
}

void IdleControlWidget::onDashpotToggled(bool enabled) {
  m_decayRateSlider->setEnabled(enabled);
  if (m_settingsManager) m_settingsManager->setValue("idleDashpotEn", enabled ? 1 : 0);
}

void IdleControlWidget::onDecayRateChanged(int value) {
  m_decayRateValue->setText(QString::number(value) + "%/s");
  if (m_settingsManager) m_settingsManager->setValue("idleDecayRate", value);
}

void IdleControlWidget::updateLiveStatus(int currentRpm, int targetRpm,
                                         double iacDuty, double error) {
  m_currentRpmLabel->setText(QString::number(currentRpm) + " rpm");
  m_targetRpmLabel->setText(QString::number(targetRpm) + " rpm");
  m_iacDutyLabel->setText(QString::number(iacDuty, 'f', 1) + "%");
  m_errorLabel->setText(QString::number(error, 'f', 1) + " rpm");
}



void IdleControlWidget::loadFromECU() {
  if (!m_settingsManager) return;
  
  QVariant v;
  v = m_settingsManager->getValue("idleRPM");
  if (v.isValid()) { m_targetRpmSlider->setValue(v.toInt()); }
  
  v = m_settingsManager->getValue("idlePGain");
  if (v.isValid()) { m_pGainSpin->setValue(v.toDouble()); }
  
  v = m_settingsManager->getValue("idleIGain");
  if (v.isValid()) { m_iGainSpin->setValue(v.toDouble()); }
  
  v = m_settingsManager->getValue("idleDGain");
  if (v.isValid()) { m_dGainSpin->setValue(v.toDouble()); }
  
  v = m_settingsManager->getValue("idleColdAdder");
  if (v.isValid()) { m_coldStartAdderSlider->setValue(v.toInt()); }
  
  v = m_settingsManager->getValue("idleAcAdder");
  if (v.isValid()) { m_acAdderSlider->setValue(v.toInt()); }
  
  v = m_settingsManager->getValue("idleDashpotEn");
  if (v.isValid()) { m_dashpotToggle->setChecked(v.toBool()); }
  
  v = m_settingsManager->getValue("idleDecayRate");
  if (v.isValid()) { m_decayRateSlider->setValue(v.toInt()); }
  
  v = m_settingsManager->getValue("idleMinDuty");
  if (v.isValid()) { m_minDutySpin->setValue(v.toInt()); }
  
  v = m_settingsManager->getValue("idleMaxDuty");
  if (v.isValid()) { m_maxDutySpin->setValue(v.toInt()); }
}

void IdleControlWidget::updateRealtimeData(const RealTimeData &data) {
  // Extract values for live status
  int currentRpm = data.rpm;
  // Note: CLIdleTarget is RPM * 10
  int targetRpm = data.CLIdleTarget * 10;
  // Note: idleLoad is in steps or duty%
  double iacDuty = data.idleLoad; 
  double error = currentRpm - targetRpm;
  
  updateLiveStatus(currentRpm, targetRpm, iacDuty, error);
}
