#include "MsqParser.h"
#include "../utils/Logger.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>

MsqParser::MsqParser() {
}

bool MsqParser::load(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::error("MsqParser: Could not open file " + filePath);
        return false;
    }
    
    QXmlStreamReader xml(&file);
    bool inConstant = false;
    QString currentName;
    QString currentData;
    
    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        
        if (token == QXmlStreamReader::StartElement) {
            QString tagName = xml.name().toString();
            
            if (tagName == "constant" || tagName == "pcVariable") {
                inConstant = true;
                currentName = xml.attributes().value("name").toString();
                currentData.clear();
            }
        } else if (token == QXmlStreamReader::Characters) {
            if (inConstant) {
                currentData += xml.text().toString();
            }
        } else if (token == QXmlStreamReader::EndElement) {
            QString tagName = xml.name().toString();
            
            if ((tagName == "constant" || tagName == "pcVariable") && inConstant) {
                // Store in map
                currentData = currentData.trimmed();
                if (!currentName.isEmpty()) {
                    // Remove quotes from strings like "Rising Edge"
                    if (currentData.startsWith('"') && currentData.endsWith('"')) {
                        currentData = currentData.mid(1, currentData.length() - 2);
                    }
                    m_constants[currentName] = currentData;
                }
                inConstant = false;
            }
        }
    }
    
    if (xml.hasError()) {
        Logger::error("MsqParser: XML error: " + xml.errorString());
        return false;
    }
    
    Logger::info(QString("MsqParser: Loaded %1 constants from MSQ").arg(m_constants.size()));
    return true;
}
