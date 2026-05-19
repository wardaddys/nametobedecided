#ifndef DECODERCONFIGWIDGET_H
#define DECODERCONFIGWIDGET_H

#include <QWidget>

class QComboBox;
class QSpinBox;
class ECUSettingsManager;

// E1: Decoder / trigger configuration UI.
//
// Surfaces the 28 trigger patterns from Speeduino firmware decoders.h plus the
// missing-tooth count, wheel teeth, secondary trigger, trigger filter level,
// and trigger edge selectors. Bound to ECUSettingsManager via setValue() so
// changes propagate to the ECU on edit.
class DecoderConfigWidget : public QWidget {
    Q_OBJECT
public:
    explicit DecoderConfigWidget(QWidget* parent = nullptr);
    void setSettingsManager(ECUSettingsManager* mgr);
    void loadFromECU();

private:
    void setupUi();
    void wireSignals();

    ECUSettingsManager* m_settings = nullptr;

    QComboBox* m_trigPattern   = nullptr;
    QSpinBox*  m_wheelTeeth    = nullptr;
    QSpinBox*  m_missingTooth  = nullptr;
    QComboBox* m_trigFilter    = nullptr;
    QComboBox* m_trigEdge      = nullptr;
    QComboBox* m_secTrigEdge   = nullptr;
    QComboBox* m_thirdTrigEdge = nullptr;
    QSpinBox*  m_skipRevolutions = nullptr;
};

#endif // DECODERCONFIGWIDGET_H
