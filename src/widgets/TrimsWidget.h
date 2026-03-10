/**
 * @file TrimsWidget.h
 * @brief Fuel & Ignition Trims configuration widget
 */

#ifndef TRIMSWIDGET_H
#define TRIMSWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QSlider>
#include <QTabWidget>
#include <QWidget>

#include "TuningWidgetBase.h"

class TrimsWidget : public TuningWidgetBase {
  Q_OBJECT

public:
  explicit TrimsWidget(QWidget *parent = nullptr);
  ~TrimsWidget() override;

  void loadFromECU() override;

private:
  void setupUi();
  QWidget *createFuelTrimsTab();
  QWidget *createIgnitionTrimsTab();
  QWidget *createGlobalTrimSection();
  QWidget *createPerCylinderSection();
  QWidget *createCylinderBalanceSection();
  QSlider *createStyledSlider(int min, int max, int value);

  QTabWidget *m_tabWidget;

  // Global Fuel Trim
  QSlider *m_globalFuelSlider;
  QLabel *m_globalFuelValue;
  QLabel *m_basePulseWidth;
  QLabel *m_afterTrim;

  // Per-Cylinder Trims
  QSlider *m_cyl1Slider;
  QSlider *m_cyl2Slider;
  QSlider *m_cyl3Slider;
  QSlider *m_cyl4Slider;
  QLabel *m_cyl1Value;
  QLabel *m_cyl2Value;
  QLabel *m_cyl3Value;
  QLabel *m_cyl4Value;

  // Balance visualization
  QFrame *m_cyl1Bar;
  QFrame *m_cyl2Bar;
  QFrame *m_cyl3Bar;
  QFrame *m_cyl4Bar;

  // Global Ignition Trim
  QSlider *m_globalIgnSlider;
  QLabel *m_globalIgnValue;
  QLabel *m_baseAdvance;
  QLabel *m_afterIgnTrim;

  // Per-Cylinder Ignition Trims
  QSlider *m_ignCyl1Slider;
  QSlider *m_ignCyl2Slider;
  QSlider *m_ignCyl3Slider;
  QSlider *m_ignCyl4Slider;
  QLabel *m_ignCyl1Value;
  QLabel *m_ignCyl2Value;
  QLabel *m_ignCyl3Value;
  QLabel *m_ignCyl4Value;

  // NEW-006: Cylinders 5-8 Ignition Trims
  QSlider *m_ignCyl5Slider;
  QSlider *m_ignCyl6Slider;
  QSlider *m_ignCyl7Slider;
  QSlider *m_ignCyl8Slider;
  QLabel *m_ignCyl5Value;
  QLabel *m_ignCyl6Value;
  QLabel *m_ignCyl7Value;
  QLabel *m_ignCyl8Value;
};

#endif // TRIMSWIDGET_H
