#include "UpdateChecker.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkRequest>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QUrl>

UpdateChecker::UpdateChecker(const QString& currentVersion, QObject* parent)
    : QObject(parent),
      m_currentVersion(currentVersion),
      m_silentMode(false)
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &UpdateChecker::onNetworkReply);

    // Placeholder URL - This would point to a real version.json hosted on GithHub Pages, S3, etc.
    m_versionJsonUrl = "https://raw.githubusercontent.com/tunerpro/tunerpro/main/version.json";
}

void UpdateChecker::checkForUpdates(bool silentMode)
{
    m_silentMode = silentMode;
    QNetworkRequest request((QUrl(m_versionJsonUrl)));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    m_networkManager->get(request);
}

void UpdateChecker::onNetworkReply(QNetworkReply* reply)
{
    // Clean up the reply when we're done
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit checkFailed("Network error: " + reply->errorString(), m_silentMode);
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit checkFailed("Invalid response format from update server.", m_silentMode);
        return;
    }

    if (!jsonDoc.isObject()) {
        emit checkFailed("Invalid JSON object from update server.", m_silentMode);
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    QString latestVersion = jsonObj.value("latest_version").toString();
    QString downloadUrl = jsonObj.value("download_url").toString();
    QString releaseNotesUrl = jsonObj.value("release_notes_url").toString();

    if (latestVersion.isEmpty()) {
        emit checkFailed("Missing version information in server response.", m_silentMode);
        return;
    }

    // Simple string comparison for versions. 
    // In a real app, you might want to split by '.' and compare integers (major/minor/patch).
    // For this prototype, if it's not equal to current and current is not empty, assume it might be newer 
    // (a more robust SemVer comparison would be better here).
    if (latestVersion != m_currentVersion) {
        emit updateAvailable(latestVersion, downloadUrl, releaseNotesUrl, m_silentMode);
    } else {
        emit upToDate(m_silentMode);
    }
}

void UpdateChecker::downloadUpdate(const QString& downloadUrl)
{
    QNetworkRequest request(downloadUrl);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::downloadProgress, this, &UpdateChecker::downloadProgress);
    connect(reply, &QNetworkReply::finished, this, [this, reply](){ this->onDownloadFinished(reply); });
}

void UpdateChecker::onDownloadFinished(QNetworkReply* reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit checkFailed("Download failed: " + reply->errorString(), false);
        return;
    }

    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString fileName = "TunerPro_Update.exe";
    QString fullPath = QDir(tempPath).absoluteFilePath(fileName);

    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit checkFailed("Could not open file to save update: " + file.errorString(), false);
        return;
    }

    file.write(reply->readAll());
    file.close();

    emit updateReadyForInstall(fullPath);
}

void UpdateChecker::applyUpdate(const QString& installerPath)
{
    // Launch the installer. We use startDetached so it persists after we exit.
    bool success = QProcess::startDetached(installerPath, QStringList());
    
    if (success) {
        // Exit TunerPro so the installer can overwrite the files
        QCoreApplication::exit(0);
    } else {
        emit checkFailed("Failed to launch installer. Please run it manually from " + installerPath, false);
    }
}
