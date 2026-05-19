#ifndef ADVANCEDFUELINGWIDGET_H
#define ADVANCEDFUELINGWIDGET_H

#include <QWidget>

class QCheckBox;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QTableWidget;
class ECUSettingsManager;

// E8: Advanced Fueling controls — flex fuel, WMI, nitrous, rotary, dual fuel.
//
// Each subsystem has its own QGroupBox with the firmware-defined inputs:
//   - Flex Fuel: ethanol % sensor, fuel correction curve, ign correction
//     curve, boost correction curve.
//   - Water/Methanol Injection: enable, output pin, RPM/TPS/MAP thresholds,
//     duty map.
//   - Nitrous (2-stage): stage 1+2 RPM/TPS/CLT thresholds, fuel & timing
//     adders per stage.
//   - Rotary: leading/trailing split angles.
//   - Dual fuel: enable, VE table 2 selection, blend mode.
class AdvancedFuelingWidget : public QWidget {
    Q_OBJECT
public:
    explicit AdvancedFuelingWidget(QWidget* parent = nullptr);
    void setSettingsManager(ECUSettingsManager* mgr);
    void loadFromECU();

private:
    void setupUi();
    QWidget* createFlexFuelSection();
    QWidget* createWmiSection();
    QWidget* createNitrousSection();
    QWidget* createRotarySection();
    QWidget* createDualFuelSection();
    void wireBoolControl(QCheckBox* w, const QString& name);
    void wireIntControl(QSpinBox* w, const QString& name);
    void wireDoubleControl(QDoubleSpinBox* w, const QString& name);
    void wireEnumControl(QComboBox* w, const QString& name);

    ECUSettingsManager* m_settings = nullptr;

    // Flex fuel
    QCheckBox*      m_flexEnabled       = nullptr;
    QSpinBox*       m_flexFuelLow       = nullptr;
    QSpinBox*       m_flexFuelHigh      = nullptr;
    QSpinBox*       m_flexIgnCorrLow    = nullptr;
    QSpinBox*       m_flexIgnCorrHigh   = nullptr;
    QSpinBox*       m_flexBoostLow      = nullptr;
    QSpinBox*       m_flexBoostHigh     = nullptr;

    // WMI
    QCheckBox*      m_wmiEnabled        = nullptr;
    QSpinBox*       m_wmiRpmThreshold   = nullptr;
    QSpinBox*       m_wmiTpsThreshold   = nullptr;
    QSpinBox*       m_wmiMapThreshold   = nullptr;
    QSpinBox*       m_wmiDuty           = nullptr;
    QComboBox*      m_wmiMode           = nullptr;

    // Nitrous
    QCheckBox*      m_n2oStage1Enabled  = nullptr;
    QSpinBox*       m_n2oStage1MinRpm   = nullptr;
    QSpinBox*       m_n2oStage1MaxRpm   = nullptr;
    QSpinBox*       m_n2oStage1MinTps   = nullptr;
    QSpinBox*       m_n2oStage1FuelAdd  = nullptr;
    QSpinBox*       m_n2oStage1IgnRetard = nullptr;

    QCheckBox*      m_n2oStage2Enabled  = nullptr;
    QSpinBox*       m_n2oStage2MinRpm   = nullptr;
    QSpinBox*       m_n2oStage2MaxRpm   = nullptr;
    QSpinBox*       m_n2oStage2MinTps   = nullptr;
    QSpinBox*       m_n2oStage2FuelAdd  = nullptr;
    QSpinBox*       m_n2oStage2IgnRetard = nullptr;

    // Rotary
    QCheckBox*      m_rotaryEnabled     = nullptr;
    QComboBox*      m_rotaryEngineType  = nullptr;
    QSpinBox*       m_rotaryTrailSplit  = nullptr;

    // Dual fuel
    QCheckBox*      m_dualFuelEnabled   = nullptr;
    QComboBox*      m_dualFuelBlendMode = nullptr;
};

#endif // ADVANCEDFUELINGWIDGET_H
