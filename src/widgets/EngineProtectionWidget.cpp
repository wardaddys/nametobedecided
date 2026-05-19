// E6: EngineProtectionWidget — configure oil, AFR, and coolant cutoffs.
#include "EngineProtectionWidget.h"

#include "core/ECUSettingsManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {
const QStringList kCutMethods = {"Disabled", "Ignition Cut", "Fuel Cut", "Both"};
}

EngineProtectionWidget::EngineProtectionWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void EngineProtectionWidget::setupUi() {
    auto* root = new QVBoxLayout(this);

    auto* hint = new QLabel(
        "<b>Safety-critical.</b> Misconfiguration can either fail to protect "
        "the engine or trigger unnecessary cuts. Verify each threshold against "
        "your hardware before connecting.",
        this);
    hint->setStyleSheet("color: #e69138; padding: 6px;");
    hint->setWordWrap(true);
    root->addWidget(hint);

    // ---- Oil pressure protection ----
    auto* oilBox = new QGroupBox("Oil Pressure Protection", this);
    auto* oilForm = new QFormLayout(oilBox);

    m_oilProtectEnabled = new QCheckBox("Enable oil pressure cut", oilBox);
    oilForm->addRow(m_oilProtectEnabled);

    m_oilMinPressure = new QSpinBox(oilBox);
    m_oilMinPressure->setRange(0, 100);
    m_oilMinPressure->setSuffix(" PSI");
    oilForm->addRow("Min Pressure", m_oilMinPressure);

    m_oilStartRpm = new QSpinBox(oilBox);
    m_oilStartRpm->setRange(0, 20000);
    m_oilStartRpm->setSingleStep(100);
    m_oilStartRpm->setSuffix(" RPM");
    oilForm->addRow("Protection Starts Above", m_oilStartRpm);

    m_oilDelay = new QSpinBox(oilBox);
    m_oilDelay->setRange(0, 30);
    m_oilDelay->setSuffix(" s");
    oilForm->addRow("Confirmation Delay", m_oilDelay);

    m_oilCutMethod = new QComboBox(oilBox);
    m_oilCutMethod->addItems(kCutMethods);
    oilForm->addRow("Cut Method", m_oilCutMethod);
    root->addWidget(oilBox);

    // ---- AFR protection (lean) ----
    auto* afrBox = new QGroupBox("AFR (Lean) Protection", this);
    auto* afrForm = new QFormLayout(afrBox);

    m_afrProtectEnabled = new QCheckBox("Enable lean AFR cut", afrBox);
    afrForm->addRow(m_afrProtectEnabled);

    m_afrMaxValue = new QDoubleSpinBox(afrBox);
    m_afrMaxValue->setRange(8.0, 25.0);
    m_afrMaxValue->setSingleStep(0.1);
    m_afrMaxValue->setSuffix(" :1");
    afrForm->addRow("Lean Limit (AFR)", m_afrMaxValue);

    m_afrMinRpm = new QSpinBox(afrBox);
    m_afrMinRpm->setRange(0, 20000);
    m_afrMinRpm->setSingleStep(100);
    m_afrMinRpm->setSuffix(" RPM");
    afrForm->addRow("Active Above", m_afrMinRpm);

    m_afrMinTps = new QSpinBox(afrBox);
    m_afrMinTps->setRange(0, 100);
    m_afrMinTps->setSuffix(" %");
    afrForm->addRow("Active Above TPS", m_afrMinTps);

    m_afrCutMethod = new QComboBox(afrBox);
    m_afrCutMethod->addItems(kCutMethods);
    afrForm->addRow("Cut Method", m_afrCutMethod);
    root->addWidget(afrBox);

    // ---- Coolant overheat ----
    auto* cltBox = new QGroupBox("Coolant Overheat Protection", this);
    auto* cltForm = new QFormLayout(cltBox);

    m_cltProtectEnabled = new QCheckBox("Enable coolant overheat cut", cltBox);
    cltForm->addRow(m_cltProtectEnabled);

    m_cltMaxTemp = new QSpinBox(cltBox);
    m_cltMaxTemp->setRange(60, 200);
    m_cltMaxTemp->setSuffix(" °C");
    cltForm->addRow("Max Coolant Temp", m_cltMaxTemp);

    m_cltMinRpm = new QSpinBox(cltBox);
    m_cltMinRpm->setRange(0, 20000);
    m_cltMinRpm->setSingleStep(100);
    m_cltMinRpm->setSuffix(" RPM");
    cltForm->addRow("Active Above", m_cltMinRpm);

    m_cltCutMethod = new QComboBox(cltBox);
    m_cltCutMethod->addItems(kCutMethods);
    cltForm->addRow("Cut Method", m_cltCutMethod);
    root->addWidget(cltBox);

    root->addStretch();
}

void EngineProtectionWidget::setSettingsManager(ECUSettingsManager* mgr) {
    if (m_settings && m_settings != mgr) {
        // Detach signals from previous manager not needed — controls write
        // directly via lambdas that capture mgr.
    }
    m_settings = mgr;

    auto wireBool = [this](QCheckBox* w, const QString& name) {
        connect(w, &QCheckBox::toggled, [this, name](bool b) {
            if (m_settings && m_settings->hasSetting(name))
                m_settings->setValue(name, b ? 1 : 0, true);
        });
    };
    auto wireInt = [this](QSpinBox* w, const QString& name) {
        connect(w, QOverload<int>::of(&QSpinBox::valueChanged),
                [this, name](int v) {
                    if (m_settings && m_settings->hasSetting(name))
                        m_settings->setValue(name, v, true);
                });
    };
    auto wireDouble = [this](QDoubleSpinBox* w, const QString& name) {
        connect(w, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this, name](double v) {
                    if (m_settings && m_settings->hasSetting(name))
                        m_settings->setValue(name, v, true);
                });
    };
    auto wireEnum = [this](QComboBox* w, const QString& name) {
        connect(w, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [this, name](int v) {
                    if (m_settings && m_settings->hasSetting(name))
                        m_settings->setValue(name, v, true);
                });
    };

    wireBool(m_oilProtectEnabled, "oilProtEnable");
    wireInt (m_oilMinPressure,    "oilProtMinPressure");
    wireInt (m_oilStartRpm,       "oilProtMinRpm");
    wireInt (m_oilDelay,          "oilProtDelay");
    wireEnum(m_oilCutMethod,      "oilProtCutType");

    wireBool(m_afrProtectEnabled, "afrProtEnable");
    wireDouble(m_afrMaxValue,     "afrProtMaxAfr");
    wireInt (m_afrMinRpm,         "afrProtMinRpm");
    wireInt (m_afrMinTps,         "afrProtMinTps");
    wireEnum(m_afrCutMethod,      "afrProtCutType");

    wireBool(m_cltProtectEnabled, "cltProtEnable");
    wireInt (m_cltMaxTemp,        "cltProtMaxTemp");
    wireInt (m_cltMinRpm,         "cltProtMinRpm");
    wireEnum(m_cltCutMethod,      "cltProtCutType");

    loadFromECU();
}

void EngineProtectionWidget::loadFromECU() {
    if (!m_settings) return;
    auto getInt = [this](const QString& name) -> int {
        return m_settings->hasSetting(name) ? m_settings->getRawValue(name) : 0;
    };
    auto getDouble = [this](const QString& name) -> double {
        return m_settings->hasSetting(name) ? m_settings->getValue(name).toDouble() : 0.0;
    };

    m_oilProtectEnabled->setChecked(getInt("oilProtEnable") != 0);
    m_oilMinPressure->setValue(getInt("oilProtMinPressure"));
    m_oilStartRpm->setValue(getInt("oilProtMinRpm"));
    m_oilDelay->setValue(getInt("oilProtDelay"));
    m_oilCutMethod->setCurrentIndex(getInt("oilProtCutType"));

    m_afrProtectEnabled->setChecked(getInt("afrProtEnable") != 0);
    m_afrMaxValue->setValue(getDouble("afrProtMaxAfr"));
    m_afrMinRpm->setValue(getInt("afrProtMinRpm"));
    m_afrMinTps->setValue(getInt("afrProtMinTps"));
    m_afrCutMethod->setCurrentIndex(getInt("afrProtCutType"));

    m_cltProtectEnabled->setChecked(getInt("cltProtEnable") != 0);
    m_cltMaxTemp->setValue(getInt("cltProtMaxTemp"));
    m_cltMinRpm->setValue(getInt("cltProtMinRpm"));
    m_cltCutMethod->setCurrentIndex(getInt("cltProtCutType"));
}
