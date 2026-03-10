#include "ProjectManager.h"
#include "ECUSettingsManager.h"
#include "../utils/Logger.h"
#include "MsqParser.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

ProjectManager::ProjectManager(ECUSettingsManager *settingsMgr, QObject *parent)
    : QObject(parent), m_settingsManager(settingsMgr) {
}

ProjectManager::~ProjectManager() {
}

bool ProjectManager::loadProject(const QString &projectDir) {
    QDir dir(projectDir);
    if (!dir.exists()) {
        emit projectLoadFailed("Project directory does not exist.");
        return false;
    }
    
    m_projectDir = projectDir;
    m_projectName = dir.dirName();
    
    // 1. Try to read project.properties
    QString propPath = dir.filePath("projectCfg/project.properties");
    if (QFile::exists(propPath)) {
        parseProjectProperties(propPath);
    }
    
    // 2. Find INI file
    if (!findIniFile()) {
        emit projectLoadFailed("Could not find mainController.ini or valid custom.ini in project.");
        return false;
    }
    
    // 3. Find MSQ file
    findMsqFile();
    
    // Load INI into ECUDefinition
    Logger::info("Loading INI: " + m_iniPath);
    bool iniLoaded = m_settingsManager->loadDefinition(m_iniPath);
    if (!iniLoaded) {
        emit projectLoadFailed("Failed to parse INI file.");
        return false;
    }
    
    // Load MSQ into ECUSettingsManager cache
    if (!m_msqPath.isEmpty()) {
        Logger::info("Loading MSQ: " + m_msqPath);
        MsqParser msq;
        if (msq.load(m_msqPath)) {
            // Populate caches using parsed strings
            m_settingsManager->injectMsqData(msq.getConstants());
        } else {
            Logger::warning("Failed to parse MSQ, continuing with default offline data.");
        }
    }
    
    emit projectLoaded(m_projectName);
    return true;
}

bool ProjectManager::parseProjectProperties(const QString &propPath) {
    QFile file(propPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;
        
        int eqIdx = line.indexOf('=');
        if (eqIdx > 0) {
            QString key = line.left(eqIdx).trimmed();
            QString val = line.mid(eqIdx + 1).trimmed();
            m_properties[key] = val;
        }
    }
    return true;
}

bool ProjectManager::findIniFile() {
    QDir cfgDir(m_projectDir + "/projectCfg");
    
    // Check mainController.ini
    if (cfgDir.exists("mainController.ini")) {
        m_iniPath = cfgDir.filePath("mainController.ini");
        return true;
    }
    
    // Fallback: Check if there's any other .ini
    QStringList inis = cfgDir.entryList(QStringList() << "*.ini", QDir::Files);
    if (!inis.isEmpty()) {
        m_iniPath = cfgDir.filePath(inis.first());
        return true;
    }
    return false;
}

bool ProjectManager::findMsqFile() {
    QDir dir(m_projectDir);
    
    // Check CurrentTune.msq
    if (dir.exists("CurrentTune.msq")) {
        m_msqPath = dir.filePath("CurrentTune.msq");
        return true;
    }
    
    // Fallback: Check if there's any other .msq
    QStringList msqs = dir.entryList(QStringList() << "*.msq", QDir::Files);
    if (!msqs.isEmpty()) {
        m_msqPath = dir.filePath(msqs.first());
        return true;
    }
    
    m_msqPath = "";
    return false;
}
