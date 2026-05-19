#ifndef FUELINGWORKSPACE_H
#define FUELINGWORKSPACE_H

#include "WorkspaceBase.h"

class ECUSettingsManager;

// D1: First real Workspace implementation.
//
// FuelingWorkspace renders its subsections by querying WorkspaceRegistry for
// the list of settings mapped to each subsection, then creating editor
// controls (spin box / line edit / combo) bound to ECUSettingsManager.
// Writes flow through ECUSettingsManager::setValue exactly the same way
// ECUSettingsWidget does, so we don't introduce a parallel write path.
class FuelingWorkspace : public WorkspaceBase {
    Q_OBJECT

public:
    explicit FuelingWorkspace(QWidget* parent = nullptr);

    void setSettingsManager(ECUSettingsManager* mgr);

    QString workspaceId() const override { return "fueling"; }
    QString workspaceLabel() const override { return "Fueling"; }
    QStringList subsections() const override;
    QWidget* createSubsectionWidget(const QString& subsectionId) override;
    QStringList crossReferences() const override {
        return {"Sensors → O2 / Lambda", "Sensors → MAP",
                "Cold Start", "Engine Protection → Lean Protection"};
    }

private:
    ECUSettingsManager* m_settingsManager = nullptr;
};

#endif // FUELINGWORKSPACE_H
