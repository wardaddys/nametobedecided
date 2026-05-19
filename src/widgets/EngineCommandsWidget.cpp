// E2: EngineCommandsWidget implementation.
#include "EngineCommandsWidget.h"

#include "core/SerialManager.h"
#include "core/SpeeduinoProtocol.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

// Firmware TS_CMD_* IDs (TS_CommandButtonHandler.h).
namespace TS {
    constexpr quint16 TEST_DSBL    = 256;
    constexpr quint16 TEST_ENBL    = 257;
    constexpr quint16 INJ1_PULSED  = 515;
    constexpr quint16 INJ2_PULSED  = 518;
    constexpr quint16 INJ3_PULSED  = 521;
    constexpr quint16 INJ4_PULSED  = 524;
    constexpr quint16 IGN1_PULSED  = 771;
    constexpr quint16 IGN2_PULSED  = 774;
    constexpr quint16 IGN3_PULSED  = 777;
    constexpr quint16 IGN4_PULSED  = 780;
    constexpr quint16 STM32_REBOOT     = 12800;
    constexpr quint16 STM32_BOOTLOADER = 12801;
    constexpr quint16 SD_FORMAT        = 13057;
    constexpr quint16 VSS_60KMH        = 39168;
    constexpr quint16 VSS_RATIO1       = 39169;
}

EngineCommandsWidget::EngineCommandsWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
    setEnabled(false);  // Until a SerialManager is attached.
}

void EngineCommandsWidget::setupUi() {
    auto* root = new QVBoxLayout(this);

    auto* hint = new QLabel(
        "Engine commands run against the connected ECU. Test-mode commands "
        "only fire when the engine is stopped.",
        this);
    hint->setWordWrap(true);
    hint->setStyleSheet("color: #888; padding: 6px;");
    root->addWidget(hint);

    // --- Test mode ---
    auto* testBox = new QGroupBox("Test Mode", this);
    auto* testLay = new QHBoxLayout(testBox);
    testLay->addWidget(makeCommandButton("Enable Test Mode",
                                         TS::TEST_ENBL, true,
                                         "Test mode disables normal ECU operation. "
                                         "Engine must be stopped. Continue?"));
    testLay->addWidget(makeCommandButton("Disable Test Mode",
                                         TS::TEST_DSBL, false));
    root->addWidget(testBox);

    // --- Injector tests ---
    auto* injBox = new QGroupBox("Injector Pulse Test", this);
    auto* injLay = new QGridLayout(injBox);
    injLay->addWidget(makeCommandButton("INJ 1 Pulse", TS::INJ1_PULSED, true,
                                        "Pulse injector 1?"),     0, 0);
    injLay->addWidget(makeCommandButton("INJ 2 Pulse", TS::INJ2_PULSED, true,
                                        "Pulse injector 2?"),     0, 1);
    injLay->addWidget(makeCommandButton("INJ 3 Pulse", TS::INJ3_PULSED, true,
                                        "Pulse injector 3?"),     1, 0);
    injLay->addWidget(makeCommandButton("INJ 4 Pulse", TS::INJ4_PULSED, true,
                                        "Pulse injector 4?"),     1, 1);
    root->addWidget(injBox);

    // --- Ignition tests ---
    auto* ignBox = new QGroupBox("Ignition Coil Pulse Test", this);
    auto* ignLay = new QGridLayout(ignBox);
    ignLay->addWidget(makeCommandButton("IGN 1 Pulse", TS::IGN1_PULSED, true,
                                        "Pulse ignition coil 1?"), 0, 0);
    ignLay->addWidget(makeCommandButton("IGN 2 Pulse", TS::IGN2_PULSED, true,
                                        "Pulse ignition coil 2?"), 0, 1);
    ignLay->addWidget(makeCommandButton("IGN 3 Pulse", TS::IGN3_PULSED, true,
                                        "Pulse ignition coil 3?"), 1, 0);
    ignLay->addWidget(makeCommandButton("IGN 4 Pulse", TS::IGN4_PULSED, true,
                                        "Pulse ignition coil 4?"), 1, 1);
    root->addWidget(ignBox);

    // --- System ---
    auto* sysBox = new QGroupBox("System", this);
    auto* sysLay = new QGridLayout(sysBox);
    sysLay->addWidget(makeCommandButton("Format SD", TS::SD_FORMAT, true,
                                        "Erase the onboard SD card?"),     0, 0);
    sysLay->addWidget(makeCommandButton("Reboot STM32", TS::STM32_REBOOT, true,
                                        "Reboot the ECU?"),                0, 1);
    sysLay->addWidget(makeCommandButton("Enter Bootloader",
                                        TS::STM32_BOOTLOADER, true,
                                        "Drop ECU into bootloader for firmware "
                                        "update?"),                        1, 0);
    sysLay->addWidget(makeCommandButton("Calibrate VSS @ 60 km/h",
                                        TS::VSS_60KMH, true,
                                        "Calibrate VSS at current speed (60 km/h)?"),
                                        1, 1);
    root->addWidget(sysBox);

    root->addStretch();
}

QPushButton* EngineCommandsWidget::makeCommandButton(const QString& label,
                                                    quint16 cmdId,
                                                    bool requireConfirm,
                                                    const QString& confirmMsg) {
    auto* btn = new QPushButton(label, this);
    connect(btn, &QPushButton::clicked, [this, cmdId, requireConfirm, confirmMsg, label] {
        if (!m_serial || !m_serial->isConnected()) {
            QMessageBox::warning(this, "Not Connected", "Connect to the ECU first.");
            return;
        }
        if (requireConfirm) {
            const auto reply = QMessageBox::question(
                this, label,
                confirmMsg.isEmpty() ? QString("Send command %1?").arg(cmdId) : confirmMsg);
            if (reply != QMessageBox::Yes) return;
        }
        SpeeduinoProtocol p;
        SerialCommand sc;
        sc.data = p.createButtonCommand(cmdId);
        sc.type = CommandType::ButtonCommand;
        sc.isRaw = false;
        m_serial->queueCommand(sc);
    });
    return btn;
}

void EngineCommandsWidget::setSerialManager(SerialManager* serial) {
    if (m_serial == serial) return;
    if (m_serial) disconnect(m_serial, nullptr, this, nullptr);
    m_serial = serial;
    if (m_serial) {
        connect(m_serial, &SerialManager::connectionStatusChanged,
                this, &EngineCommandsWidget::onConnectionChanged);
    }
    onConnectionChanged();
}

void EngineCommandsWidget::onConnectionChanged() {
    setEnabled(m_serial && m_serial->isConnected());
}
