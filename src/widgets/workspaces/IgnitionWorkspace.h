#ifndef IGNITIONWORKSPACE_H
#define IGNITIONWORKSPACE_H

#include "WorkspaceBase.h"

class ECUSettingsManager;

// D3: Second real Workspace. Same pattern as FuelingWorkspace —
// renders settings from mapping.yaml + ECUSettingsManager.
class IgnitionWorkspace : public WorkspaceBase {
    Q_OBJECT

public:
    explicit IgnitionWorkspace(QWidget* parent = nullptr);

    void setSettingsManager(ECUSettingsManager* mgr);

    QString workspaceId() const override { return "ignition"; }
    QString workspaceLabel() const override { return "Ignition"; }
    QStringList subsections() const override;
    QWidget* createSubsectionWidget(const QString& subsectionId) override;
    QStringList crossReferences() const override {
        return {"Triggers & Sync", "Sensors → Knock Sensor", "Fueling → AFR Target"};
    }

private:
    ECUSettingsManager* m_settingsManager = nullptr;
};

#endif // IGNITIONWORKSPACE_H
