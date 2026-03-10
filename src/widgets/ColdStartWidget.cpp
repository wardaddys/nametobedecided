/**
 * @file ColdStartWidget.cpp
 * @brief Implementation of Cold Start & Warmup widget
 */

#include "ColdStartWidget.h"
#include "../core/ECUSettingsManager.h"
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QVBoxLayout>


ColdStartWidget::ColdStartWidget(QWidget *parent) : TuningWidgetBase(parent) {
  setupUi();
}

ColdStartWidget::~ColdStartWidget() {}

void ColdStartWidget::setupUi() {
  QScrollArea *scrollArea = new QScrollArea(this);
  scrollArea->setWidgetResizable(true);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setStyleSheet(
      "QScrollArea { background-color: #2B2B2B; border: none; }");

  QWidget *scrollContent = new QWidget();
  scrollContent->setStyleSheet("background-color: #2B2B2B;");
  QVBoxLayout *mainLayout = new QVBoxLayout(scrollContent);
  mainLayout->setSpacing(15);
  mainLayout->setContentsMargins(20, 20, 20, 20);

  // Header
  QLabel *titleLabel = new QLabel("Cold Start & Warmup", this);
  titleLabel->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: white;");
  mainLayout->addWidget(titleLabel);

  QLabel *subtitleLabel = new QLabel(
      "Configure fuel enrichment for cold starting and warmup conditions",
      this);
  subtitleLabel->setStyleSheet("font-size: 12px; color: #00BCD4;");
  mainLayout->addWidget(subtitleLabel);

  // Two column layout
  QHBoxLayout *columnsLayout = new QHBoxLayout();
  columnsLayout->setSpacing(20);

  // Left Column
  QVBoxLayout *leftCol = new QVBoxLayout();
  leftCol->setSpacing(15);
  leftCol->addWidget(createCrankingEnrichmentSection());
  leftCol->addWidget(createAfterstartEnrichmentSection());
  leftCol->addStretch();
  columnsLayout->addLayout(leftCol, 1);

  // Right Column
  QVBoxLayout *rightCol = new QVBoxLayout();
  rightCol->setSpacing(15);
  rightCol->addWidget(createWarmupEnrichmentSection());
  rightCol->addWidget(createEnrichmentCurveSection());
  rightCol->addWidget(createTuningTipsSection());
  rightCol->addStretch();
  columnsLayout->addLayout(rightCol, 1);

  mainLayout->addLayout(columnsLayout);
  mainLayout->addStretch();

  scrollArea->setWidget(scrollContent);

  m_mainLayout->addWidget(scrollArea);
}

QWidget *ColdStartWidget::createCrankingEnrichmentSection() {
  QGroupBox *box = new QGroupBox("Cranking Enrichment", this);
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

  // Cranking Fuel Multiplier
  QHBoxLayout *multRow = new QHBoxLayout();
  QLabel *multLabel = new QLabel("Cranking Fuel Multiplier", this);
  multLabel->setStyleSheet("color: #888;");
  m_crankingValue = new QLabel("1.50x", this);
  m_crankingValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  multRow->addWidget(multLabel);
  multRow->addStretch();
  multRow->addWidget(m_crankingValue);
  layout->addLayout(multRow);

  m_crankingMultiplier = createStyledSlider(100, 300, 150);
  connect(m_crankingMultiplier, &QSlider::valueChanged, [this](int val) {
    m_crankingValue->setText(QString::number(val / 100.0, 'f', 2) + "x");
    if (m_settingsManager) m_settingsManager->setValue("crankPwCold", val / 100.0);
  });
  layout->addWidget(m_crankingMultiplier);

  QLabel *multInfo =
      new QLabel("Fuel multiplier during engine cranking (RPM < 400)", this);
  multInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(multInfo);

  // Cranking by Temperature table
  QFrame *tableFrame = new QFrame(this);
  tableFrame->setStyleSheet("background-color: #1E1E1E; border: 1px solid "
                            "#444; border-radius: 4px; padding: 10px;");
  QVBoxLayout *tableLayout = new QVBoxLayout(tableFrame);

  QLabel *tableTitle = new QLabel("Cranking by Temperature", this);
  tableTitle->setStyleSheet("color: #888; font-size: 11px;");
  tableLayout->addWidget(tableTitle);

  QGridLayout *grid = new QGridLayout();
  grid->addWidget(new QLabel("Temp", this), 0, 0);
  QLabel *multHeader = new QLabel("Multiplier", this);
  multHeader->setAlignment(Qt::AlignRight);
  grid->addWidget(multHeader, 0, 1);

  QStringList temps = {"-20°C", "0°C", "20°C", "60°C", "80°C +"};
  QStringList mults = {"1.95%", "1.70%", "1.25%", "1.25%", "1.00%"};
  for (int i = 0; i < temps.size(); i++) {
    QLabel *tempL = new QLabel(temps[i], this);
    tempL->setStyleSheet("color: #888;");
    QLabel *multL = new QLabel(mults[i], this);
    multL->setStyleSheet("color: #00BCD4;");
    multL->setAlignment(Qt::AlignRight);
    grid->addWidget(tempL, i + 1, 0);
    grid->addWidget(multL, i + 1, 1);
  }
  tableLayout->addLayout(grid);

  layout->addWidget(tableFrame);

  return box;
}

QWidget *ColdStartWidget::createWarmupEnrichmentSection() {
  QGroupBox *box = new QGroupBox("Warmup Enrichment vs Temperature", this);
  box->setStyleSheet(R"(
        QGroupBox {
            font-size: 14px; font-weight: bold; color: white;
            border: 1px solid #444; border-radius: 8px;
            margin-top: 10px; padding-top: 10px;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }
    )");

  QVBoxLayout *layout = new QVBoxLayout(box);
  layout->setSpacing(8);

  QLabel *info = new QLabel(
      "Fuel enrichment during engine warmup, indexed by coolant temperature",
      this);
  info->setStyleSheet("color: #666; font-size: 10px;");
  info->setWordWrap(true);
  layout->addWidget(info);

  // Temperature sliders
  struct TempData {
    QString temp;
    int value;
    QString key;
  };
  QList<TempData> tempData = {{"-10°C", 110, "wuePctM40"}, {"0°C", 80, "wuePct0"},  {"20°C", 60, "wuePct20"},
                              {"40°C", 40, "wuePct40"},   {"60°C", 20, "wuePct60"}, {"70°C", 10, ""},
                              {"80°C", 0, ""}};

  QList<QSlider **> sliders = {&m_warmupMinus10, &m_warmup0,  &m_warmup20,
                               &m_warmup40,      &m_warmup60, &m_warmup70,
                               &m_warmup80};
  QList<QLabel **> labels = {&m_warmupMinus10Value, &m_warmup0Value,
                             &m_warmup20Value,      &m_warmup40Value,
                             &m_warmup60Value,      &m_warmup70Value,
                             &m_warmup80Value};

  for (int i = 0; i < tempData.size(); i++) {
    QHBoxLayout *row = new QHBoxLayout();

    QLabel *tempLabel = new QLabel(tempData[i].temp, this);
    tempLabel->setStyleSheet("color: #888;");
    tempLabel->setFixedWidth(50);

    *sliders[i] = createStyledSlider(0, 150, tempData[i].value);

    QString prefix = tempData[i].value > 0 ? "+" : "";
    *labels[i] =
        new QLabel(prefix + QString::number(tempData[i].value) + "%", this);
    (*labels[i])->setStyleSheet("color: #00BCD4; font-weight: bold;");
    (*labels[i])->setFixedWidth(50);
    (*labels[i])->setAlignment(Qt::AlignRight);

    QLabel *labelPtr = *labels[i];
    QString key = tempData[i].key;
    connect(*sliders[i], &QSlider::valueChanged, [this, labelPtr, key](int val) {
      QString p = val > 0 ? "+" : "";
      labelPtr->setText(p + QString::number(val) + "%");
      if (m_settingsManager && !key.isEmpty()) m_settingsManager->setValue(key, val + 100);
    });

    row->addWidget(tempLabel);
    row->addWidget(*sliders[i], 1);
    row->addWidget(*labels[i]);
    layout->addLayout(row);
  }

  return box;
}

QWidget *ColdStartWidget::createAfterstartEnrichmentSection() {
  QGroupBox *box = new QGroupBox("Afterstart Enrichment", this);
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

  // Initial Enrichment
  QHBoxLayout *initRow = new QHBoxLayout();
  QLabel *initLabel = new QLabel("Initial Enrichment", this);
  initLabel->setStyleSheet("color: #888;");
  m_initialEnrichValue = new QLabel("1.20%", this);
  m_initialEnrichValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  initRow->addWidget(initLabel);
  initRow->addStretch();
  initRow->addWidget(m_initialEnrichValue);
  layout->addLayout(initRow);

  m_initialEnrichment = createStyledSlider(100, 200, 120);
  connect(m_initialEnrichment, &QSlider::valueChanged, [this](int val) {
    m_initialEnrichValue->setText(QString::number(val / 100.0, 'f', 2) + "%");
    if (m_settingsManager) m_settingsManager->setValue("asePctCold", val - 100);
  });
  layout->addWidget(m_initialEnrichment);

  QLabel *initInfo =
      new QLabel("Extra fuel immediately after engine starts", this);
  initInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(initInfo);

  // Decay Time
  QHBoxLayout *decayRow = new QHBoxLayout();
  QLabel *decayLabel = new QLabel("Decay Time", this);
  decayLabel->setStyleSheet("color: #888;");
  m_decayTimeValue = new QLabel("3.0s", this);
  m_decayTimeValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  decayRow->addWidget(decayLabel);
  decayRow->addStretch();
  decayRow->addWidget(m_decayTimeValue);
  layout->addLayout(decayRow);

  m_decayTime = createStyledSlider(10, 100, 30);
  connect(m_decayTime, &QSlider::valueChanged, [this](int val) {
    m_decayTimeValue->setText(QString::number(val / 10.0, 'f', 1) + "s");
  });
  layout->addWidget(m_decayTime);

  QLabel *decayInfo =
      new QLabel("Time to taper afterstart enrichment to zero", this);
  decayInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(decayInfo);

  // Decay Profile
  QFrame *profileFrame = new QFrame(this);
  profileFrame->setFixedHeight(80);
  profileFrame->setStyleSheet(
      "background-color: #1E1E1E; border: 1px solid #444; border-radius: 4px;");
  QVBoxLayout *profileLayout = new QVBoxLayout(profileFrame);
  QLabel *profileTitle = new QLabel("Decay Profile", this);
  profileTitle->setStyleSheet("color: #888; font-size: 11px;");
  profileLayout->addWidget(profileTitle);

  QHBoxLayout *axisLabels = new QHBoxLayout();
  axisLabels->addWidget(new QLabel("0s", this));
  axisLabels->addStretch();
  axisLabels->addWidget(new QLabel("3s", this));
  profileLayout->addLayout(axisLabels);

  layout->addWidget(profileFrame);

  return box;
}

QWidget *ColdStartWidget::createEnrichmentCurveSection() {
  QGroupBox *box = new QGroupBox("Enrichment Curve", this);
  box->setStyleSheet(R"(
        QGroupBox {
            font-size: 14px; font-weight: bold; color: white;
            border: 1px solid #444; border-radius: 8px;
            margin-top: 10px; padding-top: 10px;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }
    )");

  QVBoxLayout *layout = new QVBoxLayout(box);

  // Graph placeholder
  QFrame *graphFrame = new QFrame(this);
  graphFrame->setFixedHeight(100);
  graphFrame->setStyleSheet(
      "background-color: #1E1E1E; border: 1px solid #444; border-radius: 4px;");
  QVBoxLayout *graphLayout = new QVBoxLayout(graphFrame);

  QLabel *yAxis = new QLabel("+80%", this);
  yAxis->setStyleSheet("color: #888; font-size: 10px;");
  graphLayout->addWidget(yAxis);

  graphLayout->addStretch();

  QHBoxLayout *xAxis = new QHBoxLayout();
  xAxis->addWidget(new QLabel("-10°C", this));
  xAxis->addStretch();
  xAxis->addWidget(new QLabel("80°C", this));
  graphLayout->addLayout(xAxis);

  layout->addWidget(graphFrame);

  return box;
}

QWidget *ColdStartWidget::createTuningTipsSection() {
  QFrame *box = new QFrame(this);
  box->setStyleSheet(
      "QFrame { background-color: rgba(0, 188, 212, 0.1); border: 1px solid "
      "#00BCD4; border-radius: 8px; padding: 10px; }");

  QVBoxLayout *layout = new QVBoxLayout(box);

  QLabel *titleLabel = new QLabel("Tuning Tips", this);
  titleLabel->setStyleSheet(
      "color: #00BCD4; font-size: 14px; font-weight: bold;");
  layout->addWidget(titleLabel);

  QLabel *tips = new QLabel("• Start with conservative enrichment values\n"
                            "• Monitor AFR during cold starts\n"
                            "• Adjust based on ambient temperature range\n"
                            "• Use wideband data logging for tuning",
                            this);
  tips->setStyleSheet("color: #00BCD4; font-size: 11px;");
  layout->addWidget(tips);

  return box;
}

QSlider *ColdStartWidget::createStyledSlider(int min, int max, int value) {
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

void ColdStartWidget::loadFromECU() {
  if (!m_settingsManager) return;
  QVariant v;
  v = m_settingsManager->getValue("crankPwCold");
  if (v.isValid()) { m_crankingMultiplier->setValue(static_cast<int>(v.toDouble() * 100)); }
  v = m_settingsManager->getValue("wuePctM40");
  if (v.isValid()) { m_warmupMinus10->setValue(v.toInt() - 100); }
  v = m_settingsManager->getValue("wuePct0");
  if (v.isValid()) { m_warmup0->setValue(v.toInt() - 100); }
  v = m_settingsManager->getValue("wuePct20");
  if (v.isValid()) { m_warmup20->setValue(v.toInt() - 100); }
  v = m_settingsManager->getValue("wuePct40");
  if (v.isValid()) { m_warmup40->setValue(v.toInt() - 100); }
  v = m_settingsManager->getValue("wuePct60");
  if (v.isValid()) { m_warmup60->setValue(v.toInt() - 100); }
  v = m_settingsManager->getValue("asePctCold");
  if (v.isValid()) { m_initialEnrichment->setValue(v.toInt() + 100); }
}
