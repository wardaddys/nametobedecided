#ifndef ENGINEPROTECTIONWIDGET_H
#define ENGINEPROTECTIONWIDGET_H

#include <QWidget>

class QCheckBox;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QLabel;
class ECUSettingsManager;

// E6: Engine Protection configuration.
//
// Firmware engineProtection.h surfaces protection bits for oil pressure (low),
// AFR (lean), coolant overheat, and shift-cut. The status bits are parsed by
// ECUData.h:370-375 but OS Tuner has never offered a configuration page.
// This widget closes that safety-critical gap.
class EngineProtectionWidget : public QWidget {
    Q_OBJECT
public:
    explicit EngineProtectionWidget(QWidget* parent = nullptr);
    void setSettingsManager(ECUSettingsManager* mgr);
    void loadFromECU();

private:
    void setupUi();

    ECUSettingsManager* m_settings = nullptr;

    // Oil pressure
    QCheckBox*      m_oilProtectEnabled = nullptr;
    QSpinBox*       m_oilMinPressure    = nullptr;
    QSpinBox*       m_oilStartRpm       = nullptr;
    QSpinBox*       m_oilDelay          = nullptr;
    QComboBox*      m_oilCutMethod      = nullptr;

    // AFR (lean)
    QCheckBox*      m_afrProtectEnabled = nullptr;
    QDoubleSpinBox* m_afrMaxValue       = nullptr;
    QSpinBox*       m_afrMinRpm         = nullptr;
    QSpinBox*       m_afrMinTps         = nullptr;
    QComboBox*      m_afrCutMethod      = nullptr;

    // Coolant overheat
    QCheckBox*      m_cltProtectEnabled = nullptr;
    QSpinBox*       m_cltMaxTemp        = nullptr;
    QSpinBox*       m_cltMinRpm         = nullptr;
    QComboBox*      m_cltCutMethod      = nullptr;
};

#endif // ENGINEPROTECTIONWIDGET_H
