#include "DashboardWidget.h"
#include "TunerGauge.h"
#include "ThrottleBodyWidget.h"
#include "MapSensorWidget.h"
#include "TempSensorWidget.h"
#include "InjectorVisualWidget.h"
#include "core/TunerProColors.h"
#include "core/ECUSettingsManager.h"
#include "core/SerialManager.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QFontDatabase>
#include <QMessageBox>
#include <QPushButton>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QProgressBar>
#include <QFormLayout>
#include <QTimer>
#include <cmath>

// ==========================================
// CalibrationOverlay Implementation
// ==========================================
CalibrationOverlay::CalibrationOverlay(QWidget *parent) : QWidget(parent) {
    hide();
    if (parent) resize(parent->size());

    m_panel = new QWidget(this);
    m_panel->setStyleSheet(QString("QWidget { background-color: %1; border: 1px solid %2; border-radius: 8px; }")
                           .arg(TunerProColors::BG_ELEVATED).arg(TunerProColors::BORDER_SUBTLE));
    m_panel->setFixedSize(500, 440);

    QVBoxLayout *panelLay = new QVBoxLayout(m_panel);
    panelLay->setContentsMargins(24, 24, 24, 24);
    panelLay->setSpacing(16);

    QHBoxLayout *headerLay = new QHBoxLayout();
    m_titleLabel = new QLabel("Calibration", m_panel);
    m_titleLabel->setStyleSheet(QString("font-family: 'JetBrains Mono'; font-size: 18px; font-weight: bold; color: %1;")
                                .arg(TunerProColors::TEXT_PRIMARY));
    QPushButton *closeBtn = new QPushButton("X", m_panel);
    closeBtn->setFixedSize(32, 32);
    closeBtn->setStyleSheet(QString("QPushButton { background: transparent; border: none; font-size: 24px; color: %1; }"
                                    "QPushButton:hover { color: #FF2200; }")
                            .arg(TunerProColors::TEXT_SECONDARY));
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, this, &CalibrationOverlay::closeOverlay);
    headerLay->addWidget(m_titleLabel);
    headerLay->addStretch();
    headerLay->addWidget(closeBtn);
    panelLay->addLayout(headerLay);

    m_stack = new QStackedWidget(m_panel);
    panelLay->addWidget(m_stack);

    buildTpsPanel();
    buildMapPanel();
    buildTempPanel();
    buildDutyPanel();
}

void CalibrationOverlay::buildTpsPanel() {
    // --- Palette (matches the new Figma design) ---
    const QString CLR_BG_PANEL = "#12161A";
    const QString CLR_INSET    = "#0B0E12";
    const QString CLR_BORDER   = "#1E2228";
    const QString CLR_BORDER2  = "#23272E";
    const QString CLR_LABEL    = "#B0B5C0";
    const QString CLR_VALUE    = "#FFFFFF";
    const QString CLR_DIM      = "#5A6270";
    const QString CLR_CYAN     = "#22D3EE";
    const QString CLR_NEON     = "#00C853";
    const QString FONT_MONO    = "'JetBrains Mono', 'Fira Code', monospace";
    const QString FONT_SANS    = "'Inter', system-ui, sans-serif";

    m_tpsPanel = new QWidget();
    m_tpsPanel->setStyleSheet(QString("QWidget { background-color: %1; }").arg(CLR_BG_PANEL));

    QVBoxLayout *l = new QVBoxLayout(m_tpsPanel);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);

    // Two-column body
    QHBoxLayout *body = new QHBoxLayout();
    body->setContentsMargins(28, 8, 28, 8);
    body->setSpacing(24);

    // ============================================================
    // LEFT COLUMN — inputs, live bar, stat strip
    // ============================================================
    QWidget *leftCol = new QWidget();
    leftCol->setStyleSheet(QString("background: transparent; color: %1;").arg(CLR_VALUE));
    QVBoxLayout *left = new QVBoxLayout(leftCol);
    left->setContentsMargins(0, 0, 0, 0);
    left->setSpacing(20);

    // Shared input/button styles
    const QString labelCss = QString(
        "color: %1; font-family: %2; font-size: 11px; letter-spacing: 1.4px; "
        "text-transform: uppercase; background: transparent; border: none;")
        .arg(CLR_LABEL, FONT_SANS);
    const QString inputCss = QString(
        "QLineEdit { background: %1; color: %2; border: 1px solid %3; "
        "border-radius: 6px; padding: 8px 38px 8px 12px; font-family: %4; "
        "font-size: 15px; selection-background-color: %5; }"
        "QLineEdit:focus { border: 1px solid %5; }")
        .arg(CLR_INSET, CLR_VALUE, CLR_BORDER, FONT_MONO, CLR_CYAN);
    const QString getBtnCss = QString(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, "
        "stop:0 #20252D, stop:1 #161A20); color: %1; border: 1px solid #2A3038; "
        "border-radius: 6px; padding: 8px 14px; font-family: %2; font-size: 12px; "
        "font-weight: 500; }"
        "QPushButton:hover { border: 1px solid %3; color: %3; }")
        .arg(CLR_VALUE, FONT_SANS, CLR_CYAN);

    auto buildInputRow = [&](const QString &labelText, QLineEdit *&edit,
                              QPushButton *&getBtn) {
        QWidget *wrap = new QWidget();
        wrap->setStyleSheet("background: transparent;");
        QVBoxLayout *v = new QVBoxLayout(wrap);
        v->setContentsMargins(0, 0, 0, 0);
        v->setSpacing(6);

        QLabel *lab = new QLabel(labelText);
        lab->setStyleSheet(labelCss);
        v->addWidget(lab);

        QWidget *inputWrap = new QWidget();
        inputWrap->setStyleSheet("background: transparent;");
        QHBoxLayout *h = new QHBoxLayout(inputWrap);
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(8);

        // The input has an internal "ADC" suffix label, achieved by stacking
        // a QLabel over a QLineEdit with right-padding.
        QWidget *fieldHost = new QWidget();
        fieldHost->setMinimumHeight(40);
        QHBoxLayout *fh = new QHBoxLayout(fieldHost);
        fh->setContentsMargins(0, 0, 0, 0);
        edit = new QLineEdit(fieldHost);
        edit->setStyleSheet(inputCss);
        edit->setFixedHeight(40);
        QLabel *adcSuffix = new QLabel("ADC", edit);
        adcSuffix->setStyleSheet(QString(
            "color: #4A5260; font-family: %1; font-size: 10px; "
            "letter-spacing: 1px; background: transparent; border: none;")
            .arg(FONT_MONO));
        adcSuffix->setAttribute(Qt::WA_TransparentForMouseEvents);
        // Position the suffix at the right edge of the input
        QObject::connect(edit, &QLineEdit::textChanged, adcSuffix, [edit, adcSuffix]() {
            adcSuffix->move(edit->width() - adcSuffix->sizeHint().width() - 12,
                            (edit->height() - adcSuffix->sizeHint().height()) / 2);
        });
        edit->installEventFilter(nullptr); // keep simple
        // initial position after first show
        QTimer::singleShot(0, edit, [edit, adcSuffix]() {
            adcSuffix->move(edit->width() - adcSuffix->sizeHint().width() - 12,
                            (edit->height() - adcSuffix->sizeHint().height()) / 2);
        });
        fh->addWidget(edit);
        h->addWidget(fieldHost, 1);

        getBtn = new QPushButton("Get Current");
        getBtn->setStyleSheet(getBtnCss);
        getBtn->setCursor(Qt::PointingHandCursor);
        getBtn->setFixedHeight(40);
        h->addWidget(getBtn, 0);

        v->addWidget(inputWrap);
        return wrap;
    };

    QPushButton *getClosedBtn = nullptr;
    QPushButton *getFullBtn = nullptr;
    left->addWidget(buildInputRow("CLOSED THROTTLE ADC COUNT", m_tpsClosedEdit, getClosedBtn));
    left->addWidget(buildInputRow("FULL THROTTLE ADC COUNT",   m_tpsFullEdit,   getFullBtn));

    connect(getClosedBtn, &QPushButton::clicked, this, [this]() {
        if (!m_serialManager || !m_serialManager->isConnected()) {
            QMessageBox::warning(this, "Not Connected",
                                 "Cannot capture TPS ADC - no live serial connection.");
            return;
        }
        m_tpsClosedEdit->setText(QString::number(m_lastTpsADC));
    });
    connect(getFullBtn, &QPushButton::clicked, this, [this]() {
        if (!m_serialManager || !m_serialManager->isConnected()) {
            QMessageBox::warning(this, "Not Connected",
                                 "Cannot capture TPS ADC - no live serial connection.");
            return;
        }
        m_tpsFullEdit->setText(QString::number(m_lastTpsADC));
    });

    // --- Live ADC/TPS row + progress bar ---
    QWidget *liveWrap = new QWidget();
    liveWrap->setStyleSheet("background: transparent;");
    QVBoxLayout *liveCol = new QVBoxLayout(liveWrap);
    liveCol->setContentsMargins(0, 0, 0, 0);
    liveCol->setSpacing(8);

    QHBoxLayout *liveRow = new QHBoxLayout();
    liveRow->setContentsMargins(0, 0, 0, 0);

    // Build ADC + TPS readouts so we keep handles to the value labels.
    QWidget *adcPair = new QWidget(); adcPair->setStyleSheet("background:transparent;");
    QHBoxLayout *aph = new QHBoxLayout(adcPair);
    aph->setContentsMargins(0,0,0,0); aph->setSpacing(8);
    QLabel *adcLab = new QLabel("ADC");
    adcLab->setStyleSheet(QString("color:%1; font-family:%2; font-size:11px; "
                                  "letter-spacing:1.4px; background:transparent;")
                              .arg(CLR_LABEL, FONT_SANS));
    m_tpsLiveAdcLbl = new QLabel("---");
    m_tpsLiveAdcLbl->setStyleSheet(QString("color:%1; font-family:%2; font-size:16px; "
                                           "font-weight:600; background:transparent;")
                                       .arg(CLR_VALUE, FONT_MONO));
    aph->addWidget(adcLab); aph->addWidget(m_tpsLiveAdcLbl);

    QWidget *tpsPair = new QWidget(); tpsPair->setStyleSheet("background:transparent;");
    QHBoxLayout *tph = new QHBoxLayout(tpsPair);
    tph->setContentsMargins(0,0,0,0); tph->setSpacing(8);
    QLabel *tpsLab = new QLabel("TPS");
    tpsLab->setStyleSheet(adcLab->styleSheet());
    m_tpsLivePctLbl = new QLabel("---%");
    m_tpsLivePctLbl->setStyleSheet(QString("color:%1; font-family:%2; font-size:16px; "
                                           "font-weight:600; background:transparent;")
                                       .arg(CLR_CYAN, FONT_MONO));
    tph->addWidget(tpsLab); tph->addWidget(m_tpsLivePctLbl);

    liveRow->addWidget(adcPair);
    liveRow->addStretch();
    liveRow->addWidget(tpsPair);
    liveCol->addLayout(liveRow);

    // Progress bar
    m_tpsLiveBar = new QProgressBar();
    m_tpsLiveBar->setRange(0, 100);
    m_tpsLiveBar->setValue(0);
    m_tpsLiveBar->setTextVisible(false);
    m_tpsLiveBar->setFixedHeight(12);
    m_tpsLiveBar->setStyleSheet(QString(
        "QProgressBar { border: 1px solid %1; background: %2; border-radius: 6px; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, "
        "stop:0 #0891B2, stop:0.6 %3, stop:1 #67E8F9); border-radius: 6px; }")
        .arg(CLR_BORDER, CLR_INSET, CLR_CYAN));
    liveCol->addWidget(m_tpsLiveBar);

    // 0% / 50% / 100% scale row
    QHBoxLayout *scaleRow = new QHBoxLayout();
    scaleRow->setContentsMargins(0, 0, 0, 0);
    const QString scaleStyle = QString("color:%1; font-family:%2; font-size:9px; "
                                       "background:transparent;")
                                   .arg(CLR_DIM, FONT_MONO);
    QLabel *s0   = new QLabel("0%");   s0->setStyleSheet(scaleStyle);
    QLabel *s50  = new QLabel("50%");  s50->setStyleSheet(scaleStyle);
    QLabel *s100 = new QLabel("100%"); s100->setStyleSheet(scaleStyle);
    scaleRow->addWidget(s0);
    scaleRow->addStretch();
    scaleRow->addWidget(s50);
    scaleRow->addStretch();
    scaleRow->addWidget(s100);
    liveCol->addLayout(scaleRow);

    left->addWidget(liveWrap);

    // --- 4-cell stat strip (ADC / RANGE / DEADBAND / STATUS) ---
    QWidget *strip = new QWidget();
    strip->setStyleSheet(QString("background: %1; border: 1px solid %2; "
                                 "border-radius: 6px;").arg(CLR_INSET, CLR_BORDER));
    strip->setFixedHeight(56);
    QHBoxLayout *sh = new QHBoxLayout(strip);
    sh->setContentsMargins(0, 0, 0, 0);
    sh->setSpacing(0);

    auto makeStatCell = [&](const QString &label, const QString &initialVal,
                            const QString &valColor, bool divider, QLabel **outVal) {
        QWidget *cell = new QWidget();
        cell->setStyleSheet(QString("background: transparent; %1")
            .arg(divider ? "border-left: 1px solid #1A1E25;" : QString()));
        QVBoxLayout *v = new QVBoxLayout(cell);
        v->setContentsMargins(8, 8, 8, 8);
        v->setSpacing(2);
        v->setAlignment(Qt::AlignCenter);
        QLabel *lab = new QLabel(label);
        lab->setAlignment(Qt::AlignHCenter);
        lab->setStyleSheet(QString("color:%1; font-family:%2; font-size:9px; "
                                   "letter-spacing:1.4px; background:transparent;")
                               .arg(CLR_DIM, FONT_SANS));
        QLabel *val = new QLabel(initialVal);
        val->setAlignment(Qt::AlignHCenter);
        val->setStyleSheet(QString("color:%1; font-family:%2; font-size:14px; "
                                   "font-weight:600; background:transparent;")
                               .arg(valColor, FONT_MONO));
        v->addWidget(lab);
        v->addWidget(val);
        sh->addWidget(cell, 1);
        if (outVal) *outVal = val;
    };

    makeStatCell("ADC",      "0",   CLR_VALUE, false, &m_tpsStatAdc);
    makeStatCell("RANGE",    "0",   CLR_VALUE, true,  &m_tpsStatRange);
    makeStatCell("DEADBAND", "0",   CLR_VALUE, true,  &m_tpsStatDead);
    makeStatCell("STATUS",   "OK",  CLR_NEON,  true,  nullptr);

    left->addWidget(strip);
    left->addStretch();

    body->addWidget(leftCol, 1);

    // ============================================================
    // RIGHT COLUMN — throttle-body live view
    // ============================================================
    QWidget *rightCol = new QWidget();
    rightCol->setStyleSheet(QString(
        "background: qradialgradient(cx:0.5, cy:0.4, radius:0.9, "
        "stop:0 #15191F, stop:1 #0B0D11); "
        "border: 1px solid #1F242C; border-radius: 8px;"));
    rightCol->setMinimumWidth(360);

    QVBoxLayout *right = new QVBoxLayout(rightCol);
    right->setContentsMargins(16, 14, 16, 14);
    right->setSpacing(10);

    QHBoxLayout *hudHeader = new QHBoxLayout();
    QLabel *hudTitle = new QLabel("THROTTLE  ·  LIVE VIEW");
    hudTitle->setStyleSheet(QString("color:%1; font-family:%2; font-size:10px; "
                                    "letter-spacing:1.5px; background:transparent;")
                                .arg(CLR_LABEL, FONT_MONO));
    m_tpsAngleLbl = new QLabel("0.0°");
    m_tpsAngleLbl->setStyleSheet(QString("color:%1; font-family:%2; font-size:10px; "
                                         "letter-spacing:1.2px; background:transparent;")
                                     .arg(CLR_CYAN, FONT_MONO));
    hudHeader->addWidget(hudTitle);
    hudHeader->addStretch();
    hudHeader->addWidget(m_tpsAngleLbl);
    right->addLayout(hudHeader);

    m_throttleBody = new ThrottleBodyWidget(rightCol);
    m_throttleBody->setStyleSheet("background: transparent;");
    right->addWidget(m_throttleBody, 1, Qt::AlignHCenter);

    // HUD readouts (OPEN / ANGLE)
    QHBoxLayout *hudRow = new QHBoxLayout();
    hudRow->setSpacing(10);
    auto makeHud = [&](const QString &label, const QString &val,
                       const QString &valColor, QLabel **outVal) {
        QWidget *box = new QWidget();
        box->setStyleSheet(QString("background: rgba(13,15,18,180); "
                                   "border: 1px solid #1F242C; border-radius: 6px;"));
        QVBoxLayout *v = new QVBoxLayout(box);
        v->setContentsMargins(12, 8, 12, 8);
        v->setSpacing(2);
        QLabel *lab = new QLabel(label);
        lab->setStyleSheet(QString("color:%1; font-family:%2; font-size:9px; "
                                   "letter-spacing:1.4px; background:transparent;")
                               .arg(CLR_LABEL, FONT_SANS));
        QLabel *value = new QLabel(val);
        value->setStyleSheet(QString("color:%1; font-family:%2; font-size:16px; "
                                     "font-weight:600; background:transparent;")
                                 .arg(valColor, FONT_MONO));
        v->addWidget(lab);
        v->addWidget(value);
        if (outVal) *outVal = value;
        return box;
    };
    hudRow->addWidget(makeHud("OPEN",  "0.0%", CLR_CYAN,  &m_tpsHudOpenLbl),  1);
    hudRow->addWidget(makeHud("ANGLE", "0.0°", CLR_VALUE, &m_tpsHudAngleLbl), 1);
    right->addLayout(hudRow);

    body->addWidget(rightCol, 1);
    l->addLayout(body);

    // ============================================================
    // FOOTER  (ECU sync + Cancel / Accept)
    // ============================================================
    QWidget *footer = new QWidget();
    footer->setStyleSheet(QString("background:#13161A; border-top: 1px solid %1;")
                              .arg(CLR_BORDER2));
    footer->setFixedHeight(60);
    QHBoxLayout *fh = new QHBoxLayout(footer);
    fh->setContentsMargins(28, 0, 28, 0);
    QLabel *ecuLab = new QLabel("ECU  ·  SYNC OK");
    ecuLab->setStyleSheet(QString("color:%1; font-family:%2; font-size:10px; "
                                  "letter-spacing:1.4px; background:transparent;")
                              .arg(CLR_DIM, FONT_MONO));
    fh->addWidget(ecuLab);
    fh->addStretch();

    QPushButton *cancelBtn = new QPushButton("Cancel");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(QString(
        "QPushButton { background: transparent; color:%1; border:1px solid #2A3038; "
        "border-radius:6px; padding:8px 22px; font-family:%2; font-size:13px; }"
        "QPushButton:hover { background:#1B1F25; }")
        .arg(CLR_LABEL, FONT_SANS));
    QPushButton *acceptBtn = new QPushButton("Accept");
    acceptBtn->setCursor(Qt::PointingHandCursor);
    acceptBtn->setStyleSheet(QString(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, "
        "stop:0 #00F082, stop:1 %1); color:#001A0A; border:none; border-radius:6px; "
        "padding:8px 26px; font-family:%2; font-size:13px; font-weight:700; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, "
        "stop:0 #1FFF8F, stop:1 #00D85A); }")
        .arg(CLR_NEON, FONT_SANS));
    connect(cancelBtn, &QPushButton::clicked, this, &CalibrationOverlay::closeOverlay);
    connect(acceptBtn, &QPushButton::clicked, this, &CalibrationOverlay::onTpsAccept);
    fh->addWidget(cancelBtn);
    fh->addSpacing(10);
    fh->addWidget(acceptBtn);
    l->addWidget(footer);

    m_stack->addWidget(m_tpsPanel);
}

void CalibrationOverlay::buildMapPanel() {
    const QString CLR_BG_PANEL = "#12161A";
    const QString CLR_INSET    = "#0B0E12";
    const QString CLR_BORDER   = "#1E2228";
    const QString CLR_BORDER2  = "#23272E";
    const QString CLR_LABEL    = "#B0B5C0";
    const QString CLR_VALUE    = "#FFFFFF";
    const QString CLR_CYAN     = "#22D3EE";
    const QString CLR_NEON     = "#00C853";
    const QString FONT_SANS    = "'Inter', system-ui, sans-serif";
    const QString FONT_MONO    = "'JetBrains Mono', monospace";

    m_mapPanel = new QWidget();
    m_mapPanel->setStyleSheet(QString("QWidget { background-color: %1; }").arg(CLR_BG_PANEL));

    QVBoxLayout *l = new QVBoxLayout(m_mapPanel);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);

    // Two-column layout matching the TPS panel style
    QHBoxLayout *body = new QHBoxLayout();
    body->setContentsMargins(28, 8, 28, 8);
    body->setSpacing(24);

    // ── LEFT COLUMN: controls ──────────────────────────────────────────
    QWidget *leftCol = new QWidget();
    leftCol->setStyleSheet("background: transparent;");
    QVBoxLayout *left = new QVBoxLayout(leftCol);
    left->setContentsMargins(0, 0, 0, 0);
    left->setSpacing(18);

    const QString labelCss = QString(
        "color: %1; font-family: %2; font-size: 11px; letter-spacing: 1.4px; "
        "background: transparent; border: none;").arg(CLR_LABEL, FONT_SANS);
    const QString fieldCss = QString(
        "QComboBox, QLineEdit { background: %1; color: %2; border: 1px solid %3; "
        "border-radius: 6px; padding: 8px 12px; font-family: %4; font-size: 14px; }"
        "QComboBox:focus, QLineEdit:focus { border-color: %5; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { background: %1; color: %2; selection-background-color: %5; }")
        .arg(CLR_INSET, CLR_VALUE, CLR_BORDER, FONT_MONO, CLR_CYAN);

    // Sensor selector
    auto makeRow = [&](const QString &lbl, QWidget *field) {
        QWidget *w = new QWidget(); w->setStyleSheet("background:transparent;");
        QVBoxLayout *v = new QVBoxLayout(w); v->setContentsMargins(0,0,0,0); v->setSpacing(6);
        QLabel *lab = new QLabel(lbl); lab->setStyleSheet(labelCss);
        v->addWidget(lab); v->addWidget(field);
        return w;
    };

    m_mapSensorCombo = new QComboBox();
    m_mapSensorCombo->addItems({"GM 1-BAR", "GM 2-BAR", "GM 3-BAR", "MPX4115", "MPX4250", "Custom"});
    m_mapSensorCombo->setStyleSheet(fieldCss);
    left->addWidget(makeRow("COMMON PRESSURE SENSORS", m_mapSensorCombo));

    m_map0Edit = new QLineEdit(); m_map0Edit->setStyleSheet(fieldCss);
    left->addWidget(makeRow("kPa AT 0.0 VOLTS", m_map0Edit));

    m_map5Edit = new QLineEdit(); m_map5Edit->setStyleSheet(fieldCss);
    left->addWidget(makeRow("kPa AT 5.0 VOLTS", m_map5Edit));

    connect(m_mapSensorCombo, &QComboBox::currentTextChanged, this, [this](const QString &name) {
        if (name == "GM 1-BAR")        { m_map0Edit->setText("10");  m_map5Edit->setText("105"); }
        else if (name == "GM 2-BAR")   { m_map0Edit->setText("8");   m_map5Edit->setText("208"); }
        else if (name == "GM 3-BAR")   { m_map0Edit->setText("4");   m_map5Edit->setText("304"); }
        else if (name == "MPX4115")    { m_map0Edit->setText("10");  m_map5Edit->setText("115"); }
        else if (name == "MPX4250")    { m_map0Edit->setText("20");  m_map5Edit->setText("250"); }
        if (m_mapVisual) m_mapVisual->setKpa(0.0);
    });

    // Live kPa readout + bar
    QWidget *liveWrap = new QWidget(); liveWrap->setStyleSheet("background:transparent;");
    QVBoxLayout *lv = new QVBoxLayout(liveWrap); lv->setContentsMargins(0,0,0,0); lv->setSpacing(6);
    m_mapLiveKpaLbl = new QLabel("MAP: --- kPa");
    m_mapLiveKpaLbl->setStyleSheet(QString(
        "color: %1; font-family: %2; font-size: 15px; font-weight: 600; background:transparent;")
        .arg(CLR_CYAN, FONT_MONO));
    lv->addWidget(m_mapLiveKpaLbl);
    m_mapLiveBar = new QProgressBar();
    m_mapLiveBar->setRange(0, 300); m_mapLiveBar->setValue(0);
    m_mapLiveBar->setTextVisible(false); m_mapLiveBar->setFixedHeight(10);
    m_mapLiveBar->setStyleSheet(QString(
        "QProgressBar { border:1px solid %1; background:%2; border-radius:5px; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #0891B2, stop:0.6 %3, stop:1 #67E8F9); border-radius:5px; }")
        .arg(CLR_BORDER, CLR_INSET, CLR_CYAN));
    lv->addWidget(m_mapLiveBar);
    left->addWidget(liveWrap);

    left->addStretch();

    // Footer buttons
    QPushButton *burnBtn = new QPushButton("Burn");
    burnBtn->setCursor(Qt::PointingHandCursor);
    burnBtn->setStyleSheet(QString(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #F59E0B,stop:1 #B45309); "
        "color: #000; border:none; border-radius:6px; padding:9px 20px; font-family:%1; "
        "font-size:13px; font-weight:700; }")
        .arg(FONT_SANS));
    QPushButton *closeBtn2 = new QPushButton("Close");
    closeBtn2->setCursor(Qt::PointingHandCursor);
    closeBtn2->setStyleSheet(QString(
        "QPushButton { background:transparent; color:%1; border:1px solid #2A3038; "
        "border-radius:6px; padding:9px 20px; font-family:%2; font-size:13px; }")
        .arg(CLR_LABEL, FONT_SANS));
    connect(closeBtn2, &QPushButton::clicked, this, &CalibrationOverlay::closeOverlay);
    connect(burnBtn,   &QPushButton::clicked, this, &CalibrationOverlay::onMapBurn);
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(); btnRow->addWidget(burnBtn); btnRow->addSpacing(8); btnRow->addWidget(closeBtn2);
    left->addLayout(btnRow);

    body->addWidget(leftCol, 1);

    // ── RIGHT COLUMN: 3D pressure dial ────────────────────────────────
    QWidget *rightCol = new QWidget();
    rightCol->setStyleSheet(QString(
        "background: qradialgradient(cx:0.5,cy:0.4,radius:0.9,"
        "stop:0 #15191F,stop:1 #0B0D11);"
        "border:1px solid #1F242C; border-radius:8px;"));
    rightCol->setMinimumWidth(300);

    QVBoxLayout *right = new QVBoxLayout(rightCol);
    right->setContentsMargins(12, 12, 12, 12);
    right->setSpacing(8);

    QLabel *vizTitle = new QLabel("MAP  ·  LIVE VIEW");
    vizTitle->setStyleSheet(QString(
        "color:%1; font-family:%2; font-size:10px; letter-spacing:1.5px; background:transparent;")
        .arg(CLR_LABEL, FONT_MONO));
    right->addWidget(vizTitle);

    m_mapVisual = new MapSensorWidget(rightCol);
    m_mapVisual->setStyleSheet("background: transparent;");
    right->addWidget(m_mapVisual, 1, Qt::AlignHCenter);

    body->addWidget(rightCol, 1);
    l->addLayout(body);

    // Footer status bar
    QWidget *footer = new QWidget();
    footer->setStyleSheet(QString("background:#13161A; border-top:1px solid %1;").arg(CLR_BORDER2));
    footer->setFixedHeight(48);
    QHBoxLayout *fh = new QHBoxLayout(footer);
    fh->setContentsMargins(28, 0, 28, 0);
    QLabel *stat = new QLabel("SENSOR  ·  CALIBRATION");
    stat->setStyleSheet(QString("color:#3A4252; font-family:%1; font-size:10px; letter-spacing:1.4px; background:transparent;").arg(FONT_MONO));
    fh->addWidget(stat); fh->addStretch();
    l->addWidget(footer);

    m_stack->addWidget(m_mapPanel);
}

void CalibrationOverlay::buildTempPanel() {
    const QString CLR_BG_PANEL = "#12161A";
    const QString CLR_INSET    = "#0B0E12";
    const QString CLR_BORDER   = "#1E2228";
    const QString CLR_BORDER2  = "#23272E";
    const QString CLR_LABEL    = "#B0B5C0";
    const QString CLR_VALUE    = "#FFFFFF";
    const QString CLR_CYAN     = "#22D3EE";
    const QString CLR_ORANGE   = "#F59E0B";
    const QString CLR_NEON     = "#00C853";
    const QString FONT_SANS    = "'Inter', system-ui, sans-serif";
    const QString FONT_MONO    = "'JetBrains Mono', monospace";

    m_tempPanel = new QWidget();
    m_tempPanel->setStyleSheet(QString("QWidget { background-color: %1; }").arg(CLR_BG_PANEL));

    QVBoxLayout *l = new QVBoxLayout(m_tempPanel);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);

    // Two-column layout
    QHBoxLayout *body = new QHBoxLayout();
    body->setContentsMargins(28, 8, 28, 8);
    body->setSpacing(24);

    // ── LEFT COLUMN: controls ──────────────────────────────────────────
    QWidget *leftCol = new QWidget();
    leftCol->setStyleSheet("background:transparent;");
    QVBoxLayout *left = new QVBoxLayout(leftCol);
    left->setContentsMargins(0, 0, 0, 0);
    left->setSpacing(14);

    const QString labelCss = QString(
        "color:%1; font-family:%2; font-size:11px; letter-spacing:1.4px; background:transparent; border:none;")
        .arg(CLR_LABEL, FONT_SANS);
    const QString fieldCss = QString(
        "QComboBox, QLineEdit { background:%1; color:%2; border:1px solid %3; "
        "border-radius:6px; padding:7px 12px; font-family:%4; font-size:13px; }"
        "QComboBox:focus, QLineEdit:focus { border-color:%5; }"
        "QComboBox::drop-down { border:none; }"
        "QComboBox QAbstractItemView { background:%1; color:%2; selection-background-color:%5; }")
        .arg(CLR_INSET, CLR_VALUE, CLR_BORDER, FONT_MONO, CLR_CYAN);

    auto makeRow = [&](const QString &lbl, QWidget *field) {
        QWidget *w = new QWidget(); w->setStyleSheet("background:transparent;");
        QVBoxLayout *v = new QVBoxLayout(w); v->setContentsMargins(0,0,0,0); v->setSpacing(5);
        QLabel *lab = new QLabel(lbl); lab->setStyleSheet(labelCss);
        v->addWidget(lab); v->addWidget(field);
        return w;
    };

    m_tempSensorCombo = new QComboBox();
    m_tempSensorCombo->addItems({"GM", "RX-7 S4/S5", "Ford", "Bosch", "Custom"});
    m_tempSensorCombo->setStyleSheet(fieldCss);
    left->addWidget(makeRow("COMMON SENSOR VALUES", m_tempSensorCombo));

    m_tempBiasEdit = new QLineEdit(); m_tempBiasEdit->setStyleSheet(fieldCss);
    left->addWidget(makeRow("BIAS RESISTOR (OHMS)", m_tempBiasEdit));

    // 3-point calibration header
    QLabel *calHeader = new QLabel("3-POINT CALIBRATION");
    calHeader->setStyleSheet(QString("color:%1; font-family:%2; font-size:11px; font-weight:700; "
                                    "letter-spacing:1.4px; background:transparent;").arg(CLR_NEON, FONT_SANS));
    left->addWidget(calHeader);

    m_tempR1Edit = new QLineEdit(); m_tempR1Edit->setStyleSheet(fieldCss);
    m_tempT1Edit = new QLineEdit(); m_tempT1Edit->setStyleSheet(fieldCss);
    m_tempR2Edit = new QLineEdit(); m_tempR2Edit->setStyleSheet(fieldCss);
    m_tempT2Edit = new QLineEdit(); m_tempT2Edit->setStyleSheet(fieldCss);
    m_tempR3Edit = new QLineEdit(); m_tempR3Edit->setStyleSheet(fieldCss);
    m_tempT3Edit = new QLineEdit(); m_tempT3Edit->setStyleSheet(fieldCss);

    auto makePair = [&](const QString &rl, QLineEdit *re, const QString &tl, QLineEdit *te) {
        QWidget *w = new QWidget(); w->setStyleSheet("background:transparent;");
        QHBoxLayout *h = new QHBoxLayout(w); h->setContentsMargins(0,0,0,0); h->setSpacing(10);
        h->addWidget(makeRow(rl, re), 1);
        h->addWidget(makeRow(tl, te), 1);
        return w;
    };
    left->addWidget(makePair("RESISTANCE 1 (Ω)", m_tempR1Edit, "TEMP 1 (°C)", m_tempT1Edit));
    left->addWidget(makePair("RESISTANCE 2 (Ω)", m_tempR2Edit, "TEMP 2 (°C)", m_tempT2Edit));
    left->addWidget(makePair("RESISTANCE 3 (Ω)", m_tempR3Edit, "TEMP 3 (°C)", m_tempT3Edit));

    connect(m_tempSensorCombo, &QComboBox::currentTextChanged, this, [this](const QString &name) {
        auto fill = [this](const QString &bias, const QString &r1, const QString &t1,
                           const QString &r2, const QString &t2, const QString &r3, const QString &t3) {
            m_tempBiasEdit->setText(bias);
            m_tempR1Edit->setText(r1); m_tempT1Edit->setText(t1);
            m_tempR2Edit->setText(r2); m_tempT2Edit->setText(t2);
            m_tempR3Edit->setText(r3); m_tempT3Edit->setText(t3);
        };
        if (name == "GM")              fill("2490", "9420",  "0", "2490",  "25", "667",  "80");
        else if (name == "RX-7 S4/S5") fill("2490", "20000", "0", "5200",  "25", "1300", "70");
        else if (name == "Ford")       fill("2490", "95540", "0", "37300", "25", "5570", "80");
        else if (name == "Bosch")      fill("2490", "5896",  "0", "2500",  "20", "323",  "100");
    });

    // Live bar + label
    QWidget *liveWrap = new QWidget(); liveWrap->setStyleSheet("background:transparent;");
    QVBoxLayout *lv = new QVBoxLayout(liveWrap); lv->setContentsMargins(0,0,0,0); lv->setSpacing(6);
    m_tempLiveLbl = new QLabel("Temp: --- C");
    m_tempLiveLbl->setStyleSheet(QString(
        "color:%1; font-family:%2; font-size:15px; font-weight:600; background:transparent;")
        .arg(CLR_ORANGE, FONT_MONO));
    lv->addWidget(m_tempLiveLbl);
    m_tempLiveBar = new QProgressBar();
    m_tempLiveBar->setRange(-40, 215); m_tempLiveBar->setValue(-40);
    m_tempLiveBar->setTextVisible(false); m_tempLiveBar->setFixedHeight(10);
    m_tempLiveBar->setStyleSheet(QString(
        "QProgressBar { border:1px solid %1; background:%2; border-radius:5px; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #2563EB,stop:0.45 #22C55E,stop:0.75 %3,stop:1 #EF4444); border-radius:5px; }")
        .arg(CLR_BORDER, CLR_INSET, CLR_ORANGE));
    lv->addWidget(m_tempLiveBar);
    left->addWidget(liveWrap);
    left->addStretch();

    // Footer buttons
    QPushButton *burnBtn = new QPushButton("Write to Controller");
    burnBtn->setCursor(Qt::PointingHandCursor);
    burnBtn->setStyleSheet(QString(
        "QPushButton { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #F59E0B,stop:1 #B45309); "
        "color:#000; border:none; border-radius:6px; padding:9px 20px; "
        "font-family:%1; font-size:13px; font-weight:700; }").arg(FONT_SANS));
    QPushButton *closeBtn2 = new QPushButton("Close");
    closeBtn2->setCursor(Qt::PointingHandCursor);
    closeBtn2->setStyleSheet(QString(
        "QPushButton { background:transparent; color:%1; border:1px solid #2A3038; "
        "border-radius:6px; padding:9px 20px; font-family:%2; font-size:13px; }")
        .arg(CLR_LABEL, FONT_SANS));
    connect(closeBtn2, &QPushButton::clicked, this, &CalibrationOverlay::closeOverlay);
    connect(burnBtn,   &QPushButton::clicked, this, &CalibrationOverlay::onTempBurn);
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(); btnRow->addWidget(burnBtn); btnRow->addSpacing(8); btnRow->addWidget(closeBtn2);
    left->addLayout(btnRow);

    body->addWidget(leftCol, 1);

    // ── RIGHT COLUMN: 3D thermometer probe ───────────────────────────
    QWidget *rightCol = new QWidget();
    rightCol->setStyleSheet(QString(
        "background:qradialgradient(cx:0.5,cy:0.4,radius:0.9,"
        "stop:0 #15191F,stop:1 #0B0D11);"
        "border:1px solid #1F242C; border-radius:8px;"));
    rightCol->setMinimumWidth(240);

    QVBoxLayout *right = new QVBoxLayout(rightCol);
    right->setContentsMargins(12, 14, 12, 14);
    right->setSpacing(8);

    QLabel *vizTitle = new QLabel("TEMP  ·  LIVE VIEW");
    vizTitle->setStyleSheet(QString(
        "color:%1; font-family:%2; font-size:10px; letter-spacing:1.5px; background:transparent;")
        .arg(CLR_LABEL, FONT_MONO));
    right->addWidget(vizTitle);

    m_tempVisual = new TempSensorWidget(rightCol);
    m_tempVisual->setStyleSheet("background:transparent;");
    m_tempVisual->setCltMode(true);   // default; flipped in showCalibration
    right->addWidget(m_tempVisual, 1, Qt::AlignHCenter);

    body->addWidget(rightCol, 0);
    l->addLayout(body);

    // Footer
    QWidget *footer = new QWidget();
    footer->setStyleSheet(QString("background:#13161A; border-top:1px solid %1;").arg(CLR_BORDER2));
    footer->setFixedHeight(48);
    QHBoxLayout *fh = new QHBoxLayout(footer);
    fh->setContentsMargins(28, 0, 28, 0);
    QLabel *stat = new QLabel("SENSOR  ·  CALIBRATION");
    stat->setStyleSheet(QString("color:#3A4252; font-family:%1; font-size:10px; letter-spacing:1.4px; background:transparent;").arg(FONT_MONO));
    fh->addWidget(stat); fh->addStretch();
    l->addWidget(footer);

    m_stack->addWidget(m_tempPanel);
}

void CalibrationOverlay::buildDutyPanel() {
    const QString CLR_BG_PANEL = "#12161A";
    const QString CLR_INSET    = "#0B0E12";
    const QString CLR_BORDER   = "#1E2228";
    const QString CLR_BORDER2  = "#23272E";
    const QString CLR_LABEL    = "#B0B5C0";
    const QString CLR_CYAN     = "#22D3EE";
    const QString FONT_SANS    = "'Inter', system-ui, sans-serif";
    const QString FONT_MONO    = "'JetBrains Mono', monospace";

    m_dutyPanel = new QWidget();
    m_dutyPanel->setStyleSheet(QString("QWidget { background-color: %1; }").arg(CLR_BG_PANEL));

    QVBoxLayout *l = new QVBoxLayout(m_dutyPanel);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);

    // Two-column layout
    QHBoxLayout *body = new QHBoxLayout();
    body->setContentsMargins(28, 8, 28, 8);
    body->setSpacing(24);

    // ── LEFT COLUMN: Information ─────────────────────────────────────────
    QWidget *leftCol = new QWidget();
    leftCol->setStyleSheet("background:transparent;");
    QVBoxLayout *left = new QVBoxLayout(leftCol);
    left->setContentsMargins(0, 0, 0, 0);
    left->setSpacing(14);

    QLabel *infoIcon = new QLabel("ℹ️");
    infoIcon->setStyleSheet("font-size: 24px; background: transparent;");
    left->addWidget(infoIcon, 0, Qt::AlignLeft);

    QLabel *infoDesc = new QLabel(
        "Injector Duty Cycle represents the percentage of time "
        "the fuel injector remains open during one engine cycle.\n\n"
        "A duty cycle above 85% suggests the injectors are "
        "near maximum capacity and may go static (staying open 100% of the time).\n\n"
        "Adjust required fuel and injector dead-time in Fuel Settings."
    );
    infoDesc->setWordWrap(true);
    infoDesc->setStyleSheet(QString(
        "color:%1; font-family:%2; font-size:13px; line-height: 1.5; "
        "background:transparent;")
        .arg(CLR_LABEL, FONT_SANS));
    left->addWidget(infoDesc);

    // Live bar + label
    QWidget *liveWrap = new QWidget(); liveWrap->setStyleSheet("background:transparent;");
    QVBoxLayout *lv = new QVBoxLayout(liveWrap); lv->setContentsMargins(0,0,0,0); lv->setSpacing(6);
    m_dutyLiveLbl = new QLabel("Duty: --- %");
    m_dutyLiveLbl->setStyleSheet(QString(
        "color:%1; font-family:%2; font-size:15px; font-weight:600; background:transparent;")
        .arg(CLR_CYAN, FONT_MONO));
    lv->addWidget(m_dutyLiveLbl);
    m_dutyLiveBar = new QProgressBar();
    m_dutyLiveBar->setRange(0, 100); m_dutyLiveBar->setValue(0);
    m_dutyLiveBar->setTextVisible(false); m_dutyLiveBar->setFixedHeight(10);
    m_dutyLiveBar->setStyleSheet(QString(
        "QProgressBar { border:1px solid %1; background:%2; border-radius:5px; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #3B82F6,stop:0.6 #0EA5E9,stop:0.85 #F59E0B,stop:1 #EF4444); border-radius:5px; }")
        .arg(CLR_BORDER, CLR_INSET));
    lv->addWidget(m_dutyLiveBar);
    left->addWidget(liveWrap);
    left->addStretch();

    // Footer buttons
    QPushButton *closeBtn2 = new QPushButton("Close");
    closeBtn2->setCursor(Qt::PointingHandCursor);
    closeBtn2->setStyleSheet(QString(
        "QPushButton { background:transparent; color:%1; border:1px solid #2A3038; "
        "border-radius:6px; padding:9px 20px; font-family:%2; font-size:13px; }")
        .arg(CLR_LABEL, FONT_SANS));
    connect(closeBtn2, &QPushButton::clicked, this, &CalibrationOverlay::closeOverlay);
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(); btnRow->addWidget(closeBtn2);
    left->addLayout(btnRow);

    body->addWidget(leftCol, 1);

    // ── RIGHT COLUMN: 3D injector view ───────────────────────────────
    QWidget *rightCol = new QWidget();
    rightCol->setStyleSheet(QString(
        "background:qradialgradient(cx:0.5,cy:0.4,radius:0.9,"
        "stop:0 #15191F,stop:1 #0B0D11);"
        "border:1px solid #1F242C; border-radius:8px;"));
    rightCol->setMinimumWidth(280);

    QVBoxLayout *right = new QVBoxLayout(rightCol);
    right->setContentsMargins(12, 14, 12, 14);
    right->setSpacing(8);

    QLabel *vizTitle = new QLabel("INJECTOR  ·  LIVE VIEW");
    vizTitle->setStyleSheet(QString(
        "color:%1; font-family:%2; font-size:10px; letter-spacing:1.5px; background:transparent;")
        .arg(CLR_LABEL, FONT_MONO));
    right->addWidget(vizTitle);

    m_dutyVisual = new InjectorVisualWidget(rightCol);
    m_dutyVisual->setStyleSheet("background:transparent;");
    right->addWidget(m_dutyVisual, 1, Qt::AlignHCenter);

    body->addWidget(rightCol, 1);
    l->addLayout(body);

    // Footer
    QWidget *footer = new QWidget();
    footer->setStyleSheet(QString("background:#13161A; border-top:1px solid %1;").arg(CLR_BORDER2));
    footer->setFixedHeight(48);
    QHBoxLayout *fh = new QHBoxLayout(footer);
    fh->setContentsMargins(28, 0, 28, 0);
    QLabel *stat = new QLabel("STATUS  ·  INFORMATIONAL");
    stat->setStyleSheet(QString("color:#3A4252; font-family:%1; font-size:10px; letter-spacing:1.4px; background:transparent;").arg(FONT_MONO));
    fh->addWidget(stat); fh->addStretch();
    l->addWidget(footer);

    m_stack->addWidget(m_dutyPanel);
}

void CalibrationOverlay::showCalibration(const QString &title) {
    m_titleLabel->setText(title);
    m_currentTitle = title;

    if (title.contains("TPS", Qt::CaseInsensitive)) {
        // Redesigned TPS modal needs a larger canvas
        m_panel->setFixedSize(920, 620);
        loadTpsFromSettings();
        m_stack->setCurrentWidget(m_tpsPanel);
    } else if (title.contains("MAP", Qt::CaseInsensitive)) {
        // Redesigned MAP modal: two columns + 3D pressure dial
        m_panel->setFixedSize(820, 520);
        loadMapFromSettings();
        if (m_mapVisual) m_mapVisual->setKpa(0.0);
        m_stack->setCurrentWidget(m_mapPanel);
    } else if (title.contains("Duty", Qt::CaseInsensitive)) {
        // Redesigned Duty modal: two columns + 3D injector visual
        m_panel->setFixedSize(780, 520);
        if (m_dutyVisual) m_dutyVisual->setDuty(0.0);
        m_stack->setCurrentWidget(m_dutyPanel);
    } else {
        // Redesigned Temp modal: two columns + 3D thermometer probe
        m_panel->setFixedSize(820, 580);
        const bool isClt = title.contains("CLT", Qt::CaseInsensitive);
        loadTempFromSettings();
        m_tempLiveLbl->setText(isClt ? "CLT: --- °C" : "IAT: --- °C");
        if (m_tempVisual) {
            m_tempVisual->setCltMode(isClt);
            m_tempVisual->setTemp(20.0);
        }
        m_stack->setCurrentWidget(m_tempPanel);
    }

    if (parentWidget()) {
        resize(parentWidget()->size());
        m_panel->move((width() - m_panel->width()) / 2, (height() - m_panel->height()) / 2);
    }

    show();
    raise();
}

void CalibrationOverlay::loadTpsFromSettings() {
    int closed = 0;
    int full = 1023;
    if (m_settingsManager) {
        if (m_settingsManager->hasSetting("tpsMin"))
            closed = m_settingsManager->getValue("tpsMin").toInt();
        if (m_settingsManager->hasSetting("tpsMax"))
            full = m_settingsManager->getValue("tpsMax").toInt();
    }
    m_tpsClosedEdit->setText(QString::number(closed));
    m_tpsFullEdit->setText(QString::number(full));
    m_tpsLiveAdcLbl->setText("---");
    m_tpsLivePctLbl->setText("---%");
    m_tpsLiveBar->setValue(0);
    if (m_tpsAngleLbl)    m_tpsAngleLbl->setText("0.0°");
    if (m_tpsHudOpenLbl)  m_tpsHudOpenLbl->setText("0.0%");
    if (m_tpsHudAngleLbl) m_tpsHudAngleLbl->setText("0.0°");
    if (m_tpsStatAdc)     m_tpsStatAdc->setText("0");
    if (m_tpsStatRange)   m_tpsStatRange->setText(QString::number(qMax(0, full - closed)));
    if (m_tpsStatDead)    m_tpsStatDead->setText(QString::number(closed));
    if (m_throttleBody)   m_throttleBody->setTPS(0.0);
}

void CalibrationOverlay::loadMapFromSettings() {
    int mapMin = 10;
    int mapMax = 260;
    if (m_settingsManager) {
        if (m_settingsManager->hasSetting("mapMin"))
            mapMin = m_settingsManager->getValue("mapMin").toInt();
        if (m_settingsManager->hasSetting("mapMax"))
            mapMax = m_settingsManager->getValue("mapMax").toInt();
    }
    m_map0Edit->setText(QString::number(mapMin));
    m_map5Edit->setText(QString::number(mapMax));
    m_mapLiveBar->setRange(0, qMax(100, mapMax));
    m_mapLiveBar->setValue(0);
    m_mapLiveKpaLbl->setText("MAP: --- kPa");
}

void CalibrationOverlay::loadTempFromSettings() {
    m_tempSensorCombo->blockSignals(true);
    m_tempSensorCombo->setCurrentIndex(0);
    m_tempSensorCombo->blockSignals(false);
    m_tempBiasEdit->setText("2490");
    m_tempR1Edit->setText("9420"); m_tempT1Edit->setText("0");
    m_tempR2Edit->setText("2490"); m_tempT2Edit->setText("25");
    m_tempR3Edit->setText("667");  m_tempT3Edit->setText("80");
    m_tempLiveBar->setValue(-40);
    m_tempLiveLbl->setText("Temp: --- C");
}

void CalibrationOverlay::updateLiveData(const RealTimeData &data) {
    if (!isVisible()) return;

    if (m_stack->currentWidget() == m_tpsPanel) {
        m_lastTpsADC = static_cast<int>(data.tpsADC);
        const double tpsPct = data.getTPS();
        const double angle  = (tpsPct / 100.0) * 80.0;

        // Bar + ADC/percent readouts
        m_tpsLiveAdcLbl->setText(QString::number(m_lastTpsADC));
        m_tpsLivePctLbl->setText(QString("%1%").arg(tpsPct, 0, 'f', 1));
        m_tpsLiveBar->setValue(qBound(0, static_cast<int>(tpsPct + 0.5), 100));

        // Live throttle-body animation — drives the rotating butterfly valve.
        if (m_throttleBody) m_throttleBody->setTPS(tpsPct);

        // HUD readouts (next to / under the throttle body)
        if (m_tpsAngleLbl)    m_tpsAngleLbl->setText(QString("%1°").arg(angle, 0, 'f', 1));
        if (m_tpsHudOpenLbl)  m_tpsHudOpenLbl->setText(QString("%1%").arg(tpsPct, 0, 'f', 1));
        if (m_tpsHudAngleLbl) m_tpsHudAngleLbl->setText(QString("%1°").arg(angle, 0, 'f', 1));

        // 4-cell stat strip (ADC / RANGE / DEADBAND / STATUS)
        const int closed = m_tpsClosedEdit->text().toInt();
        const int full   = m_tpsFullEdit->text().toInt();
        if (m_tpsStatAdc)   m_tpsStatAdc->setText(QString::number(m_lastTpsADC));
        if (m_tpsStatRange) m_tpsStatRange->setText(QString::number(qMax(0, full - closed)));
        if (m_tpsStatDead)  m_tpsStatDead->setText(QString::number(closed));
    } else if (m_stack->currentWidget() == m_mapPanel) {
        const double kpa = data.getMAP();
        m_mapLiveKpaLbl->setText(QString("MAP: %1 kPa").arg(kpa, 0, 'f', 0));
        m_mapLiveBar->setValue(qBound(m_mapLiveBar->minimum(),
                                      static_cast<int>(kpa + 0.5),
                                      m_mapLiveBar->maximum()));
        if (m_mapVisual) m_mapVisual->setKpa(kpa);
    } else if (m_stack->currentWidget() == m_tempPanel) {
        const bool isIat = m_currentTitle.contains("IAT", Qt::CaseInsensitive);
        const double tempC = isIat ? data.getIAT() : data.getCoolant();
        m_tempLiveLbl->setText(QString("%1: %2 °C").arg(isIat ? "IAT" : "CLT").arg(tempC, 0, 'f', 1));
        m_tempLiveBar->setValue(qBound(m_tempLiveBar->minimum(),
                                       static_cast<int>(tempC + 0.5),
                                       m_tempLiveBar->maximum()));
        if (m_tempVisual) m_tempVisual->setTemp(tempC);
    } else if (m_stack->currentWidget() == m_dutyPanel) {
        const double duty = data.getInjectorDuty();
        m_dutyLiveLbl->setText(QString("Duty: %1 %").arg(duty, 0, 'f', 1));
        m_dutyLiveBar->setValue(qBound(m_dutyLiveBar->minimum(),
                                       static_cast<int>(duty + 0.5),
                                       m_dutyLiveBar->maximum()));
        if (m_dutyVisual) m_dutyVisual->setDuty(duty);
    }
}

void CalibrationOverlay::closeOverlay() {
    hide();
    emit closed();
}

void CalibrationOverlay::awaitBurnAndShowResult(const QString &opName,
                                                bool closeOverlayOnSuccess) {
    // Defer the success/failure dialog until ECUSettingsManager finishes
    // the asynchronous burn chain. burnAllDirty() is fire-and-forget: it
    // queues pages and only reports the outcome later via burnAllComplete
    // / burnAllFailed. Without this wait, the UI would claim success the
    // instant the function returns, even though the ECU may still reject
    // or never acknowledge the burn.
    if (!m_settingsManager) return;

    // Lock out re-entry and freeze the form so the user can't queue another
    // burn (or edit values being burned) while this one is in flight.
    m_burnInProgress = true;
    if (m_panel) m_panel->setEnabled(false);

    // Watchdog: if neither completion nor failure ever arrives (e.g. the
    // ECU is unplugged mid-burn) we still need to release the UI and tell
    // the user. 15s is comfortably longer than a normal page burn cycle.
    auto *watchdog = new QTimer(this);
    watchdog->setSingleShot(true);
    watchdog->setInterval(15000);

    // Held by reference in each lambda capture so any one handler can
    // tear down the other two. std::make_shared keeps the connections
    // alive for the lifetime of the wait, regardless of which side fires.
    struct WaitState {
        QMetaObject::Connection okConn;
        QMetaObject::Connection failConn;
        QMetaObject::Connection watchConn;
        bool done = false;
    };
    auto state = std::make_shared<WaitState>();

    auto finish = [this, state, watchdog, opName, closeOverlayOnSuccess]
                  (bool success, const QString &errorReason) {
        if (state->done) return; // First handler wins; ignore the others.
        state->done = true;

        QObject::disconnect(state->okConn);
        QObject::disconnect(state->failConn);
        QObject::disconnect(state->watchConn);
        watchdog->stop();
        watchdog->deleteLater();

        m_burnInProgress = false;
        if (m_panel) m_panel->setEnabled(true);

        if (success) {
            QMessageBox::information(this, "Success",
                QString("%1 Calibration successfully written to controller.").arg(opName));
            if (closeOverlayOnSuccess) closeOverlay();
        } else {
            QMessageBox::critical(this, "Write Failed",
                QString("%1 calibration write failed: %2")
                    .arg(opName,
                         errorReason.isEmpty() ? QStringLiteral("unknown error") : errorReason));
        }
    };

    state->okConn = connect(m_settingsManager,
                            &ECUSettingsManager::burnAllComplete,
                            this,
                            [finish]() { finish(true, QString()); });

    state->failConn = connect(m_settingsManager,
                              &ECUSettingsManager::burnAllFailed,
                              this,
                              [finish](quint8 page, const QString &reason) {
        finish(false, QString("page %1: %2").arg(page).arg(reason));
    });

    state->watchConn = connect(watchdog, &QTimer::timeout, this, [finish]() {
        finish(false, QStringLiteral("timed out waiting for ECU acknowledgement"));
    });

    watchdog->start();
}

void CalibrationOverlay::onTpsAccept() {
    if (!m_settingsManager) { closeOverlay(); return; }
    bool ok1, ok2;
    int closedCount = m_tpsClosedEdit->text().toInt(&ok1);
    int fullCount = m_tpsFullEdit->text().toInt(&ok2);
    if (!ok1 || !ok2 || closedCount < 0 || fullCount < 0 || closedCount > 1023 || fullCount > 1023) {
        QMessageBox::warning(this, "Validation Error", "TPS values must be valid integers between 0 and 1023.");
        return;
    }
    if (closedCount == fullCount) {
        QMessageBox::warning(this, "Validation Error", "Closed and Full throttle values cannot be identical.");
        return;
    }
    if (!m_settingsManager->hasSetting("tpsMin") || !m_settingsManager->hasSetting("tpsMax")) {
        QMessageBox::critical(this, "ECU Error", "Could not find 'tpsMin' or 'tpsMax' in the loaded ECU definition. Calibration aborted.");
        return;
    }
    // Don't claim success if the burn is going to be silently dropped because
    // the ECU isn't connected or the signature hasn't been validated.
    if (!m_serialManager
        || (!m_serialManager->isSimulation()
            && (!m_serialManager->isConnected()
                || !m_serialManager->isSignatureValidated()))) {
        QMessageBox::critical(this, "Cannot Write",
                              "TPS calibration cannot be written: the ECU is not "
                              "connected, or its signature has not been validated "
                              "against the loaded definition.");
        return;
    }
    if (m_burnInProgress) {
        // Defensive: button should already be disabled via the overlay
        // setEnabled(false) gate, but reject any stray re-entry.
        return;
    }
    m_settingsManager->setValue("tpsMin", closedCount, true);
    m_settingsManager->setValue("tpsMax", fullCount, true);
    // Kick off the async burn chain. Success/error dialogs are deferred to the
    // burnAllComplete / burnAllFailed signals so we never lie about a write
    // that the ECU hasn't actually acknowledged yet.
    awaitBurnAndShowResult("TPS");
    m_settingsManager->burnAllDirty();
}

void CalibrationOverlay::onMapBurn() {
    if (!m_settingsManager) { closeOverlay(); return; }
    bool ok1, ok2;
    int map0 = m_map0Edit->text().toInt(&ok1);
    int map5 = m_map5Edit->text().toInt(&ok2);
    if (!ok1 || !ok2 || map0 < 0 || map5 < 0) {
        QMessageBox::warning(this, "Validation Error", "MAP calibration values must be valid positive integers.");
        return;
    }
    if (map0 == map5) {
        QMessageBox::warning(this, "Validation Error", "0V and 5V MAP values cannot be identical.");
        return;
    }
    if (!m_settingsManager->hasSetting("mapMin") || !m_settingsManager->hasSetting("mapMax")) {
        QMessageBox::critical(this, "ECU Error", "Could not find 'mapMin' or 'mapMax' in the loaded ECU definition. Calibration aborted.");
        return;
    }
    // Don't claim success if the burn is going to be silently dropped because
    // the ECU isn't connected or the signature hasn't been validated.
    if (!m_serialManager
        || (!m_serialManager->isSimulation()
            && (!m_serialManager->isConnected()
                || !m_serialManager->isSignatureValidated()))) {
        QMessageBox::critical(this, "Cannot Write",
                              "MAP calibration cannot be written: the ECU is not "
                              "connected, or its signature has not been validated "
                              "against the loaded definition.");
        return;
    }
    if (m_burnInProgress) {
        return; // Defensive re-entry guard; UI should already be disabled.
    }
    m_settingsManager->setValue("mapMin", map0, true);
    m_settingsManager->setValue("mapMax", map5, true);
    awaitBurnAndShowResult("MAP");
    m_settingsManager->burnAllDirty();
}

void CalibrationOverlay::onTempBurn() {
    if (!m_serialManager) {
        QMessageBox::warning(this, "Not Connected", "Cannot upload calibration - no serial connection.");
        return;
    }
    bool okBias, okR1, okT1, okR2, okT2, okR3, okT3;
    double biasR = m_tempBiasEdit->text().toDouble(&okBias);
    double r1 = m_tempR1Edit->text().toDouble(&okR1);
    double t1 = m_tempT1Edit->text().toDouble(&okT1);
    double r2 = m_tempR2Edit->text().toDouble(&okR2);
    double t2 = m_tempT2Edit->text().toDouble(&okT2);
    double r3 = m_tempR3Edit->text().toDouble(&okR3);
    double t3 = m_tempT3Edit->text().toDouble(&okT3);
    if (!okBias || !okR1 || !okT1 || !okR2 || !okT2 || !okR3 || !okT3) {
        QMessageBox::warning(this, "Invalid Input", "All fields must contain valid numbers.");
        return;
    }
    if (biasR <= 0 || r1 <= 0 || r2 <= 0 || r3 <= 0) {
        QMessageBox::warning(this, "Invalid Input", "Resistance values must be strictly greater than zero.");
        return;
    }
    if (std::abs(r1 - r2) < 0.001 || std::abs(r2 - r3) < 0.001 || std::abs(r1 - r3) < 0.001) {
        QMessageBox::critical(this, "Calculation Error", "Resistance values for the 3 points must be strictly distinct.");
        return;
    }
    if (std::abs(t1 - t2) < 0.001 || std::abs(t2 - t3) < 0.001 || std::abs(t1 - t3) < 0.001) {
        QMessageBox::critical(this, "Calculation Error", "Temperature values for the 3 points must be strictly distinct.");
        return;
    }
    QByteArray table = generateThermistorTable(biasR, r1, t1, r2, t2, r3, t3);
    quint8 tableIdx = m_currentTitle.contains("IAT", Qt::CaseInsensitive) ? 1 : 0;

    // Guard against showing a false "sent" dialog when the ECU isn't actually
    // ready to receive writes. SerialManager::sendCalibrationTable() silently
    // drops the upload (with a logged error) if the ECU isn't connected or the
    // signature hasn't been validated against the loaded definition, so we
    // must check those conditions BEFORE claiming success in the UI.
    const bool inSim = m_serialManager->isSimulation();
    if (!inSim && !m_serialManager->isConnected()) {
        QMessageBox::critical(this, "Not Connected",
                              "Cannot upload calibration: ECU is not connected.");
        return;
    }
    if (!inSim && !m_serialManager->isSignatureValidated()) {
        QMessageBox::critical(this, "Calibration Blocked",
                              "Calibration upload was blocked: the ECU signature has "
                              "not been validated against the loaded definition. "
                              "Connect to a compatible ECU and try again.");
        return;
    }

    m_serialManager->sendCalibrationTable(tableIdx, table);
    QMessageBox::information(this, "Calibration Sent",
                             QString("%1 calibration table (%2 bytes) sent to ECU via 't' command.")
                             .arg(tableIdx == 0 ? "CLT" : "IAT").arg(table.size()));
    closeOverlay();
}

QByteArray CalibrationOverlay::generateThermistorTable(double biasResistor, double r1, double t1, double r2, double t2, double r3, double t3) {
    const int TABLE_SIZE = 1024;
    const double TEMP_OFFSET = 40.0;
    double tk1 = t1 + 273.15;
    double tk2 = t2 + 273.15;
    double tk3 = t3 + 273.15;
    double ln1 = log(r1);
    double ln2 = log(r2);
    double ln3 = log(r3);
    double y1 = 1.0 / tk1;
    double y2 = 1.0 / tk2;
    double y3 = 1.0 / tk3;
    double g2 = (y2 - y1) / (ln2 - ln1);
    double g3 = (y3 - y1) / (ln3 - ln1);
    double C = ((g3 - g2) / (ln3 - ln2)) * (1.0 / (ln1 + ln2 + ln3));
    double B = g2 - C * (ln1 * ln1 + ln1 * ln2 + ln2 * ln2);
    double A = y1 - (B + C * ln1 * ln1) * ln1;
    QByteArray table;
    table.resize(TABLE_SIZE);
    for (int adc = 0; adc < TABLE_SIZE; adc++) {
        double tempC;
        if (adc == 0) tempC = -40.0;
        else if (adc >= 1023) tempC = 200.0;
        else {
            double resistance = biasResistor * (double)adc / (1023.0 - (double)adc);
            if (resistance <= 0) tempC = 200.0;
            else {
                double lnR = log(resistance);
                double invT = A + B * lnR + C * lnR * lnR * lnR;
                if (invT <= 0) tempC = 200.0;
                else tempC = (1.0 / invT) - 273.15;
            }
        }
        if (tempC < -40.0) tempC = -40.0;
        if (tempC > 215.0) tempC = 215.0;
        int byteVal = (int)(tempC + TEMP_OFFSET + 0.5);
        if (byteVal < 0) byteVal = 0;
        if (byteVal > 255) byteVal = 255;
        table[adc] = static_cast<char>(byteVal);
    }
    return table;
}

void CalibrationOverlay::mousePressEvent(QMouseEvent *event) {
    if (!m_panel->geometry().contains(event->pos())) closeOverlay();
}

void CalibrationOverlay::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor(0, 0, 0, 180));
}

void CalibrationOverlay::resizeEvent(QResizeEvent *) {
    if (m_panel) m_panel->move((width() - m_panel->width()) / 2, (height() - m_panel->height()) / 2);
}

// DataCard
DataCard::DataCard(const QString &label, const QColor &dotColor, QWidget *parent) : QFrame(parent) {
    setFixedSize(106, 56);
    setStyleSheet(QString("QFrame { background-color: %1; border-radius: 6px; }").arg(TunerProColors::BG_RAISED));
    QWidget *dot = new QWidget(this);
    dot->setFixedSize(6, 6);
    dot->setStyleSheet(QString("background-color: %1; border-radius: 3px;").arg(dotColor.name()));
    dot->move(width() - 10, 6);
    QLabel *lbl = new QLabel(label, this);
    lbl->setStyleSheet(QString("font-family: 'JetBrains Mono'; font-size: 10px; color: %1;").arg(TunerProColors::TEXT_SECONDARY));
    lbl->move(6, 4);
    m_valLabel = new QLabel("0.0", this);
    m_valLabel->setStyleSheet(QString("font-family: 'JetBrains Mono'; font-size: 20px; font-weight: bold; color: %1;").arg(TunerProColors::TEXT_PRIMARY));
    m_valLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    m_valLabel->setGeometry(8, 20, width() - 16, 30);
}

void DataCard::setValue(const QString &value) {
    m_valLabel->setText(value);
}

// StatusLine
StatusLine::StatusLine(const QString &label, QWidget *parent) : QWidget(parent) {
    setFixedHeight(20);
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(8);
    m_label = new QLabel(label, this);
    m_label->setStyleSheet(QString("font-family: 'Inter'; font-size: 11px; font-weight: 500; color: %1;").arg(TunerProColors::TEXT_SECONDARY));
    m_label->setFixedWidth(80);
    m_indicator = new QWidget(this);
    m_indicator->setFixedSize(8, 8);
    m_indicator->setStyleSheet(QString("background-color: %1; border-radius: 4px;").arg(TunerProColors::BG_OVERLAY));
    lay->addWidget(m_label);
    lay->addStretch();
    lay->addWidget(m_indicator);
    lay->addSpacing(4);
}

void StatusLine::setStatus(bool active, const QColor &activeColor) {
    if (active) {
        m_indicator->setStyleSheet(QString("background-color: %1; border-radius: 4px;").arg(activeColor.name()));
        m_label->setStyleSheet(QString("font-family: 'Inter'; font-size: 11px; font-weight: bold; color: %1;").arg(TunerProColors::TEXT_PRIMARY));
    } else {
        m_indicator->setStyleSheet(QString("background-color: %1; border-radius: 4px;").arg(TunerProColors::BG_OVERLAY));
        m_label->setStyleSheet(QString("font-family: 'Inter'; font-size: 11px; font-weight: 500; color: %1;").arg(TunerProColors::TEXT_SECONDARY));
    }
}

void StatusLine::setPercentage(int percent, const QColor &color) {
    m_indicator->setFixedSize(50 * percent / 100 + 1, 8);
    m_indicator->setStyleSheet(QString("background-color: %1; border-radius: 2px;").arg(color.name()));
}

// DashboardWidget
DashboardWidget::DashboardWidget(QWidget *parent) : QWidget(parent) {
    setupUi();
}

DashboardWidget::~DashboardWidget() {}

void DashboardWidget::setSettingsManager(ECUSettingsManager *mgr) {
    m_settingsManager = mgr;
    if (mgr) {
        connect(mgr, &ECUSettingsManager::settingChanged,
                this, &DashboardWidget::onSettingChanged, Qt::UniqueConnection);
    }
    if (m_calOverlay) m_calOverlay->setSettingsManager(mgr);
}

void DashboardWidget::setSerialManager(SerialManager *sm) {
    if (m_calOverlay) m_calOverlay->setSerialManager(sm);
}

void DashboardWidget::onSettingChanged(const QString &name, const QVariant &value) {
    const double v = value.toDouble();
    const QString fmt1 = QString::number(v, 'f', 1);
    if (name == "coolant" || name == "ect" || name == "egoMinClt") {
        m_cardClt->setValue(fmt1 + " C");
    } else if (name == "iat" || name == "mat") {
        m_cardIat->setValue(fmt1 + " C");
    } else if (name == "batVoltage" || name == "battery" || name == "voltage") {
        m_cardBat->setValue(fmt1 + " V");
    } else if (name == "tps" || name == "tpsThresh") {
        m_cardTps->setValue(fmt1 + " %");
    } else if (name == "stoich" || name == "displayTargetAfr") {
        m_cardO2->setValue(QString::number(v, 'f', 2));
    } else if (name == "crankingAdvance" || name == "fixedDwell") {
        m_cardIgn->setValue(fmt1 + " deg");
    }
}

void DashboardWidget::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    QWidget *topContainer = new QWidget(this);
    topContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QHBoxLayout *topLay = new QHBoxLayout(topContainer);
    topLay->setContentsMargins(0,0,0,0);
    topLay->setSpacing(16);

    m_rpmGauge = new TunerGauge(this);
    m_rpmGauge->setRange(0, 8000);
    m_rpmGauge->setLabel("RPM");
    m_rpmGauge->setDangerThreshold(7000);
    topLay->addStretch(1);
    topLay->addWidget(m_rpmGauge, 0, Qt::AlignCenter);
    topLay->addStretch(1);

    QWidget *gridContainer = new QWidget(this);
    QGridLayout *gridLay = new QGridLayout(gridContainer);
    gridLay->setSpacing(4);
    gridLay->setContentsMargins(0,0,0,0);

    m_cardTps = new DataCard("TPS %", QColor(TunerProColors::CH_TPS));
    m_cardIat = new DataCard("IAT C", QColor(TunerProColors::CH_IAT));
    m_cardClt = new DataCard("CLT C", QColor(TunerProColors::CH_CLT));
    m_cardBat = new DataCard("BATT V", QColor(TunerProColors::CH_BATTERY));
    m_cardO2 = new DataCard("O2 AFR", QColor(TunerProColors::CH_AFR));
    m_cardIgn = new DataCard("IGN BTDC", QColor(TunerProColors::CH_ADVANCE));
    m_cardPw = new DataCard("PW ms", QColor(TunerProColors::CH_FUEL_PW));
    m_cardBaro = new DataCard("BARO kPa", QColor(TunerProColors::CH_BARO));

    gridLay->addWidget(m_cardTps, 0, 0);
    gridLay->addWidget(m_cardIat, 0, 1);
    gridLay->addWidget(m_cardClt, 1, 0);
    gridLay->addWidget(m_cardBat, 1, 1);
    gridLay->addWidget(m_cardO2, 2, 0);
    gridLay->addWidget(m_cardIgn, 2, 1);
    gridLay->addWidget(m_cardPw, 3, 0);
    gridLay->addWidget(m_cardBaro, 3, 1);

    QWidget *gridWrapper = new QWidget(this);
    QVBoxLayout *gridWrapperLay = new QVBoxLayout(gridWrapper);
    gridWrapperLay->addStretch();
    gridWrapperLay->addWidget(gridContainer);
    gridWrapperLay->addStretch();
    topLay->addWidget(gridWrapper, 0, Qt::AlignCenter);
    topLay->addStretch(1);

    QWidget *panelsContainer = new QWidget(this);
    panelsContainer->setFixedWidth(160);
    QVBoxLayout *panelsLay = new QVBoxLayout(panelsContainer);
    panelsLay->setContentsMargins(0,0,0,0);
    panelsLay->setSpacing(16);

    QFrame *fuelPanel = new QFrame(this);
    fuelPanel->setStyleSheet(QString("QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; }").arg(TunerProColors::BG_ELEVATED).arg(TunerProColors::BORDER_SUBTLE));
    QVBoxLayout *fLay = new QVBoxLayout(fuelPanel);
    fLay->setContentsMargins(12, 12, 12, 12);
    fLay->setSpacing(2);
    QLabel *fTitle = new QLabel("FUEL STATUS", fuelPanel);
    fTitle->setStyleSheet(QString("font-family: 'Barlow Condensed'; font-size: 13px; font-weight: bold; color: %1; margin-bottom: 4px;").arg(TunerProColors::TEXT_MUTED));
    m_stFuelPump = new StatusLine("Pump", fuelPanel);
    m_stDuty = new StatusLine("Duty / 100", fuelPanel);
    m_stWarmup = new StatusLine("Warmup", fuelPanel);
    m_stAse = new StatusLine("ASE", fuelPanel);
    m_stTpsWot = new StatusLine("TPS WOT", fuelPanel);
    m_stOverrun = new StatusLine("Decel Cut", fuelPanel);
    fLay->addWidget(fTitle);
    fLay->addWidget(m_stFuelPump);
    fLay->addWidget(m_stDuty);
    fLay->addWidget(m_stWarmup);
    fLay->addWidget(m_stAse);
    fLay->addWidget(m_stTpsWot);
    fLay->addWidget(m_stOverrun);

    QFrame *engPanel = new QFrame(this);
    engPanel->setStyleSheet(fuelPanel->styleSheet());
    QVBoxLayout *eLay = new QVBoxLayout(engPanel);
    eLay->setContentsMargins(12, 12, 12, 12);
    eLay->setSpacing(2);
    QLabel *eTitle = new QLabel("ENGINE STATUS", engPanel);
    eTitle->setStyleSheet(fTitle->styleSheet());
    m_stCranking = new StatusLine("Cranking", engPanel);
    m_stSync = new StatusLine("Sync", engPanel);
    m_stHardLimit = new StatusLine("Hard Limit", engPanel);
    m_stSoftLimit = new StatusLine("Soft Limit", engPanel);
    m_stFan = new StatusLine("Cooling Fan", engPanel);
    m_stKnock = new StatusLine("Knock", engPanel);
    eLay->addWidget(eTitle);
    eLay->addWidget(m_stSync);
    eLay->addWidget(m_stCranking);
    eLay->addWidget(m_stSoftLimit);
    eLay->addWidget(m_stHardLimit);
    eLay->addWidget(m_stFan);
    eLay->addWidget(m_stKnock);

    panelsLay->addWidget(fuelPanel);
    panelsLay->addWidget(engPanel);
    panelsLay->addStretch();
    topLay->addWidget(panelsContainer, 0, Qt::AlignCenter);
    topLay->addStretch(1);

    QWidget *rightGaugesPanel = new QWidget(this);
    QGridLayout *rGLay = new QGridLayout(rightGaugesPanel);
    rGLay->setContentsMargins(0,0,0,0);
    rGLay->setSpacing(8);

    auto setupGauge = [this](TunerGauge*& gauge, const QString& label, double minV, double maxV, double danger) {
        gauge = new TunerGauge(this);
        gauge->setRange(minV, maxV);
        gauge->setLabel(label);
        gauge->setDangerThreshold(danger);
        gauge->setFixedSize(182, 182);
        gauge->setCursor(Qt::PointingHandCursor);
    };

    setupGauge(m_mapGauge, "KPA", 0, 300, 250);
    setupGauge(m_tpsGauge, "TPS %", 0, 100, 90);
    setupGauge(m_cltGauge, "CLT C", 0, 120, 100);
    setupGauge(m_iatGauge, "IAT C", 0, 100, 80);
    setupGauge(m_dutyGauge, "DUTY %", 0, 100, 85);

    rGLay->addWidget(m_mapGauge, 0, 0, 1, 2, Qt::AlignCenter);
    rGLay->addWidget(m_tpsGauge, 0, 2, 1, 2, Qt::AlignCenter);
    rGLay->addWidget(m_dutyGauge, 0, 4, 1, 2, Qt::AlignCenter);
    rGLay->addWidget(m_cltGauge, 1, 1, 1, 2, Qt::AlignCenter);
    rGLay->addWidget(m_iatGauge, 1, 3, 1, 2, Qt::AlignCenter);

    auto handleCalibClick = [this](const QString& title) {
        m_calOverlay->showCalibration(title);
    };
    connect(m_mapGauge, &TunerGauge::clicked, this, [=](){ handleCalibClick("MAP Sensor Calibration"); });
    connect(m_tpsGauge, &TunerGauge::clicked, this, [=](){ handleCalibClick("TPS Calibration"); });
    connect(m_cltGauge, &TunerGauge::clicked, this, [=](){ handleCalibClick("Coolant Temp Calibration"); });
    connect(m_iatGauge, &TunerGauge::clicked, this, [=](){ handleCalibClick("Intake Air Temp Calibration"); });
    connect(m_dutyGauge, &TunerGauge::clicked, this, [=](){ handleCalibClick("Injector Duty %"); });

    topLay->addWidget(rightGaugesPanel, 0, Qt::AlignCenter);
    topLay->addStretch(1);
    mainLayout->addWidget(topContainer);

    QWidget *graphsContainer = new QWidget(this);
    graphsContainer->setMinimumHeight(200);
    graphsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout *gLay = new QHBoxLayout(graphsContainer);
    gLay->setContentsMargins(0,0,0,0);
    gLay->setSpacing(16);

    m_afrGraph = new DataGraph(this);
    m_afrGraph->setTitle("Air/Fuel Ratio");
    m_afrGraph->setUnit("AFR");
    m_afrGraph->setRange(10, 20);
    m_afrGraph->setLineColor(QColor(TunerProColors::CH_AFR));
    m_afrGraph->setBackgroundColor(QColor(TunerProColors::BG_ELEVATED));
    m_afrGraph->setGridColor(QColor(TunerProColors::BORDER_SUBTLE));

    m_advanceGraph = new DataGraph(this);
    m_advanceGraph->setTitle("Ignition Advance");
    m_advanceGraph->setUnit("BTDC");
    m_advanceGraph->setRange(0, 50);
    m_advanceGraph->setLineColor(QColor(TunerProColors::CH_ADVANCE));
    m_advanceGraph->setBackgroundColor(QColor(TunerProColors::BG_ELEVATED));
    m_advanceGraph->setGridColor(QColor(TunerProColors::BORDER_SUBTLE));

    gLay->addWidget(m_afrGraph);
    gLay->addWidget(m_advanceGraph);
    mainLayout->addWidget(graphsContainer, 1);

    m_calOverlay = new CalibrationOverlay(this);
}

void DashboardWidget::resizeEvent(QResizeEvent *event) {
    if (m_calOverlay) m_calOverlay->resize(event->size());
    QWidget::resizeEvent(event);
}

void DashboardWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void DashboardWidget::updateData(const RealTimeData &data) {
    if (m_calOverlay) m_calOverlay->updateLiveData(data);

    m_rpmGauge->setValue(data.getValue("rpm"));
    m_mapGauge->setValue(data.getValue("map"));
    m_tpsGauge->setValue(data.getValue("tps"));
    m_cltGauge->setValue(data.getValue("clt"));
    m_iatGauge->setValue(data.getValue("iat"));

    double pw = data.getValue("pw1");
    if (pw == 0.0) pw = data.getPulseWidth();
    double rpm = data.getValue("rpm");
    double dutyCalc = (rpm * pw) / 1200.0;
    if (dutyCalc > 100) dutyCalc = 100;
    m_dutyGauge->setValue(dutyCalc);

    m_cardTps->setValue(QString::number(data.getValue("tps"), 'f', 1));
    m_cardIat->setValue(QString::number(data.getValue("iat"), 'f', 1));
    m_cardClt->setValue(QString::number(data.getValue("clt"), 'f', 1));
    m_cardBat->setValue(QString::number(data.getValue("battery"), 'f', 1));
    m_cardO2->setValue(QString::number(data.getValue("afr"), 'f', 2));
    m_cardIgn->setValue(QString::number(data.getValue("advance"), 'f', 1));
    m_cardPw->setValue(QString::number(pw, 'f', 2));
    m_cardBaro->setValue(QString::number(data.getValue("baro"), 'f', 1));

    m_stSync->setStatus(data.hasTriggerSync(), QColor(TunerProColors::SAFE));
    m_stCranking->setStatus(data.isCranking(), QColor(TunerProColors::WARN));
    m_stHardLimit->setStatus(data.isHardLimitActive(), QColor(TunerProColors::DANGER));
    m_stSoftLimit->setStatus(data.isSoftLimitActive(), QColor(TunerProColors::WARN));
    m_stFuelPump->setStatus(true, QColor(TunerProColors::SAFE));

    int duty = qBound(0, (int)(dutyCalc), 100);
    m_stDuty->setPercentage(duty, QColor(TunerProColors::CH_FUEL_PW));

    m_afrGraph->addValue(data.getAFR());
    m_advanceGraph->addValue(data.getAdvance());
}
