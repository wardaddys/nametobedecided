#ifndef IDLEALGORITHMWIDGET_H
#define IDLEALGORITHMWIDGET_H

#include <QWidget>

class QComboBox;
class QSpinBox;
class QTableWidget;
class ECUSettingsManager;

// E3: Idle algorithm picker + 10-point target curve (cold-start aware).
//
// IdleControlWidget already presents PID tuning sliders. This companion
// widget adds the firmware controls it's missing:
//   - Algorithm selector (idle.h: None / OnOff / PWM-OL / PWM-CL /
//     Stepper-OL / Stepper-CL / OLCL).
//   - 10-bin target idle vs CLT curve.
//   - Stepper parameters (step count, cool time, home steps) shown only
//     when a stepper algorithm is selected.
class IdleAlgorithmWidget : public QWidget {
    Q_OBJECT
public:
    explicit IdleAlgorithmWidget(QWidget* parent = nullptr);
    void setSettingsManager(ECUSettingsManager* mgr);
    void loadFromECU();

private slots:
    void onAlgorithmChanged(int index);
    void onTargetCellChanged(int row, int col);

private:
    void setupUi();
    void updateStepperVisibility(int algorithm);

    ECUSettingsManager* m_settings = nullptr;
    QComboBox*    m_algorithm     = nullptr;
    QTableWidget* m_targetCurve   = nullptr;
    QSpinBox*     m_stepperSteps  = nullptr;
    QSpinBox*     m_stepperCool   = nullptr;
    QSpinBox*     m_stepperHome   = nullptr;
    QWidget*      m_stepperGroup  = nullptr;
};

#endif // IDLEALGORITHMWIDGET_H
