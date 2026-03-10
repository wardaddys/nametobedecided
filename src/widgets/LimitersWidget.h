/**
 * @file LimitersWidget.h
 * @brief Engine Limiters & Protection Widget
 */

#ifndef LIMITERSWIDGET_H
#define LIMITERSWIDGET_H

#include <QComboBox>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QTabWidget>
#include <QWidget>
#include <QSpinBox>
#include <QCheckBox>

#include "TuningWidgetBase.h"

class LimitersWidget : public TuningWidgetBase {
  Q_OBJECT

public:
  explicit LimitersWidget(QWidget *parent = nullptr);
  ~LimitersWidget() override;

  void loadFromECU() override;

private slots:
  void onSoftLimitRpmChanged(int value);
  void onHardLimitRpmChanged(int value);
  void onIgnitionCutClicked();
  void onFuelCutClicked();

private:
  void setupUi();
  QWidget *createRevLimiterTab();
  QWidget *createSpeedLimiterTab();
  QWidget *createLaunchControlTab();
  QWidget *createSoftLimiterSection();
  QWidget *createHardLimiterSection();
  QWidget *createRevLimitZonesSection();
  QSlider *createStyledSlider(int min, int max, int value);

  QTabWidget *m_tabWidget;

  // Soft Limiter
  QSlider *m_softLimitSlider;
  QLabel *m_softLimitValue;
  QPushButton *m_ignitionCutBtn;
  QPushButton *m_fuelCutBtn;

  // Hard Limiter
  QSlider *m_hardLimitSlider;
  QLabel *m_hardLimitValue;

  // Cut settings
  QComboBox *m_cutTypeCombo;
  QComboBox *m_cutPatternCombo;

  // Speed Limiter
  QCheckBox *m_speedLimEnabled;
  QSpinBox *m_speedLimit;
  QComboBox *m_speedLimAction;
  QSpinBox *m_speedLimHyst;

  // Launch Control
  QCheckBox *m_launchEnabled;
  QSpinBox *m_launchRpm;
  QSpinBox *m_launchRetard;
  QSpinBox *m_launchTps;
  QCheckBox *m_flatShiftEnabled;
  QSpinBox *m_flatShiftRpmDrop;

  // Rev Limit Zones visual
  QFrame *m_zonesBar;
  QFrame *m_safeZone;
  QFrame *m_softZone;
  QFrame *m_hardZone;
  QLabel *m_softZoneLabel;
  QLabel *m_hardZoneLabel;
  QHBoxLayout *m_zonesBarLayout;
  
  void updateZonesBar();
};

#endif // LIMITERSWIDGET_H
