/**
 * @file AccelEnrichmentWidget.cpp
 * @brief Implementation of Acceleration Enrichment widget
 */

#include "AccelEnrichmentWidget.h"
#include "../core/ECUSettingsManager.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QVBoxLayout>


AccelEnrichmentWidget::AccelEnrichmentWidget(QWidget *parent)
    : TuningWidgetBase(parent) {
  setupUi();
}

AccelEnrichmentWidget::~AccelEnrichmentWidget() {}

void AccelEnrichmentWidget::setupUi() {
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
  QLabel *titleLabel = new QLabel("Acceleration Enrichment", this);
  titleLabel->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: white;");
  scrollLayout->addWidget(titleLabel);

  QLabel *subtitleLabel = new QLabel(
      "Configure transient fuel compensation for throttle and load changes",
      this);
  subtitleLabel->setStyleSheet("font-size: 12px; color: #00BCD4;");
  scrollLayout->addWidget(subtitleLabel);

  // Two column layout
  QHBoxLayout *columnsLayout = new QHBoxLayout();
  columnsLayout->setSpacing(20);

  // Left Column
  QVBoxLayout *leftCol = new QVBoxLayout();
  leftCol->setSpacing(15);
  leftCol->addWidget(createTPSEnrichmentSection());
  leftCol->addStretch();
  columnsLayout->addLayout(leftCol, 1);

  // Right Column
  QVBoxLayout *rightCol = new QVBoxLayout();
  rightCol->setSpacing(15);
  rightCol->addWidget(createMAPEnrichmentSection());
  rightCol->addWidget(createDecelFuelCutSection());
  rightCol->addWidget(createLiveStatusSection());
  rightCol->addStretch();
  columnsLayout->addLayout(rightCol, 1);

  scrollLayout->addLayout(columnsLayout);
  scrollLayout->addStretch();

  scrollArea->setWidget(scrollContent);

  m_mainLayout->addWidget(scrollArea);
}

QWidget *AccelEnrichmentWidget::createTPSEnrichmentSection() {
  QGroupBox *box = new QGroupBox("TPS-Based Enrichment", this);
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
  m_tpsEnableToggle = new QCheckBox("Enable", this);
  m_tpsEnableToggle->setChecked(true);
  m_tpsEnableToggle->setStyleSheet("QCheckBox { color: white; }");
  layout->addWidget(m_tpsEnableToggle, 0, Qt::AlignRight);

  // TPS Rate Threshold
  QHBoxLayout *rateRow = new QHBoxLayout();
  QLabel *rateLabel = new QLabel("TPS Rate Threshold", this);
  rateLabel->setStyleSheet("color: #888;");
  m_tpsRateValue = new QLabel("50%/s", this);
  m_tpsRateValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  rateRow->addWidget(rateLabel);
  rateRow->addStretch();
  rateRow->addWidget(m_tpsRateValue);
  layout->addLayout(rateRow);

  m_tpsRateThreshold = createStyledSlider(10, 200, 50);
  connect(m_tpsRateThreshold, &QSlider::valueChanged, [this](int val) {
    m_tpsRateValue->setText(QString::number(val) + "%/s");
    if (m_settingsManager) m_settingsManager->setValue("aeThreshold", val);
  });
  layout->addWidget(m_tpsRateThreshold);

  QLabel *rateInfo =
      new QLabel("Minimum TPS change rate to trigger enrichment", this);
  rateInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(rateInfo);

  // Enrichment Amount
  QHBoxLayout *enrichRow = new QHBoxLayout();
  QLabel *enrichLabel = new QLabel("Enrichment Amount", this);
  enrichLabel->setStyleSheet("color: #888;");
  m_tpsEnrichValue = new QLabel("40%", this);
  m_tpsEnrichValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  enrichRow->addWidget(enrichLabel);
  enrichRow->addStretch();
  enrichRow->addWidget(m_tpsEnrichValue);
  layout->addLayout(enrichRow);

  m_tpsEnrichAmount = createStyledSlider(0, 100, 40);
  connect(m_tpsEnrichAmount, &QSlider::valueChanged, [this](int val) {
    m_tpsEnrichValue->setText(QString::number(val) + "%");
    if (m_settingsManager) m_settingsManager->setValue("aeMultiplier", val / 100.0);
  });
  layout->addWidget(m_tpsEnrichAmount);

  QLabel *enrichInfo =
      new QLabel("Fuel added during rapid throttle opening", this);
  enrichInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(enrichInfo);

  // Decay Time Constant
  QHBoxLayout *decayRow = new QHBoxLayout();
  QLabel *decayLabel = new QLabel("Decay Time Constant", this);
  decayLabel->setStyleSheet("color: #888;");
  m_tpsDecayValue = new QLabel("0.5s", this);
  m_tpsDecayValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  decayRow->addWidget(decayLabel);
  decayRow->addStretch();
  decayRow->addWidget(m_tpsDecayValue);
  layout->addLayout(decayRow);

  m_tpsDecayTime = createStyledSlider(1, 30, 5);
  connect(m_tpsDecayTime, &QSlider::valueChanged, [this](int val) {
    m_tpsDecayValue->setText(QString::number(val / 10.0, 'f', 1) + "s");
    if (m_settingsManager) m_settingsManager->setValue("aeDuration", val / 10.0);
  });
  layout->addWidget(m_tpsDecayTime);

  QLabel *decayInfo = new QLabel("How quickly enrichment tapers off", this);
  decayInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(decayInfo);

  // TPS Response Curve preview
  QFrame *curveFrame = new QFrame(this);
  curveFrame->setFixedHeight(100);
  curveFrame->setStyleSheet(
      "background-color: #1E1E1E; border: 1px solid #444; border-radius: 4px;");
  QVBoxLayout *curveLayout = new QVBoxLayout(curveFrame);
  QLabel *curveTitle = new QLabel("TPS Response Curve", this);
  curveTitle->setStyleSheet("color: #888; font-size: 11px;");
  curveLayout->addWidget(curveTitle);
  curveLayout->addStretch();
  layout->addWidget(curveFrame);

  // Enrichment vs TPS Rate table
  QLabel *tableTitle = new QLabel("Enrichment vs TPS Rate", this);
  tableTitle->setStyleSheet(
      "color: white; font-weight: bold; margin-top: 10px;");
  layout->addWidget(tableTitle);

  QGridLayout *tableGrid = new QGridLayout();
  tableGrid->addWidget(new QLabel("TPS Rate", this), 0, 0);
  tableGrid->addWidget(new QLabel("Fuel Add", this), 0, 1);

  QStringList rates = {"50%/s", "100%/s", "200%/s", "400%/s", "800%/s"};
  QStringList adds = {"1.0%", "2.4%", "4.0%", "5.6%", "8.0%"};
  for (int i = 0; i < rates.size(); i++) {
    QLabel *rateL = new QLabel(rates[i], this);
    rateL->setStyleSheet("color: #888;");
    QLabel *addL = new QLabel(adds[i], this);
    addL->setStyleSheet("color: #00BCD4;");
    addL->setAlignment(Qt::AlignRight);
    tableGrid->addWidget(rateL, i + 1, 0);
    tableGrid->addWidget(addL, i + 1, 1);
  }
  layout->addLayout(tableGrid);

  return box;
}

QWidget *AccelEnrichmentWidget::createMAPEnrichmentSection() {
  QGroupBox *box = new QGroupBox("MAP-Based Enrichment", this);
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
  m_mapEnableToggle = new QCheckBox("Enable", this);
  m_mapEnableToggle->setChecked(true);
  m_mapEnableToggle->setStyleSheet("QCheckBox { color: white; }");
  layout->addWidget(m_mapEnableToggle, 0, Qt::AlignRight);

  // MAP Rate Threshold
  QHBoxLayout *rateRow = new QHBoxLayout();
  QLabel *rateLabel = new QLabel("MAP Rate Threshold", this);
  rateLabel->setStyleSheet("color: #888;");
  m_mapRateValue = new QLabel("10 kPa/s", this);
  m_mapRateValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  rateRow->addWidget(rateLabel);
  rateRow->addStretch();
  rateRow->addWidget(m_mapRateValue);
  layout->addLayout(rateRow);

  m_mapRateThreshold = createStyledSlider(1, 50, 10);
  connect(m_mapRateThreshold, &QSlider::valueChanged, [this](int val) {
    m_mapRateValue->setText(QString::number(val) + " kPa/s");
  });
  layout->addWidget(m_mapRateThreshold);

  QLabel *rateInfo =
      new QLabel("Minimum MAP change rate to trigger enrichment", this);
  rateInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(rateInfo);

  // Enrichment Amount
  QHBoxLayout *enrichRow = new QHBoxLayout();
  QLabel *enrichLabel = new QLabel("Enrichment Amount", this);
  enrichLabel->setStyleSheet("color: #888;");
  m_mapEnrichValue = new QLabel("15%", this);
  m_mapEnrichValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  enrichRow->addWidget(enrichLabel);
  enrichRow->addStretch();
  enrichRow->addWidget(m_mapEnrichValue);
  layout->addLayout(enrichRow);

  m_mapEnrichAmount = createStyledSlider(0, 50, 15);
  connect(m_mapEnrichAmount, &QSlider::valueChanged, [this](int val) {
    m_mapEnrichValue->setText(QString::number(val) + "%");
  });
  layout->addWidget(m_mapEnrichAmount);

  QLabel *enrichInfo =
      new QLabel("Fuel added during rapid load increase", this);
  enrichInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(enrichInfo);

  // Decay Time
  QHBoxLayout *decayRow = new QHBoxLayout();
  QLabel *decayLabel = new QLabel("Decay Time Constant", this);
  decayLabel->setStyleSheet("color: #888;");
  m_mapDecayValue = new QLabel("0.4s", this);
  m_mapDecayValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  decayRow->addWidget(decayLabel);
  decayRow->addStretch();
  decayRow->addWidget(m_mapDecayValue);
  layout->addLayout(decayRow);

  m_mapDecayTime = createStyledSlider(1, 20, 4);
  connect(m_mapDecayTime, &QSlider::valueChanged, [this](int val) {
    m_mapDecayValue->setText(QString::number(val / 10.0, 'f', 1) + "s");
  });
  layout->addWidget(m_mapDecayTime);

  // Tip
  QFrame *tipFrame = new QFrame(this);
  tipFrame->setStyleSheet(
      "background-color: rgba(0, 188, 212, 0.1); border: 1px solid #00BCD4; "
      "border-radius: 4px; padding: 8px;");
  QHBoxLayout *tipLayout = new QHBoxLayout(tipFrame);
  QLabel *tipLabel = new QLabel(
      "TIP: MAP-based is better for turbo engines with slow TPS response",
      this);
  tipLabel->setStyleSheet("color: #00BCD4; font-size: 10px;");
  tipLayout->addWidget(tipLabel);
  layout->addWidget(tipFrame);

  return box;
}

QWidget *AccelEnrichmentWidget::createDecelFuelCutSection() {
  QGroupBox *box = new QGroupBox("Deceleration Fuel Cut", this);
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
  m_decelEnableToggle = new QCheckBox("Enable", this);
  m_decelEnableToggle->setChecked(true);
  m_decelEnableToggle->setStyleSheet("QCheckBox { color: white; }");
  layout->addWidget(m_decelEnableToggle, 0, Qt::AlignRight);

  // TPS Close Threshold
  QHBoxLayout *tpsRow = new QHBoxLayout();
  QLabel *tpsLabel = new QLabel("TPS Close Threshold", this);
  tpsLabel->setStyleSheet("color: #888;");
  m_decelTpsValue = new QLabel("10%/s", this);
  m_decelTpsValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  tpsRow->addWidget(tpsLabel);
  tpsRow->addStretch();
  tpsRow->addWidget(m_decelTpsValue);
  layout->addLayout(tpsRow);

  m_decelTpsThreshold = createStyledSlider(5, 50, 10);
  connect(m_decelTpsThreshold, &QSlider::valueChanged, [this](int val) {
    m_decelTpsValue->setText(QString::number(val) + "%/s");
    if (m_settingsManager) m_settingsManager->setValue("daeThreshold", val);
  });
  layout->addWidget(m_decelTpsThreshold);

  QLabel *tpsInfo =
      new QLabel("Rate of throttle closure to trigger fuel reduction", this);
  tpsInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(tpsInfo);

  // Min RPM and Min TPS
  QHBoxLayout *minRow = new QHBoxLayout();

  QVBoxLayout *minRpmCol = new QVBoxLayout();
  QLabel *minRpmLabel = new QLabel("Min RPM", this);
  minRpmLabel->setStyleSheet("color: #888;");
  m_decelMinRpm = new QSpinBox(this);
  m_decelMinRpm->setRange(500, 5000);
  m_decelMinRpm->setValue(1500);
  m_decelMinRpm->setSuffix(" rpm");
  m_decelMinRpm->setStyleSheet(
      "background-color: #1E1E1E; color: white; padding: 8px;");
  minRpmCol->addWidget(minRpmLabel);
  minRpmCol->addWidget(m_decelMinRpm);
  minRow->addLayout(minRpmCol);

  QVBoxLayout *minTpsCol = new QVBoxLayout();
  QLabel *minTpsLabel = new QLabel("Min TPS", this);
  minTpsLabel->setStyleSheet("color: #888;");
  m_decelMinTps = new QSpinBox(this);
  m_decelMinTps->setRange(0, 20);
  m_decelMinTps->setValue(2);
  m_decelMinTps->setSuffix("%");
  m_decelMinTps->setStyleSheet(
      "background-color: #1E1E1E; color: white; padding: 8px;");
  connect(m_decelMinTps, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val) {
    if (m_settingsManager) m_settingsManager->setValue("daeFuelCut", val);
  });
  minTpsCol->addWidget(minTpsLabel);
  minTpsCol->addWidget(m_decelMinTps);
  minRow->addLayout(minTpsCol);

  layout->addLayout(minRow);

  QLabel *minInfo = new QLabel(
      "Cuts fuel on deceleration to improve fuel economy and reduce emissions",
      this);
  minInfo->setStyleSheet("color: #666; font-size: 10px;");
  minInfo->setWordWrap(true);
  layout->addWidget(minInfo);

  return box;
}

QWidget *AccelEnrichmentWidget::createLiveStatusSection() {
  QFrame *box = new QFrame(this);
  box->setStyleSheet(
      "QFrame { background-color: rgba(0, 188, 212, 0.1); border: 1px solid "
      "#00BCD4; border-radius: 8px; padding: 10px; }");

  QVBoxLayout *layout = new QVBoxLayout(box);
  layout->setSpacing(8);

  QLabel *titleLabel = new QLabel("Live Enrichment Status", this);
  titleLabel->setStyleSheet(
      "color: #00BCD4; font-size: 14px; font-weight: bold;");
  layout->addWidget(titleLabel);

  QGridLayout *grid = new QGridLayout();
  grid->setSpacing(10);

  QLabel *tpsRateTitle = new QLabel("TPS Rate", this);
  tpsRateTitle->setStyleSheet("color: #888;");
  m_liveTpsRate = new QLabel("2.5%/s", this);
  m_liveTpsRate->setStyleSheet("color: white; font-weight: bold;");
  grid->addWidget(tpsRateTitle, 0, 0);
  grid->addWidget(m_liveTpsRate, 1, 0);

  QLabel *mapRateTitle = new QLabel("MAP Rate", this);
  mapRateTitle->setStyleSheet("color: #888;");
  m_liveMapRate = new QLabel("5.1 kPa/s", this);
  m_liveMapRate->setStyleSheet("color: white; font-weight: bold;");
  grid->addWidget(mapRateTitle, 0, 1);
  grid->addWidget(m_liveMapRate, 1, 1);

  QLabel *tpsEnrichTitle = new QLabel("TPS Enrich", this);
  tpsEnrichTitle->setStyleSheet("color: #888;");
  m_liveTpsEnrich = new QLabel("+0%", this);
  m_liveTpsEnrich->setStyleSheet("color: #00BCD4;");
  grid->addWidget(tpsEnrichTitle, 2, 0);
  grid->addWidget(m_liveTpsEnrich, 3, 0);

  QLabel *mapEnrichTitle = new QLabel("MAP Enrich", this);
  mapEnrichTitle->setStyleSheet("color: #888;");
  m_liveMapEnrich = new QLabel("+0%", this);
  m_liveMapEnrich->setStyleSheet("color: #00BCD4;");
  grid->addWidget(mapEnrichTitle, 2, 1);
  grid->addWidget(m_liveMapEnrich, 3, 1);

  layout->addLayout(grid);

  return box;
}

QSlider *AccelEnrichmentWidget::createStyledSlider(int min, int max,
                                                   int value) {
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

void AccelEnrichmentWidget::updateLiveStatus(double tpsRate, double mapRate,
                                             double tpsEnrich,
                                             double mapEnrich) {
  m_liveTpsRate->setText(QString::number(tpsRate, 'f', 1) + "%/s");
  m_liveMapRate->setText(QString::number(mapRate, 'f', 1) + " kPa/s");
  m_liveTpsEnrich->setText("+" + QString::number(tpsEnrich, 'f', 0) + "%");
  m_liveMapEnrich->setText("+" + QString::number(mapEnrich, 'f', 0) + "%");
}

void AccelEnrichmentWidget::loadFromECU() {
  if (!m_settingsManager) return;
  QVariant v;
  v = m_settingsManager->getValue("aeThreshold");
  if (v.isValid()) { m_tpsRateThreshold->setValue(v.toInt()); }
  v = m_settingsManager->getValue("aeMultiplier");
  if (v.isValid()) { m_tpsEnrichAmount->setValue(static_cast<int>(v.toDouble() * 100)); }
  v = m_settingsManager->getValue("aeDuration");
  if (v.isValid()) { m_tpsDecayTime->setValue(static_cast<int>(v.toDouble() * 10)); }
  v = m_settingsManager->getValue("daeThreshold");
  if (v.isValid()) { m_decelTpsThreshold->setValue(v.toInt()); }
  v = m_settingsManager->getValue("daeFuelCut");
  if (v.isValid()) { m_decelMinTps->setValue(v.toInt()); }
}

void AccelEnrichmentWidget::updateRealtimeData(const RealTimeData &data) {
  // Speeduino currently doesn't provide these specific granular rates back via secondary serial, 
  // but they can be inferred or derived if needed from TPS/MAP delta, or parsed out of the engine bitfields.
  // For now, mapping some relevant placeholders to show live data routing:
  updateLiveStatus(
      data.tpsDOT, 
      data.mapDOT, 
      data.aeAmount,
      data.aeAmount // MAP enrich uses the same variable in basic Speeduino implementation currently
  );
}
