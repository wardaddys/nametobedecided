#ifndef MSQPARSER_H
#define MSQPARSER_H

#include <QDateTime>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>

class ECUDefinition;
class ECUSettingsManager;

// F1: Round-trip metadata cache.
//
// MsqParser previously only kept constant values. Round-tripping requires
// preserving the bibliography, versionInfo, settings, and userComments
// blocks the file came with — otherwise saving discards information the
// user can never get back.
struct MsqMetadata {
    // <bibliography> attributes
    QString author;
    QString tuneComment;
    QString writeDate;       // Java-style: "Fri Dec 26 11:50:13 PKT 2025"

    // <versionInfo> attributes
    QString fileFormat = "5.0";
    QString firmwareInfo;
    QString signature;
    int     nPages = 0;

    // <settings> block — names of currently-active #set directives
    QStringList settingsBlock;
    QString     userComments;
};

// F3: SaveOptions.
//
// All optional — fall back to MsqMetadata::fileFormat if loaded from a
// previous round-trip, or sensible defaults if creating a new file.
struct MsqSaveOptions {
    QString author             = "OS Tuner";
    QString tuneComment;
    QString firmwareInfo;
    QString tunerStudioVersion;
    QString signature;            // override; default = definition->getSignature()
    QString fileFormat;           // empty = inherit MsqMetadata::fileFormat / "5.0"
    bool    includeUnusedPages = true;
    bool    includeSettingsBlock = true;
    bool    prettyPrint        = true;
    bool    useUtcWriteDate    = false;
};

class MsqParser {
public:
    MsqParser();

    bool load(const QString &filePath);

    // name -> text value (untyped — preserved as the XML text payload).
    const QMap<QString, QString> &getConstants() const { return m_constants; }

    // F1 accessors.
    const MsqMetadata &metadata() const { return m_metadata; }
    bool isPcVariable(const QString &name) const { return m_pcVariableNames.contains(name); }
    const QSet<QString> &pcVariableNames() const { return m_pcVariableNames; }

    // F3: write the current page-cache state out as a TS-compatible MSQ file.
    bool save(const QString &filePath,
              const ECUDefinition *definition,
              const ECUSettingsManager *settings,
              const MsqSaveOptions &options = MsqSaveOptions{});

    QString lastError() const { return m_lastError; }

private:
    QMap<QString, QString> m_constants;
    QSet<QString>          m_pcVariableNames;   // F1: distinguish PC vars from page constants
    MsqMetadata            m_metadata;          // F1: round-trip cache
    QString                m_lastError;

    static QString formatJavaDate(const QDateTime &dt, bool utc);
};

#endif // MSQPARSER_H
