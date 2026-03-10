/**
 * @file TrimsWidget.cpp
 * @brief Implementation of Fuel & Ignition Trims widget
 */

#include "TrimsWidget.h"
#include "../core/ECUSettingsManager.h"
#include <QGroupBox>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QVBoxLayout>


TrimsWidget::TrimsWidget(QWidget *parent) : TuningWidgetBase(parent) { setupUi(); }

TrimsWidget::~TrimsWidget() {}

void TrimsWidget::setupUi() {
  // Set dark background for this widget
  this->setStyleSheet("background-color: #2B2B2B;");

  m_mainLayout->setSpacing(15);
  m_mainLayout->setContentsMargins(20, 20, 20, 20);

  // Header
  QLabel *titleLabel = new QLabel("Fuel & Ignition Trims", this);
  titleLabel->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: white;");
  m_mainLayout->addWidget(titleLabel);

  QLabel *subtitleLabel = new QLabel(
      "Apply global and per-cylinder corrections to fuel and ignition timing",
      this);
  subtitleLabel->setStyleSheet("font-size: 12px; color: #00BCD4;");
  m_mainLayout->addWidget(subtitleLabel);

  // Tab Widget
  m_tabWidget = new QTabWidget(this);
  m_tabWidget->addTab(createFuelTrimsTab(), "Fuel Trims");
  m_tabWidget->addTab(createIgnitionTrimsTab(), "Ignition Trims");
  m_mainLayout->addWidget(m_tabWidget, 1);
}

QWidget *TrimsWidget::createFuelTrimsTab() {
  QWidget *tab = new QWidget(this);
  QVBoxLayout *tabLayout = new QVBoxLayout(tab);
  tabLayout->setSpacing(15);

  // Two column layout
  QHBoxLayout *columnsLayout = new QHBoxLayout();
  columnsLayout->setSpacing(20);

  // Left: Global Fuel Trim
  columnsLayout->addWidget(createGlobalTrimSection(), 1);

  // Right: Per-Cylinder Trims
  columnsLayout->addWidget(createPerCylinderSection(), 1);

  tabLayout->addLayout(columnsLayout);

  // Cylinder Fuel Balance
  tabLayout->addWidget(createCylinderBalanceSection());

  tabLayout->addStretch();

  return tab;
}

QWidget *TrimsWidget::createIgnitionTrimsTab() {
  QWidget *tab = new QWidget(this);
  QVBoxLayout *tabLayout = new QVBoxLayout(tab);
  tabLayout->setSpacing(15);

  QHBoxLayout *columnsLayout = new QHBoxLayout();
  columnsLayout->setSpacing(20);

  // Left: Global Ignition Trim
  QGroupBox *globalBox = new QGroupBox("Global Ignition Trim", this);
  globalBox->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; color: white; border: 1px solid #444; border-radius: 8px; margin-top: 10px; padding-top: 10px; } "
                           "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }");
  QVBoxLayout *gLayout = new QVBoxLayout(globalBox);
  
  QHBoxLayout *gRow = new QHBoxLayout();
  gRow->addWidget(new QLabel("All Cylinders Adjustment:", this));
  gRow->addStretch();
  m_globalIgnValue = new QLabel("+0°", this);
  m_globalIgnValue->setStyleSheet("color: #FF9800; font-weight: bold;");
  gRow->addWidget(m_globalIgnValue);
  gLayout->addLayout(gRow);

  m_globalIgnSlider = createStyledSlider(-10, 10, 0);
  connect(m_globalIgnSlider, &QSlider::valueChanged, [this](int val) {
      m_globalIgnValue->setText((val > 0 ? "+" : "") + QString::number(val) + "°");
      if (m_settingsManager) m_settingsManager->setValue("ignTrimGlobal", val);
  });
  gLayout->addWidget(m_globalIgnSlider);

  // Effect on Base Advance
  QFrame *efFrame = new QFrame(this);
  efFrame->setStyleSheet("background-color: #1E1E1E; border: 1px solid #444; border-radius: 4px; padding: 10px;");
  QVBoxLayout *efLayout = new QVBoxLayout(efFrame);
  efLayout->addWidget(new QLabel("Effect on Base Advance:", this));
  
  QHBoxLayout *baseRow = new QHBoxLayout();
  baseRow->addWidget(new QLabel("Base Advance:", this));
  baseRow->addStretch();
  m_baseAdvance = new QLabel("15.0°", this);
  m_baseAdvance->setStyleSheet("color: #FF9800; font-weight: bold;");
  baseRow->addWidget(m_baseAdvance);
  efLayout->addLayout(baseRow);

  QHBoxLayout *aftRow = new QHBoxLayout();
  aftRow->addWidget(new QLabel("After Trim:", this));
  aftRow->addStretch();
  m_afterIgnTrim = new QLabel("15.0°", this);
  m_afterIgnTrim->setStyleSheet("color: #FF9800; font-weight: bold;");
  aftRow->addWidget(m_afterIgnTrim);
  efLayout->addLayout(aftRow);

  gLayout->addWidget(efFrame);
  columnsLayout->addWidget(globalBox, 1);

  // Right: Per-Cylinder Trims
  QGroupBox *cylBox = new QGroupBox("Per-Cylinder Ignition Trims", this);
  cylBox->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; color: white; border: 1px solid #444; border-radius: 8px; margin-top: 10px; padding-top: 10px; } "
                        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }");
  QVBoxLayout *cylLayout = new QVBoxLayout(cylBox);

  auto addCylRow = [&](int cyl, QSlider*& slider, QLabel*& valLabel, const QString& settingName) {
      QHBoxLayout *row = new QHBoxLayout();
      row->addWidget(new QLabel(QString("Cylinder %1").arg(cyl), this));
      row->addStretch();
      valLabel = new QLabel("+0°", this);
      valLabel->setStyleSheet("color: #FF9800; font-weight: bold;");
      row->addWidget(valLabel);
      cylLayout->addLayout(row);
      slider = createStyledSlider(-5, 5, 0);
      connect(slider, &QSlider::valueChanged, [this, valLabel, settingName](int val) {
          valLabel->setText((val > 0 ? "+" : "") + QString::number(val) + "°");
          if (m_settingsManager) m_settingsManager->setValue(settingName, val);
      });
      cylLayout->addWidget(slider);
  };

  addCylRow(1, m_ignCyl1Slider, m_ignCyl1Value, "ignTrimCyl1");
  addCylRow(2, m_ignCyl2Slider, m_ignCyl2Value, "ignTrimCyl2");
  addCylRow(3, m_ignCyl3Slider, m_ignCyl3Value, "ignTrimCyl3");
  addCylRow(4, m_ignCyl4Slider, m_ignCyl4Value, "ignTrimCyl4");

  // NEW-006: Cylinders 5-8
  addCylRow(5, m_ignCyl5Slider, m_ignCyl5Value, "ignTrimCyl5");
  addCylRow(6, m_ignCyl6Slider, m_ignCyl6Value, "ignTrimCyl6");
  addCylRow(7, m_ignCyl7Slider, m_ignCyl7Value, "ignTrimCyl7");
  addCylRow(8, m_ignCyl8Slider, m_ignCyl8Value, "ignTrimCyl8");

  columnsLayout->addWidget(cylBox, 1);
  tabLayout->addLayout(columnsLayout);

  // Cylinder Advance Balance (vis)
  QGroupBox *balBox = new QGroupBox("Cylinder Advance Balance", this);
  balBox->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; color: white; border: 1px solid #444; border-radius: 8px; margin-top: 10px; padding-top: 10px; } "
                        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }");
  QHBoxLayout *balLayout = new QHBoxLayout(balBox);

  // Balance bars for all 8 cylinders
  QStringList cylNames = {"Cyl 1", "Cyl 2", "Cyl 3", "Cyl 4", "Cyl 5", "Cyl 6", "Cyl 7", "Cyl 8"};
  for (int i = 0; i < 8; i++) {
      QVBoxLayout *col = new QVBoxLayout();
      col->setAlignment(Qt::AlignBottom);
      QLabel *nLabel = new QLabel(cylNames[i], this);
      nLabel->setAlignment(Qt::AlignCenter);
      QFrame *bFrame = new QFrame(this);
      bFrame->setFixedSize(100, 100);
      bFrame->setStyleSheet("background-color: #1E1E1E; border: 1px solid #444; border-radius: 4px;");
      QVBoxLayout *bLayout = new QVBoxLayout(bFrame);
      bLayout->setContentsMargins(0, 0, 0, 0);
      bLayout->setSpacing(0);
      QLabel *vLabel = new QLabel("+0°", this);
      vLabel->setStyleSheet("color: white; font-weight: bold;");
      vLabel->setAlignment(Qt::AlignCenter);
      bLayout->addWidget(vLabel, 0, Qt::AlignCenter);
      bLayout->addStretch();
      QFrame *cBar = new QFrame(this);
      cBar->setFixedHeight(40);
      cBar->setStyleSheet("background-color: #FF9800; border-radius: 0 0 4px 4px;");
      bLayout->addWidget(cBar);
      col->addWidget(nLabel);
      col->addWidget(bFrame);
      balLayout->addLayout(col);
  }

  tabLayout->addWidget(balBox);
  tabLayout->addStretch();

  return tab;
}

QWidget *TrimsWidget::createGlobalTrimSection() {
  QGroupBox *box = new QGroupBox("Global Fuel Trim", this);
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

  // All Cylinders Adjustment
  QHBoxLayout *adjustRow = new QHBoxLayout();
  QLabel *adjustLabel = new QLabel("All Cylinders Adjustment", this);
  adjustLabel->setStyleSheet("color: #888;");
  m_globalFuelValue = new QLabel("+0%", this);
  m_globalFuelValue->setStyleSheet("color: #00BCD4; font-weight: bold;");
  adjustRow->addWidget(adjustLabel);
  adjustRow->addStretch();
  adjustRow->addWidget(m_globalFuelValue);
  layout->addLayout(adjustRow);

  m_globalFuelSlider = createStyledSlider(-20, 20, 0);
  connect(m_globalFuelSlider, &QSlider::valueChanged, [this](int val) {
    QString prefix = val >= 0 ? "+" : "";
    m_globalFuelValue->setText(prefix + QString::number(val) + "%");
  });
  layout->addWidget(m_globalFuelSlider);

  QLabel *adjustInfo =
      new QLabel("Applies to all cylinders across entire fuel map", this);
  adjustInfo->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(adjustInfo);

  // Effect on Base Fueling
  QFrame *effectFrame = new QFrame(this);
  effectFrame->setStyleSheet("background-color: #1E1E1E; border: 1px solid "
                             "#444; border-radius: 4px; padding: 10px;");
  QVBoxLayout *effectLayout = new QVBoxLayout(effectFrame);

  QLabel *effectTitle = new QLabel("Effect on Base Fueling", this);
  effectTitle->setStyleSheet("color: #888; font-size: 11px;");
  effectLayout->addWidget(effectTitle);

  QHBoxLayout *pulseRow = new QHBoxLayout();
  QLabel *pulseLabel = new QLabel("Base Pulse Width:", this);
  pulseLabel->setStyleSheet("color: #666;");
  m_basePulseWidth = new QLabel("10.5 ms", this);
  m_basePulseWidth->setStyleSheet("color: #00BCD4; font-weight: bold;");
  pulseRow->addWidget(pulseLabel);
  pulseRow->addStretch();
  pulseRow->addWidget(m_basePulseWidth);
  effectLayout->addLayout(pulseRow);

  QHBoxLayout *afterRow = new QHBoxLayout();
  QLabel *afterLabel = new QLabel("After Trim:", this);
  afterLabel->setStyleSheet("color: #666;");
  m_afterTrim = new QLabel("10.50 ms", this);
  m_afterTrim->setStyleSheet("color: #00BCD4; font-weight: bold;");
  afterRow->addWidget(afterLabel);
  afterRow->addStretch();
  afterRow->addWidget(m_afterTrim);
  effectLayout->addLayout(afterRow);

  layout->addWidget(effectFrame);

  // Tip
  QFrame *tipFrame = new QFrame(this);
  tipFrame->setStyleSheet(
      "background-color: rgba(0, 188, 212, 0.1); border: 1px solid #00BCD4; "
      "border-radius: 4px; padding: 8px;");
  QLabel *tipLabel = new QLabel(
      "TIP: Use global trim for quick AFR adjustments across all conditions",
      this);
  tipLabel->setStyleSheet("color: #00BCD4; font-size: 10px;");
  tipLabel->setWordWrap(true);
  QVBoxLayout *tipLayout = new QVBoxLayout(tipFrame);
  tipLayout->addWidget(tipLabel);
  layout->addWidget(tipFrame);

  return box;
}

QWidget *TrimsWidget::createPerCylinderSection() {
  QGroupBox *box = new QGroupBox("Per-Cylinder Fuel Trims", this);
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

  // Cylinder 1
  QHBoxLayout *cyl1Row = new QHBoxLayout();
  QLabel *cyl1Label = new QLabel("Cylinder 1", this);
  cyl1Label->setStyleSheet("color: #888;");
  m_cyl1Value = new QLabel("+0%", this);
  m_cyl1Value->setStyleSheet("color: #00BCD4; font-weight: bold;");
  cyl1Row->addWidget(cyl1Label);
  cyl1Row->addStretch();
  cyl1Row->addWidget(m_cyl1Value);
  layout->addLayout(cyl1Row);
  m_cyl1Slider = createStyledSlider(-10, 10, 0);
  connect(m_cyl1Slider, &QSlider::valueChanged, [this](int val) {
    QString prefix = val >= 0 ? "+" : "";
    m_cyl1Value->setText(prefix + QString::number(val) + "%");
    if (m_settingsManager) m_settingsManager->setValue("fuelTrimCyl1", val);
  });
  layout->addWidget(m_cyl1Slider);

  // Cylinder 2
  QHBoxLayout *cyl2Row = new QHBoxLayout();
  QLabel *cyl2Label = new QLabel("Cylinder 2", this);
  cyl2Label->setStyleSheet("color: #888;");
  m_cyl2Value = new QLabel("+0%", this);
  m_cyl2Value->setStyleSheet("color: #00BCD4; font-weight: bold;");
  cyl2Row->addWidget(cyl2Label);
  cyl2Row->addStretch();
  cyl2Row->addWidget(m_cyl2Value);
  layout->addLayout(cyl2Row);
  m_cyl2Slider = createStyledSlider(-10, 10, 0);
  connect(m_cyl2Slider, &QSlider::valueChanged, [this](int val) {
    QString prefix = val >= 0 ? "+" : "";
    m_cyl2Value->setText(prefix + QString::number(val) + "%");
    if (m_settingsManager) m_settingsManager->setValue("fuelTrimCyl2", val);
  });
  layout->addWidget(m_cyl2Slider);

  // Cylinder 3
  QHBoxLayout *cyl3Row = new QHBoxLayout();
  QLabel *cyl3Label = new QLabel("Cylinder 3", this);
  cyl3Label->setStyleSheet("color: #888;");
  m_cyl3Value = new QLabel("+1%", this);
  m_cyl3Value->setStyleSheet("color: #00BCD4; font-weight: bold;");
  cyl3Row->addWidget(cyl3Label);
  cyl3Row->addStretch();
  cyl3Row->addWidget(m_cyl3Value);
  layout->addLayout(cyl3Row);
  m_cyl3Slider = createStyledSlider(-10, 10, 1);
  connect(m_cyl3Slider, &QSlider::valueChanged, [this](int val) {
    QString prefix = val >= 0 ? "+" : "";
    m_cyl3Value->setText(prefix + QString::number(val) + "%");
    if (m_settingsManager) m_settingsManager->setValue("fuelTrimCyl3", val);
  });
  layout->addWidget(m_cyl3Slider);

  // Cylinder 4
  QHBoxLayout *cyl4Row = new QHBoxLayout();
  QLabel *cyl4Label = new QLabel("Cylinder 4", this);
  cyl4Label->setStyleSheet("color: #888;");
  m_cyl4Value = new QLabel("+0%", this);
  m_cyl4Value->setStyleSheet("color: #00BCD4; font-weight: bold;");
  cyl4Row->addWidget(cyl4Label);
  cyl4Row->addStretch();
  cyl4Row->addWidget(m_cyl4Value);
  layout->addLayout(cyl4Row);
  m_cyl4Slider = createStyledSlider(-10, 10, 0);
  connect(m_cyl4Slider, &QSlider::valueChanged, [this](int val) {
    QString prefix = val >= 0 ? "+" : "";
    m_cyl4Value->setText(prefix + QString::number(val) + "%");
    if (m_settingsManager) m_settingsManager->setValue("fuelTrimCyl4", val);
  });
  layout->addWidget(m_cyl4Slider);

  // Tip
  QFrame *tipFrame = new QFrame(this);
  tipFrame->setStyleSheet(
      "background-color: rgba(0, 188, 212, 0.1); border: 1px solid #00BCD4; "
      "border-radius: 4px; padding: 8px;");
  QLabel *tipLabel =
      new QLabel("TIP: Use cylinder trims to balance AFR between cylinders. "
                 "Check individual O2 sensors.",
                 this);
  tipLabel->setStyleSheet("color: #00BCD4; font-size: 10px;");
  tipLabel->setWordWrap(true);
  QVBoxLayout *tipLayout = new QVBoxLayout(tipFrame);
  tipLayout->addWidget(tipLabel);
  layout->addWidget(tipFrame);

  return box;
}

QWidget *TrimsWidget::createCylinderBalanceSection() {
  QGroupBox *box = new QGroupBox("Cylinder Fuel Balance", this);
  box->setStyleSheet(R"(
        QGroupBox {
            font-size: 14px; font-weight: bold; color: white;
            border: 1px solid #444; border-radius: 8px;
            margin-top: 10px; padding-top: 10px;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }
    )");

  QHBoxLayout *layout = new QHBoxLayout(box);
  layout->setSpacing(20);

  // Create 4 cylinder bars
  QStringList cylNames = {"Cyl 1", "Cyl 2", "Cyl 3", "Cyl 4"};
  QList<int> cylValues = {0, 0, 14, 0}; // Matching Figma

  for (int i = 0; i < 4; i++) {
    QVBoxLayout *cylCol = new QVBoxLayout();
    cylCol->setAlignment(Qt::AlignBottom);

    QLabel *nameLabel = new QLabel(cylNames[i], this);
    nameLabel->setStyleSheet("color: #888;");
    nameLabel->setAlignment(Qt::AlignCenter);

    // Bar frame
    QFrame *barFrame = new QFrame(this);
    barFrame->setFixedSize(100, 100);
    barFrame->setStyleSheet("background-color: #1E1E1E; border: 1px solid "
                            "#444; border-radius: 4px;");
    QVBoxLayout *barLayout = new QVBoxLayout(barFrame);
    barLayout->setContentsMargins(0, 0, 0, 0);
    barLayout->setSpacing(0);

    // Value label
    QLabel *valLabel =
        new QLabel("+" + QString::number(cylValues[i]) + "%", this);
    valLabel->setStyleSheet("color: white; font-weight: bold;");
    valLabel->setAlignment(Qt::AlignCenter);
    barLayout->addWidget(valLabel, 0, Qt::AlignCenter);

    barLayout->addStretch();

    // Colored bar
    QFrame *colorBar = new QFrame(this);
    int barHeight = 40 + cylValues[i] * 2;
    colorBar->setFixedHeight(barHeight);
    colorBar->setStyleSheet(
        "background-color: #00BCD4; border-radius: 0 0 4px 4px;");
    barLayout->addWidget(colorBar);

    cylCol->addWidget(nameLabel);
    cylCol->addWidget(barFrame);
    layout->addLayout(cylCol);
  }

  return box;
}

QSlider *TrimsWidget::createStyledSlider(int min, int max, int value) {
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

void TrimsWidget::loadFromECU() {
  if (!m_settingsManager) return;
  QVariant v;
  v = m_settingsManager->getValue("fuelTrimCyl1");
  if (v.isValid()) { m_cyl1Slider->setValue(v.toInt()); }
  v = m_settingsManager->getValue("fuelTrimCyl2");
  if (v.isValid()) { m_cyl2Slider->setValue(v.toInt()); }
  v = m_settingsManager->getValue("fuelTrimCyl3");
  if (v.isValid()) { m_cyl3Slider->setValue(v.toInt()); }
  v = m_settingsManager->getValue("fuelTrimCyl4");
  if (v.isValid()) { m_cyl4Slider->setValue(v.toInt()); }
}
