/**
 * @file ColdStartWidget.h
 * @brief Cold Start & Warmup configuration widget
 */

#ifndef COLDSTARTWIDGET_H
#define COLDSTARTWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QSlider>
#include <QWidget>

#include "TuningWidgetBase.h"

class ColdStartWidget : public TuningWidgetBase {
  Q_OBJECT

public:
  explicit ColdStartWidget(QWidget *parent = nullptr);
  ~ColdStartWidget();

  void loadFromECU() override;

private:
  void setupUi();
  QWidget *createCrankingEnrichmentSection();
  QWidget *createWarmupEnrichmentSection();
  QWidget *createAfterstartEnrichmentSection();
  QWidget *createEnrichmentCurveSection();
  QWidget *createTuningTipsSection();
  QSlider *createStyledSlider(int min, int max, int value);

  // Cranking Enrichment
  QSlider *m_crankingMultiplier;
  QLabel *m_crankingValue;

  // Warmup Enrichment by Temperature
  QSlider *m_warmupMinus10;
  QSlider *m_warmup0;
  QSlider *m_warmup20;
  QSlider *m_warmup40;
  QSlider *m_warmup60;
  QSlider *m_warmup70;
  QSlider *m_warmup80;
  QLabel *m_warmupMinus10Value;
  QLabel *m_warmup0Value;
  QLabel *m_warmup20Value;
  QLabel *m_warmup40Value;
  QLabel *m_warmup60Value;
  QLabel *m_warmup70Value;
  QLabel *m_warmup80Value;

  // Afterstart Enrichment
  QSlider *m_initialEnrichment;
  QLabel *m_initialEnrichValue;
  QSlider *m_decayTime;
  QLabel *m_decayTimeValue;

};

#endif // COLDSTARTWIDGET_H
