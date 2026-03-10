/**
 * @file KnockWidget.h
 * @brief Knock Detection and Control Widget
 */

#ifndef KNOCKWIDGET_H
#define KNOCKWIDGET_H

#include <QCheckBox>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QSlider>
#include <QWidget>

#include "TuningWidgetBase.h"

class KnockWidget : public TuningWidgetBase {
  Q_OBJECT

public:
  explicit KnockWidget(QWidget *parent = nullptr);
  ~KnockWidget() override;

  void loadFromECU() override;
  void updateRealtimeData(const RealTimeData &data) override;

  void updateSensorLevels(int s1, int s2, int s3, int s4);
  void updateTimingRetard(double cyl1, double cyl2, double cyl3, double cyl4);

private slots:
  void onKnockEnableToggled(bool enabled);
  void onThresholdChanged(int value);
  void onWindowStartChanged(int value);
  void onWindowEndChanged(int value);
  void onMaxRetardChanged(int value);
  void onRetardStepChanged(int value);
  void onRecoveryRateChanged(int value);

private:
  void setupUi();
  QWidget *createKnockDetectionSection();
  QWidget *createLiveSensorSection();
  QWidget *createTimingRetardSection();
  QWidget *createActiveRetardSection();
  QSlider *createStyledSlider(int min, int max, int value);

  // Knock Detection
  QCheckBox *m_knockEnableToggle;
  QSlider *m_thresholdSlider;
  QLabel *m_thresholdValue;
  QSlider *m_windowStartSlider;
  QLabel *m_windowStartValue;
  QSlider *m_windowEndSlider;
  QLabel *m_windowEndValue;
  QFrame *m_detectionWindow;

  // Live Sensor Levels
  QProgressBar *m_sensor1Bar;
  QProgressBar *m_sensor2Bar;
  QProgressBar *m_sensor3Bar;
  QProgressBar *m_sensor4Bar;
  QLabel *m_sensor1Value;
  QLabel *m_sensor2Value;
  QLabel *m_sensor3Value;
  QLabel *m_sensor4Value;

  // Timing Retard Response
  QSlider *m_maxRetardSlider;
  QLabel *m_maxRetardValue;
  QSlider *m_retardStepSlider;
  QLabel *m_retardStepValue;
  QSlider *m_recoveryRateSlider;
  QLabel *m_recoveryRateValue;

  // Active Timing Retard
  QLabel *m_cyl1Retard;
  QLabel *m_cyl2Retard;
  QLabel *m_cyl3Retard;
  QLabel *m_cyl4Retard;
  QLabel *m_totalKnockEvents;
};

#endif // KNOCKWIDGET_H
