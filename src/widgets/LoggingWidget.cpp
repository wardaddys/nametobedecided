#include "LoggingWidget.h"
#include "core/TunerProColors.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPainter>
#include <QDateTime>

// ===================================
// BufferBar Custom Visualizer
// ===================================
BufferBar::BufferBar(QWidget *parent) : QWidget(parent) {
    setFixedHeight(16);
}

void BufferBar::setValue(double v) {
    if (qFuzzyCompare(m_val, v)) return;
    m_val = qBound(0.0, v, 100.0);
    update();
}

void BufferBar::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
    // Background track
    QRectF bgRect(0, 0, width(), height());
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(TunerProColors::BG_BASE));
    p.drawRoundedRect(bgRect, 4, 4);
    
    // Fill
    double ratio = m_val / 100.0;
    if (ratio > 0.0) {
        QRectF fillRect(0, 0, width() * ratio, height());
        // Color shifts from ACCENT to DANGER
        QColor c = QColor(TunerProColors::ACCENT);
        if (ratio > 0.8) c = QColor(TunerProColors::DANGER);
        p.setBrush(c);
        p.drawRoundedRect(fillRect, 4, 4);
    }
}

// ===================================
// LoggingWidget Custom Implementation
// ===================================

LoggingWidget::LoggingWidget(QWidget *parent) : QWidget(parent) { setupUi(); }
LoggingWidget::~LoggingWidget() {}

void LoggingWidget::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(24);

    QLabel *title = new QLabel("SESSION LOGGER", this);
    title->setStyleSheet(QString("font-family: 'Inter'; font-size: 24px; font-weight: bold; color: %1;")
                         .arg(TunerProColors::TEXT_PRIMARY));
    mainLayout->addWidget(title);

    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(24);
    
    // Helper to create cards
    auto createCard = [](const QString &cardTitle) -> QFrame* {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString("QFrame { background-color: %1; border-radius: 8px; border: 1px solid %2; }")
                            .arg(TunerProColors::BG_ELEVATED).arg(TunerProColors::BORDER_SUBTLE));
        return card;
    };
    
    auto createCardTitle = [](const QString &t) -> QLabel* {
        QLabel *lbl = new QLabel(t);
        lbl->setStyleSheet(QString("font-family: 'Barlow Condensed'; font-size: 14px; font-weight: bold; color: %1;")
                           .arg(TunerProColors::TEXT_MUTED));
        return lbl;
    };

    // ----------------------------
    // Card 1: Control
    // ----------------------------
    QFrame *cardControl = createCard("CONTROL");
    QVBoxLayout *cL = new QVBoxLayout(cardControl);
    cL->setContentsMargins(20, 20, 20, 20);
    cL->addWidget(createCardTitle("CONTROL"));
    
    m_statusLabel = new QLabel("READY TO RECORD");
    m_statusLabel->setStyleSheet(QString("font-family: 'JetBrains Mono'; font-size: 12px; color: %1;").arg(TunerProColors::TEXT_SECONDARY));
    cL->addWidget(m_statusLabel);
    
    m_toggleButton = new QPushButton("START LOGGING", this);
    m_toggleButton->setFixedHeight(48);
    m_toggleButton->setCursor(Qt::PointingHandCursor);
    m_toggleButton->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: %2; border: none; border-radius: 6px; font-family: 'Inter'; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: %3; }"
    ).arg(TunerProColors::DANGER).arg(TunerProColors::TEXT_PRIMARY).arg("#FF5A5F"));
    
    m_pulseEffect = new QGraphicsOpacityEffect(this);
    m_toggleButton->setGraphicsEffect(m_pulseEffect);
    m_pulseAnim = new QPropertyAnimation(m_pulseEffect, "opacity");
    m_pulseAnim->setDuration(1000);
    m_pulseAnim->setStartValue(1.0);
    m_pulseAnim->setEndValue(0.4);
    m_pulseAnim->setLoopCount(-1); // Infinite
    
    connect(m_toggleButton, &QPushButton::clicked, [this]() {
        if (m_toggleButton->text() == "START LOGGING") emit startRequested();
        else emit stopRequested();
    });
    cL->addWidget(m_toggleButton);
    
    cL->addSpacing(16);
    QLabel *bufLbl = new QLabel("Buffer Utilization", this);
    bufLbl->setStyleSheet(QString("color: %1; font-size: 11px;").arg(TunerProColors::TEXT_MUTED));
    cL->addWidget(bufLbl);
    
    m_bufferBar = new BufferBar(this);
    cL->addWidget(m_bufferBar);
    
    m_fileLabel = new QLabel("Path: Documents/OSTuner/Logs", this);
    m_fileLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(TunerProColors::TEXT_MUTED));
    cL->addWidget(m_fileLabel);

    grid->addWidget(cardControl, 0, 0);

    // ----------------------------
    // Card 2: Session Statistics
    // ----------------------------
    QFrame *cardStats = createCard("STATISTICS");
    QVBoxLayout *sL = new QVBoxLayout(cardStats);
    sL->setContentsMargins(20, 20, 20, 20);
    sL->addWidget(createCardTitle("STATISTICS"));
    
    QGridLayout *stGrid = new QGridLayout();
    auto addStat = [this](QGridLayout *g, const QString &labelStr, int r, int c) -> QLabel* {
        QVBoxLayout *v = new QVBoxLayout();
        QLabel *lbl = new QLabel(labelStr);
        lbl->setStyleSheet(QString("color: %1; font-size: 12px;").arg(TunerProColors::TEXT_SECONDARY));
        QLabel *val = new QLabel("0");
        val->setStyleSheet(QString("font-family: 'JetBrains Mono'; font-size: 24px; font-weight: bold; color: %1;").arg(TunerProColors::TEXT_PRIMARY));
        v->addWidget(lbl);
        v->addWidget(val);
        g->addLayout(v, r, c);
        return val;
    };
    
    m_recordCountLabel = addStat(stGrid, "Records Written", 0, 0);
    m_fileSizeLabel = addStat(stGrid, "File Size", 0, 1);
    m_rateLabel = addStat(stGrid, "Data Rate", 1, 0);
    sL->addLayout(stGrid);
    sL->addStretch();
    
    grid->addWidget(cardStats, 0, 1);

    // ----------------------------
    // Card 3: Logged Channels
    // ----------------------------
    QFrame *cardChan = createCard("CHANNELS");
    QVBoxLayout *chL = new QVBoxLayout(cardChan);
    chL->setContentsMargins(20, 20, 20, 20);
    chL->addWidget(createCardTitle("LOGGED CHANNELS (Standard + High-Speed)"));
    
    // Dummy Pills
    QWidget *pillWrap = new QWidget();
    QHBoxLayout *pL = new QHBoxLayout(pillWrap);
    pL->setContentsMargins(0,0,0,0);
    pL->setSpacing(8);
    auto createPill = [](const QString &t) {
        QLabel *l = new QLabel(t);
        l->setStyleSheet(QString("background-color: %1; color: %2; border-radius: 12px; padding: 4px 12px; font-size: 11px; font-weight: bold;")
                         .arg(TunerProColors::BG_INTERACTIVE).arg(TunerProColors::TEXT_PRIMARY));
        return l;
    };
    pL->addWidget(createPill("RPM"));
    pL->addWidget(createPill("MAP"));
    pL->addWidget(createPill("AFR"));
    pL->addWidget(createPill("TPS"));
    pL->addWidget(createPill("SYNC"));
    pL->addStretch();
    chL->addWidget(pillWrap);
    chL->addStretch();
    
    grid->addWidget(cardChan, 1, 0);

    // ----------------------------
    // Card 4: Recent Logs
    // ----------------------------
    QFrame *cardLogs = createCard("RECENT LOGS");
    QVBoxLayout *rlL = new QVBoxLayout(cardLogs);
    rlL->setContentsMargins(20, 20, 20, 20);
    rlL->addWidget(createCardTitle("RECENT SESSIONS"));
    m_recentLogsList = new QListWidget(this);
    m_recentLogsList->setStyleSheet(QString("QListWidget { background: transparent; border: none; color: %1; }").arg(TunerProColors::TEXT_PRIMARY));
    m_recentLogsList->addItem(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm_track_session.csv"));
    rlL->addWidget(m_recentLogsList);
    
    grid->addWidget(cardLogs, 1, 1);
    grid->setRowStretch(1, 1); // Allow bottom rows to stretch somewhat

    mainLayout->addLayout(grid);
}

void LoggingWidget::setStatus(bool isLogging, const QString &filePath) {
  if (isLogging) {
    m_statusLabel->setText("RECORDING TO DISK...");
    m_statusLabel->setStyleSheet(QString("color: %1;").arg(TunerProColors::INFO));
    m_toggleButton->setText("STOP LOGGING");
    m_toggleButton->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: %2; border: none; border-radius: 6px; font-family: 'Inter'; font-size: 14px; font-weight: bold; }"
    ).arg(TunerProColors::BG_BASE).arg(TunerProColors::TEXT_PRIMARY)); // Subdued stop button
    m_pulseAnim->start();
    
    if (!filePath.isEmpty()) {
      m_fileLabel->setText("Writing to: " + filePath);
    }
  } else {
    m_statusLabel->setText("READY TO RECORD");
    m_statusLabel->setStyleSheet(QString("color: %1;").arg(TunerProColors::TEXT_SECONDARY));
    m_toggleButton->setText("START LOGGING");
    m_toggleButton->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: %2; border: none; border-radius: 6px; font-family: 'Inter'; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: %3; }"
    ).arg(TunerProColors::DANGER).arg(TunerProColors::TEXT_PRIMARY).arg("#FF5A5F"));
    m_pulseAnim->stop();
    m_pulseEffect->setOpacity(1.0); // Reset
  }
}

void LoggingWidget::updateStats(qint64 records, qint64 bytes, double rate) {
  m_recordCountLabel->setText(QString::number(records));

  QString sizeStr;
  if (bytes < 1024) sizeStr = QString::number(bytes) + " B";
  else if (bytes < 1024 * 1024) sizeStr = QString::number(bytes / 1024.0, 'f', 1) + " KB";
  else sizeStr = QString::number(bytes / (1024.0 * 1024.0), 'f', 2) + " MB";
  m_fileSizeLabel->setText(sizeStr);
  m_rateLabel->setText(QString::number(rate, 'f', 1) + " Hz");
}

void LoggingWidget::setBufferUsage(int percentage) {
  m_bufferBar->setValue(percentage);
}
