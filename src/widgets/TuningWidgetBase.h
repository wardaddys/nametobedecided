#ifndef TUNINGWIDGETBASE_H
#define TUNINGWIDGETBASE_H

#include <QWidget>

#include "core/ECUData.h"
class ECUSettingsManager;
class QVBoxLayout;

/**
 * @brief Base class for tuning widgets to handle boilerplate ECUSettingsManager binding & layout.
 */
class TuningWidgetBase : public QWidget {
    Q_OBJECT

public:
    explicit TuningWidgetBase(QWidget *parent = nullptr);
    virtual ~TuningWidgetBase() = default;

    void setSettingsManager(ECUSettingsManager *mgr);
    virtual void loadFromECU() = 0;
    virtual void updateRealtimeData(const RealTimeData &data) {}

protected:
    virtual void setupUi() = 0;
    
    ECUSettingsManager *m_settingsManager = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;
};

#endif // TUNINGWIDGETBASE_H
