// E10: CompositeLoggerWidget — drive the J/j/O/X command lifecycle.
#include "CompositeLoggerWidget.h"

#include "core/SerialManager.h"
#include "core/SpeeduinoProtocol.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

CompositeLoggerWidget::CompositeLoggerWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
    setEnabled(false);
}

void CompositeLoggerWidget::setupUi() {
    auto* root = new QVBoxLayout(this);
    auto* hint = new QLabel(
        "Composite logger captures primary, secondary, and tertiary trigger "
        "signals together. Use it when the basic Tooth Logger isn't enough to "
        "diagnose sync problems.",
        this);
    hint->setWordWrap(true);
    hint->setStyleSheet("color: #888;");
    root->addWidget(hint);

    auto* btnBox = new QGroupBox("Capture", this);
    auto* btnLay = new QHBoxLayout(btnBox);
    m_startBtn = new QPushButton("Start (J)", btnBox);
    m_stopBtn  = new QPushButton("Stop (j)",  btnBox);
    m_readBtn  = new QPushButton("Read (O)",  btnBox);
    m_resetBtn = new QPushButton("Reset (X)", btnBox);
    btnLay->addWidget(m_startBtn);
    btnLay->addWidget(m_stopBtn);
    btnLay->addWidget(m_readBtn);
    btnLay->addWidget(m_resetBtn);
    root->addWidget(btnBox);

    m_statusLbl = new QLabel("Idle.", this);
    m_statusLbl->setStyleSheet("color: #ccc;");
    root->addWidget(m_statusLbl);
    root->addStretch();

    connect(m_startBtn, &QPushButton::clicked, this, &CompositeLoggerWidget::onStartClicked);
    connect(m_stopBtn,  &QPushButton::clicked, this, &CompositeLoggerWidget::onStopClicked);
    connect(m_readBtn,  &QPushButton::clicked, this, &CompositeLoggerWidget::onReadClicked);
    connect(m_resetBtn, &QPushButton::clicked, this, &CompositeLoggerWidget::onResetClicked);
}

void CompositeLoggerWidget::setSerialManager(SerialManager* serial) {
    if (m_serial) disconnect(m_serial, nullptr, this, nullptr);
    m_serial = serial;
    if (m_serial) {
        connect(m_serial, &SerialManager::connectionStatusChanged,
                this, &CompositeLoggerWidget::onConnectionChanged);
    }
    onConnectionChanged();
}

void CompositeLoggerWidget::onConnectionChanged() {
    setEnabled(m_serial && m_serial->isConnected());
}

namespace {
void queueRaw(SerialManager* serial, const QByteArray& bytes) {
    if (!serial) return;
    SerialCommand sc;
    sc.data = bytes;
    sc.type = CommandType::Unknown;
    sc.isRaw = true; // composite/tooth logger commands are single-byte legacy
    serial->queueCommand(sc);
}
} // namespace

void CompositeLoggerWidget::onStartClicked() {
    SpeeduinoProtocol p;
    queueRaw(m_serial, p.createStartCompositeLog());
    m_statusLbl->setText("Capturing…");
}

void CompositeLoggerWidget::onStopClicked() {
    SpeeduinoProtocol p;
    queueRaw(m_serial, p.createStopCompositeLog());
    m_statusLbl->setText("Stopped.");
}

void CompositeLoggerWidget::onReadClicked() {
    SpeeduinoProtocol p;
    queueRaw(m_serial, p.createReadCompositeLog());
    m_statusLbl->setText("Read requested — incoming bytes will be forwarded.");
}

void CompositeLoggerWidget::onResetClicked() {
    SpeeduinoProtocol p;
    queueRaw(m_serial, p.createResetCompositeBuffer());
    m_statusLbl->setText("Buffer reset.");
}
