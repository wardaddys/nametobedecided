#ifndef COMPOSITELOGGERWIDGET_H
#define COMPOSITELOGGERWIDGET_H

#include <QWidget>

class QPushButton;
class QLabel;
class SerialManager;

// E10: Composite logger UI.
//
// Tooth logger captures only the primary trigger. Composite logger captures
// the primary, secondary (cam), and (when available) tertiary trigger in
// time-synchronised fashion — essential for diagnosing sync issues on
// unusual decoder patterns (e.g. Honda J32, Mazda AU, dual-cam VVT).
class CompositeLoggerWidget : public QWidget {
    Q_OBJECT
public:
    explicit CompositeLoggerWidget(QWidget* parent = nullptr);
    void setSerialManager(SerialManager* serial);

private slots:
    void onStartClicked();
    void onStopClicked();
    void onReadClicked();
    void onResetClicked();
    void onConnectionChanged();

private:
    void setupUi();
    SerialManager* m_serial = nullptr;
    QPushButton*   m_startBtn = nullptr;
    QPushButton*   m_stopBtn  = nullptr;
    QPushButton*   m_readBtn  = nullptr;
    QPushButton*   m_resetBtn = nullptr;
    QLabel*        m_statusLbl = nullptr;
};

#endif // COMPOSITELOGGERWIDGET_H
