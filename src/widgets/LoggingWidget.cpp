#include "LoggingWidget.h"
#include "core/TunerProColors.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPainter>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QSlider>
#include <QCoreApplication>

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

void LoggingWidget::refreshLogList() {
    m_recentLogsList->clear();
    
    QFileInfoList allLogs;
    
    // Check Documents
    QString docs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QDir docsDir(docs + "/OSTuner/Logs");
    if (docsDir.exists()) {
        allLogs.append(docsDir.entryInfoList(QStringList() << "*.csv", QDir::Files, QDir::Time));
    }
    
    // Check App Path (Fallback)
    QDir appDir(QCoreApplication::applicationDirPath() + "/Logs");
    if (appDir.exists()) {
        allLogs.append(appDir.entryInfoList(QStringList() << "*.csv", QDir::Files, QDir::Time));
    }
    
    // Sort by time (newest first)
    std::sort(allLogs.begin(), allLogs.end(), [](const QFileInfo &a, const QFileInfo &b) {
        return a.lastModified() > b.lastModified();
    });
    
    for (const QFileInfo &fi : allLogs) {
        QListWidgetItem *item = new QListWidgetItem("📄 " + fi.fileName(), m_recentLogsList);
        item->setData(Qt::UserRole, fi.absoluteFilePath());
    }
}

void LoggingWidget::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 32, 32, 32);
    mainLayout->setSpacing(24);

    // Header Area
    QWidget *headerWidget = new QWidget(this);
    QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 8);
    headerLayout->setSpacing(4);

    QLabel *title = new QLabel("SESSION LOGGER", headerWidget);
    title->setStyleSheet(QString("font-family: 'Inter'; font-size: 28px; font-weight: 900; letter-spacing: 1px; color: %1;")
                         .arg(TunerProColors::TEXT_PRIMARY));
    
    QLabel *subtitle = new QLabel("Record high-speed telemetry and engine parameters directly to disk", headerWidget);
    subtitle->setStyleSheet(QString("font-family: 'Inter'; font-size: 14px; color: %1;")
                            .arg(TunerProColors::TEXT_MUTED));
                            
    headerLayout->addWidget(title);
    headerLayout->addWidget(subtitle);
    mainLayout->addWidget(headerWidget);

    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(24);
    
    // Helper to create cards
    auto createCard = []() -> QFrame* {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString(
            "QFrame { "
            "  background-color: #1A1A1F; "
            "  border-radius: 12px; "
            "  border: 1px solid #2A2A30; "
            "}"));
        return card;
    };
    
    auto createCardHeader = [](const QString &t) -> QWidget* {
        QWidget *wrap = new QWidget();
        wrap->setStyleSheet("background: transparent;");
        QVBoxLayout *vl = new QVBoxLayout(wrap);
        vl->setContentsMargins(0, 0, 0, 16);
        vl->setSpacing(12);

        QLabel *lbl = new QLabel(t);
        lbl->setStyleSheet(QString("font-family: 'Inter'; font-size: 13px; font-weight: 800; letter-spacing: 1.5px; color: %1;")
                           .arg(TunerProColors::TEXT_SECONDARY));
        vl->addWidget(lbl);

        QFrame *line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setStyleSheet("border: none; background-color: #2A2A30;");
        line->setFixedHeight(1);
        vl->addWidget(line);

        return wrap;
    };

    // ----------------------------
    // Card 1: Control
    // ----------------------------
    QFrame *cardControl = createCard();
    QVBoxLayout *cL = new QVBoxLayout(cardControl);
    cL->setContentsMargins(24, 24, 24, 24);
    cL->addWidget(createCardHeader("CONTROL"));
    
    m_statusLabel = new QLabel("READY TO RECORD");
    m_statusLabel->setStyleSheet(QString("font-family: 'JetBrains Mono'; font-size: 13px; font-weight: bold; color: %1;").arg(TunerProColors::TEXT_SECONDARY));
    cL->addWidget(m_statusLabel);
    cL->addSpacing(8);
    
    m_toggleButton = new QPushButton("START LOGGING", this);
    m_toggleButton->setFixedHeight(56);
    m_toggleButton->setCursor(Qt::PointingHandCursor);
    m_toggleButton->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: %2; border: none; border-radius: 8px; font-family: 'Inter'; font-size: 16px; font-weight: 900; letter-spacing: 2px; }"
        "QPushButton:hover { background-color: #FF5A5F; }"
    ).arg(TunerProColors::DANGER).arg(TunerProColors::TEXT_PRIMARY));
    
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
    
    cL->addSpacing(24);
    QLabel *bufLbl = new QLabel("Buffer Utilization", this);
    bufLbl->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 600;").arg(TunerProColors::TEXT_MUTED));
    cL->addWidget(bufLbl);
    
    m_bufferBar = new BufferBar(this);
    cL->addWidget(m_bufferBar);
    cL->addSpacing(8);
    
    m_fileLabel = new QLabel("Path: Documents/OSTuner/Logs", this);
    m_fileLabel->setStyleSheet(QString("font-family: 'JetBrains Mono'; color: %1; font-size: 11px;").arg(TunerProColors::TEXT_MUTED));
    cL->addWidget(m_fileLabel);

    grid->addWidget(cardControl, 0, 0);

    // ----------------------------
    // Card 2: Session Statistics
    // ----------------------------
    QFrame *cardStats = createCard();
    QVBoxLayout *sL = new QVBoxLayout(cardStats);
    sL->setContentsMargins(24, 24, 24, 24);
    sL->addWidget(createCardHeader("STATISTICS"));
    
    QHBoxLayout *stGrid = new QHBoxLayout();
    stGrid->setSpacing(32);
    auto addStat = [this](QHBoxLayout *g, const QString &labelStr) -> QLabel* {
        QVBoxLayout *v = new QVBoxLayout();
        v->setSpacing(4);
        QLabel *lbl = new QLabel(labelStr);
        lbl->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 600; text-transform: uppercase; letter-spacing: 1px;").arg(TunerProColors::TEXT_MUTED));
        QLabel *val = new QLabel("0");
        val->setStyleSheet(QString("font-family: 'JetBrains Mono'; font-size: 32px; font-weight: 900; color: #00BCD4;"));
        v->addWidget(lbl);
        v->addWidget(val);
        g->addLayout(v);
        return val;
    };
    
    m_recordCountLabel = addStat(stGrid, "Records");
    m_fileSizeLabel = addStat(stGrid, "File Size");
    m_rateLabel = addStat(stGrid, "Data Rate");
    sL->addLayout(stGrid);
    sL->addStretch();
    
    grid->addWidget(cardStats, 0, 1);

    // ----------------------------
    // Card 3: Logged Channels
    // ----------------------------
    QFrame *cardChan = createCard();
    QVBoxLayout *chL = new QVBoxLayout(cardChan);
    chL->setContentsMargins(24, 24, 24, 24);
    chL->addWidget(createCardHeader("LOGGED CHANNELS (Standard + High-Speed)"));
    
    QWidget *pillWrap = new QWidget();
    QHBoxLayout *pL = new QHBoxLayout(pillWrap);
    pL->setContentsMargins(0,0,0,0);
    pL->setSpacing(12);
    auto createPill = [](const QString &t) {
        QLabel *l = new QLabel(t);
        l->setAlignment(Qt::AlignCenter);
        l->setStyleSheet(QString(
            "background-color: #24242A; "
            "color: %1; "
            "border: 1px solid #33333C; "
            "border-radius: 14px; "
            "padding: 6px 18px; "
            "font-size: 12px; "
            "font-weight: 800; "
            "letter-spacing: 0.5px;"
        ).arg(TunerProColors::TEXT_PRIMARY));
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
    QFrame *cardLogs = createCard();
    QVBoxLayout *rlL = new QVBoxLayout(cardLogs);
    rlL->setContentsMargins(24, 24, 24, 24);
    rlL->addWidget(createCardHeader("RECENT SESSIONS"));
    m_recentLogsList = new QListWidget(this);
    m_recentLogsList->setStyleSheet(QString(
        "QListWidget { background: transparent; border: none; outline: none; } "
        "QListWidget::item { "
        "  background-color: #202025; "
        "  color: %1; "
        "  border: 1px solid #2B2B32; "
        "  border-radius: 8px; "
        "  margin-bottom: 10px; "
        "  padding: 14px 16px; "
        "  font-family: 'JetBrains Mono'; "
        "  font-size: 13px; "
        "} "
        "QListWidget::item:hover { background-color: #2A2A30; border-color: #3D3D48; }"
        ).arg(TunerProColors::TEXT_PRIMARY));
    
    m_recentLogsList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_recentLogsList, &QListWidget::itemClicked, this, &LoggingWidget::onLogSelected);
    rlL->addWidget(m_recentLogsList);
    
    // Playback Controls (hidden by default)
    m_playbackControlsWidget = new QWidget(cardLogs);
    QVBoxLayout *pbL = new QVBoxLayout(m_playbackControlsWidget);
    pbL->setContentsMargins(0, 8, 0, 0);
    
    QLabel *pbTitle = new QLabel("PLAYBACK CONTROLS", m_playbackControlsWidget);
    pbTitle->setStyleSheet(QString("font-family: 'Inter'; font-size: 11px; font-weight: 800; color: %1;").arg(TunerProColors::TEXT_MUTED));
    pbL->addWidget(pbTitle);
    
    QHBoxLayout *pbBtnsL = new QHBoxLayout();
    m_playPauseBtn = new QPushButton("PLAY", m_playbackControlsWidget);
    m_playPauseBtn->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: %2; border: none; border-radius: 4px; padding: 6px 12px; font-weight: bold; }"
        "QPushButton:hover { background-color: #69F0AE; }"
    ).arg(TunerProColors::SAFE).arg(TunerProColors::BG_BASE));
    connect(m_playPauseBtn, &QPushButton::clicked, this, &LoggingWidget::onPlayPauseClicked);
    
    m_stopBtn = new QPushButton("STOP", m_playbackControlsWidget);
    m_stopBtn->setStyleSheet(QString(
        "QPushButton { background-color: #333; color: %1; border: none; border-radius: 4px; padding: 6px 12px; font-weight: bold; }"
        "QPushButton:hover { background-color: #444; }"
    ).arg(TunerProColors::TEXT_PRIMARY));
    connect(m_stopBtn, &QPushButton::clicked, this, &LoggingWidget::onStopClicked);
    
    pbBtnsL->addWidget(m_playPauseBtn);
    pbBtnsL->addWidget(m_stopBtn);
    pbBtnsL->addStretch();
    pbL->addLayout(pbBtnsL);
    
    QHBoxLayout *pbSliderL = new QHBoxLayout();
    m_playbackTimeLabel = new QLabel("00:00 / 00:00", m_playbackControlsWidget);
    m_playbackTimeLabel->setStyleSheet(QString("font-family: 'JetBrains Mono'; color: %1; font-size: 11px;").arg(TunerProColors::TEXT_MUTED));
    m_playbackSlider = new QSlider(Qt::Horizontal, m_playbackControlsWidget);
    m_playbackSlider->setRange(0, 100);
    m_playbackSlider->setStyleSheet(
        "QSlider::groove:horizontal { border: 1px solid #333; height: 4px; background: #222; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: #00BCD4; width: 12px; height: 12px; margin: -4px 0; border-radius: 6px; }"
    );
    connect(m_playbackSlider, &QSlider::sliderMoved, this, &LoggingWidget::onSliderMoved);
    
    pbSliderL->addWidget(m_playbackSlider);
    pbSliderL->addWidget(m_playbackTimeLabel);
    pbL->addLayout(pbSliderL);
    
    m_playbackControlsWidget->setVisible(false);
    rlL->addWidget(m_playbackControlsWidget);
    
    refreshLogList(); // Populate list
    
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

void LoggingWidget::onLogSelected() {
    QListWidgetItem *item = m_recentLogsList->currentItem();
    if (item) {
        m_selectedLogPath = item->data(Qt::UserRole).toString();
        m_playbackControlsWidget->setVisible(true);
        // Reset slider
        m_playbackSlider->setValue(0);
        m_playbackTimeLabel->setText("Ready");
        
        // Let main window load it? Or emit play request when PLAY is clicked.
    }
}

void LoggingWidget::onPlayPauseClicked() {
    if (m_playPauseBtn->text() == "PLAY") {
        if (!m_selectedLogPath.isEmpty()) {
            emit playbackPlayRequested(m_selectedLogPath);
        }
    } else {
        emit playbackPauseRequested();
    }
}

void LoggingWidget::onStopClicked() {
    emit playbackStopRequested();
}

void LoggingWidget::onSliderMoved(int value) {
    emit playbackSeekRequested(value);
}

void LoggingWidget::setPlaybackState(bool isPlaying) {
    if (isPlaying) {
        m_playPauseBtn->setText("PAUSE");
        m_playPauseBtn->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: %2; border: none; border-radius: 4px; padding: 6px 12px; font-weight: bold; }"
            "QPushButton:hover { background-color: #FF7B00; }"
        ).arg(TunerProColors::WARN).arg(TunerProColors::BG_BASE));
    } else {
        m_playPauseBtn->setText("PLAY");
        m_playPauseBtn->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: %2; border: none; border-radius: 4px; padding: 6px 12px; font-weight: bold; }"
            "QPushButton:hover { background-color: #69F0AE; }"
        ).arg(TunerProColors::SAFE).arg(TunerProColors::BG_BASE));
    }
}

void LoggingWidget::setPlaybackProgress(int currentMs, int totalMs) {
    if (!m_playbackSlider->isSliderDown()) {
        int pct = totalMs > 0 ? (currentMs * 100) / totalMs : 0;
        m_playbackSlider->setValue(pct);
    }
    
    QTime cur(0, 0, 0);
    cur = cur.addMSecs(currentMs);
    QTime tot(0, 0, 0);
    tot = tot.addMSecs(totalMs);
    
    m_playbackTimeLabel->setText(cur.toString("mm:ss") + " / " + tot.toString("mm:ss"));
}
