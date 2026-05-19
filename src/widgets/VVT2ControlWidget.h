#ifndef VVT2CONTROLWIDGET_H
#define VVT2CONTROLWIDGET_H

#include <QWidget>

class QComboBox;
class QSpinBox;
class QTableWidget;
class ECUSettingsManager;

// E5: VVT2 control panel.
//
// VVTVTECWidget covers VVT1 only. Firmware globals.h:691-693 exposes a full
// VVT2 control surface (mode, PWM frequency, target table, hysteresis) which
// OS Tuner has never surfaced. This widget closes that gap.
class VVT2ControlWidget : public QWidget {
    Q_OBJECT
public:
    explicit VVT2ControlWidget(QWidget* parent = nullptr);
    void setSettingsManager(ECUSettingsManager* mgr);
    void loadFromECU();

private:
    void setupUi();
    void writeTargetTableToECU();

    ECUSettingsManager* m_settings = nullptr;
    QComboBox*    m_mode       = nullptr;
    QSpinBox*     m_pwmFreq    = nullptr;
    QSpinBox*     m_hysteresis = nullptr;
    QTableWidget* m_targetTable = nullptr;
};

#endif // VVT2CONTROLWIDGET_H
