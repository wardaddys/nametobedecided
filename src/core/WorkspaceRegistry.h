#ifndef WORKSPACEREGISTRY_H
#define WORKSPACEREGISTRY_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QList>

struct SettingMapping {
    QString name;
    QString primaryWorkspace;
    QStringList secondaryWorkspaces;
    QString subsection;
    int displayOrder = 0;
    QString humanLabel;
    QString helpText;
};

class WorkspaceRegistry {
public:
    static WorkspaceRegistry& instance() {
        static WorkspaceRegistry instance;
        return instance;
    }

    QStringList workspaceIds() const;
    QString workspaceLabel(const QString& id) const;
    
    QList<QString> settingsInWorkspace(const QString& workspaceId, 
                                        const QString& subsection = QString()) const;
    
    QString primaryWorkspaceFor(const QString& settingName) const;
    QStringList secondaryWorkspacesFor(const QString& settingName) const;
    
    void loadMappingFromYaml(const QString& path);
    
    // Check if the setting is known in the mapping
    bool isSettingMapped(const QString& settingName) const;

    const SettingMapping& getMapping(const QString& settingName) const;

    // All setting names known to the mapping (for lint / consistency checks).
    QStringList allMappedNames() const;

private:
    WorkspaceRegistry();
    ~WorkspaceRegistry() = default;
    
    WorkspaceRegistry(const WorkspaceRegistry&) = delete;
    WorkspaceRegistry& operator=(const WorkspaceRegistry&) = delete;

    QMap<QString, SettingMapping> m_mappings;
    
    // Locked workspaces list based on the prompt
    QStringList m_workspaces = {
        "engine_basics",
        "sensors",
        "triggers",
        "fueling",
        "ignition",
        "cold_start",
        "idle",
        "boost",
        "vvt",
        "engine_protection",
        "io",
        "comms"
    };

    QMap<QString, QString> m_workspaceLabels = {
        {"engine_basics", "Engine Basics"},
        {"sensors", "Sensors & Calibration"},
        {"triggers", "Triggers & Sync"},
        {"fueling", "Fueling"},
        {"ignition", "Ignition"},
        {"cold_start", "Cold Start"},
        {"idle", "Idle Control"},
        {"boost", "Boost & Aspiration"},
        {"vvt", "VVT"},
        {"engine_protection", "Engine Protection"},
        {"io", "Inputs & Outputs"},
        {"comms", "Communications & Auxiliary"}
    };
};

#endif // WORKSPACEREGISTRY_H
