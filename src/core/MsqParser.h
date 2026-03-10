#ifndef MSQPARSER_H
#define MSQPARSER_H

#include <QString>
#include <QMap>
#include <QStringList>

class MsqParser {
public:
    MsqParser();
    
    // Returns true if parsing is successful
    bool load(const QString &filePath);
    
    // name -> text value
    const QMap<QString, QString> &getConstants() const { return m_constants; }
    
private:
    QMap<QString, QString> m_constants;
};

#endif // MSQPARSER_H
