#include "UpdateChecker.h"
#include <QDesktopServices>
#include <QUrl>

UpdateChecker::UpdateChecker(const QString& currentVersion, QObject* parent)
    : QObject(parent),
      m_currentVersion(currentVersion),
      m_releasesUrl("https://github.com/AeroSaeed/OS-Tuner/releases")
{
}

void UpdateChecker::checkForUpdates(bool silentMode)
{
    if (silentMode) {
        // Silent mode: do nothing. No network request, no popup.
        // In Phase 2+ this would do a background version.json fetch
        // with GPG signature verification.
        return;
    }

    // Manual "Check for Updates" → just open the browser
    QDesktopServices::openUrl(QUrl(m_releasesUrl));
}
