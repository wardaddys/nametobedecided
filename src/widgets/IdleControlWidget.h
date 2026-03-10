/**
 * @file IdleControlWidget.h
 * @brief Idle Control configuration widget
 *
 * @author Safeerullah Afridi (FA-22-151)
 * @author Muhammad Saeed Sajid (FA-22-143)
 * @supervisor Prof. Dr. Shariq Hussain
 * @date December 2025
 */

#ifndef IDLECONTROLWIDGET_H
#define IDLECONTROLWIDGET_H

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include "TuningWidgetBase.h"

class IdleControlWidget : public TuningWidgetBase {
  Q_OBJECT

public:
  explicit IdleControlWidget(QWidget *parent = nullptr);
  ~IdleControlWidget() override;

  void updateLiveStatus(int currentRpm, int targetRpm, double iacDuty,
                        double error);

  // ECU Settings integration
  void loadFromECU() override;
  void updateRealtimeData(const RealTimeData &data) override;

private slots:
  void onTargetRpmChanged(int value);
  void onPGainChanged(double value);
  void onIGainChanged(double value);
  void onDGainChanged(double value);
  void onColdStartAdderChanged(int value);
  void onAcAdderChanged(int value);
  void onDashpotToggled(bool enabled);
  void onDecayRateChanged(int value);

private:
  void setupUi();
  QWidget *createTargetIdleSection();
  QWidget *createPIDSection();
  QWidget *createWarmupSection();
  QWidget *createAcIdleUpSection();
  QWidget *createDashpotSection();
  QWidget *createOutputLimitsSection();
  QWidget *createLiveStatusSection();
  QSlider *createStyledSlider(int min, int max, int value);

  // Target Idle Speed
  QSlider *m_targetRpmSlider;
  QLabel *m_targetRpmValue;

  // PID Parameters
  QDoubleSpinBox *m_pGainSpin;
  QDoubleSpinBox *m_iGainSpin;
  QDoubleSpinBox *m_dGainSpin;
  QSlider *m_pGainSlider;
  QSlider *m_iGainSlider;
  QSlider *m_dGainSlider;

  // Warmup Idle-Up
  QSlider *m_coldStartAdderSlider;
  QLabel *m_coldStartAdderValue;
  QFrame *m_warmupCurvePreview;

  // A/C Idle-Up
  QSlider *m_acAdderSlider;
  QLabel *m_acAdderValue;

  // Dashpot
  QCheckBox *m_dashpotToggle;
  QSlider *m_decayRateSlider;
  QLabel *m_decayRateValue;

  // Output Limits
  QSpinBox *m_minDutySpin;
  QSpinBox *m_maxDutySpin;

  // Live Status
  QLabel *m_currentRpmLabel;
  QLabel *m_targetRpmLabel;
  QLabel *m_iacDutyLabel;
  QLabel *m_errorLabel;
};

#endif // IDLECONTROLWIDGET_H
