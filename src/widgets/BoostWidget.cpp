/**
 * @file BoostWidget.cpp
 * @brief Implementation of Boost Control widget - Figma Design Match
 */

#include "BoostWidget.h"
#include "../core/ECUSettingsManager.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QScrollArea>
#include <QVBoxLayout>

BoostWidget::BoostWidget(QWidget *parent) : TuningWidgetBase(parent) { setupUi(); }

BoostWidget::~BoostWidget() {}

void BoostWidget::setupUi() {
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

  // Header Section
  scrollLayout->addWidget(createHeaderSection());

  // Boost Target Section
  scrollLayout->addWidget(createBoostTargetSection());

  // Table and 3D view side by side
  QHBoxLayout *contentLayout = new QHBoxLayout();
  contentLayout->setSpacing(15);
  contentLayout->addWidget(createTableSection(), 2);
  contentLayout->addWidget(create3DViewSection(), 1);
  scrollLayout->addLayout(contentLayout);

  scrollLayout->addStretch();

  scrollArea->setWidget(scrollContent);

  m_mainLayout->addWidget(scrollArea);
}

QWidget *BoostWidget::createHeaderSection() {
  QWidget *header = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout(header);
  layout->setSpacing(10);

  // Title row
  QHBoxLayout *titleRow = new QHBoxLayout();
  QLabel *titleLabel =
      new QLabel("Comprehensive Boost & Forced Induction Settings", this);
  titleLabel->setStyleSheet(
      "font-size: 20px; font-weight: bold; color: white;");
  titleRow->addWidget(titleLabel);
  titleRow->addStretch();

  // Live Tuning badge
  QFrame *liveBadge = new QFrame(this);
  liveBadge->setStyleSheet(
      "background-color: #E53935; border-radius: 4px; padding: 4px 10px;");
  QHBoxLayout *badgeLayout = new QHBoxLayout(liveBadge);
  badgeLayout->setContentsMargins(8, 4, 8, 4);
  QLabel *liveLabel = new QLabel("● LIVE TUNING", this);
  liveLabel->setStyleSheet("color: white; font-weight: bold; font-size: 11px;");
  badgeLayout->addWidget(liveLabel);
  titleRow->addWidget(liveBadge);

  layout->addLayout(titleRow);

  // Subtitle
  QLabel *subtitle = new QLabel(
      "Professional-grade boost control with newbie-friendly guidance", this);
  subtitle->setStyleSheet("color: #00BCD4; font-size: 12px;");
  layout->addWidget(subtitle);

  // Forced Induction Type and Setup Completion
  QHBoxLayout *controlRow = new QHBoxLayout();

  // Induction Type
  QVBoxLayout *typeCol = new QVBoxLayout();
  QLabel *typeLabel = new QLabel("Forced Induction Type:", this);
  typeLabel->setStyleSheet("color: #888;");
  m_inductionTypeCombo = new QComboBox(this);
  m_inductionTypeCombo->addItems(
      {"Single Turbo", "Twin Turbo", "Supercharger", "Centrifugal SC"});
  m_inductionTypeCombo->setStyleSheet(
      "QComboBox { background-color: #1E1E1E; color: white; padding: 8px; border: 1px solid #444; border-radius: 4px; }"
      "QComboBox QAbstractItemView { background-color: #1E1E1E; color: white; selection-background-color: #00BCD4; selection-color: black; border: 1px solid #444; }");
  connect(m_inductionTypeCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &BoostWidget::onInductionTypeChanged);
  typeCol->addWidget(typeLabel);
  typeCol->addWidget(m_inductionTypeCombo);
  controlRow->addLayout(typeCol);

  controlRow->addSpacing(30);

  // Setup Completion
  QVBoxLayout *progressCol = new QVBoxLayout();
  QLabel *progressLabel = new QLabel("Setup Completion:", this);
  progressLabel->setStyleSheet("color: #888;");
  QHBoxLayout *progressRow = new QHBoxLayout();
  m_setupProgress = new QProgressBar(this);
  m_setupProgress->setRange(0, 100);
  m_setupProgress->setValue(100);
  m_setupProgress->setTextVisible(true);
  m_setupProgress->setFormat("%p%");
  m_setupProgress->setStyleSheet(R"(
        QProgressBar {
            border: 1px solid #444; border-radius: 4px;
            background-color: #1E1E1E; text-align: center; color: white;
        }
        QProgressBar::chunk { background-color: #4CAF50; border-radius: 3px; }
    )");
  m_setupProgress->setFixedWidth(200);
  QLabel *readyLabel = new QLabel("✓ Ready for dyno tuning", this);
  readyLabel->setStyleSheet("color: #4CAF50; font-size: 11px;");
  progressRow->addWidget(m_setupProgress);
  progressRow->addWidget(readyLabel);
  progressCol->addWidget(progressLabel);
  progressCol->addLayout(progressRow);
  controlRow->addLayout(progressCol);

  controlRow->addStretch();
  layout->addLayout(controlRow);

  // Configuration info
  QHBoxLayout *infoRow = new QHBoxLayout();

  // Configuration column
  QVBoxLayout *configCol = new QVBoxLayout();
  QLabel *configTitle = new QLabel("Configuration:", this);
  configTitle->setStyleSheet("color: #666; font-size: 11px;");
  QLabel *configValue = new QLabel("Single Turbo", this);
  configValue->setStyleSheet("color: white; font-size: 12px;");
  QLabel *configDesc =
      new QLabel("One big fat pushing air in a single strong push", this);
  configDesc->setStyleSheet("color: #00BCD4; font-size: 10px;");
  configCol->addWidget(configTitle);
  configCol->addWidget(configValue);
  configCol->addWidget(configDesc);
  infoRow->addLayout(configCol);

  // Key Differences column
  QVBoxLayout *diffCol = new QVBoxLayout();
  QLabel *diffTitle = new QLabel("Key Differences:", this);
  diffTitle->setStyleSheet("color: #666; font-size: 11px;");
  QLabel *diffValue =
      new QLabel("Balanced spool, focus on lag management", this);
  diffValue->setStyleSheet("color: white; font-size: 12px;");
  diffCol->addWidget(diffTitle);
  diffCol->addWidget(diffValue);
  diffCol->addStretch();
  infoRow->addLayout(diffCol);

  // Defaults column
  QVBoxLayout *defaultsCol = new QVBoxLayout();
  QLabel *defaultsTitle = new QLabel("Defaults:", this);
  defaultsTitle->setStyleSheet("color: #666; font-size: 11px;");
  QLabel *defaultsVal1 = new QLabel("Target: 10 psi", this);
  defaultsVal1->setStyleSheet("color: white; font-size: 12px;");
  QLabel *defaultsVal2 = new QLabel("Range: 30 kPa/s", this);
  defaultsVal2->setStyleSheet("color: white; font-size: 12px;");
  defaultsCol->addWidget(defaultsTitle);
  defaultsCol->addWidget(defaultsVal1);
  defaultsCol->addWidget(defaultsVal2);
  infoRow->addLayout(defaultsCol);

  // Complexity column
  QVBoxLayout *complexCol = new QVBoxLayout();
  QLabel *complexTitle = new QLabel("Complexity:", this);
  complexTitle->setStyleSheet("color: #666; font-size: 11px;");
  QLabel *complexBadge = new QLabel("BEGINNER", this);
  complexBadge->setStyleSheet(
      "background-color: #4CAF50; color: white; padding: 4px 12px; "
      "border-radius: 4px; font-weight: bold;");
  complexCol->addWidget(complexTitle);
  complexCol->addWidget(complexBadge);
  complexCol->addStretch();
  infoRow->addLayout(complexCol);

  infoRow->addStretch();
  layout->addLayout(infoRow);

  return header;
}

QWidget *BoostWidget::createBoostTargetSection() {
  QGroupBox *box = new QGroupBox("", this);
  box->setStyleSheet("QGroupBox { border: none; }");

  QVBoxLayout *layout = new QVBoxLayout(box);
  layout->setSpacing(10);

  // Title row with Novice Mode
  QHBoxLayout *titleRow = new QHBoxLayout();

  QHBoxLayout *boostTargetRow = new QHBoxLayout();
  QLabel *icon = new QLabel("⚙", this);
  icon->setStyleSheet("color: #00BCD4; font-size: 16px;");
  QLabel *titleLabel = new QLabel("Boost Target", this);
  titleLabel->setStyleSheet(
      "color: white; font-size: 16px; font-weight: bold;");
  boostTargetRow->addWidget(icon);
  boostTargetRow->addWidget(titleLabel);
  titleRow->addLayout(boostTargetRow);

  titleRow->addSpacing(20);

  QLabel *noviceLabel = new QLabel("Novice Mode:", this);
  noviceLabel->setStyleSheet("color: #888;");
  m_noviceModeToggle = new QCheckBox(this);
  m_noviceModeToggle->setStyleSheet(R"(
        QCheckBox::indicator { width: 40px; height: 20px; border-radius: 10px; background-color: #444; }
        QCheckBox::indicator:checked { background-color: #00BCD4; }
    )");
  connect(m_noviceModeToggle, &QCheckBox::toggled, this,
          &BoostWidget::onNoviceModeToggled);
  titleRow->addWidget(noviceLabel);
  titleRow->addWidget(m_noviceModeToggle);

  titleRow->addStretch();

  // 2D/3D buttons
  m_2dButton = new QPushButton("2D", this);
  m_2dButton->setFixedSize(40, 30);
  m_2dButton->setStyleSheet("background-color: #00BCD4; color: white; "
                            "border-radius: 4px; font-weight: bold;");
  connect(m_2dButton, &QPushButton::clicked, this,
          &BoostWidget::on2DViewClicked);

  m_3dButton = new QPushButton("3D", this);
  m_3dButton->setFixedSize(40, 30);
  m_3dButton->setStyleSheet(
      "background-color: #444; color: #888; border-radius: 4px;");
  connect(m_3dButton, &QPushButton::clicked, this,
          &BoostWidget::on3DViewClicked);

  titleRow->addWidget(m_2dButton);
  titleRow->addWidget(m_3dButton);

  layout->addLayout(titleRow);

  // Description
  QLabel *desc = new QLabel(
      "Desired boost pressure across RPM and throttle position.", this);
  desc->setStyleSheet("color: #888;");
  layout->addWidget(desc);

  // Tips
  QFrame *tipFrame = new QFrame(this);
  tipFrame->setStyleSheet(
      "background-color: rgba(76, 175, 80, 0.1); border: 1px solid #4CAF50; "
      "border-radius: 4px; padding: 8px;");
  QVBoxLayout *tipLayout = new QVBoxLayout(tipFrame);
  tipLayout->setSpacing(4);

  QLabel *tip1 = new QLabel(
      "○ Beginner: Your power goal—too fast = engine stress, too slow = weak "
      "acceleration. Ramp like loading into highway—start slow, build to max.",
      this);
  tip1->setStyleSheet("color: #4CAF50; font-size: 11px;");
  tip1->setWordWrap(true);

  QLabel *tip2 =
      new QLabel("○ Pro: MAP target above atmospheric. Flat on compressor map "
                 "is anti-surge. Linear ramp for street, aggressive for track.",
                 this);
  tip2->setStyleSheet("color: #4CAF50; font-size: 11px;");
  tip2->setWordWrap(true);

  tipLayout->addWidget(tip1);
  tipLayout->addWidget(tip2);
  layout->addWidget(tipFrame);

  // Warning
  QFrame *warnFrame = new QFrame(this);
  warnFrame->setStyleSheet(
      "background-color: rgba(255, 152, 0, 0.1); border: 1px solid #FF9800; "
      "border-radius: 4px; padding: 8px;");
  QHBoxLayout *warnLayout = new QHBoxLayout(warnFrame);
  QLabel *warnIcon = new QLabel("⚠", this);
  warnIcon->setStyleSheet("color: #FF9800; font-size: 16px;");
  QLabel *warnText = new QLabel(
      "High boost first! Verify intercooler capacity and octane rating.", this);
  warnText->setStyleSheet("color: #FF9800; font-size: 11px;");
  warnLayout->addWidget(warnIcon);
  warnLayout->addWidget(warnText);
  warnLayout->addStretch();
  layout->addWidget(warnFrame);

  return box;
}

QWidget *BoostWidget::createTableSection() {
  QWidget *container = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);

  m_boostTable = new QTableWidget(8, 11, this);

  // Headers
  QStringList hHeaders = {"TPS/RPM", "1000", "2000", "3000", "4000", "5000",
                          "6000",    "7000", "8000", "9000", "10000"};
  m_boostTable->setHorizontalHeaderLabels(hHeaders);

  // TPS values
  QStringList tpsValues = {"20", "30", "40", "50", "60", "70", "80", "100"};

  // Initialize with zeros until real ECU data is loaded
  double sampleData[8][10] = {
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};

  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 11; col++) {
      QTableWidgetItem *item;
      if (col == 0) {
        item = new QTableWidgetItem(tpsValues[row]);
        item->setBackground(QColor("#1E1E1E"));
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
      } else {
        item = new QTableWidgetItem(
            QString::number(sampleData[row][col - 1], 'f', 1));
      }
      item->setTextAlignment(Qt::AlignCenter);
      item->setForeground(QColor("#00BCD4"));
      m_boostTable->setItem(row, col, item);
    }
  }

  m_boostTable->setStyleSheet(R"(
        QTableWidget {
            background-color: #1E1E1E; gridline-color: #333;
            color: #00BCD4; border: 1px solid #444; border-radius: 4px;
        }
        QTableWidget::item { padding: 4px; }
        QTableWidget::item:selected { background-color: #00BCD4; color: black; }
        QHeaderView::section {
            background-color: #2B2B2B; color: #888;
            border: 1px solid #333; padding: 4px;
        }
    )");

  m_boostTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  m_boostTable->verticalHeader()->setVisible(false);

  layout->addWidget(m_boostTable);

  return container;
}

QWidget *BoostWidget::create3DViewSection() {
  QGroupBox *box = new QGroupBox("Boost Target", this);
  box->setStyleSheet(R"(
        QGroupBox {
            font-size: 12px; font-weight: bold; color: white;
            border: 1px solid #444; border-radius: 8px;
            margin-top: 10px; padding-top: 10px;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }
    )");

  QVBoxLayout *layout = new QVBoxLayout(box);

  // Subtitle
  QLabel *subtitle = new QLabel("Using kMod <> Accel. Elem.", this);
  subtitle->setStyleSheet("color: #888; font-size: 10px;");
  layout->addWidget(subtitle);

  // 3D placeholder
  m_3dViewPlaceholder = new QWidget(this);
  m_3dViewPlaceholder->setMinimumHeight(200);
  m_3dViewPlaceholder->setStyleSheet(
      "background-color: #1E1E1E; border: 1px solid #333; border-radius: 4px;");

  QVBoxLayout *placeholderLayout = new QVBoxLayout(m_3dViewPlaceholder);
  QLabel *placeholderLabel =
      new QLabel("3D Surface View\n(DataVisualization Required)", this);
  placeholderLabel->setStyleSheet("color: #666;");
  placeholderLabel->setAlignment(Qt::AlignCenter);
  placeholderLayout->addWidget(placeholderLabel);

  layout->addWidget(m_3dViewPlaceholder);

  // Axis labels
  QLabel *xLabel = new QLabel("kPa / ms²", this);
  xLabel->setStyleSheet("color: #666; font-size: 10px;");
  layout->addWidget(xLabel);

  return box;
}

QSlider *BoostWidget::createStyledSlider(int min, int max, int value) {
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

void BoostWidget::onInductionTypeChanged(int index) {
  if (m_settingsManager) {
    m_settingsManager->setValue("boostMode", index);
  }
}

void BoostWidget::onNoviceModeToggled(bool enabled) { Q_UNUSED(enabled); }

void BoostWidget::on2DViewClicked() {
  m_2dButton->setStyleSheet("background-color: #00BCD4; color: white; "
                            "border-radius: 4px; font-weight: bold;");
  m_3dButton->setStyleSheet(
      "background-color: #444; color: #888; border-radius: 4px;");
}

void BoostWidget::on3DViewClicked() {
  m_3dButton->setStyleSheet("background-color: #00BCD4; color: white; "
                            "border-radius: 4px; font-weight: bold;");
  m_2dButton->setStyleSheet(
      "background-color: #444; color: #888; border-radius: 4px;");
}

void BoostWidget::loadFromECU() {
  if (!m_settingsManager) return;
  // Boost widget primarily uses table data - scalar settings loaded here
  QVariant v;
  v = m_settingsManager->getValue("boostMode");
  if (v.isValid()) { m_inductionTypeCombo->setCurrentIndex(v.toInt()); }
  v = m_settingsManager->getValue("boostTarget");
  // Target is displayed in table, no direct slider
}
