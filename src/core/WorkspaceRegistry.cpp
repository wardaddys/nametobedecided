#include "WorkspaceRegistry.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

WorkspaceRegistry::WorkspaceRegistry() {
}

QStringList WorkspaceRegistry::workspaceIds() const {
    return m_workspaces;
}

QString WorkspaceRegistry::workspaceLabel(const QString& id) const {
    return m_workspaceLabels.value(id, "Unknown Workspace");
}

QList<QString> WorkspaceRegistry::settingsInWorkspace(const QString& workspaceId, const QString& subsection) const {
    QList<QString> result;
    
    // Convert m_mappings to an ordered list based on displayOrder
    QList<SettingMapping> orderedMappings;
    for (auto it = m_mappings.constBegin(); it != m_mappings.constEnd(); ++it) {
        if (it.value().primaryWorkspace == workspaceId) {
            if (subsection.isEmpty() || it.value().subsection == subsection) {
                orderedMappings.append(it.value());
            }
        }
    }
    
    // Sort by displayOrder
    std::sort(orderedMappings.begin(), orderedMappings.end(), [](const SettingMapping& a, const SettingMapping& b) {
        return a.displayOrder < b.displayOrder;
    });
    
    for (const auto& mapping : orderedMappings) {
        result.append(mapping.name);
    }
    
    return result;
}

QString WorkspaceRegistry::primaryWorkspaceFor(const QString& settingName) const {
    if (m_mappings.contains(settingName.toLower())) {
        return m_mappings.value(settingName.toLower()).primaryWorkspace;
    }
    return QString();
}

QStringList WorkspaceRegistry::secondaryWorkspacesFor(const QString& settingName) const {
    if (m_mappings.contains(settingName.toLower())) {
        return m_mappings.value(settingName.toLower()).secondaryWorkspaces;
    }
    return QStringList();
}

bool WorkspaceRegistry::isSettingMapped(const QString& settingName) const {
    return m_mappings.contains(settingName.toLower());
}

const SettingMapping& WorkspaceRegistry::getMapping(const QString& settingName) const {
    auto it = m_mappings.constFind(settingName.toLower());
    if (it != m_mappings.constEnd()) {
        return it.value();
    }
    static const SettingMapping emptyMapping;
    return emptyMapping;
}

QStringList WorkspaceRegistry::allMappedNames() const {
    return m_mappings.keys();
}

void WorkspaceRegistry::loadMappingFromYaml(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open mapping file:" << path;
        return;
    }

    QTextStream in(&file);
    SettingMapping currentMapping;
    bool inItem = false;
    
    QRegularExpression listRe("^\\s*-\\s+name:\\s*(.+)$");
    QRegularExpression keyValRe("^\\s*([a-zA-Z0-9_]+):\\s*(.*)$");
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        
        // Skip comments and empty lines
        if (line.trimmed().isEmpty() || line.trimmed().startsWith("#")) {
            continue;
        }
        
        QRegularExpressionMatch listMatch = listRe.match(line);
        if (listMatch.hasMatch()) {
            if (inItem && !currentMapping.name.isEmpty()) {
                m_mappings[currentMapping.name.toLower()] = currentMapping;
            }
            currentMapping = SettingMapping();
            
            // Remove quotes if any
            QString name = listMatch.captured(1).trimmed();
            if (name.startsWith('"') && name.endsWith('"')) {
                name = name.mid(1, name.length() - 2);
            }
            
            currentMapping.name = name;
            inItem = true;
            continue;
        }
        
        if (inItem) {
            QRegularExpressionMatch kvMatch = keyValRe.match(line);
            if (kvMatch.hasMatch()) {
                QString key = kvMatch.captured(1).trimmed();
                QString val = kvMatch.captured(2).trimmed();
                
                // Remove quotes
                if (val.startsWith('"') && val.endsWith('"')) {
                    val = val.mid(1, val.length() - 2);
                } else if (val.startsWith('\'') && val.endsWith('\'')) {
                    val = val.mid(1, val.length() - 2);
                }
                
                if (key == "primary_workspace") {
                    currentMapping.primaryWorkspace = val;
                } else if (key == "secondary_workspaces") {
                    // Expecting something like: [ws1, ws2]
                    if (val.startsWith('[') && val.endsWith(']')) {
                        val = val.mid(1, val.length() - 2);
                        QStringList parts = val.split(',');
                        for (auto& p : parts) {
                            currentMapping.secondaryWorkspaces.append(p.trimmed());
                        }
                    }
                } else if (key == "subsection") {
                    currentMapping.subsection = val;
                } else if (key == "display_order") {
                    currentMapping.displayOrder = val.toInt();
                } else if (key == "human_label") {
                    currentMapping.humanLabel = val;
                } else if (key == "help_text") {
                    currentMapping.helpText = val;
                }
            }
        }
    }
    
    if (inItem && !currentMapping.name.isEmpty()) {
        m_mappings[currentMapping.name.toLower()] = currentMapping;
    }
    
    file.close();
}
