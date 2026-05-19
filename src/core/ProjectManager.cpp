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
        emit projectLoadFailed(
            QString("No INI file found in %1.\n\n"
                    "Looked in:\n"
                    "  • %1/projectCfg/mainController.ini\n"
                    "  • %1/projectCfg/*.ini\n"
                    "  • %1/mainController.ini\n"
                    "  • %1/*.ini\n\n"
                    "Pick a TunerStudio project folder (the one that contains "
                    "either a projectCfg/ subfolder or an .ini file directly).")
                .arg(projectDir));
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
    // Look in every place a real TunerStudio project might put the INI:
    //   1. <project>/projectCfg/mainController.ini  (TS-managed projects)
    //   2. <project>/projectCfg/*.ini               (any other INI in projectCfg)
    //   3. <project>/mainController.ini             (flat layout)
    //   4. <project>/*.ini                          (loose INI in project root)
    //   5. <project>'s parent — if the user selected projectCfg/ by mistake
    //      we resolve up one level and try again.
    auto tryDir = [this](const QString& path) -> bool {
        QDir dir(path);
        if (!dir.exists()) return false;
        if (dir.exists("mainController.ini")) {
            m_iniPath = dir.filePath("mainController.ini");
            return true;
        }
        QStringList inis = dir.entryList(QStringList() << "*.ini", QDir::Files);
        if (!inis.isEmpty()) {
            m_iniPath = dir.filePath(inis.first());
            return true;
        }
        return false;
    };

    if (tryDir(m_projectDir + "/projectCfg")) return true;
    if (tryDir(m_projectDir)) return true;

    // The user may have selected projectCfg/ instead of the project root.
    QDir asProject(m_projectDir);
    if (asProject.dirName() == "projectCfg") {
        QDir parent = asProject;
        parent.cdUp();
        if (tryDir(parent.absolutePath())) {
            // Adjust the project root so the MSQ lookup also works.
            m_projectDir = parent.absolutePath();
            m_projectName = parent.dirName();
            return true;
        }
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
