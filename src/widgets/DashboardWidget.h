#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include "TunerGauge.h"
#include "DataGraph.h"
#include "core/ECUData.h"

class ECUSettingsManager;  // forward declaration -- avoids circular include
class QVBoxLayout;

class QStackedWidget;
class QComboBox;
class QLineEdit;
class SerialManager;
class ThrottleBodyWidget;
class MapSensorWidget;
class TempSensorWidget;
class InjectorVisualWidget;

class QProgressBar;

class CalibrationOverlay : public QWidget {
    Q_OBJECT
public:
    explicit CalibrationOverlay(QWidget *parent = nullptr);
    void setSettingsManager(ECUSettingsManager *manager) { m_settingsManager = manager; }
    void setSerialManager(SerialManager *sm) { m_serialManager = sm; }
    void showCalibration(const QString &title);
    void closeOverlay();

    /** Push live ECU data into the open panel -- drives the live ADC, percent, C, etc. */
    void updateLiveData(const RealTimeData &data);

signals:
    void closed();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void buildTpsPanel();
    void buildMapPanel();
    void buildTempPanel();
    void buildDutyPanel();
    QByteArray generateThermistorTable(double biasResistor, double r1, double t1, double r2, double t2, double r3, double t3);

    // Loads min/max + sensor selection from the active ECU definition / settings.
    void loadTpsFromSettings();
    void loadMapFromSettings();
    void loadTempFromSettings();

    /**
     * Wait for ECUSettingsManager's async burn chain to finish, then show
     * a Success / Error dialog. Disables the overlay UI while in flight and
     * applies a watchdog timeout so the user is never left without feedback.
     *
     * @param opName  Human-readable label used in dialog text ("TPS", "MAP", ...).
     * @param closeOverlayOnSuccess  Whether to close the overlay after Success.
     */
    void awaitBurnAndShowResult(const QString &opName,
                                bool closeOverlayOnSuccess = true);

    QWidget *m_panel;
    QLabel *m_titleLabel;
    QStackedWidget *m_stack;

    // TPS Panel
    QWidget *m_tpsPanel;
    QLineEdit *m_tpsClosedEdit;
    QLineEdit *m_tpsFullEdit;
    QLabel    *m_tpsLiveAdcLbl;     ///< Live raw TPS ADC count
    QLabel    *m_tpsLivePctLbl;     ///< Live TPS percentage
    QProgressBar *m_tpsLiveBar;     ///< Live bar tied to %TPS
    int       m_lastTpsADC = 0;     ///< Cached so "Get Current" buttons can capture
    // Extra widgets for the redesigned TPS modal
    ThrottleBodyWidget *m_throttleBody = nullptr;
    QLabel *m_tpsAngleLbl    = nullptr; ///< "28.3°" in the live-view header
    QLabel *m_tpsHudOpenLbl  = nullptr; ///< Big "OPEN" readout under throttle body
    QLabel *m_tpsHudAngleLbl = nullptr; ///< Big "ANGLE" readout under throttle body
    QLabel *m_tpsStatAdc     = nullptr; ///< Stat strip ADC value
    QLabel *m_tpsStatRange   = nullptr; ///< Stat strip RANGE value
    QLabel *m_tpsStatDead    = nullptr; ///< Stat strip DEADBAND value

    // MAP Panel
    QWidget *m_mapPanel;
    QComboBox *m_mapSensorCombo;
    QLineEdit *m_map0Edit;
    QLineEdit *m_map5Edit;
    QLabel    *m_mapLiveKpaLbl;     ///< Live MAP kPa
    QProgressBar *m_mapLiveBar;     ///< Live bar (0..maxKpa)
    MapSensorWidget *m_mapVisual = nullptr;  ///< 3D pressure dial

    // Temp Panel (CLT / IAT)
    QWidget *m_tempPanel;
    QComboBox *m_tempSensorCombo;
    QLineEdit *m_tempBiasEdit;
    QLineEdit *m_tempR1Edit;
    QLineEdit *m_tempT1Edit;
    QLineEdit *m_tempR2Edit;
    QLineEdit *m_tempT2Edit;
    QLineEdit *m_tempR3Edit;
    QLineEdit *m_tempT3Edit;
    QLabel    *m_tempLiveLbl;       ///< Live temp C (CLT or IAT)
    QProgressBar *m_tempLiveBar;
    TempSensorWidget *m_tempVisual = nullptr; ///< 3D thermometer probe

    // Duty Panel
    QWidget *m_dutyPanel = nullptr;
    QLabel *m_dutyLiveLbl = nullptr;
    QProgressBar *m_dutyLiveBar = nullptr;
    InjectorVisualWidget *m_dutyVisual = nullptr;

    ECUSettingsManager *m_settingsManager = nullptr;
    SerialManager *m_serialManager = nullptr;
    QString m_currentTitle; // To track CLT vs IAT
    bool m_burnInProgress = false; ///< Guards against re-entry while async burn is in flight

private slots:
    void onTpsAccept();
    void onMapBurn();
    void onTempBurn();
};

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

  /** Connect to an ECUSettingsManager so offline/MSQ scalar values are shown. */
  void setSettingsManager(ECUSettingsManager *mgr);
  void setSerialManager(SerialManager *sm);

  void updateData(const RealTimeData &data);

  // === Getters for Onboarding/ProductTour (Phase A) ===
  QWidget* getRpmGauge() const { return m_rpmGauge; }
  QWidget* getMapGauge() const { return m_mapGauge; }
  QWidget* getTpsGauge() const { return m_tpsGauge; }
  QWidget* getCltGauge() const { return m_cltGauge; }
  QWidget* getIatGauge() const { return m_iatGauge; }
  QWidget* getDutyGauge() const { return m_dutyGauge; }

public slots:
  /** Receives settingChanged() from ECUSettingsManager -- updates cards offline. */
  void onSettingChanged(const QString &name, const QVariant &value);

protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private:
  void setupUi();

  ECUSettingsManager *m_settingsManager = nullptr;
  CalibrationOverlay *m_calOverlay = nullptr;

  // Left & Right Gauges
  TunerGauge *m_rpmGauge;
  TunerGauge *m_mapGauge;
  TunerGauge *m_iatGauge;
  TunerGauge *m_cltGauge;
  TunerGauge *m_tpsGauge;
  TunerGauge *m_dutyGauge;

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
