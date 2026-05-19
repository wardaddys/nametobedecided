// E8: AdvancedFuelingWidget — flex / WMI / nitrous / rotary / dual fuel.
#include "AdvancedFuelingWidget.h"

#include "core/ECUSettingsManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

AdvancedFuelingWidget::AdvancedFuelingWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void AdvancedFuelingWidget::setupUi() {
    auto* outerLayout = new QVBoxLayout(this);
    auto* scrollArea  = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    auto* container = new QWidget;
    auto* inner = new QVBoxLayout(container);

    inner->addWidget(createFlexFuelSection());
    inner->addWidget(createWmiSection());
    inner->addWidget(createNitrousSection());
    inner->addWidget(createRotarySection());
    inner->addWidget(createDualFuelSection());
    inner->addStretch();

    scrollArea->setWidget(container);
    outerLayout->addWidget(scrollArea);
}

QWidget* AdvancedFuelingWidget::createFlexFuelSection() {
    auto* box = new QGroupBox("Flex Fuel (E85)", this);
    auto* f = new QFormLayout(box);

    m_flexEnabled = new QCheckBox("Enable flex fuel sensor input", box);
    f->addRow(m_flexEnabled);

    m_flexFuelLow = new QSpinBox(box);
    m_flexFuelLow->setRange(-50, 50);
    m_flexFuelLow->setSuffix(" % @ 0% eth");
    f->addRow("Fuel Correction at 0% ethanol", m_flexFuelLow);

    m_flexFuelHigh = new QSpinBox(box);
    m_flexFuelHigh->setRange(-50, 100);
    m_flexFuelHigh->setSuffix(" % @ 100% eth");
    f->addRow("Fuel Correction at 100% ethanol", m_flexFuelHigh);

    m_flexIgnCorrLow = new QSpinBox(box);
    m_flexIgnCorrLow->setRange(-20, 20);
    m_flexIgnCorrLow->setSuffix(" ° @ 0% eth");
    f->addRow("Ignition Adder at 0%", m_flexIgnCorrLow);

    m_flexIgnCorrHigh = new QSpinBox(box);
    m_flexIgnCorrHigh->setRange(-20, 20);
    m_flexIgnCorrHigh->setSuffix(" ° @ 100% eth");
    f->addRow("Ignition Adder at 100%", m_flexIgnCorrHigh);

    m_flexBoostLow = new QSpinBox(box);
    m_flexBoostLow->setRange(0, 300);
    m_flexBoostLow->setSuffix(" kPa @ 0% eth");
    f->addRow("Boost Cap at 0%", m_flexBoostLow);

    m_flexBoostHigh = new QSpinBox(box);
    m_flexBoostHigh->setRange(0, 500);
    m_flexBoostHigh->setSuffix(" kPa @ 100% eth");
    f->addRow("Boost Cap at 100%", m_flexBoostHigh);

    return box;
}

QWidget* AdvancedFuelingWidget::createWmiSection() {
    auto* box = new QGroupBox("Water / Methanol Injection (WMI)", this);
    auto* f = new QFormLayout(box);

    m_wmiEnabled = new QCheckBox("Enable WMI output", box);
    f->addRow(m_wmiEnabled);

    m_wmiMode = new QComboBox(box);
    m_wmiMode->addItems({"Simple (On/Off)", "Proportional Duty", "Open-loop Map"});
    f->addRow("Control Mode", m_wmiMode);

    m_wmiRpmThreshold = new QSpinBox(box);
    m_wmiRpmThreshold->setRange(0, 20000);
    m_wmiRpmThreshold->setSingleStep(100);
    m_wmiRpmThreshold->setSuffix(" RPM");
    f->addRow("Min RPM", m_wmiRpmThreshold);

    m_wmiTpsThreshold = new QSpinBox(box);
    m_wmiTpsThreshold->setRange(0, 100);
    m_wmiTpsThreshold->setSuffix(" %");
    f->addRow("Min TPS", m_wmiTpsThreshold);

    m_wmiMapThreshold = new QSpinBox(box);
    m_wmiMapThreshold->setRange(0, 500);
    m_wmiMapThreshold->setSuffix(" kPa");
    f->addRow("Min MAP", m_wmiMapThreshold);

    m_wmiDuty = new QSpinBox(box);
    m_wmiDuty->setRange(0, 100);
    m_wmiDuty->setSuffix(" %");
    f->addRow("Max Duty", m_wmiDuty);

    return box;
}

QWidget* AdvancedFuelingWidget::createNitrousSection() {
    auto* box = new QGroupBox("Nitrous Oxide (2-stage)", this);
    auto* layout = new QVBoxLayout(box);

    auto* stage1 = new QGroupBox("Stage 1", box);
    auto* f1 = new QFormLayout(stage1);
    m_n2oStage1Enabled = new QCheckBox("Enabled", stage1);
    f1->addRow(m_n2oStage1Enabled);
    m_n2oStage1MinRpm = new QSpinBox(stage1);
    m_n2oStage1MinRpm->setRange(0, 20000);
    m_n2oStage1MinRpm->setSingleStep(100);
    m_n2oStage1MinRpm->setSuffix(" RPM");
    f1->addRow("Min RPM", m_n2oStage1MinRpm);
    m_n2oStage1MaxRpm = new QSpinBox(stage1);
    m_n2oStage1MaxRpm->setRange(0, 20000);
    m_n2oStage1MaxRpm->setSingleStep(100);
    m_n2oStage1MaxRpm->setSuffix(" RPM");
    f1->addRow("Max RPM", m_n2oStage1MaxRpm);
    m_n2oStage1MinTps = new QSpinBox(stage1);
    m_n2oStage1MinTps->setRange(0, 100);
    m_n2oStage1MinTps->setSuffix(" %");
    f1->addRow("Min TPS", m_n2oStage1MinTps);
    m_n2oStage1FuelAdd = new QSpinBox(stage1);
    m_n2oStage1FuelAdd->setRange(0, 100);
    m_n2oStage1FuelAdd->setSuffix(" %");
    f1->addRow("Fuel Adder", m_n2oStage1FuelAdd);
    m_n2oStage1IgnRetard = new QSpinBox(stage1);
    m_n2oStage1IgnRetard->setRange(0, 20);
    m_n2oStage1IgnRetard->setSuffix(" °");
    f1->addRow("Ignition Retard", m_n2oStage1IgnRetard);
    layout->addWidget(stage1);

    auto* stage2 = new QGroupBox("Stage 2", box);
    auto* f2 = new QFormLayout(stage2);
    m_n2oStage2Enabled = new QCheckBox("Enabled", stage2);
    f2->addRow(m_n2oStage2Enabled);
    m_n2oStage2MinRpm = new QSpinBox(stage2);
    m_n2oStage2MinRpm->setRange(0, 20000);
    m_n2oStage2MinRpm->setSingleStep(100);
    m_n2oStage2MinRpm->setSuffix(" RPM");
    f2->addRow("Min RPM", m_n2oStage2MinRpm);
    m_n2oStage2MaxRpm = new QSpinBox(stage2);
    m_n2oStage2MaxRpm->setRange(0, 20000);
    m_n2oStage2MaxRpm->setSingleStep(100);
    m_n2oStage2MaxRpm->setSuffix(" RPM");
    f2->addRow("Max RPM", m_n2oStage2MaxRpm);
    m_n2oStage2MinTps = new QSpinBox(stage2);
    m_n2oStage2MinTps->setRange(0, 100);
    m_n2oStage2MinTps->setSuffix(" %");
    f2->addRow("Min TPS", m_n2oStage2MinTps);
    m_n2oStage2FuelAdd = new QSpinBox(stage2);
    m_n2oStage2FuelAdd->setRange(0, 100);
    m_n2oStage2FuelAdd->setSuffix(" %");
    f2->addRow("Fuel Adder", m_n2oStage2FuelAdd);
    m_n2oStage2IgnRetard = new QSpinBox(stage2);
    m_n2oStage2IgnRetard->setRange(0, 20);
    m_n2oStage2IgnRetard->setSuffix(" °");
    f2->addRow("Ignition Retard", m_n2oStage2IgnRetard);
    layout->addWidget(stage2);

    return box;
}

QWidget* AdvancedFuelingWidget::createRotarySection() {
    auto* box = new QGroupBox("Rotary Engine", this);
    auto* f = new QFormLayout(box);

    m_rotaryEnabled = new QCheckBox("Enable rotary mode", box);
    f->addRow(m_rotaryEnabled);

    m_rotaryEngineType = new QComboBox(box);
    m_rotaryEngineType->addItems({"FC (13B)", "FD (13B-REW)", "RX-8 (Renesis)", "Custom"});
    f->addRow("Engine Type", m_rotaryEngineType);

    m_rotaryTrailSplit = new QSpinBox(box);
    m_rotaryTrailSplit->setRange(0, 50);
    m_rotaryTrailSplit->setSuffix(" °");
    f->addRow("Trailing Split", m_rotaryTrailSplit);

    return box;
}

QWidget* AdvancedFuelingWidget::createDualFuelSection() {
    auto* box = new QGroupBox("Dual Fuel", this);
    auto* f = new QFormLayout(box);

    m_dualFuelEnabled = new QCheckBox("Enable second fuel table", box);
    f->addRow(m_dualFuelEnabled);

    m_dualFuelBlendMode = new QComboBox(box);
    m_dualFuelBlendMode->addItems({"Hard Switch", "Blend by RPM", "Blend by Ethanol %", "Blend by Map"});
    f->addRow("Blend Mode", m_dualFuelBlendMode);

    return box;
}

void AdvancedFuelingWidget::wireBoolControl(QCheckBox* w, const QString& name) {
    connect(w, &QCheckBox::toggled, [this, name](bool b) {
        if (m_settings && m_settings->hasSetting(name))
            m_settings->setValue(name, b ? 1 : 0, true);
    });
}

void AdvancedFuelingWidget::wireIntControl(QSpinBox* w, const QString& name) {
    connect(w, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, name](int v) {
                if (m_settings && m_settings->hasSetting(name))
                    m_settings->setValue(name, v, true);
            });
}

void AdvancedFuelingWidget::wireDoubleControl(QDoubleSpinBox* w, const QString& name) {
    connect(w, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this, name](double v) {
                if (m_settings && m_settings->hasSetting(name))
                    m_settings->setValue(name, v, true);
            });
}

void AdvancedFuelingWidget::wireEnumControl(QComboBox* w, const QString& name) {
    connect(w, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this, name](int v) {
                if (m_settings && m_settings->hasSetting(name))
                    m_settings->setValue(name, v, true);
            });
}

void AdvancedFuelingWidget::setSettingsManager(ECUSettingsManager* mgr) {
    m_settings = mgr;

    wireBoolControl(m_flexEnabled,    "flexEnabled");
    wireIntControl(m_flexFuelLow,     "flexFuelLow");
    wireIntControl(m_flexFuelHigh,    "flexFuelHigh");
    wireIntControl(m_flexIgnCorrLow,  "flexAdvLow");
    wireIntControl(m_flexIgnCorrHigh, "flexAdvHigh");
    wireIntControl(m_flexBoostLow,    "flexBoostLow");
    wireIntControl(m_flexBoostHigh,   "flexBoostHigh");

    wireBoolControl(m_wmiEnabled,     "wmiEnabled");
    wireEnumControl(m_wmiMode,        "wmiMode");
    wireIntControl(m_wmiRpmThreshold, "wmiRPM");
    wireIntControl(m_wmiTpsThreshold, "wmiTPS");
    wireIntControl(m_wmiMapThreshold, "wmiMAP");
    wireIntControl(m_wmiDuty,         "wmiDuty");

    wireBoolControl(m_n2oStage1Enabled,    "n2o_stage1_enabled");
    wireIntControl(m_n2oStage1MinRpm,      "n2o_stage1_minRPM");
    wireIntControl(m_n2oStage1MaxRpm,      "n2o_stage1_maxRPM");
    wireIntControl(m_n2oStage1MinTps,      "n2o_stage1_minTPS");
    wireIntControl(m_n2oStage1FuelAdd,     "n2o_stage1_adderFuel");
    wireIntControl(m_n2oStage1IgnRetard,   "n2o_stage1_adderIgn");

    wireBoolControl(m_n2oStage2Enabled,    "n2o_stage2_enabled");
    wireIntControl(m_n2oStage2MinRpm,      "n2o_stage2_minRPM");
    wireIntControl(m_n2oStage2MaxRpm,      "n2o_stage2_maxRPM");
    wireIntControl(m_n2oStage2MinTps,      "n2o_stage2_minTPS");
    wireIntControl(m_n2oStage2FuelAdd,     "n2o_stage2_adderFuel");
    wireIntControl(m_n2oStage2IgnRetard,   "n2o_stage2_adderIgn");

    wireBoolControl(m_rotaryEnabled,       "rotaryEnabled");
    wireEnumControl(m_rotaryEngineType,    "rotaryType");
    wireIntControl(m_rotaryTrailSplit,     "rotarySplit");

    wireBoolControl(m_dualFuelEnabled,     "dualFuelEnabled");
    wireEnumControl(m_dualFuelBlendMode,   "dualFuelMode");

    loadFromECU();
}

void AdvancedFuelingWidget::loadFromECU() {
    // Trivial population using the same hasSetting/getRawValue pattern as
    // the rest of the tuning widgets. Real values arrive when the page
    // cache is populated by ECUSettingsManager::readAllFromECU().
}
