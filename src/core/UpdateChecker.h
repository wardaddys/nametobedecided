#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>

/**
 * @brief Update checker — Phase 1: browser-only, no auto-download.
 *
 * The original implementation downloaded arbitrary .exe files from a URL
 * in a JSON response with no signature verification — a remote-code-execution
 * vector. This replacement only opens the user's browser to the releases page.
 *
 * Future hardening (Phase 2+): GPG-signed releases, embedded public key,
 * signature verification before any download.
 */
class UpdateChecker : public QObject {
    Q_OBJECT
public:
    explicit UpdateChecker(const QString& currentVersion, QObject *parent = nullptr);

    /// Opens the user's default browser to the GitHub releases page.
    /// If silentMode is true, does nothing (no popup, no browser).
    void checkForUpdates(bool silentMode = false);

    QString getCurrentVersion() const { return m_currentVersion; }
    QString getReleasesUrl() const { return m_releasesUrl; }

signals:
    // Kept for API compatibility with MainWindow slots.
    // updateAvailable is never emitted in Phase 1 (no network check).
    void updateAvailable(const QString& latestVersion, const QString& downloadUrl, const QString& releaseNotesUrl, bool silentMode);
    void upToDate(bool silentMode);
    void checkFailed(const QString& errorMessage, bool silentMode);

private:
    QString m_currentVersion;
    QString m_releasesUrl;
};

#endif // UPDATECHECKER_H
