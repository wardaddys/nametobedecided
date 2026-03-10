/**
 * @file VVTVTECWidget.h
 * @brief VVT/VTEC Control Widget
 */

#ifndef VVTVTECWIDGET_H
#define VVTVTECWIDGET_H

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QTableWidget>
#include <QWidget>

#include "TuningWidgetBase.h"

class VVTVTECWidget : public TuningWidgetBase {
  Q_OBJECT

public:
  explicit VVTVTECWidget(QWidget *parent = nullptr);
  ~VVTVTECWidget() override;

  void loadFromECU() override;

private slots:
  void onEngagementRpmChanged(int value);
  void onHysteresisChanged(int value);
  void onMaxAdvanceChanged(int value);
  void onMinAdvanceChanged(int value);
  void onEnableToggled(bool enabled);

private:
  void setupUi();
  QWidget *createVtecEngagementSection();
  QWidget *createVvtControlSection();
  QWidget *createAdvanceMapSection();
  QSlider *createStyledSlider(int min, int max, int value);

  // VTEC Engagement
  QSlider *m_engagementRpmSlider;
  QLabel *m_engagementRpmValue;
  QSpinBox *m_hysteresisSpin;

  // VVT Control
  QSpinBox *m_maxAdvanceSpin;
  QSpinBox *m_minAdvanceSpin;

  // Enable Toggle
  QCheckBox *m_enableToggle;

  QTableWidget *m_advanceMapTable;
};

#endif // VVTVTECWIDGET_H
