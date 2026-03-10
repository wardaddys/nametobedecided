#include "DashboardWidget.h"
#include "TunerGauge.h"
#include "core/TunerProColors.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QFontDatabase>

// ==========================================
// DataCard Implementation
// ==========================================
DataCard::DataCard(const QString &label, const QColor &dotColor, QWidget *parent)
    : QFrame(parent) {
    setFixedSize(106, 56);
    setStyleSheet(QString("QFrame { background-color: %1; border-radius: 6px; }")
                  .arg(TunerProColors::BG_RAISED));

    // Dot
    QWidget *dot = new QWidget(this);
    dot->setFixedSize(6, 6);
    dot->setStyleSheet(QString("background-color: %1; border-radius: 3px;").arg(dotColor.name()));
    dot->move(width() - 10, 6);

    // Label
    QLabel *lbl = new QLabel(label, this);
    lbl->setStyleSheet(QString("font-family: 'JetBrains Mono'; font-size: 10px; color: %1;")
                       .arg(TunerProColors::TEXT_SECONDARY));
    lbl->move(6, 4);

    // Value
    m_valLabel = new QLabel("0.0", this);
    m_valLabel->setStyleSheet(QString("font-family: 'JetBrains Mono'; font-size: 20px; font-weight: bold; color: %1;")
                              .arg(TunerProColors::TEXT_PRIMARY));
    m_valLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    m_valLabel->setGeometry(8, 20, width() - 16, 30);
}

void DataCard::setValue(const QString &value) {
    m_valLabel->setText(value);
}

// ==========================================
// StatusLine Implementation
// ==========================================
StatusLine::StatusLine(const QString &label, QWidget *parent)
    : QWidget(parent) {
    setFixedHeight(20);
    
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(8);

    m_label = new QLabel(label, this);
    m_label->setStyleSheet(QString("font-family: 'Inter'; font-size: 11px; font-weight: 500; color: %1;")
                           .arg(TunerProColors::TEXT_SECONDARY));
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
        m_label->setStyleSheet(QString("font-family: 'Inter'; font-size: 11px; font-weight: bold; color: %1;")
                               .arg(TunerProColors::TEXT_PRIMARY));
    } else {
        m_indicator->setStyleSheet(QString("background-color: %1; border-radius: 4px;").arg(TunerProColors::BG_OVERLAY));
        m_label->setStyleSheet(QString("font-family: 'Inter'; font-size: 11px; font-weight: 500; color: %1;")
                               .arg(TunerProColors::TEXT_SECONDARY));
    }
}

void StatusLine::setPercentage(int percent, const QColor &color) {
    // Hacky bar reuse
    m_indicator->setFixedSize(50 * percent / 100 + 1, 8);
    m_indicator->setStyleSheet(QString("background-color: %1; border-radius: 2px;").arg(color.name()));
}

// ==========================================
// DashboardWidget Implementation
// ==========================================
DashboardWidget::DashboardWidget(QWidget *parent)
    : QWidget(parent) {
    setupUi();
}

DashboardWidget::~DashboardWidget() {}

void DashboardWidget::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // ==================
    // TOP HALF (Gauges + Cards)
    // ==================
    QWidget *topContainer = new QWidget(this);
    topContainer->setFixedHeight(320); // Rigid height from spec
    QHBoxLayout *topLay = new QHBoxLayout(topContainer);
    topLay->setContentsMargins(0,0,0,0);
    topLay->setSpacing(24);

    // 1. Left Gauge (RPM)
    m_rpmGauge = new TunerGauge(this);
    m_rpmGauge->setRange(0, 8000);
    m_rpmGauge->setLabel("RPM");
    m_rpmGauge->setDangerThreshold(7000);
    topLay->addWidget(m_rpmGauge, 0, Qt::AlignLeft);

    // 2. Center Data Grid (2 cols, 4 rows)
    QWidget *gridContainer = new QWidget(this);
    QGridLayout *gridLay = new QGridLayout(gridContainer);
    gridLay->setSpacing(4);
    gridLay->setContentsMargins(0,0,0,0);

    m_cardTps = new DataCard("TPS %", QColor(TunerProColors::CH_TPS));
    m_cardIat = new DataCard("IAT °C", QColor(TunerProColors::CH_IAT));
    m_cardClt = new DataCard("CLT °C", QColor(TunerProColors::CH_CLT));
    m_cardBat = new DataCard("BATT V", QColor(TunerProColors::CH_BATTERY));
    m_cardO2 = new DataCard("O2 AFR", QColor(TunerProColors::CH_AFR));
    m_cardIgn = new DataCard("IGN °BTDC", QColor(TunerProColors::CH_ADVANCE));
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
    
    // Add vertical stretch wrappers to center the grid
    QWidget *gridWrapper = new QWidget(this);
    QVBoxLayout *gridWrapperLay = new QVBoxLayout(gridWrapper);
    gridWrapperLay->addStretch();
    gridWrapperLay->addWidget(gridContainer);
    gridWrapperLay->addStretch();

    topLay->addWidget(gridWrapper, 0, Qt::AlignCenter);

    // 3. Right Status Panels
    QWidget *panelsContainer = new QWidget(this);
    panelsContainer->setFixedWidth(160);
    QVBoxLayout *panelsLay = new QVBoxLayout(panelsContainer);
    panelsLay->setContentsMargins(0,0,0,0);
    panelsLay->setSpacing(16);

    // Fuel Panel
    QFrame *fuelPanel = new QFrame(this);
    fuelPanel->setStyleSheet(QString("QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; }")
                             .arg(TunerProColors::BG_ELEVATED).arg(TunerProColors::BORDER_SUBTLE));
    QVBoxLayout *fLay = new QVBoxLayout(fuelPanel);
    fLay->setContentsMargins(12, 12, 12, 12);
    fLay->setSpacing(2);
    QLabel *fTitle = new QLabel("FUEL STATUS", fuelPanel);
    fTitle->setStyleSheet(QString("font-family: 'Barlow Condensed'; font-size: 13px; font-weight: bold; color: %1; margin-bottom: 4px;")
                          .arg(TunerProColors::TEXT_MUTED));
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

    // Engine Panel
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

    topLay->addWidget(panelsContainer);

    // 4. Right Gauge (MAP)
    m_mapGauge = new TunerGauge(this);
    m_mapGauge->setRange(0, 300);
    m_mapGauge->setLabel("KPA");
    m_mapGauge->setDangerThreshold(250);
    topLay->addWidget(m_mapGauge, 0, Qt::AlignRight);

    mainLayout->addWidget(topContainer);

    // ==================
    // BOTTOM HALF (Graphs)
    // ==================
    QWidget *graphsContainer = new QWidget(this);
    graphsContainer->setFixedHeight(200);
    QHBoxLayout *gLay = new QHBoxLayout(graphsContainer);
    gLay->setContentsMargins(0,0,0,0);
    gLay->setSpacing(16);

    // AFR Graph
    m_afrGraph = new DataGraph(this);
    m_afrGraph->setTitle("Air/Fuel Ratio");
    m_afrGraph->setUnit("AFR");
    m_afrGraph->setRange(10, 20);
    m_afrGraph->setLineColor(QColor(TunerProColors::CH_AFR));
    m_afrGraph->setBackgroundColor(QColor(TunerProColors::BG_ELEVATED));
    m_afrGraph->setGridColor(QColor(TunerProColors::BORDER_SUBTLE));

    // Advance Graph
    m_advanceGraph = new DataGraph(this);
    m_advanceGraph->setTitle("Ignition Advance");
    m_advanceGraph->setUnit("°BTDC");
    m_advanceGraph->setRange(0, 50);
    m_advanceGraph->setLineColor(QColor(TunerProColors::CH_ADVANCE));
    m_advanceGraph->setBackgroundColor(QColor(TunerProColors::BG_ELEVATED));
    m_advanceGraph->setGridColor(QColor(TunerProColors::BORDER_SUBTLE));

    gLay->addWidget(m_afrGraph);
    gLay->addWidget(m_advanceGraph);

    mainLayout->addWidget(graphsContainer);
    
    // Fill rest
    mainLayout->addStretch();
}

void DashboardWidget::paintEvent(QPaintEvent *event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void DashboardWidget::updateData(const RealTimeData &data) {
    // 1. Gauges
    m_rpmGauge->setValue(data.getRPM());
    m_mapGauge->setValue(data.getMAP());

    // 2. Center Cards
    m_cardTps->setValue(QString::number(data.getTPS(), 'f', 1));
    m_cardIat->setValue(QString::number(data.getIAT(), 'f', 1));
    m_cardClt->setValue(QString::number(data.getCoolant(), 'f', 1));
    m_cardBat->setValue(QString::number(data.getBatteryVoltage(), 'f', 1));
    m_cardO2->setValue(QString::number(data.getAFR(), 'f', 2));
    m_cardIgn->setValue(QString::number(data.getAdvance(), 'f', 1));
    // Calculate pulse width estimation for display if available
    m_cardPw->setValue("0.0"); // TODO: Add PW to RealTimeData
    m_cardBaro->setValue("100"); // TODO: Add Baro to RealTimeData

    // 3. Status Panels
    m_stSync->setStatus(data.engine & 0x01, QColor(TunerProColors::SAFE));
    m_stCranking->setStatus(data.engine & 0x04, QColor(TunerProColors::WARN));
    m_stHardLimit->setStatus(data.engine & 0x20, QColor(TunerProColors::DANGER));
    m_stSoftLimit->setStatus(false, QColor(TunerProColors::WARN));
    
    m_stFuelPump->setStatus(true, QColor(TunerProColors::SAFE)); // Simulated
    
    // Duty cycle percentage bar (Placeholder representation)
    int duty = qBound(0, (int)(data.getTPS()), 100);
    m_stDuty->setPercentage(duty, QColor(TunerProColors::CH_FUEL_PW));

    // 4. Graphs
    m_afrGraph->addValue(data.getAFR());
    m_advanceGraph->addValue(data.getAdvance());
}
