// E7: SDLoggerWidget — onboard SD logger controls.
#include "SDLoggerWidget.h"

#include "core/ECUData.h"
#include "core/SerialManager.h"
#include "core/SpeeduinoProtocol.h"

#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
// Speeduino button IDs for SD log control.
constexpr quint16 TS_CMD_SD_LOG_START = 13056;
constexpr quint16 TS_CMD_SD_LOG_STOP  = 13058;
constexpr quint16 TS_CMD_SD_FORMAT    = 13057;
}

SDLoggerWidget::SDLoggerWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
    setEnabled(false);
}

void SDLoggerWidget::setupUi() {
    auto* root = new QVBoxLayout(this);

    auto* statusBox = new QGroupBox("SD Logger Status", this);
    auto* statusLay = new QVBoxLayout(statusBox);

    m_statusLabel = new QLabel("Status: Unknown", statusBox);
    m_statusLabel->setStyleSheet("font-weight: 500; padding: 4px;");
    statusLay->addWidget(m_statusLabel);

    m_freeSpaceLabel = new QLabel("Free Space: -- MB", statusBox);
    statusLay->addWidget(m_freeSpaceLabel);

    m_capacityBar = new QProgressBar(statusBox);
    m_capacityBar->setRange(0, 100);
    m_capacityBar->setValue(0);
    m_capacityBar->setFormat("Card Usage: %p%");
    statusLay->addWidget(m_capacityBar);

    root->addWidget(statusBox);

    auto* btnBox = new QGroupBox("Actions", this);
    auto* btnLay = new QHBoxLayout(btnBox);

    m_startBtn  = new QPushButton("Start Recording", btnBox);
    m_stopBtn   = new QPushButton("Stop Recording",  btnBox);
    m_formatBtn = new QPushButton("Format SD",        btnBox);

    btnLay->addWidget(m_startBtn);
    btnLay->addWidget(m_stopBtn);
    btnLay->addWidget(m_formatBtn);
    root->addWidget(btnBox);

    connect(m_startBtn,  &QPushButton::clicked, this, &SDLoggerWidget::onStartClicked);
    connect(m_stopBtn,   &QPushButton::clicked, this, &SDLoggerWidget::onStopClicked);
    connect(m_formatBtn, &QPushButton::clicked, this, &SDLoggerWidget::onFormatClicked);

    root->addStretch();
}

void SDLoggerWidget::setSerialManager(SerialManager* serial) {
    if (m_serial) disconnect(m_serial, nullptr, this, nullptr);
    m_serial = serial;
    if (m_serial) {
        connect(m_serial, &SerialManager::connectionStatusChanged,
                this, &SDLoggerWidget::onConnectionChanged);
        connect(m_serial, &SerialManager::dataReceived,
                this, &SDLoggerWidget::updateRealtimeStatus);
    }
    onConnectionChanged();
}

void SDLoggerWidget::onConnectionChanged() {
    setEnabled(m_serial && m_serial->isConnected());
}

namespace {
void queueButtonCommand(SerialManager* serial, quint16 cmdId) {
    if (!serial) return;
    SpeeduinoProtocol p;
    SerialCommand sc;
    sc.data = p.createButtonCommand(cmdId);
    sc.type = CommandType::ButtonCommand;
    sc.isRaw = false;
    serial->queueCommand(sc);
}
} // namespace

void SDLoggerWidget::onStartClicked() {
    queueButtonCommand(m_serial, TS_CMD_SD_LOG_START);
}

void SDLoggerWidget::onStopClicked() {
    queueButtonCommand(m_serial, TS_CMD_SD_LOG_STOP);
}

void SDLoggerWidget::onFormatClicked() {
    if (!m_serial) return;
    const auto reply = QMessageBox::warning(
        this, "Format SD Card",
        "This will erase every log file on the onboard SD card. Continue?",
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);
    if (reply != QMessageBox::Yes) return;
    queueButtonCommand(m_serial, TS_CMD_SD_FORMAT);
}

void SDLoggerWidget::updateRealtimeStatus(const RealTimeData& data) {
    // SD status byte semantics (from firmware SD_logger.h):
    //   bit 0: SD card present
    //   bit 1: SD card initialized / ready
    //   bit 2: logging active
    //   bit 3: file open
    //   bit 4: write error
    //   bit 5: format requested
    //   bit 6-7: card type (low: SDHC, etc.)
    const quint8 status = data.TS_SD_Status;
    QString label;
    if (!(status & 0x01))      label = "No SD card present";
    else if (!(status & 0x02)) label = "Card present, initialising";
    else if (status & 0x10)    label = "Write ERROR — check card";
    else if (status & 0x04)    label = "Recording…";
    else                       label = "Ready (idle)";
    m_statusLabel->setText("Status: " + label);
}
