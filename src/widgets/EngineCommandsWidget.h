#ifndef ENGINECOMMANDSWIDGET_H
#define ENGINECOMMANDSWIDGET_H

#include <QWidget>

class QPushButton;
class SerialManager;

// E2: Engine Commands panel — exposes the TS_CMD_* button infrastructure
// that the firmware accepts via the 'E' (16-bit) command.
//
// SpeeduinoProtocol::createButtonCommand has existed for a while but no UI
// surface ever called it. This widget changes that, giving the user one-
// click access to: test-mode enable/disable, INJ/IGN channel tests,
// SD format, STM32 reboot/bootloader, and VSS calibration.
//
// Safety: all buttons are gated by isConnected(). Confirmations are
// requested for any command that touches engine state.
class EngineCommandsWidget : public QWidget {
    Q_OBJECT
public:
    explicit EngineCommandsWidget(QWidget* parent = nullptr);
    void setSerialManager(SerialManager* serial);

private slots:
    void onConnectionChanged();

private:
    QPushButton* makeCommandButton(const QString& label, quint16 cmdId,
                                   bool requireConfirm,
                                   const QString& confirmMsg = QString());
    void setupUi();

    SerialManager* m_serial = nullptr;
};

#endif // ENGINECOMMANDSWIDGET_H
