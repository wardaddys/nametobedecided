#ifndef COMMSCONFIGWIDGET_H
#define COMMSCONFIGWIDGET_H

#include <QWidget>

class QCheckBox;
class QComboBox;
class QSpinBox;
class ECUSettingsManager;

// E9: Communications configuration — CAN bus + secondary serial + OBD-II.
//
// Surfaces firmware comms_CAN.h options:
//   - CAN broadcast (BMW E46/E90, VAG, Haltech, custom 11-bit/29-bit)
//   - CAN WBO receive (rusEFI, AEM)
//   - OBD-II responder
//   - Secondary UART (baud + role)
class CommsConfigWidget : public QWidget {
    Q_OBJECT
public:
    explicit CommsConfigWidget(QWidget* parent = nullptr);
    void setSettingsManager(ECUSettingsManager* mgr);
    void loadFromECU();

private:
    void setupUi();

    ECUSettingsManager* m_settings = nullptr;

    // CAN broadcast
    QCheckBox*  m_canBcastEnabled = nullptr;
    QComboBox*  m_canBcastMode    = nullptr;
    QSpinBox*   m_canBcastBaseId  = nullptr;

    // CAN WBO receive
    QCheckBox*  m_canWboEnabled   = nullptr;
    QComboBox*  m_canWboProtocol  = nullptr;

    // OBD-II
    QCheckBox*  m_obd2Enabled     = nullptr;

    // Secondary serial
    QCheckBox*  m_serial2Enabled  = nullptr;
    QComboBox*  m_serial2Baud     = nullptr;
    QComboBox*  m_serial2Role     = nullptr;
};

#endif // COMMSCONFIGWIDGET_H
