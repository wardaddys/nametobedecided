/**
 * @file AccelEnrichmentWidget.h
 * @brief Acceleration Enrichment configuration widget
 */

#ifndef ACCELENRICHMENTWIDGET_H
#define ACCELENRICHMENTWIDGET_H

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QWidget>

#include "TuningWidgetBase.h"

class AccelEnrichmentWidget : public TuningWidgetBase {
  Q_OBJECT

public:
  explicit AccelEnrichmentWidget(QWidget *parent = nullptr);
  ~AccelEnrichmentWidget() override;

  void loadFromECU() override;
  void updateRealtimeData(const RealTimeData &data) override;

  void updateLiveStatus(double tpsRate, double mapRate, double tpsEnrich,
                        double mapEnrich);

private:
  void setupUi();
  QWidget *createTPSEnrichmentSection();
  QWidget *createMAPEnrichmentSection();
  QWidget *createDecelFuelCutSection();
  QWidget *createLiveStatusSection();
  QSlider *createStyledSlider(int min, int max, int value);

  // TPS-Based
  QCheckBox *m_tpsEnableToggle;
  QSlider *m_tpsRateThreshold;
  QLabel *m_tpsRateValue;
  QSlider *m_tpsEnrichAmount;
  QLabel *m_tpsEnrichValue;
  QSlider *m_tpsDecayTime;
  QLabel *m_tpsDecayValue;

  // MAP-Based
  QCheckBox *m_mapEnableToggle;
  QSlider *m_mapRateThreshold;
  QLabel *m_mapRateValue;
  QSlider *m_mapEnrichAmount;
  QLabel *m_mapEnrichValue;
  QSlider *m_mapDecayTime;
  QLabel *m_mapDecayValue;

  // Decel Fuel Cut
  QCheckBox *m_decelEnableToggle;
  QSlider *m_decelTpsThreshold;
  QLabel *m_decelTpsValue;
  QSpinBox *m_decelMinRpm;
  QSpinBox *m_decelMinTps;

  // Live Status
  QLabel *m_liveTpsRate;
  QLabel *m_liveMapRate;
  QLabel *m_liveTpsEnrich;
  QLabel *m_liveMapEnrich;
};

#endif // ACCELENRICHMENTWIDGET_H
