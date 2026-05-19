#ifndef SDLOGGERWIDGET_H
#define SDLOGGERWIDGET_H

#include <QWidget>

class QLabel;
class QPushButton;
class QProgressBar;
class SerialManager;
struct RealTimeData;

// E7: SD onboard logger control panel.
//
// Firmware SD_logger.h exposes onboard logging with start/stop/format actions
// and a TS_SD_Status byte (RT byte 120, already parsed by ECUData.h:139).
// This widget surfaces the actions and shows live status.
class SDLoggerWidget : public QWidget {
    Q_OBJECT
public:
    explicit SDLoggerWidget(QWidget* parent = nullptr);
    void setSerialManager(SerialManager* serial);

public slots:
    void updateRealtimeStatus(const RealTimeData& data);

private slots:
    void onStartClicked();
    void onStopClicked();
    void onFormatClicked();
    void onConnectionChanged();

private:
    void setupUi();

    SerialManager* m_serial = nullptr;
    QLabel*        m_statusLabel    = nullptr;
    QLabel*        m_freeSpaceLabel = nullptr;
    QPushButton*   m_startBtn       = nullptr;
    QPushButton*   m_stopBtn        = nullptr;
    QPushButton*   m_formatBtn      = nullptr;
    QProgressBar*  m_capacityBar    = nullptr;
};

#endif // SDLOGGERWIDGET_H
