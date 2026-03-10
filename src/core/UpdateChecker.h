#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class UpdateChecker : public QObject {
    Q_OBJECT
public:
    explicit UpdateChecker(const QString& currentVersion, QObject *parent = nullptr);

    // Initiates the network request
    void checkForUpdates(bool silentMode = false);

    QString getCurrentVersion() const { return m_currentVersion; }

signals:
    // Emitted when a newer version is found
    void updateAvailable(const QString& latestVersion, const QString& downloadUrl, const QString& releaseNotesUrl, bool silentMode);
    
    // Emitted when the current version is up to date
    void upToDate(bool silentMode);
    
    // Emitted when the network request or JSON parsing fails
    void checkFailed(const QString& errorMessage, bool silentMode);

    // Emitted during download progress
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

    // Emitted when the update is ready to be installed
    void updateReadyForInstall(const QString& installerPath);

public slots:
    // Downloads the installer from the given URL
    void downloadUpdate(const QString& downloadUrl);

    // Launches the installer and exits the app
    void applyUpdate(const QString& installerPath);

private slots:
    void onNetworkReply(QNetworkReply* reply);
    void onDownloadFinished(QNetworkReply* reply);

private:
    QString m_currentVersion;
    QNetworkAccessManager* m_networkManager;
    QString m_versionJsonUrl;
    bool m_silentMode;
};

#endif // UPDATECHECKER_H
