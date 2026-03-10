/**
 * @file VVTVTECWidget.cpp
 * @brief Implementation of VVT/VTEC Control widget - Figma Design Match
 */

#include "VVTVTECWidget.h"
#include "../core/ECUSettingsManager.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QScrollArea>
#include <QVBoxLayout>

VVTVTECWidget::VVTVTECWidget(QWidget *parent) : TuningWidgetBase(parent) { setupUi(); }

VVTVTECWidget::~VVTVTECWidget() {}

void VVTVTECWidget::setupUi() {
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

  // Header with title and enable toggle
  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *titleLabel = new QLabel("VVT/VTEC Control", this);
  titleLabel->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: white;");
  headerLayout->addWidget(titleLabel);
  headerLayout->addStretch();

  // Enable VTEC/Dual Maps toggle
  QHBoxLayout *enableLayout = new QHBoxLayout();
  QLabel *enableLabel = new QLabel("Enable VTEC/Dual Maps", this);
  enableLabel->setStyleSheet("color: #888;");
  m_enableToggle = new QCheckBox(this);
  m_enableToggle->setChecked(true);
  m_enableToggle->setStyleSheet(R"(
        QCheckBox::indicator {
            width: 40px; height: 20px; border-radius: 10px;
            background-color: #444;
        }
        QCheckBox::indicator:checked {
            background-color: #00BCD4;
        }
    )");
  connect(m_enableToggle, &QCheckBox::toggled, this,
          &VVTVTECWidget::onEnableToggled);
  enableLayout->addWidget(enableLabel);
  enableLayout->addWidget(m_enableToggle);
  headerLayout->addLayout(enableLayout);

  scrollLayout->addLayout(headerLayout);

  // Two column layout for VTEC Engagement and VVT Control
  QHBoxLayout *columnsLayout = new QHBoxLayout();
  columnsLayout->setSpacing(20);
  columnsLayout->addWidget(createVtecEngagementSection(), 1);
  columnsLayout->addWidget(createVvtControlSection(), 1);
  scrollLayout->addLayout(columnsLayout);

  // VVT Advance Map
  scrollLayout->addWidget(createAdvanceMapSection());
  scrollLayout->addStretch();

  scrollArea->setWidget(scrollContent);

  m_mainLayout->addWidget(scrollArea);
}

QWidget *VVTVTECWidget::createVtecEngagementSection() {
  QGroupBox *box = new QGroupBox("VTEC Engagement", this);
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

  // Engagement RPM
  QHBoxLayout *rpmRow = new QHBoxLayout();
  QLabel *rpmLabel = new QLabel("Engagement RPM:", this);
  rpmLabel->setStyleSheet("color: #888;");
  m_engagementRpmValue = new QLabel("5800", this);
  m_engagementRpmValue->setStyleSheet(
      "color: #00BCD4; font-weight: bold; background-color: #1E1E1E; padding: "
      "4px 8px; border-radius: 4px;");
  m_engagementRpmValue->setFixedWidth(80);
  m_engagementRpmValue->setAlignment(Qt::AlignCenter);
  rpmRow->addWidget(rpmLabel);
  rpmRow->addStretch();
  rpmRow->addWidget(m_engagementRpmValue);
  layout->addLayout(rpmRow);

  m_engagementRpmSlider = createStyledSlider(3000, 8000, 5800);
  connect(m_engagementRpmSlider, &QSlider::valueChanged, this,
          &VVTVTECWidget::onEngagementRpmChanged);
  layout->addWidget(m_engagementRpmSlider);

  // Hysteresis
  QHBoxLayout *hystRow = new QHBoxLayout();
  QLabel *hystLabel = new QLabel("Hysteresis:", this);
  hystLabel->setStyleSheet("color: #888;");
  m_hysteresisSpin = new QSpinBox(this);
  m_hysteresisSpin->setRange(0, 500);
  m_hysteresisSpin->setValue(200);
  m_hysteresisSpin->setSuffix(" rpm");
  m_hysteresisSpin->setStyleSheet(
      "background-color: #1E1E1E; color: #00BCD4; padding: 4px; font-weight: "
      "bold;");
  connect(m_hysteresisSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &VVTVTECWidget::onHysteresisChanged);
  hystRow->addWidget(hystLabel);
  hystRow->addStretch();
  hystRow->addWidget(m_hysteresisSpin);
  layout->addLayout(hystRow);

  return box;
}

QWidget *VVTVTECWidget::createVvtControlSection() {
  QGroupBox *box = new QGroupBox("VVT Control", this);
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

  // Max Advance
  QHBoxLayout *maxRow = new QHBoxLayout();
  QLabel *maxLabel = new QLabel("Max Advance:", this);
  maxLabel->setStyleSheet("color: #888;");
  m_maxAdvanceSpin = new QSpinBox(this);
  m_maxAdvanceSpin->setRange(0, 60);
  m_maxAdvanceSpin->setValue(50);
  m_maxAdvanceSpin->setSuffix("°");
  m_maxAdvanceSpin->setStyleSheet(
      "background-color: #1E1E1E; color: #00BCD4; padding: 4px; font-weight: "
      "bold;");
  connect(m_maxAdvanceSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &VVTVTECWidget::onMaxAdvanceChanged);
  maxRow->addWidget(maxLabel);
  maxRow->addStretch();
  maxRow->addWidget(m_maxAdvanceSpin);
  layout->addLayout(maxRow);

  // Min Advance
  QHBoxLayout *minRow = new QHBoxLayout();
  QLabel *minLabel = new QLabel("Min Advance:", this);
  minLabel->setStyleSheet("color: #888;");
  m_minAdvanceSpin = new QSpinBox(this);
  m_minAdvanceSpin->setRange(-30, 30);
  m_minAdvanceSpin->setValue(0);
  m_minAdvanceSpin->setSuffix("°");
  m_minAdvanceSpin->setStyleSheet(
      "background-color: #1E1E1E; color: #00BCD4; padding: 4px; font-weight: "
      "bold;");
  connect(m_minAdvanceSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &VVTVTECWidget::onMinAdvanceChanged);
  minRow->addWidget(minLabel);
  minRow->addStretch();
  minRow->addWidget(m_minAdvanceSpin);
  layout->addLayout(minRow);

  layout->addStretch();

  return box;
}

QWidget *VVTVTECWidget::createAdvanceMapSection() {
  QGroupBox *box = new QGroupBox("VVT Advance Map (Degrees)", this);
  box->setStyleSheet(R"(
        QGroupBox {
            font-size: 14px; font-weight: bold; color: white;
            border: 1px solid #444; border-radius: 8px;
            margin-top: 10px; padding-top: 10px;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }
    )");

  QVBoxLayout *layout = new QVBoxLayout(box);

  m_advanceMapTable = new QTableWidget(6, 6, this);

  // RPM header
  QStringList hHeaders = {"20", "40", "60", "80", "100"};
  hHeaders.prepend("RPM/MAP");
  m_advanceMapTable->setHorizontalHeaderLabels(hHeaders);

  // MAP (vertical) values
  QStringList mapValues = {"2000", "3000", "4000", "5000", "6000", "7000"};

  // Initialize with zeros until real ECU data is loaded
  int sampleData[6][6] = {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0},
                          {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0},
                          {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}};

  for (int row = 0; row < 6; row++) {
    QTableWidgetItem *vHeader = new QTableWidgetItem(mapValues[row]);
    m_advanceMapTable->setVerticalHeaderItem(row, vHeader);

    for (int col = 0; col < 6; col++) {
      QTableWidgetItem *item;
      if (col == 0) {
        item = new QTableWidgetItem(mapValues[row]);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setBackground(QColor("#1E1E1E"));
      } else {
        item = new QTableWidgetItem(QString::number(sampleData[row][col - 1]));
      }
      item->setTextAlignment(Qt::AlignCenter);
      item->setForeground(QColor("#00BCD4"));
      m_advanceMapTable->setItem(row, col, item);
    }
  }

  m_advanceMapTable->setStyleSheet(R"(
        QTableWidget {
            background-color: #1E1E1E; gridline-color: #444;
            color: #00BCD4; border: 1px solid #444; border-radius: 4px;
        }
        QTableWidget::item { padding: 8px; }
        QTableWidget::item:selected { background-color: #00BCD4; color: black; }
        QHeaderView::section {
            background-color: #2B2B2B; color: #888;
            border: 1px solid #444; padding: 6px;
        }
    )");

  m_advanceMapTable->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  m_advanceMapTable->verticalHeader()->setVisible(false);
  m_advanceMapTable->setFixedHeight(250);

  layout->addWidget(m_advanceMapTable);

  return box;
}

QSlider *VVTVTECWidget::createStyledSlider(int min, int max, int value) {
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

void VVTVTECWidget::onEngagementRpmChanged(int value) {
  m_engagementRpmValue->setText(QString::number(value));
  if (m_settingsManager) m_settingsManager->setValue("vtecEngageRpm", value);
}

void VVTVTECWidget::onHysteresisChanged(int value) {
  if (m_settingsManager) m_settingsManager->setValue("vtecDisenRpm", m_engagementRpmSlider->value() - value);
}

void VVTVTECWidget::onMaxAdvanceChanged(int value) {
  if (m_settingsManager) m_settingsManager->setValue("vvtTargetAngle", value);
}

void VVTVTECWidget::onMinAdvanceChanged(int value) { Q_UNUSED(value); }

void VVTVTECWidget::onEnableToggled(bool enabled) {
  m_engagementRpmSlider->setEnabled(enabled);
  m_hysteresisSpin->setEnabled(enabled);
  m_maxAdvanceSpin->setEnabled(enabled);
  m_minAdvanceSpin->setEnabled(enabled);
  m_advanceMapTable->setEnabled(enabled);
  if (m_settingsManager) m_settingsManager->setValue("vtecEnabled", enabled ? 1 : 0);
}

void VVTVTECWidget::loadFromECU() {
  if (!m_settingsManager) return;
  QVariant v;
  v = m_settingsManager->getValue("vtecEnabled");
  if (v.isValid()) { m_enableToggle->setChecked(v.toBool()); }
  v = m_settingsManager->getValue("vtecEngageRpm");
  if (v.isValid()) { m_engagementRpmSlider->setValue(v.toInt()); }
  v = m_settingsManager->getValue("vvtTargetAngle");
  if (v.isValid()) { m_maxAdvanceSpin->setValue(v.toInt()); }
}
