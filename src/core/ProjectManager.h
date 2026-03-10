#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include "ECUDefinition.h"
#include <QObject>
#include <QString>
#include <QDir>
#include <QMap>

class ECUSettingsManager;

class ProjectManager : public QObject {
    Q_OBJECT
    
public:
    explicit ProjectManager(ECUSettingsManager *settingsMgr, QObject *parent = nullptr);
    ~ProjectManager();

    bool loadProject(const QString &projectDir);
    
    QString getProjectName() const { return m_projectName; }
    QString getProjectDir() const { return m_projectDir; }
    QString getIniPath() const { return m_iniPath; }
    QString getMsqPath() const { return m_msqPath; }

signals:
    void projectLoaded(const QString &projectName);
    void projectLoadFailed(const QString &error);

private:
    bool parseProjectProperties(const QString &propPath);
    bool findIniFile();
    bool findMsqFile();
    
    ECUSettingsManager *m_settingsManager;
    QString m_projectDir;
    QString m_projectName;
    QString m_iniPath;
    QString m_msqPath;
    
    QMap<QString, QString> m_properties;
};

#endif // PROJECTMANAGER_H
