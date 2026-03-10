/**
 * @file KnockWidget.cpp
 * @brief Implementation of Knock Detection widget - Figma Design Match
 */

#include "KnockWidget.h"
#include "../core/ECUSettingsManager.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QVBoxLayout>

KnockWidget::KnockWidget(QWidget *parent) : TuningWidgetBase(parent) { setupUi(); }

KnockWidget::~KnockWidget() {}

void KnockWidget::setupUi() {
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

  // Header
  QLabel *titleLabel = new QLabel("Knock Detection", this);
  titleLabel->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: white;");
  scrollLayout->addWidget(titleLabel);

  QLabel *subtitleLabel = new QLabel(
      "Configure knock sensor detection and automatic timing retard protection",
      this);
  subtitleLabel->setStyleSheet("font-size: 12px; color: #00BCD4;");
  scrollLayout->addWidget(subtitleLabel);

  // Two column layout
  QHBoxLayout *columnsLayout = new QHBoxLayout();
  columnsLayout->setSpacing(20);

  // Left column
  QVBoxLayout *leftCol = new QVBoxLayout();
  leftCol->setSpacing(15);
  leftCol->addWidget(createKnockDetectionSection());
  leftCol->addWidget(createTimingRetardSection());
  leftCol->addStretch();
  columnsLayout->addLayout(leftCol, 1);

  // Right column
  QVBoxLayout *rightCol = new QVBoxLayout();
  rightCol->setSpacing(15);
  rightCol->addWidget(createLiveSensorSection());
  rightCol->addWidget(createActiveRetardSection());
  rightCol->addStretch();
  columnsLayout->addLayout(rightCol, 1);

  scrollLayout->addLayout(columnsLayout);
  scrollLayout->addStretch();

  scrollArea->setWidget(scrollContent);

  m_mainLayout->addWidget(scrollArea);
}

QWidget *KnockWidget::createKnockDetectionSection() {
  QGroupBox *box = new QGroupBox("Knock Detection", this);
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

  // Enable toggle
  QHBoxLayout *enableRow = new QHBoxLayout();
  enableRow->addStretch();
  m_knockEnableToggle = new QCheckBox(this);
  m_knockEnableToggle->setChecked(true);
  m_knockEnableToggle->setStyleSheet(R"(
        QCheckBox::indicator { width: 40px; height: 20px; border-radius: 10px; background-color: #444; }
        QCheckBox::indicator:checked { background-color: #00BCD4; }
    )");
  connect(m_knockEnableToggle, &QCheckBox::toggled, this,
          &KnockWidget::onKnockEnableToggled);
  enableRow->addWidget(m_knockEnableToggle);
  layout->addLayout(enableRow);

  // Detection Threshold
  QHBoxLayout *threshRow = new QHBoxLayout();
  QLabel *threshLabel = new QLabel("Detection Threshold", this);
  threshLabel->setStyleSheet("color: #888;");
  m_thresholdValue = new QLabel("45 counts", this);
  m_thresholdValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  threshRow->addWidget(threshLabel);
  threshRow->addStretch();
  threshRow->addWidget(m_thresholdValue);
  layout->addLayout(threshRow);

  m_thresholdSlider = createStyledSlider(0, 100, 45);
  connect(m_thresholdSlider, &QSlider::valueChanged, this,
          &KnockWidget::onThresholdChanged);
  layout->addWidget(m_thresholdSlider);

  QLabel *threshInfo =
      new QLabel("Sensor value above which knock is detected", this);
  threshInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(threshInfo);

  // Window Start / End
  QHBoxLayout *windowRow = new QHBoxLayout();

  QVBoxLayout *startCol = new QVBoxLayout();
  QLabel *startLabel = new QLabel("Window Start", this);
  startLabel->setStyleSheet("color: #888;");
  m_windowStartSlider = createStyledSlider(0, 90, 15);
  m_windowStartSlider->setFixedWidth(150);
  connect(m_windowStartSlider, &QSlider::valueChanged, this,
          &KnockWidget::onWindowStartChanged);
  m_windowStartValue = new QLabel("15°", this);
  m_windowStartValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  startCol->addWidget(startLabel);
  startCol->addWidget(m_windowStartSlider);
  startCol->addWidget(m_windowStartValue);
  windowRow->addLayout(startCol);

  QVBoxLayout *endCol = new QVBoxLayout();
  QLabel *endLabel = new QLabel("Window End", this);
  endLabel->setStyleSheet("color: #888;");
  m_windowEndSlider = createStyledSlider(0, 90, 45);
  m_windowEndSlider->setFixedWidth(150);
  connect(m_windowEndSlider, &QSlider::valueChanged, this,
          &KnockWidget::onWindowEndChanged);
  m_windowEndValue = new QLabel("45°", this);
  m_windowEndValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  endCol->addWidget(endLabel);
  endCol->addWidget(m_windowEndSlider);
  endCol->addWidget(m_windowEndValue);
  windowRow->addLayout(endCol);

  layout->addLayout(windowRow);

  QLabel *windowInfo = new QLabel(
      "Crank angle window (ATDC) for knock detection sampling", this);
  windowInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(windowInfo);

  // Detection Window visual
  QFrame *windowFrame = new QFrame(this);
  windowFrame->setFixedHeight(80);
  windowFrame->setStyleSheet(
      "background-color: #1E1E1E; border: 1px solid #444; border-radius: 4px;");
  QVBoxLayout *windowLayout = new QVBoxLayout(windowFrame);
  QLabel *windowTitle = new QLabel("Detection Window", this);
  windowTitle->setStyleSheet("color: #888; font-size: 11px;");
  windowLayout->addWidget(windowTitle);

  // Simple visualization
  QLabel *tdc = new QLabel("TDC", this);
  tdc->setStyleSheet("color: #00BCD4; font-size: 10px;");
  windowLayout->addWidget(tdc);
  windowLayout->addStretch();

  m_detectionWindow = windowFrame;
  layout->addWidget(m_detectionWindow);

  return box;
}

QWidget *KnockWidget::createLiveSensorSection() {
  QGroupBox *box = new QGroupBox("Live Knock Sensor Levels", this);
  box->setStyleSheet(R"(
        QGroupBox {
            font-size: 14px; font-weight: bold; color: white;
            border: 1px solid #00BCD4; border-radius: 8px;
            margin-top: 10px; padding-top: 10px;
            background-color: rgba(0, 188, 212, 0.05);
        }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }
    )");

  QVBoxLayout *layout = new QVBoxLayout(box);
  layout->setSpacing(12);

  // Sensor 1
  QHBoxLayout *s1Row = new QHBoxLayout();
  QLabel *s1Label = new QLabel("Sensor 1", this);
  s1Label->setStyleSheet("color: #888;");
  m_sensor1Bar = new QProgressBar(this);
  m_sensor1Bar->setRange(0, 100);
  m_sensor1Bar->setValue(23);
  m_sensor1Bar->setTextVisible(false);
  m_sensor1Bar->setStyleSheet(R"(
        QProgressBar { border: none; border-radius: 4px; background-color: #1E1E1E; }
        QProgressBar::chunk { background-color: #00BCD4; border-radius: 4px; }
    )");
  m_sensor1Value = new QLabel("23 counts", this);
  m_sensor1Value->setStyleSheet("color: #00BCD4; font-weight: bold;");
  s1Row->addWidget(s1Label);
  s1Row->addWidget(m_sensor1Bar, 1);
  s1Row->addWidget(m_sensor1Value);
  layout->addLayout(s1Row);

  // Sensor 2
  QHBoxLayout *s2Row = new QHBoxLayout();
  QLabel *s2Label = new QLabel("Sensor 2", this);
  s2Label->setStyleSheet("color: #888;");
  m_sensor2Bar = new QProgressBar(this);
  m_sensor2Bar->setRange(0, 100);
  m_sensor2Bar->setValue(42);
  m_sensor2Bar->setTextVisible(false);
  m_sensor2Bar->setStyleSheet(R"(
        QProgressBar { border: none; border-radius: 4px; background-color: #1E1E1E; }
        QProgressBar::chunk { background-color: #00BCD4; border-radius: 4px; }
    )");
  m_sensor2Value = new QLabel("42 counts", this);
  m_sensor2Value->setStyleSheet("color: #00BCD4; font-weight: bold;");
  s2Row->addWidget(s2Label);
  s2Row->addWidget(m_sensor2Bar, 1);
  s2Row->addWidget(m_sensor2Value);
  layout->addLayout(s2Row);

  // Sensor 3
  QHBoxLayout *s3Row = new QHBoxLayout();
  QLabel *s3Label = new QLabel("Sensor 3", this);
  s3Label->setStyleSheet("color: #888;");
  m_sensor3Bar = new QProgressBar(this);
  m_sensor3Bar->setRange(0, 100);
  m_sensor3Bar->setValue(18);
  m_sensor3Bar->setTextVisible(false);
  m_sensor3Bar->setStyleSheet(R"(
        QProgressBar { border: none; border-radius: 4px; background-color: #1E1E1E; }
        QProgressBar::chunk { background-color: #00BCD4; border-radius: 4px; }
    )");
  m_sensor3Value = new QLabel("18 counts", this);
  m_sensor3Value->setStyleSheet("color: #00BCD4; font-weight: bold;");
  s3Row->addWidget(s3Label);
  s3Row->addWidget(m_sensor3Bar, 1);
  s3Row->addWidget(m_sensor3Value);
  layout->addLayout(s3Row);

  // Sensor 4
  QHBoxLayout *s4Row = new QHBoxLayout();
  QLabel *s4Label = new QLabel("Sensor 4", this);
  s4Label->setStyleSheet("color: #888;");
  m_sensor4Bar = new QProgressBar(this);
  m_sensor4Bar->setRange(0, 100);
  m_sensor4Bar->setValue(31);
  m_sensor4Bar->setTextVisible(false);
  m_sensor4Bar->setStyleSheet(R"(
        QProgressBar { border: none; border-radius: 4px; background-color: #1E1E1E; }
        QProgressBar::chunk { background-color: #00BCD4; border-radius: 4px; }
    )");
  m_sensor4Value = new QLabel("31 counts", this);
  m_sensor4Value->setStyleSheet("color: #00BCD4; font-weight: bold;");
  s4Row->addWidget(s4Label);
  s4Row->addWidget(m_sensor4Bar, 1);
  s4Row->addWidget(m_sensor4Value);
  layout->addLayout(s4Row);

  // Threshold indicator
  QFrame *threshFrame = new QFrame(this);
  threshFrame->setStyleSheet("background-color: #1E1E1E; border: 1px solid "
                             "#444; border-radius: 4px; padding: 4px;");
  QHBoxLayout *threshLayout = new QHBoxLayout(threshFrame);
  QLabel *threshIcon = new QLabel("⬜", this);
  threshIcon->setStyleSheet("color: #FF5722;");
  QLabel *threshText = new QLabel("Threshold: 45 counts", this);
  threshText->setStyleSheet("color: #888;");
  threshLayout->addWidget(threshIcon);
  threshLayout->addWidget(threshText);
  threshLayout->addStretch();
  layout->addWidget(threshFrame);

  return box;
}

QWidget *KnockWidget::createTimingRetardSection() {
  QGroupBox *box = new QGroupBox("Timing Retard Response", this);
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

  // Maximum Retard
  QHBoxLayout *maxRow = new QHBoxLayout();
  QLabel *maxLabel = new QLabel("Maximum Retard", this);
  maxLabel->setStyleSheet("color: #888;");
  m_maxRetardValue = new QLabel("8°", this);
  m_maxRetardValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  maxRow->addWidget(maxLabel);
  maxRow->addStretch();
  maxRow->addWidget(m_maxRetardValue);
  layout->addLayout(maxRow);

  m_maxRetardSlider = createStyledSlider(0, 20, 8);
  connect(m_maxRetardSlider, &QSlider::valueChanged, this,
          &KnockWidget::onMaxRetardChanged);
  layout->addWidget(m_maxRetardSlider);

  QLabel *maxInfo =
      new QLabel("Maximum timing retard allowed per cylinder", this);
  maxInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(maxInfo);

  // Retard Step Size
  QHBoxLayout *stepRow = new QHBoxLayout();
  QLabel *stepLabel = new QLabel("Retard Step Size", this);
  stepLabel->setStyleSheet("color: #888;");
  m_retardStepValue = new QLabel("2°", this);
  m_retardStepValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  stepRow->addWidget(stepLabel);
  stepRow->addStretch();
  stepRow->addWidget(m_retardStepValue);
  layout->addLayout(stepRow);

  m_retardStepSlider = createStyledSlider(1, 10, 2);
  connect(m_retardStepSlider, &QSlider::valueChanged, this,
          &KnockWidget::onRetardStepChanged);
  layout->addWidget(m_retardStepSlider);

  QLabel *stepInfo =
      new QLabel("Amount to retard timing per knock event", this);
  stepInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(stepInfo);

  // Recovery Rate
  QHBoxLayout *recRow = new QHBoxLayout();
  QLabel *recLabel = new QLabel("Recovery Rate", this);
  recLabel->setStyleSheet("color: #888;");
  m_recoveryRateValue = new QLabel("0.5°/s", this);
  m_recoveryRateValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  recRow->addWidget(recLabel);
  recRow->addStretch();
  recRow->addWidget(m_recoveryRateValue);
  layout->addLayout(recRow);

  m_recoveryRateSlider = createStyledSlider(1, 20, 5);
  connect(m_recoveryRateSlider, &QSlider::valueChanged, this,
          &KnockWidget::onRecoveryRateChanged);
  layout->addWidget(m_recoveryRateSlider);

  QLabel *recInfo =
      new QLabel("Rate timing advances back after knock clears", this);
  recInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(recInfo);

  // Retard Response Profile placeholder
  QFrame *profileFrame = new QFrame(this);
  profileFrame->setFixedHeight(60);
  profileFrame->setStyleSheet(
      "background-color: #1E1E1E; border: 1px solid #444; border-radius: 4px;");
  QVBoxLayout *profileLayout = new QVBoxLayout(profileFrame);
  QLabel *profileTitle = new QLabel("Retard Response Profile", this);
  profileTitle->setStyleSheet("color: #888; font-size: 11px;");
  profileLayout->addWidget(profileTitle);
  QLabel *profileLegend = new QLabel("● Knock    ● Recovery", this);
  profileLegend->setStyleSheet("color: #00BCD4; font-size: 10px;");
  profileLayout->addWidget(profileLegend);
  layout->addWidget(profileFrame);

  return box;
}

QWidget *KnockWidget::createActiveRetardSection() {
  QGroupBox *box = new QGroupBox("Active Timing Retard", this);
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

  // Cylinder grid
  QGridLayout *cylGrid = new QGridLayout();
  cylGrid->setSpacing(10);

  // Cylinder 1
  QFrame *cyl1Frame = new QFrame(this);
  cyl1Frame->setStyleSheet(
      "background-color: #1E1E1E; border: 1px solid #444; border-radius: 8px;");
  QVBoxLayout *cyl1Layout = new QVBoxLayout(cyl1Frame);
  QLabel *cyl1Title = new QLabel("Cylinder 1", this);
  cyl1Title->setStyleSheet("color: #888; font-size: 11px;");
  cyl1Title->setAlignment(Qt::AlignCenter);
  m_cyl1Retard = new QLabel("0°", this);
  m_cyl1Retard->setStyleSheet(
      "color: #00BCD4; font-size: 24px; font-weight: bold;");
  m_cyl1Retard->setAlignment(Qt::AlignCenter);
  cyl1Layout->addWidget(cyl1Title);
  cyl1Layout->addWidget(m_cyl1Retard);
  cylGrid->addWidget(cyl1Frame, 0, 0);

  // Cylinder 2
  QFrame *cyl2Frame = new QFrame(this);
  cyl2Frame->setStyleSheet(
      "background-color: #1E1E1E; border: 1px solid #444; border-radius: 8px;");
  QVBoxLayout *cyl2Layout = new QVBoxLayout(cyl2Frame);
  QLabel *cyl2Title = new QLabel("Cylinder 2", this);
  cyl2Title->setStyleSheet("color: #888; font-size: 11px;");
  cyl2Title->setAlignment(Qt::AlignCenter);
  m_cyl2Retard = new QLabel("0°", this);
  m_cyl2Retard->setStyleSheet(
      "color: #00BCD4; font-size: 24px; font-weight: bold;");
  m_cyl2Retard->setAlignment(Qt::AlignCenter);
  cyl2Layout->addWidget(cyl2Title);
  cyl2Layout->addWidget(m_cyl2Retard);
  cylGrid->addWidget(cyl2Frame, 0, 1);

  // Cylinder 3
  QFrame *cyl3Frame = new QFrame(this);
  cyl3Frame->setStyleSheet(
      "background-color: #1E1E1E; border: 1px solid #444; border-radius: 8px;");
  QVBoxLayout *cyl3Layout = new QVBoxLayout(cyl3Frame);
  QLabel *cyl3Title = new QLabel("Cylinder 3", this);
  cyl3Title->setStyleSheet("color: #888; font-size: 11px;");
  cyl3Title->setAlignment(Qt::AlignCenter);
  m_cyl3Retard = new QLabel("0°", this);
  m_cyl3Retard->setStyleSheet(
      "color: #00BCD4; font-size: 24px; font-weight: bold;");
  m_cyl3Retard->setAlignment(Qt::AlignCenter);
  cyl3Layout->addWidget(cyl3Title);
  cyl3Layout->addWidget(m_cyl3Retard);
  cylGrid->addWidget(cyl3Frame, 1, 0);

  // Cylinder 4
  QFrame *cyl4Frame = new QFrame(this);
  cyl4Frame->setStyleSheet(
      "background-color: #1E1E1E; border: 1px solid #444; border-radius: 8px;");
  QVBoxLayout *cyl4Layout = new QVBoxLayout(cyl4Frame);
  QLabel *cyl4Title = new QLabel("Cylinder 4", this);
  cyl4Title->setStyleSheet("color: #888; font-size: 11px;");
  cyl4Title->setAlignment(Qt::AlignCenter);
  m_cyl4Retard = new QLabel("0°", this);
  m_cyl4Retard->setStyleSheet(
      "color: #00BCD4; font-size: 24px; font-weight: bold;");
  m_cyl4Retard->setAlignment(Qt::AlignCenter);
  cyl4Layout->addWidget(cyl4Title);
  cyl4Layout->addWidget(m_cyl4Retard);
  cylGrid->addWidget(cyl4Frame, 1, 1);

  layout->addLayout(cylGrid);

  // Total Knock Events
  QHBoxLayout *eventsRow = new QHBoxLayout();
  eventsRow->addStretch();
  QLabel *eventsLabel = new QLabel("Total Knock Events (Session)", this);
  eventsLabel->setStyleSheet("color: #888;");
  m_totalKnockEvents = new QLabel("0", this);
  m_totalKnockEvents->setStyleSheet(
      "color: white; font-size: 16px; font-weight: bold;");
  eventsRow->addWidget(eventsLabel);
  eventsRow->addWidget(m_totalKnockEvents);
  eventsRow->addStretch();
  layout->addLayout(eventsRow);

  return box;
}

QSlider *KnockWidget::createStyledSlider(int min, int max, int value) {
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

void KnockWidget::onKnockEnableToggled(bool enabled) {
  if (m_settingsManager) m_settingsManager->setValue("knockEnabled", enabled ? 1 : 0);
}

void KnockWidget::onThresholdChanged(int value) {
  m_thresholdValue->setText(QString::number(value) + " counts");
  if (m_settingsManager) m_settingsManager->setValue("knockThreshold", value);
}

void KnockWidget::onWindowStartChanged(int value) {
  m_windowStartValue->setText(QString::number(value) + "°");
  if (m_settingsManager) m_settingsManager->setValue("knockWindowStart", value);
}

void KnockWidget::onWindowEndChanged(int value) {
  m_windowEndValue->setText(QString::number(value) + "°");
  if (m_settingsManager) m_settingsManager->setValue("knockWindowEnd", value);
}

void KnockWidget::onMaxRetardChanged(int value) {
  m_maxRetardValue->setText(QString::number(value) + "°");
  if (m_settingsManager) m_settingsManager->setValue("knockMaxRetard", value);
}

void KnockWidget::onRetardStepChanged(int value) {
  m_retardStepValue->setText(QString::number(value) + "°");
  if (m_settingsManager) m_settingsManager->setValue("knockRetardDeg", value);
}

void KnockWidget::onRecoveryRateChanged(int value) {
  m_recoveryRateValue->setText(QString::number(value / 10.0, 'f', 1) + "°/s");
  if (m_settingsManager) m_settingsManager->setValue("knockRecovery", value / 10.0);
}

void KnockWidget::updateSensorLevels(int s1, int s2, int s3, int s4) {
  m_sensor1Bar->setValue(s1);
  m_sensor1Value->setText(QString::number(s1) + " counts");
  m_sensor2Bar->setValue(s2);
  m_sensor2Value->setText(QString::number(s2) + " counts");
  m_sensor3Bar->setValue(s3);
  m_sensor3Value->setText(QString::number(s3) + " counts");
  m_sensor4Bar->setValue(s4);
  m_sensor4Value->setText(QString::number(s4) + " counts");
}

void KnockWidget::updateTimingRetard(double cyl1, double cyl2, double cyl3,
                                     double cyl4) {
  m_cyl1Retard->setText(QString::number(cyl1, 'f', 0) + "°");
  m_cyl2Retard->setText(QString::number(cyl2, 'f', 0) + "°");
  m_cyl3Retard->setText(QString::number(cyl3, 'f', 0) + "°");
  m_cyl4Retard->setText(QString::number(cyl4, 'f', 0) + "°");
}

void KnockWidget::loadFromECU() {
  if (!m_settingsManager) return;
  QVariant v;
  v = m_settingsManager->getValue("knockEnabled");
  if (v.isValid()) { m_knockEnableToggle->setChecked(v.toBool()); }
  v = m_settingsManager->getValue("knockThreshold");
  if (v.isValid()) { m_thresholdSlider->setValue(v.toInt()); }
  v = m_settingsManager->getValue("knockWindowStart");
  if (v.isValid()) { m_windowStartSlider->setValue(v.toInt()); }
  v = m_settingsManager->getValue("knockWindowEnd");
  if (v.isValid()) { m_windowEndSlider->setValue(v.toInt()); }
  v = m_settingsManager->getValue("knockMaxRetard");
  if (v.isValid()) { m_maxRetardSlider->setValue(v.toInt()); }
  v = m_settingsManager->getValue("knockRetardDeg");
  if (v.isValid()) { m_retardStepSlider->setValue(v.toInt()); }
  v = m_settingsManager->getValue("knockRecovery");
  if (v.isValid()) { m_recoveryRateSlider->setValue(static_cast<int>(v.toDouble() * 10)); }
}

void KnockWidget::updateRealtimeData(const RealTimeData &data) {
  // Update live sensor levels from the knock array if available
  // The exact struct might just trace "knockRetard" for total retard applied
  updateSensorLevels(
      data.knockCount, // Using knock count as proxy for general activity if discrete sensors aren't available
      0, 0, 0          // Only single channel knock supported universally in basic firmware
  );
  
  updateTimingRetard(
      data.knockRetard,
      data.knockRetard,
      data.knockRetard,
      data.knockRetard
  );
}
