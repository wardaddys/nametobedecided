// F1 + F3: MsqParser load with round-trip metadata, plus save().
//
// The save format mirrors what TunerStudio Lite/Pro emits — XML with the
// fixed "http://www.msefi.com/:msq" namespace, ISO-8859-1 encoding,
// <bibliography>/<versionInfo>/<page>/<settings>/<userComments> blocks.
#include "MsqParser.h"

#include "ECUDefinition.h"
#include "ECUSettingsManager.h"
#include "../utils/Logger.h"

#include <QDateTime>
#include <QFile>
#include <QSaveFile>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

MsqParser::MsqParser() {}

// ---------------------------------------------------------------------------
// load() — also populates m_metadata + m_pcVariableNames for round-tripping.
// ---------------------------------------------------------------------------
bool MsqParser::load(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Could not open file %1").arg(filePath);
        Logger::error("MsqParser: " + m_lastError);
        return false;
    }

    m_constants.clear();
    m_pcVariableNames.clear();
    m_metadata = MsqMetadata();

    QXmlStreamReader xml(&file);
    bool inConstant = false;
    bool isPcVar    = false;
    QString currentName;
    QString currentData;
    bool inSettings = false;
    bool inUserComments = false;

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartElement) {
            const QString tag = xml.name().toString();
            const auto attrs = xml.attributes();

            if (tag == "bibliography") {
                m_metadata.author      = attrs.value("author").toString();
                m_metadata.tuneComment = attrs.value("tuneComment").toString();
                m_metadata.writeDate   = attrs.value("writeDate").toString();
            } else if (tag == "versionInfo") {
                if (attrs.hasAttribute("fileFormat"))
                    m_metadata.fileFormat = attrs.value("fileFormat").toString();
                m_metadata.firmwareInfo = attrs.value("firmwareInfo").toString();
                m_metadata.signature    = attrs.value("signature").toString();
                if (attrs.hasAttribute("nPages"))
                    m_metadata.nPages = attrs.value("nPages").toInt();
            } else if (tag == "constant" || tag == "pcVariable") {
                inConstant   = true;
                isPcVar      = (tag == "pcVariable");
                currentName  = attrs.value("name").toString();
                currentData.clear();
            } else if (tag == "settings") {
                inSettings = true;
            } else if (tag == "setting" && inSettings) {
                const QString nm = attrs.value("name").toString();
                if (!nm.isEmpty()) m_metadata.settingsBlock.append(nm);
            } else if (tag == "userComments") {
                inUserComments = true;
                m_metadata.userComments = attrs.value("Comment").toString();
            }
        } else if (token == QXmlStreamReader::Characters) {
            if (inConstant) currentData += xml.text().toString();
        } else if (token == QXmlStreamReader::EndElement) {
            const QString tag = xml.name().toString();
            if ((tag == "constant" || tag == "pcVariable") && inConstant) {
                currentData = currentData.trimmed();
                if (!currentName.isEmpty()) {
                    if (currentData.startsWith('"') && currentData.endsWith('"')) {
                        currentData = currentData.mid(1, currentData.length() - 2);
                    }
                    m_constants[currentName] = currentData;
                    if (isPcVar) m_pcVariableNames.insert(currentName);
                }
                inConstant = false;
                isPcVar    = false;
            } else if (tag == "settings") {
                inSettings = false;
            } else if (tag == "userComments") {
                inUserComments = false;
            }
        }
    }

    if (xml.hasError()) {
        m_lastError = QString("XML error: %1").arg(xml.errorString());
        Logger::error("MsqParser: " + m_lastError);
        return false;
    }

    Logger::info(QString("MsqParser: Loaded %1 constants + %2 pcVariables; "
                         "fileFormat=%3, signature=%4")
                     .arg(m_constants.size())
                     .arg(m_pcVariableNames.size())
                     .arg(m_metadata.fileFormat)
                     .arg(m_metadata.signature));
    return true;
}

// ---------------------------------------------------------------------------
// Java Date.toString()-style formatter — matches "Fri Dec 26 11:50:13 PKT 2025"
// because TunerStudio expects that exact format in <bibliography writeDate>.
// ---------------------------------------------------------------------------
QString MsqParser::formatJavaDate(const QDateTime &dt, bool utc) {
    const QDateTime use = utc ? dt.toUTC() : dt;
    // Qt's "ddd MMM d HH:mm:ss t yyyy" gives "Fri Dec 26 11:50:13 +05:00 2025";
    // we manually substitute the timezone abbreviation when available because
    // TS prefers e.g. "PKT" / "UTC" / "EST" over "+05:00".
    const QString core = use.toString("ddd MMM d HH:mm:ss ");
    QString tz = utc ? QStringLiteral("UTC") : use.timeZoneAbbreviation();
    if (tz.isEmpty()) tz = use.toString("t");
    return core + tz + use.toString(" yyyy");
}

// ---------------------------------------------------------------------------
// save() — emit the current page-cache state as a TS-compatible MSQ.
// ---------------------------------------------------------------------------
bool MsqParser::save(const QString &filePath,
                     const ECUDefinition *definition,
                     const ECUSettingsManager *settings,
                     const MsqSaveOptions &options) {
    if (!definition) {
        m_lastError = "save(): no ECUDefinition provided.";
        return false;
    }
    if (!settings) {
        m_lastError = "save(): no ECUSettingsManager provided.";
        return false;
    }

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        m_lastError = QString("Could not open file %1 for writing")
            .arg(filePath);
        return false;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(options.prettyPrint);
    xml.setAutoFormattingIndent(2);
    // Qt 6 removed QXmlStreamWriter::setCodec — UTF-8 is the default and TS
    // accepts UTF-8 .msq files (it sniffs the declaration). For round-trip
    // fidelity with legacy ISO-8859-1 files we rely on QXmlStreamWriter's
    // automatic numeric-entity escaping of high-byte characters.

    xml.writeStartDocument("1.0");
    xml.writeStartElement("msq");
    xml.writeDefaultNamespace("http://www.msefi.com/:msq");

    // ---- <bibliography> ----
    const QString authorLine = options.tunerStudioVersion.isEmpty()
        ? options.author
        : (options.author + " " + options.tunerStudioVersion);
    xml.writeStartElement("bibliography");
    xml.writeAttribute("author", authorLine);
    xml.writeAttribute("tuneComment",
                       options.tuneComment.isEmpty()
                           ? m_metadata.tuneComment
                           : options.tuneComment);
    xml.writeAttribute("writeDate",
                       formatJavaDate(QDateTime::currentDateTime(),
                                      options.useUtcWriteDate));
    xml.writeEndElement();

    // ---- <versionInfo> ----
    const QString fileFormat = options.fileFormat.isEmpty()
        ? (m_metadata.fileFormat.isEmpty() ? QStringLiteral("5.0")
                                           : m_metadata.fileFormat)
        : options.fileFormat;
    const QString signature = options.signature.isEmpty()
        ? definition->getSignature()
        : options.signature;
    xml.writeStartElement("versionInfo");
    xml.writeAttribute("fileFormat", fileFormat);
    xml.writeAttribute("firmwareInfo",
                       options.firmwareInfo.isEmpty()
                           ? m_metadata.firmwareInfo
                           : options.firmwareInfo);
    // nPages: count distinct page IDs across constants.
    QSet<int> pageSet;
    for (const QString& nm : settings->getAllConstantNames()) {
        const auto def = settings->getDefinition(nm);
        if (def.page > 0) pageSet.insert(def.page);
    }
    xml.writeAttribute("nPages", QString::number(pageSet.size()));
    xml.writeAttribute("signature", signature);
    xml.writeEndElement();

    // ---- pcVariables <page> ----
    xml.writeStartElement("page");
    for (const QString& nm : settings->getPcVariableNames()) {
        xml.writeStartElement("pcVariable");
        xml.writeAttribute("name", nm);
        const QVariant v = settings->getValue(nm);
        xml.writeCharacters(v.toString());
        xml.writeEndElement();
    }
    xml.writeEndElement();

    // ---- per-page <constant> blocks ----
    QList<int> pages = QList<int>(pageSet.begin(), pageSet.end());
    std::sort(pages.begin(), pages.end());

    QMap<int, QStringList> byPage;
    for (const QString& nm : settings->getAllConstantNames()) {
        if (settings->isPcVariable(nm)) continue;
        const auto def = settings->getDefinition(nm);
        if (def.noMsqSave) continue;
        if (def.page > 0) byPage[def.page].append(nm);
    }

    for (int pageId : pages) {
        const QStringList& names = byPage.value(pageId);
        if (names.isEmpty() && !options.includeUnusedPages) continue;

        xml.writeStartElement("page");
        xml.writeAttribute("number", QString::number(pageId));
        // size is best-effort — fall back to 256 when unknown.
        xml.writeAttribute("size", QString::number(256));

        for (const QString& nm : names) {
            const auto def = settings->getDefinition(nm);
            xml.writeStartElement("constant");
            xml.writeAttribute("name", nm);
            if (!def.units.isEmpty())   xml.writeAttribute("units", def.units);
            if (def.cols > 1 || def.rows > 1) {
                xml.writeAttribute("cols", QString::number(def.cols));
                xml.writeAttribute("rows", QString::number(def.rows));
            }
            if (def.digits >= 0) xml.writeAttribute("digits", QString::number(def.digits));

            if (def.paramClass == "array" && (def.rows > 1 || def.cols > 1)) {
                // Table-shaped array: emit the underlying table data.
                const auto data = settings->getTableData(nm);
                QString body;
                for (const auto& row : data) {
                    for (double v : row) body += QString::number(v, 'f', def.digits > 0 ? def.digits : 0) + " ";
                    body += "\n";
                }
                xml.writeCharacters("\n" + body);
            } else if (def.paramClass == "bits" && !def.bitField.options.isEmpty()) {
                const int raw = settings->getRawValue(nm);
                if (raw >= 0 && raw < def.bitField.options.size())
                    xml.writeCharacters("\"" + def.bitField.options.at(raw) + "\"");
                else
                    xml.writeCharacters(QString::number(raw));
            } else {
                const QVariant v = settings->getValue(nm);
                xml.writeCharacters(v.toString());
            }
            xml.writeEndElement(); // constant
        }
        xml.writeEndElement(); // page
    }

    // ---- <settings> block ----
    if (options.includeSettingsBlock) {
        xml.writeStartElement("settings");
        xml.writeAttribute("Comment",
                           "These setting are only used if this msq is opened without a project.");
        const auto flags = settings->getActiveConditionalFlags();
        for (auto it = flags.constBegin(); it != flags.constEnd(); ++it) {
            if (!it.value()) continue;
            xml.writeStartElement("setting");
            xml.writeAttribute("name",  it.key());
            xml.writeAttribute("value", it.key());
            xml.writeEndElement();
        }
        xml.writeEndElement();
    }

    // ---- <userComments> ----
    xml.writeStartElement("userComments");
    xml.writeAttribute("Comment", m_metadata.userComments);
    xml.writeEndElement();

    xml.writeEndElement();   // msq
    xml.writeEndDocument();

    if (!file.commit()) {
        m_lastError = QString("Failed to commit file: %1").arg(file.errorString());
        return false;
    }

    Logger::info(QString("MsqParser: Saved tune to %1 (%2 pages)").arg(filePath).arg(pages.size()));
    return true;
}
