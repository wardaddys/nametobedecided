#include "ToothLoggerWidget.h"
#include "core/TunerProColors.h"
#include "core/SerialManager.h"
#include "utils/Logger.h"
#include <QFrame>
#include <QHeaderView>
#include <QPainter>
#include <QPainterPath>
#include <QSplitter>

// ===================================
// GapVisualizer Implementation
// ===================================
GapVisualizer::GapVisualizer(QWidget *parent) : QWidget(parent) {
    setMinimumSize(400, 300);
    setStyleSheet(QString("background-color: %1; border: 1px solid %2; border-radius: 6px;")
                  .arg(TunerProColors::BG_BASE).arg(TunerProColors::BORDER_SUBTLE));
}

void GapVisualizer::setData(const QVector<uint16_t> &data) {
    m_toothTimes = data;
    update();
}

void GapVisualizer::clearData() {
    m_toothTimes.clear();
    update();
}

void GapVisualizer::paintEvent(QPaintEvent *event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    
    p.setRenderHint(QPainter::Antialiasing);

    if (m_toothTimes.isEmpty()) {
        // Ghost empty state
        p.setPen(QPen(QColor(TunerProColors::BORDER_DEFAULT), 2, Qt::DashLine));
        QPainterPath path;
        int y = height() / 2;
        int gw = 40;
        int pt = 10;
        for (int x = 20; x < width() - 20; x += gw) {
            path.moveTo(x, y);
            path.lineTo(x, y - 40);
            path.lineTo(x + pt, y - 40);
            path.lineTo(x + pt, y);
            if (x == 20 + gw * 4) { // Missing tooth ghost
                path.lineTo(x + gw + pt, y);
                x += gw;
                continue;
            }
            path.lineTo(x + gw, y);
        }
        p.drawPath(path);
        
        p.setPen(QColor(TunerProColors::TEXT_MUTED));
        p.setFont(QFont("Inter", 12));
        p.drawText(rect(), Qt::AlignCenter, "Await trigger capture to visualize gap timings.");
        return;
    }

    // Draw bars
    int maxTime = 1;
    for (int t : m_toothTimes) if (t > maxTime) maxTime = t;

    int numTeeth = m_toothTimes.size();
    if (numTeeth == 0) return;

    double barWidth = (double)(width() - 40) / numTeeth;
    int bottom = height() - 20;
    int top = 20;
    int availH = bottom - top;

    for (int i = 0; i < numTeeth; ++i) {
        double val = m_toothTimes[i];
        double ratio = (double)val / maxTime;
        double h = availH * ratio;
        
        QRectF b(20 + i * barWidth, bottom - h, barWidth - 1, h);
        
        // Color based on gap vs previous
        QColor barColor(TunerProColors::CH_TPS); // info blue
        if (i > 0) {
            double prev = m_toothTimes[i - 1];
            if (prev > 0) {
                double gapRatio = val / prev;
                // Missing tooth detection (ratio approx > 1.5)
                if (gapRatio > 1.5) {
                    barColor = QColor(TunerProColors::DANGER);
                } else if (gapRatio < 0.6) {
                    barColor = QColor(TunerProColors::WARN);
                }
            }
        }
        
        p.setBrush(barColor);
        p.setPen(Qt::NoPen);
        p.drawRect(b);
    }
}


// ===================================
// ToothLoggerWidget Implementation
// ===================================

ToothLoggerWidget::ToothLoggerWidget(QWidget *parent)
    : QWidget(parent), m_serialManager(nullptr) {
  setupUi();
}

void ToothLoggerWidget::setSerialManager(SerialManager *serial) {
  if (m_serialManager) {
    disconnect(m_serialManager, &SerialManager::toothDataReceived,
               this, &ToothLoggerWidget::onToothDataReceived);
  }
  m_serialManager = serial;
  if (m_serialManager) {
    connect(m_serialManager, &SerialManager::toothDataReceived,
            this, &ToothLoggerWidget::onToothDataReceived);
  }
}

void ToothLoggerWidget::setupUi() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(16, 16, 16, 16);
  mainLayout->setSpacing(16);

  // Splitter
  QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
  splitter->setStyleSheet("QSplitter::handle { background-color: transparent; }");

  // === LEFT: Controls & Table ===
  QWidget *leftPanel = new QWidget(splitter);
  QVBoxLayout *leftLay = new QVBoxLayout(leftPanel);
  leftLay->setContentsMargins(0,0,0,0);
  leftLay->setSpacing(16);

  // Control Card
  QFrame *cCard = new QFrame(leftPanel);
  cCard->setStyleSheet(
      "QFrame { "
      "  background-color: #1A1A1F; "
      "  border-radius: 8px; "
      "  border-top: 1px solid #111; "
      "  border-left: 1px solid #111; "
      "  border-bottom: 1px solid #2A2A30; "
      "  border-right: 1px solid #2A2A30; "
      "}");
  QVBoxLayout *cL = new QVBoxLayout(cCard);
  cL->setContentsMargins(16, 16, 16, 16);
  cL->setSpacing(16);
  
  QLabel *cTitle = new QLabel("TRIGGER PATTERN DIAGNOSTICS");
  cTitle->setStyleSheet(QString("font-family: 'Barlow Condensed'; font-size: 14px; font-weight: bold; color: %1;")
                        .arg(TunerProColors::TEXT_MUTED));
  cL->addWidget(cTitle);

  QHBoxLayout *btnLay = new QHBoxLayout();
  m_captureBtn = new QPushButton("START CAPTURE", cCard);
  m_captureBtn->setCursor(Qt::PointingHandCursor);
  m_captureBtn->setFixedHeight(40);
  m_captureBtn->setStyleSheet(
      "QPushButton { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #00E676, stop:0.4 #00C853, stop:1 #00A300); "
      "  color: #FFFFFF; "
      "  border: 1px solid #003300; "
      "  border-top: 1px solid #69F0AE; "
      "  border-left: 1px solid #00E676; "
      "  border-radius: 5px; "
      "  font-family: 'Inter'; font-weight: 900; font-size: 13px; letter-spacing: 1px; "
      "} "
      "QPushButton:hover { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #69F0AE, stop:0.4 #00E676, stop:1 #00C853); "
      "} "
      "QPushButton:pressed { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #004D00, stop:1 #00C853); "
      "  border-top: 1px solid #003300; "
      "  border-left: 1px solid #003300; "
      "  border-bottom: 1px solid #00E676; "
      "  border-right: 1px solid #00E676; "
      "}");
  
  m_clearBtn = new QPushButton("CLEAR", cCard);
  m_clearBtn->setCursor(Qt::PointingHandCursor);
  m_clearBtn->setFixedHeight(40);
  m_clearBtn->setStyleSheet(
      "QPushButton { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5A5A5A, stop:0.4 #3D3D3D, stop:1 #2B2B2B); "
      "  color: #F0F0F0; "
      "  border: 1px solid #1A1A1A; "
      "  border-top: 1px solid #777; "
      "  border-left: 1px solid #555; "
      "  border-radius: 5px; "
      "  font-family: 'Inter'; font-weight: bold; font-size: 12px; letter-spacing: 1px; "
      "} "
      "QPushButton:hover { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6A6A6A, stop:0.4 #4D4D4D, stop:1 #3B3B3B); "
      "} "
      "QPushButton:pressed { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #222, stop:1 #3D3D3D); "
      "  border-top: 1px solid #111; "
      "  border-left: 1px solid #111; "
      "  border-bottom: 1px solid #555; "
      "  border-right: 1px solid #555; "
      "}");
  
  btnLay->addWidget(m_captureBtn);
  btnLay->addWidget(m_clearBtn);
  cL->addLayout(btnLay);

  QString lcdStyle = "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0A0A0A, stop:1 #141414); "
                     "border: 1px solid #000; border-bottom: 1px solid #3A3A3A; border-right: 1px solid #2A2A2A; "
                     "border-radius: 4px; padding: 8px 12px; color: #00BCD4; font-family: 'JetBrains Mono'; "
                     "font-size: 13px; font-weight: bold;";

  m_statusLabel = new QLabel("Status: Waiting", cCard);
  m_statusLabel->setStyleSheet(lcdStyle);
  m_toothCountLabel = new QLabel("Teeth: 0", cCard);
  m_toothCountLabel->setStyleSheet(lcdStyle);
  
  cL->addWidget(m_statusLabel);
  cL->addWidget(m_toothCountLabel);
  
  leftLay->addWidget(cCard);

  // Table Card
  QFrame *tCard = new QFrame(leftPanel);
  tCard->setStyleSheet(cCard->styleSheet());
  QVBoxLayout *tL = new QVBoxLayout(tCard);
  tL->setContentsMargins(16, 16, 16, 16);
  
  m_toothTable = new QTableWidget(0, 3, tCard);
  m_toothTable->setHorizontalHeaderLabels({"Tooth #", "Time (µs)", "Ratio"});
  m_toothTable->horizontalHeader()->setStretchLastSection(true);
  m_toothTable->verticalHeader()->hide();
  m_toothTable->setStyleSheet(
      "QTableWidget { background: #141414; border: 1px solid #111; border-bottom: 1px solid #3A3A3A; border-right: 1px solid #2A2A2A; border-radius: 4px; color: #E0E0E0; gridline-color: #222; font-family: 'JetBrains Mono'; font-size: 12px; }"
      "QHeaderView::section { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #333, stop:1 #222); color: #FFF; padding: 4px; border: 1px solid #111; border-right: 1px solid #444; border-bottom: 1px solid #444; font-family: 'Inter'; font-size: 11px; font-weight: bold; }"
  );
   
  tL->addWidget(m_toothTable);
  leftLay->addWidget(tCard, 1);

  // === RIGHT: Gap Visualizer ===
  QWidget *rightPanel = new QWidget(splitter);
  QVBoxLayout *rightLay = new QVBoxLayout(rightPanel);
  rightLay->setContentsMargins(0,0,0,0);
  
  QFrame *vCard = new QFrame(rightPanel);
  vCard->setStyleSheet(cCard->styleSheet());
  QVBoxLayout *vL = new QVBoxLayout(vCard);
  vL->setContentsMargins(16, 16, 16, 16);
  
  QLabel *vTitle = new QLabel("GAP TIME VISUALIZER");
  vTitle->setStyleSheet(cTitle->styleSheet());
  vL->addWidget(vTitle);
  
  m_gapVisualizer = new GapVisualizer(vCard);
  vL->addWidget(m_gapVisualizer, 1);
  rightLay->addWidget(vCard, 1);

  splitter->addWidget(leftPanel);
  splitter->addWidget(rightPanel);
  
  // Size ratios
  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 2);

  mainLayout->addWidget(splitter);

  // Connections
  connect(m_captureBtn, &QPushButton::clicked, this, &ToothLoggerWidget::onCaptureClicked);
  connect(m_clearBtn, &QPushButton::clicked, this, &ToothLoggerWidget::onClearClicked);
}

void ToothLoggerWidget::onCaptureClicked() {
  m_capturing = !m_capturing;
  if (m_capturing) {
    m_captureBtn->setText("STOP CAPTURE");
    m_captureBtn->setStyleSheet(
      "QPushButton { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FF5252, stop:0.4 #E53935, stop:1 #C62828); "
      "  color: #FFFFFF; "
      "  border: 1px solid #880E4F; "
      "  border-top: 1px solid #FF8A80; "
      "  border-left: 1px solid #FF5252; "
      "  border-radius: 5px; "
      "  font-family: 'Inter'; font-weight: 900; font-size: 13px; letter-spacing: 1px; "
      "} "
      "QPushButton:hover { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FF8A80, stop:0.4 #EF5350, stop:1 #D32F2F); "
      "} "
      "QPushButton:pressed { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B71C1C, stop:1 #D32F2F); "
      "  border-top: 1px solid #880E4F; "
      "  border-left: 1px solid #880E4F; "
      "  border-bottom: 1px solid #EF5350; "
      "  border-right: 1px solid #EF5350; "
      "}");
    m_statusLabel->setText("Status: Capturing...");
    m_statusLabel->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0A0A0A, stop:1 #141414); "
        "border: 1px solid #000; border-bottom: 1px solid #3A3A3A; border-right: 1px solid #2A2A2A; "
        "border-radius: 4px; padding: 8px 12px; color: #00E676; font-family: 'JetBrains Mono'; "
        "font-size: 13px; font-weight: bold;");
    Logger::info("ToothLogger: Capture started");

    // Send 'T' command to ECU to request tooth log data
    if (m_serialManager && m_serialManager->isConnected()) {
      SerialCommand cmd;
      cmd.data = m_serialManager->m_protocol->createToothLogRequest();
      cmd.type = CommandType::ToothLog;
      cmd.expectedResponse = -1;
      cmd.timeoutMs = ProtocolTiming::TIMEOUT_MS;
      cmd.retryCount = 1;
      m_serialManager->queueCommand(cmd);
    }
  } else {
    m_captureBtn->setText("START CAPTURE");
    m_captureBtn->setStyleSheet(
      "QPushButton { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #00E676, stop:0.4 #00C853, stop:1 #00A300); "
      "  color: #FFFFFF; "
      "  border: 1px solid #003300; "
      "  border-top: 1px solid #69F0AE; "
      "  border-left: 1px solid #00E676; "
      "  border-radius: 5px; "
      "  font-family: 'Inter'; font-weight: 900; font-size: 13px; letter-spacing: 1px; "
      "} "
      "QPushButton:hover { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #69F0AE, stop:0.4 #00E676, stop:1 #00C853); "
      "} "
      "QPushButton:pressed { "
      "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #004D00, stop:1 #00C853); "
      "  border-top: 1px solid #003300; "
      "  border-left: 1px solid #003300; "
      "  border-bottom: 1px solid #00E676; "
      "  border-right: 1px solid #00E676; "
      "}");
    m_statusLabel->setText("Status: Stopped");
    m_statusLabel->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0A0A0A, stop:1 #141414); "
        "border: 1px solid #000; border-bottom: 1px solid #3A3A3A; border-right: 1px solid #2A2A2A; "
        "border-radius: 4px; padding: 8px 12px; color: #FF5252; font-family: 'JetBrains Mono'; "
        "font-size: 13px; font-weight: bold;");
    Logger::info("ToothLogger: Capture stopped");
  }
}

void ToothLoggerWidget::onClearClicked() {
  m_toothTimes.clear();
  m_gapVisualizer->clearData();
  m_toothTable->setRowCount(0);
  m_toothCountLabel->setText("Teeth: 0");
  m_statusLabel->setText("Status: Waiting");
}

void ToothLoggerWidget::onToothDataReceived(const QByteArray &data) {
  if (!m_capturing) return;
  
  for (int i = 0; i + 1 < data.size(); i += 2) {
    uint16_t time = static_cast<uint8_t>(data.at(i)) | (static_cast<uint8_t>(data.at(i + 1)) << 8);
    if (time > 0) m_toothTimes.append(time);
  }
  
  m_toothTable->setRowCount(m_toothTimes.size());
  for (int i = 0; i < m_toothTimes.size(); ++i) {
    m_toothTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
    m_toothTable->setItem(i, 1, new QTableWidgetItem(QString::number(m_toothTimes[i])));
    
    QTableWidgetItem *ratioItem = new QTableWidgetItem("-");
    if (i > 0 && m_toothTimes[i - 1] > 0) {
      double ratio = static_cast<double>(m_toothTimes[i]) / m_toothTimes[i - 1];
      ratioItem->setText(QString::number(ratio, 'f', 2));
      
      // Styling Table rows based on gap ratio
      if (ratio > 1.5) {
        ratioItem->setBackground(QColor(TunerProColors::DANGER).darker(300));
        ratioItem->setForeground(QColor(TunerProColors::DANGER));
      } else if (ratio < 0.67) {
        ratioItem->setBackground(QColor(TunerProColors::WARN).darker(300));
        ratioItem->setForeground(QColor(TunerProColors::WARN));
      }
    }
    ratioItem->setTextAlignment(Qt::AlignCenter);
    
    m_toothTable->item(i, 0)->setTextAlignment(Qt::AlignCenter);
    m_toothTable->item(i, 1)->setTextAlignment(Qt::AlignCenter);
    m_toothTable->setItem(i, 2, ratioItem);
  }
  
  m_toothCountLabel->setText(QString("Teeth: %1").arg(m_toothTimes.size()));
  m_gapVisualizer->setData(m_toothTimes);
  
  // Continuous polling: re-queue another tooth log request while capturing
  if (m_capturing && m_serialManager && m_serialManager->isConnected()) {
      SerialCommand cmd;
      cmd.data = m_serialManager->m_protocol->createToothLogRequest();
      cmd.type = CommandType::ToothLog;
      cmd.expectedResponse = -1;
      cmd.timeoutMs = ProtocolTiming::TIMEOUT_MS;
      cmd.retryCount = 1;
      
      // Use QTimer::singleShot to break synchronous infinite recursion in Simulation mode
      // and provide a realistic 50ms interval between requests
      QTimer::singleShot(50, this, [this, cmd]() {
          if (m_capturing && m_serialManager && m_serialManager->isConnected()) {
              m_serialManager->queueCommand(cmd);
          }
      });
  }
}
