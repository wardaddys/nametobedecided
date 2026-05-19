// E9: CommsConfigWidget — CAN + secondary serial + OBD-II configuration.
#include "CommsConfigWidget.h"

#include "core/ECUSettingsManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

CommsConfigWidget::CommsConfigWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void CommsConfigWidget::setupUi() {
    auto* root = new QVBoxLayout(this);

    // ---- CAN broadcast ----
    auto* bcastBox = new QGroupBox("CAN Broadcast (Dash / Datalogger)", this);
    auto* bf = new QFormLayout(bcastBox);
    m_canBcastEnabled = new QCheckBox("Enable CAN broadcast output", bcastBox);
    bf->addRow(m_canBcastEnabled);
    m_canBcastMode = new QComboBox(bcastBox);
    m_canBcastMode->addItems({
        "Custom 11-bit",
        "Custom 29-bit",
        "BMW E46/E90",
        "VAG (Audi/VW)",
        "Haltech",
        "Holley"
    });
    bf->addRow("Protocol", m_canBcastMode);
    m_canBcastBaseId = new QSpinBox(bcastBox);
    m_canBcastBaseId->setRange(0, 2047);
    m_canBcastBaseId->setPrefix("0x");
    m_canBcastBaseId->setDisplayIntegerBase(16);
    bf->addRow("Base CAN ID", m_canBcastBaseId);
    root->addWidget(bcastBox);

    // ---- CAN WBO receive ----
    auto* wboBox = new QGroupBox("CAN Wideband Receive", this);
    auto* wf = new QFormLayout(wboBox);
    m_canWboEnabled = new QCheckBox("Receive AFR / Lambda via CAN", wboBox);
    wf->addRow(m_canWboEnabled);
    m_canWboProtocol = new QComboBox(wboBox);
    m_canWboProtocol->addItems({
        "rusEFI Wideband",
        "AEM X-Series",
        "Innovate LC-2",
        "ECUMaster",
        "Generic 11-bit"
    });
    wf->addRow("Source Device", m_canWboProtocol);
    root->addWidget(wboBox);

    // ---- OBD-II ----
    auto* obdBox = new QGroupBox("OBD-II Responder", this);
    auto* of = new QFormLayout(obdBox);
    m_obd2Enabled = new QCheckBox("Respond to OBD-II diagnostic requests", obdBox);
    of->addRow(m_obd2Enabled);
    of->addRow(new QLabel(
        "<i>OBD-II responder lets generic scanners read live data and DTCs.</i>",
        obdBox));
    root->addWidget(obdBox);

    // ---- Secondary serial ----
    auto* serBox = new QGroupBox("Secondary Serial (UART2)", this);
    auto* sf = new QFormLayout(serBox);
    m_serial2Enabled = new QCheckBox("Enable secondary serial port", serBox);
    sf->addRow(m_serial2Enabled);
    m_serial2Baud = new QComboBox(serBox);
    m_serial2Baud->addItems({"9600", "19200", "38400", "57600", "115200", "230400"});
    m_serial2Baud->setCurrentText("115200");
    sf->addRow("Baud Rate", m_serial2Baud);
    m_serial2Role = new QComboBox(serBox);
    m_serial2Role->addItems({"TS Generic", "Bluetooth Dash", "GPS NMEA", "Generic Sensor"});
    sf->addRow("Role", m_serial2Role);
    root->addWidget(serBox);

    root->addStretch();

    auto writeBool = [this](QCheckBox* w, const QString& name) {
        connect(w, &QCheckBox::toggled, [this, name](bool b) {
            if (m_settings && m_settings->hasSetting(name))
                m_settings->setValue(name, b ? 1 : 0, true);
        });
    };
    auto writeInt = [this](QSpinBox* w, const QString& name) {
        connect(w, QOverload<int>::of(&QSpinBox::valueChanged),
                [this, name](int v) {
                    if (m_settings && m_settings->hasSetting(name))
                        m_settings->setValue(name, v, true);
                });
    };
    auto writeEnum = [this](QComboBox* w, const QString& name) {
        connect(w, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [this, name](int v) {
                    if (m_settings && m_settings->hasSetting(name))
                        m_settings->setValue(name, v, true);
                });
    };
    writeBool(m_canBcastEnabled,  "canBcastEnabled");
    writeEnum(m_canBcastMode,     "canBcastMode");
    writeInt (m_canBcastBaseId,   "canBcastBaseId");
    writeBool(m_canWboEnabled,    "canWboEnabled");
    writeEnum(m_canWboProtocol,   "canWboProtocol");
    writeBool(m_obd2Enabled,      "obd2Enabled");
    writeBool(m_serial2Enabled,   "serial2Enabled");
    writeEnum(m_serial2Baud,      "serial2Baud");
    writeEnum(m_serial2Role,      "serial2Role");
}

void CommsConfigWidget::setSettingsManager(ECUSettingsManager* mgr) {
    m_settings = mgr;
    loadFromECU();
}

void CommsConfigWidget::loadFromECU() {
    if (!m_settings) return;
    auto loadBool = [this](QCheckBox* w, const QString& name) {
        if (m_settings->hasSetting(name)) w->setChecked(m_settings->getRawValue(name) != 0);
    };
    auto loadInt = [this](QSpinBox* w, const QString& name) {
        if (m_settings->hasSetting(name)) w->setValue(m_settings->getRawValue(name));
    };
    auto loadEnum = [this](QComboBox* w, const QString& name) {
        if (m_settings->hasSetting(name)) {
            const int v = m_settings->getRawValue(name);
            if (v >= 0 && v < w->count()) w->setCurrentIndex(v);
        }
    };
    loadBool(m_canBcastEnabled,  "canBcastEnabled");
    loadEnum(m_canBcastMode,     "canBcastMode");
    loadInt (m_canBcastBaseId,   "canBcastBaseId");
    loadBool(m_canWboEnabled,    "canWboEnabled");
    loadEnum(m_canWboProtocol,   "canWboProtocol");
    loadBool(m_obd2Enabled,      "obd2Enabled");
    loadBool(m_serial2Enabled,   "serial2Enabled");
    loadEnum(m_serial2Baud,      "serial2Baud");
    loadEnum(m_serial2Role,      "serial2Role");
}
