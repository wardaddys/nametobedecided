#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include "TunerGauge.h"
#include "DataGraph.h"
#include "core/ECUData.h"

// Custom DataCard for the center grid
class DataCard : public QFrame {
    Q_OBJECT
public:
    explicit DataCard(const QString &label, const QColor &dotColor, QWidget *parent = nullptr);
    void setValue(const QString &value);
    
private:
    QLabel *m_valLabel;
};

// Custom StatusLine component for the side panels
class StatusLine : public QWidget {
    Q_OBJECT
public:
    explicit StatusLine(const QString &label, QWidget *parent = nullptr);
    void setStatus(bool active, const QColor &activeColor);
    void setPercentage(int percent, const QColor &color);
private:
    QLabel *m_label;
    QWidget *m_indicator;
    bool m_isPercent = false;
};

class DashboardWidget : public QWidget {
  Q_OBJECT

public:
  explicit DashboardWidget(QWidget *parent = nullptr);
  ~DashboardWidget();

  void updateData(const RealTimeData &data);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  void setupUi();

  // Left & Right Gauges
  TunerGauge *m_rpmGauge;
  TunerGauge *m_mapGauge;
  
  // Center Cards
  DataCard *m_cardTps;
  DataCard *m_cardIat;
  DataCard *m_cardClt;
  DataCard *m_cardBat;
  DataCard *m_cardO2;
  DataCard *m_cardIgn;
  DataCard *m_cardPw;
  DataCard *m_cardBaro;
  
  // Bottom Graphs
  DataGraph *m_afrGraph;
  DataGraph *m_advanceGraph;
  
  // Fuel Status Panel
  StatusLine *m_stFuelPump;
  StatusLine *m_stWarmup;
  StatusLine *m_stAse;
  StatusLine *m_stTpsWot;
  StatusLine *m_stOverrun;
  StatusLine *m_stDuty; // Shows percentage
  
  // Engine Status Panel
  StatusLine *m_stCranking;
  StatusLine *m_stSync;
  StatusLine *m_stHardLimit;
  StatusLine *m_stSoftLimit;
  StatusLine *m_stFan;
  StatusLine *m_stKnock;
};

#endif // DASHBOARDWIDGET_H
