
#include "ECUDefinition.h"
#include "ECUData.h"
#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

ECUDefinition::ECUDefinition() {
    // Initialize with default condition
    m_conditionalStack.push(true);
}

bool ECUDefinition::load(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open ECU definition file:" << filePath;
        return false;
    }

    int currentPageId = 1;
    QTextStream in(&file);
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        QString trimmedLine = line.trimmed();
        
        // Skip empty lines and comments
        if (trimmedLine.isEmpty() || trimmedLine.startsWith(";")) {
            continue;
        }
        
        // Process directives (#if, #set, #define, etc.)
        if (trimmedLine.startsWith("#")) {
            processDirective(trimmedLine);
            continue;
        }
        
        // Skip if inside a false conditional block
        if (shouldSkipLine()) {
            continue;
        }
        
        // Expand #define macros
        QString expandedLine = expandDefines(trimmedLine);

        // Parse section headers
        if (expandedLine.compare("[TunerStudio]", Qt::CaseInsensitive) == 0) {
            parseTunerStudio(in);
        } else if (expandedLine.compare("[OutputChannels]", Qt::CaseInsensitive) == 0) {
            parseOutputChannels(in);
        } else if (expandedLine.compare("[Constants]", Qt::CaseInsensitive) == 0) {
            parseConstants(in, currentPageId);
        } else if (expandedLine.compare("[ControllerCommands]", Qt::CaseInsensitive) == 0) {
            parseControllerCommands(in);
        } else if (expandedLine.compare("[TableEditor]", Qt::CaseInsensitive) == 0) {
            parseTableEditor(in);
        } else if (expandedLine.startsWith("page", Qt::CaseInsensitive)) {
            // Parse page = N
            QRegularExpression pageRegex(R"(page\s*=\s*(\d+))");
            QRegularExpressionMatch match = pageRegex.match(expandedLine);
            if (match.hasMatch()) {
                currentPageId = match.captured(1).toInt();
            }
        }
    }

    return true;
}

bool ECUDefinition::processDirective(const QString &line) {
    QString directive = line.trimmed();
    
    // #set CONDITION
    if (directive.startsWith("#set ")) {
        QString name = directive.mid(5).trimmed();
        m_conditions[name] = true;
        return true;
    }
    
    // #unset CONDITION
    if (directive.startsWith("#unset ")) {
        QString name = directive.mid(7).trimmed();
        m_conditions[name] = false;
        return true;
    }
    
    // #define NAME = value
    if (directive.startsWith("#define ")) {
        QString rest = directive.mid(8).trimmed();
        int eqPos = rest.indexOf('=');
        if (eqPos > 0) {
            QString name = rest.left(eqPos).trimmed();
            QString value = rest.mid(eqPos + 1).trimmed();
            m_defines[name] = value;
        } else {
            // Simple #define NAME value (space separated)
            int spacePos = rest.indexOf(' ');
            if (spacePos > 0) {
                QString name = rest.left(spacePos).trimmed();
                QString value = rest.mid(spacePos + 1).trimmed();
                m_defines[name] = value;
            }
        }
        return true;
    }
    
    // #if CONDITION
    if (directive.startsWith("#if ")) {
        QString condName = directive.mid(4).trimmed();
        bool condValue = m_conditions.value(condName, false);
        m_conditionalStack.push(condValue && m_conditionalStack.top());
        return true;
    }
    
    // #elif CONDITION
    // [CRIT-6] Must AND with parent context, same as #if
    if (directive.startsWith("#elif ")) {
        if (!m_conditionalStack.isEmpty()) {
            m_conditionalStack.pop();
        }
        QString condName = directive.mid(6).trimmed();
        bool condValue = m_conditions.value(condName, false);
        bool parentActive = m_conditionalStack.isEmpty() ? true : m_conditionalStack.top();
        m_conditionalStack.push(condValue && parentActive);
        return true;
    }
    
    // #else
    if (directive == "#else") {
        if (!m_conditionalStack.isEmpty()) {
            bool current = m_conditionalStack.pop();
            m_conditionalStack.push(!current);
        }
        return true;
    }
    
    // #endif
    if (directive == "#endif") {
        if (m_conditionalStack.size() > 1) {
            m_conditionalStack.pop();
        }
        return true;
    }
    
    // #include (simplified - just note it)
    if (directive.startsWith("#include ")) {
        qDebug() << "Include directive found:" << directive.mid(9);
        return true;
    }
    
    return false;
}

bool ECUDefinition::shouldSkipLine() const {
    return !m_conditionalStack.isEmpty() && !m_conditionalStack.top();
}

QString ECUDefinition::expandDefines(const QString &line) {
    QString result = line;
    for (auto it = m_defines.constBegin(); it != m_defines.constEnd(); ++it) {
        // Expand $DEFINE_NAME or just DEFINE_NAME
        result.replace("$" + it.key(), it.value());
    }
    return result;
}

void ECUDefinition::parseTunerStudio(QTextStream &in) {
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        if (line.isEmpty() || line.startsWith(";")) continue;
        if (line.startsWith("[")) return; // Next section — main loop handles it
        if (line.startsWith("#")) {
            processDirective(line);
            continue;
        }
        if (shouldSkipLine()) continue;
        
        // Parse signature
        QRegularExpression sigRegex(R"(signature\s*=\s*\"([^\"]+)\")");
        QRegularExpressionMatch match = sigRegex.match(line);
        if (match.hasMatch()) {
            m_signature = match.captured(1);
        }
    }
}

void ECUDefinition::parseOutputChannels(QTextStream &in) {
    // Regex for scalar: name = scalar, type, offset, "units", scale, translate
    QRegularExpression scalarRegex(
        R"(^(\w+)\s*=\s*scalar\s*,\s*(\w+)\s*,\s*(\d+)\s*,\s*\"([^\"]*)\"\s*,\s*([\d\.-]+)\s*,\s*([\d\.-]+))");
    
    // Regex for bits with bracket notation: name = bits, type, offset, [low:high]
    QRegularExpression bitsRegex(
        R"(^(\w+)\s*=\s*bits\s*,\s*(\w+)\s*,\s*(\d+)\s*,\s*\[(\d+):(\d+)(\+\d+)?\])");
    
    // [SER-2] Regex for bits with simple comma-separated format:
    //   name = bits, type, offset, "label", scale, translate
    QRegularExpression bitsSimpleRegex(
        R"(^(\w+)\s*=\s*bits\s*,\s*(\w+)\s*,\s*(\d+)\s*,\s*\"([^\"]*)\"\s*,\s*([\d\.-]+)\s*,\s*([\d\.-]+))");
    
    // Regex for expression: name = { expression }, "units"
    QRegularExpression exprRegex(
        R"(^(\w+)\s*=\s*\{([^}]+)\}\s*,\s*\"([^\"]*)\")");

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        if (line.isEmpty() || line.startsWith(";")) continue;
        if (line.startsWith("[")) return; // Next section — main loop handles it
        if (line.startsWith("#")) {
            processDirective(line);
            continue;
        }
        if (shouldSkipLine()) continue;
        
        line = expandDefines(line);

        // Try scalar match
        QRegularExpressionMatch match = scalarRegex.match(line);
        if (match.hasMatch()) {
            OutputChannel channel;
            channel.name = match.captured(1);
            channel.type = match.captured(2);
            channel.offset = match.captured(3).toInt();
            channel.units = match.captured(4);
            channel.scale = match.captured(5).toDouble();
            channel.translate = match.captured(6).toDouble();
            channel.isBits = false;
            
            m_outputChannels.insert(channel.name, channel);
            
            int typeSize = getTypeSize(channel.type);
            int end = channel.offset + typeSize;
            if (end > m_outputChannelsSize) m_outputChannelsSize = end;
            continue;
        }
        
        // Try bits match (bracket notation: [low:high])
        match = bitsRegex.match(line);
        if (match.hasMatch()) {
            OutputChannel channel;
            channel.name = match.captured(1);
            channel.type = match.captured(2);
            channel.offset = match.captured(3).toInt();
            channel.isBits = true;
            channel.bitField.lowBit = match.captured(4).toInt();
            channel.bitField.highBit = match.captured(5).toInt();
            channel.bitField.offset = 0;
            if (match.capturedLength(6) > 0) {
                channel.bitField.offset = match.captured(6).mid(1).toInt();
            }
            
            m_outputChannels.insert(channel.name, channel);
            
            int typeSize = getTypeSize(channel.type);
            int end = channel.offset + typeSize;
            if (end > m_outputChannelsSize) m_outputChannelsSize = end;
            continue;
        }
        
        // [SER-2] Try simple bits match (comma-separated: "label", scale, translate)
        match = bitsSimpleRegex.match(line);
        if (match.hasMatch()) {
            OutputChannel channel;
            channel.name = match.captured(1);
            channel.type = match.captured(2);
            channel.offset = match.captured(3).toInt();
            channel.units = match.captured(4);
            channel.scale = match.captured(5).toDouble();
            channel.translate = match.captured(6).toDouble();
            channel.isBits = true;
            // Default: entire byte is the bit field
            channel.bitField.lowBit = 0;
            channel.bitField.highBit = (getTypeSize(channel.type) * 8) - 1;
            channel.bitField.offset = 0;
            
            m_outputChannels.insert(channel.name, channel);
            
            int typeSize = getTypeSize(channel.type);
            int end = channel.offset + typeSize;
            if (end > m_outputChannelsSize) m_outputChannelsSize = end;
            continue;
        }
        
        // Try expression match
        match = exprRegex.match(line);
        if (match.hasMatch()) {
            OutputChannel channel;
            channel.name = match.captured(1);
            channel.expression = match.captured(2).trimmed();
            channel.units = match.captured(3);
            channel.type = "EXPR";
            
            m_outputChannels.insert(channel.name, channel);
            continue;
        }
    }
}

void ECUDefinition::parseConstants(QTextStream &in, int pageId) {
    // Regex for scalar constant
    QRegularExpression scalarRegex(
        R"(^(\w+)\s*=\s*scalar\s*,\s*(\w+)\s*,\s*(\w+)\s*,\s*\"([^\"]*)\"\s*,\s*([\d\.\-e]+)\s*,\s*([\d\.\-e]+)\s*,\s*([\d\.\-e]+)\s*,\s*([\d\.\-e]+)\s*,\s*(\d+))");
    
    // Regex for bits constant with options
    QRegularExpression bitsRegex(
        R"(^(\w+)\s*=\s*bits\s*,\s*(\w+)\s*,\s*(\w+)\s*,\s*\[(\d+):(\d+)(\+\d+)?\](.*)$)");
    
    // Regex for array constant
    QRegularExpression arrayRegex(
        R"(^(\w+)\s*=\s*array\s*,\s*(\w+)\s*,\s*(\w+)\s*,\s*\[(\d+)(?:x(\d+))?\])");

    int nextOffset = 0;
    int lastOffset = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        if (line.isEmpty() || line.startsWith(";")) continue;
        if (line.startsWith("[")) return; // Next section — main loop handles it
        if (line.startsWith("#")) {
            processDirective(line);
            continue;
        }
        if (shouldSkipLine()) continue;
        
        line = expandDefines(line);
        
        // Handle page directive
        if (line.startsWith("page", Qt::CaseInsensitive)) {
            QRegularExpression pageRegex(R"(page\s*=\s*(\d+))");
            QRegularExpressionMatch match = pageRegex.match(line);
            if (match.hasMatch()) {
                pageId = match.captured(1).toInt();
                nextOffset = 0;
                lastOffset = 0;
            }
            continue;
        }
        
        // [CRIT-5] Replace offset keywords using word-boundary-safe regex
        //   Prevents corruption of names like "injAngleOffset" that contain "Offset"
        {
            static QRegularExpression rxNext(QStringLiteral("\\bnextOffset\\b"));
            static QRegularExpression rxLast(QStringLiteral("\\blastOffset\\b"));
            line.replace(rxNext, QString::number(nextOffset));
            line.replace(rxLast, QString::number(lastOffset));
        }

        // Try scalar match
        QRegularExpressionMatch match = scalarRegex.match(line);
        if (match.hasMatch()) {
            Constant constant;
            constant.name = match.captured(1);
            constant.paramClass = "scalar";
            constant.type = match.captured(2);
            constant.offset = match.captured(3).toInt();
            constant.units = match.captured(4);
            constant.scale = match.captured(5).toDouble();
            constant.translate = match.captured(6).toDouble();
            constant.min = match.captured(7).toDouble();
            constant.max = match.captured(8).toDouble();
            constant.digits = match.captured(9).toInt();
            
            // Check for flags at end
            if (line.contains("noMsqSave")) constant.noMsqSave = true;
            if (line.contains("controllerPriority")) constant.controllerPriority = true;
            
            m_constants.insert(constant.name, constant);
            
            lastOffset = constant.offset;
            nextOffset = constant.offset + getTypeSize(constant.type);
            continue;
        }
        
        // Try bits match
        match = bitsRegex.match(line);
        if (match.hasMatch()) {
            Constant constant;
            constant.name = match.captured(1);
            constant.paramClass = "bits";
            constant.type = match.captured(2);
            constant.offset = match.captured(3).toInt();
            constant.bitField.lowBit = match.captured(4).toInt();
            constant.bitField.highBit = match.captured(5).toInt();
            
            // Parse offset modifier (+1)
            if (match.capturedLength(6) > 0) {
                constant.bitField.offset = match.captured(6).mid(1).toInt();
            }
            
            // Parse options from rest of line
            QString optionsStr = match.captured(7).trimmed();
            if (!optionsStr.isEmpty()) {
                QRegularExpression optRegex(R"(\"([^\"]*)\")");
                QRegularExpressionMatchIterator it = optRegex.globalMatch(optionsStr);
                while (it.hasNext()) {
                    QRegularExpressionMatch optMatch = it.next();
                    QString option = optMatch.captured(1);
                    constant.bitField.options.append(option);
                    
                    // Track valid values (non-INVALID)
                    if (option.compare("INVALID", Qt::CaseInsensitive) != 0) {
                        constant.bitField.validValues.append(constant.bitField.options.size() - 1);
                    }
                }
            }
            
            m_constants.insert(constant.name, constant);
            
            lastOffset = constant.offset;
            nextOffset = constant.offset + getTypeSize(constant.type);
            continue;
        }
        
        // Try array match
        match = arrayRegex.match(line);
        if (match.hasMatch()) {
            Constant constant;
            constant.name = match.captured(1);
            constant.paramClass = "array";
            constant.type = match.captured(2);
            constant.offset = match.captured(3).toInt();
            constant.cols = match.captured(4).toInt();
            constant.rows = match.capturedLength(5) > 0 ? match.captured(5).toInt() : 1;
            
            m_constants.insert(constant.name, constant);
            
            lastOffset = constant.offset;
            nextOffset = constant.offset + getTypeSize(constant.type) * constant.cols * constant.rows;
            continue;
        }
    }
}

void ECUDefinition::parseControllerCommands(QTextStream &in) {
    // Format: commandName = "cmd1", "cmd2", ...
    // Or: commandName = otherCmd1, otherCmd2
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        if (line.isEmpty() || line.startsWith(";")) continue;
        if (line.startsWith("[")) return; // Next section — main loop handles it
        if (line.startsWith("#")) {
            processDirective(line);
            continue;
        }
        if (shouldSkipLine()) continue;
        
        line = expandDefines(line);
        
        int eqPos = line.indexOf('=');
        if (eqPos > 0) {
            QString name = line.left(eqPos).trimmed();
            QString value = line.mid(eqPos + 1).trimmed();
            
            ControllerCommand cmd;
            cmd.name = name;
            
            // Parse command chain
            QStringList parts = value.split(',');
            for (const QString &part : parts) {
                QString p = part.trimmed();
                if (p.startsWith("\"") && p.endsWith("\"")) {
                    // String command - parse bytes
                    cmd.commands.append(parseCommandBytes(p.mid(1, p.length() - 2)));
                } else {
                    // Reference to another command
                    if (m_controllerCommands.contains(p)) {
                        cmd.commands.append(m_controllerCommands[p].commands);
                    }
                }
            }
            
            m_controllerCommands.insert(name, cmd);
        }
    }
}

QByteArray ECUDefinition::parseCommandBytes(const QString &cmdStr) {
    QByteArray result;
    int i = 0;
    
    while (i < cmdStr.length()) {
        if (cmdStr[i] == '\\') {
            i++;
            if (i >= cmdStr.length()) break;
            
            if (cmdStr[i] == 'x' || cmdStr[i] == 'X') {
                // Hex byte \xNN
                i++;
                QString hex = cmdStr.mid(i, 2);
                bool ok;
                int byte = hex.toInt(&ok, 16);
                if (ok) {
                    result.append(static_cast<char>(byte));
                }
                i += 2;
            } else if (cmdStr[i] == '$') {
                // Variable substitution \$varName
                i++;
                QString varName;
                while (i < cmdStr.length() && (cmdStr[i].isLetterOrNumber() || cmdStr[i] == '_')) {
                    varName += cmdStr[i++];
                }
                // Lookup variable value (would need variable resolver)
                // For now, just append 0
                result.append('\0');
            } else if (cmdStr[i] == 'n') {
                result.append('\n');
                i++;
            } else if (cmdStr[i] == 'r') {
                result.append('\r');
                i++;
            } else if (cmdStr[i] == 't') {
                result.append('\t');
                i++;
            } else {
                result.append(cmdStr[i].toLatin1());
                i++;
            }
        } else {
            result.append(cmdStr[i].toLatin1());
            i++;
        }
    }
    
    return result;
}

ECUDefinition::BitField ECUDefinition::parseBitField(const QString &shapeStr, const QStringList &options) {
    BitField bf;
    bf.lowBit = 0;
    bf.highBit = 0;
    bf.offset = 0;
    bf.options = options;
    
    // Parse [lowBit:highBit] or [lowBit:highBit+offset]
    QRegularExpression regex(R"(\[(\d+):(\d+)(\+\d+)?\])");
    QRegularExpressionMatch match = regex.match(shapeStr);
    if (match.hasMatch()) {
        bf.lowBit = match.captured(1).toInt();
        bf.highBit = match.captured(2).toInt();
        if (match.capturedLength(3) > 0) {
            bf.offset = match.captured(3).mid(1).toInt();
        }
    }
    
    // Build valid values list (indices of non-INVALID options)
    for (int i = 0; i < options.size(); i++) {
        if (options[i].compare("INVALID", Qt::CaseInsensitive) != 0) {
            bf.validValues.append(i);
        }
    }
    
    return bf;
}

int ECUDefinition::getTypeSize(const QString &type) const {
    if (type == "U08" || type == "S08") return 1;
    if (type == "U16" || type == "S16") return 2;
    if (type == "U32" || type == "S32" || type == "F32") return 4;
    return 1;
}

bool ECUDefinition::isConditionActive(const QString& name) const {
    return m_conditions.value(name, false);
}

const QMap<QString, ECUDefinition::OutputChannel> &ECUDefinition::getOutputChannels() const {
    return m_outputChannels;
}

const QMap<QString, ECUDefinition::Constant> &ECUDefinition::getConstants() const {
    return m_constants;
}

const QMap<QString, ECUDefinition::ControllerCommand> &ECUDefinition::getControllerCommands() const {
    return m_controllerCommands;
}

const QMap<int, ECUDefinition::Page> &ECUDefinition::getPages() const {
    return m_pages;
}

// Helper macro to create Constant entries more concisely
#define MAKE_CONSTANT(n, p, o, t, sc, tr, mn, mx, u, cat) \
    { n, ECUDefinition::Constant{ n, "scalar", t, p, o, u, sc, tr, mn, mx, 2, cat, {}, 1, 1, false, false, false } }

QMap<QString, ECUDefinition::Constant> ECUDefinition::getDefaultSpeeduinoConstants() {
    QMap<QString, Constant> constants;
    
    // Engine Settings (Page 5)
    auto addConst = [&](const QString &name, int page, int offset, const QString &type, 
                        double scale, double translate, double min, double max,
                        const QString &units, const QString &category) {
        Constant c;
        c.name = name;
        c.paramClass = "scalar";
        c.type = type;
        c.page = page;
        c.offset = offset;
        c.units = units;
        c.scale = scale;
        c.translate = translate;
        c.min = min;
        c.max = max;
        c.digits = 2;
        c.category = category;
        constants.insert(name, c);
    };
    
    auto addBits = [&](const QString &name, int page, int offset, int lowBit, int highBit,
                       double min, double max, const QString &category) {
        Constant c;
        c.name = name;
        c.paramClass = "bits";
        c.type = "U08";
        c.page = page;
        c.offset = offset;
        c.category = category;
        c.min = min;
        c.max = max;
        c.bitField.lowBit = lowBit;
        c.bitField.highBit = highBit;
        constants.insert(name, c);
    };
    
    // ENGINE SETTINGS (Page 5)
    addConst("nCylinders",     5,   0, "U08", 1.0, 0.0, 1, 16, "", "Engine");
    addBits ("engineType",     5,   1, 0, 2, 0, 7, "Engine");
    addConst("nInjectors",     5,   2, "U08", 1.0, 0.0, 1, 16, "", "Engine");
    addBits ("injLayout",      5,   3, 0, 1, 0, 3, "Engine");
    addConst("stoich",         5,   4, "U08", 0.1, 0.0, 9.0, 25.5, "AFR", "Engine");
    addConst("reqFuel",        5,   5, "U16", 0.1, 0.0, 0.1, 25.5, "ms", "Engine");
    addConst("engineCc",       5,   7, "U16", 1.0, 0.0, 100, 10000, "cc", "Engine");
    addConst("injectorCc",     5,   9, "U16", 1.0, 0.0, 10, 2000, "cc/min", "Engine");
    addBits ("algorithm",      5,  11, 0, 2, 0, 7, "Engine");
    addBits ("loadSource",     5,  12, 0, 1, 0, 3, "Engine");
    addBits ("baroCorr",       5,  13, 0, 0, 0, 1, "Engine");
    addBits ("triggerType",    5,  14, 0, 4, 0, 31, "Engine");
    addConst("triggerWheel",   5,  15, "U08", 1.0, 0.0, 1, 255, "teeth", "Engine");
    addConst("triggerMissing", 5,  16, "U08", 1.0, 0.0, 0, 10, "teeth", "Engine");
    addConst("triggerAngle",   5,  17, "S16", 1.0, 0.0, -360, 360, "°", "Engine");
    
    // INJECTION SETTINGS (Page 6)
    addBits ("injMode",        6,   0, 0, 1, 0, 3, "Injection");
    addBits ("injTiming",      6,   1, 0, 0, 0, 1, "Injection");
    addConst("injAngle",       6,   2, "U16", 1.0, 0.0, 0, 720, "°", "Injection");
    addConst("injAngleOffset", 6,   4, "S16", 1.0, 0.0, -180, 180, "°", "Injection");
    addBits ("stagingEnabled", 6,   6, 0, 0, 0, 1, "Injection");
    addConst("stagingRpm",     6,   7, "U16", 1.0, 0.0, 0, 15000, "RPM", "Injection");
    addConst("stagingTps",     6,   9, "U08", 1.0, 0.0, 0, 100, "%", "Injection");
    addConst("stagingInjSize", 6,  10, "U16", 1.0, 0.0, 0, 2000, "cc/min", "Injection");
    
    // IGNITION SETTINGS (Page 7)
    addBits ("sparkMode",      7,   0, 0, 2, 0, 7, "Ignition");
    addBits ("sparkEdge",      7,   1, 0, 0, 0, 1, "Ignition");
    addConst("fixedDwell",     7,   2, "U08", 0.1, 0.0, 0.5, 10.0, "ms", "Ignition");
    addBits ("useDwellTable",  7,   3, 0, 0, 0, 1, "Ignition");
    addConst("dwellRunning",   7,   4, "U08", 0.1, 0.0, 0.5, 10.0, "ms", "Ignition");
    addConst("dwellCranking",  7,   5, "U08", 0.1, 0.0, 0.5, 20.0, "ms", "Ignition");
    
    // LIMITER SETTINGS (Page 8)
    addBits ("revLimMode",     8,   0, 0, 1, 0, 3, "Limiters");
    addConst("hardRevLim",     8,   1, "U16", 1.0, 0.0, 1000, 20000, "RPM", "Limiters");
    addConst("softRevLim",     8,   3, "U16", 1.0, 0.0, 1000, 20000, "RPM", "Limiters");
    addConst("softLimRetard",  8,   5, "U08", 1.0, 0.0, 0, 45, "°", "Limiters");
    addConst("revLimHyst",     8,   6, "U08", 10.0, 0.0, 0, 500, "RPM", "Limiters");
    addConst("fuelCutPct",     8,   7, "U08", 1.0, 0.0, 0, 100, "%", "Limiters");
    addBits ("launchEnabled",  8,   8, 0, 0, 0, 1, "Limiters");
    addConst("launchRpm",      8,   9, "U16", 1.0, 0.0, 1000, 10000, "RPM", "Limiters");
    addConst("launchRetard",   8,  11, "U08", 1.0, 0.0, 0, 45, "°", "Limiters");
    addBits ("boostCutEnable", 8,  12, 0, 0, 0, 1, "Limiters");
    addConst("boostCutPsi",    8,  13, "U08", 0.5, 0.0, 0, 50, "psi", "Limiters");
    
    // ENGINE STATE (Page 8)
    addConst("crankingRpm",    8,  20, "U16", 1.0, 0.0, 100, 1000, "RPM", "EngineState");
    addConst("floodClearTps",  8,  22, "U08", 1.0, 0.0, 50, 100, "%", "EngineState");
    addConst("aseHold",        8,  23, "U08", 1.0, 0.0, 0, 60, "sec", "EngineState");
    addConst("aseDecay",       8,  24, "U08", 0.1, 0.0, 0, 10, "%/sec", "EngineState");
    addBits ("dfcoEnabled",    8,  25, 0, 0, 0, 1, "EngineState");
    addConst("dfcoRpm",        8,  26, "U16", 1.0, 0.0, 1000, 5000, "RPM", "EngineState");
    addConst("dfcoTps",        8,  28, "U08", 1.0, 0.0, 0, 20, "%", "EngineState");
    
    // FAN CONTROL (Page 8)
    addBits ("fanEnabled",     8,  40, 0, 0, 0, 1, "FanControl");
    addBits ("fanOutput",      8,  41, 0, 2, 0, 7, "FanControl");
    addConst("fanOnTemp",      8,  42, "U08", 1.0, -40.0, 50, 120, "°C", "FanControl");
    addConst("fanOffTemp",     8,  43, "U08", 1.0, -40.0, 50, 120, "°C", "FanControl");
    addConst("fanHyst",        8,  44, "U08", 1.0, 0.0, 1, 20, "°C", "FanControl");
    addBits ("fanInvert",      8,  45, 0, 0, 0, 1, "FanControl");
    addBits ("fanAcRequest",   8,  46, 0, 0, 0, 1, "FanControl");
    addConst("fanMinRpmAc",    8,  47, "U16", 1.0, 0.0, 500, 2000, "RPM", "FanControl");
    addBits ("fanIdleUp",      8,  49, 0, 0, 0, 1, "FanControl");
    addConst("fanIdleUpRpm",   8,  50, "U16", 1.0, 0.0, 0, 500, "RPM", "FanControl");
    
    // TACHO OUTPUT (Page 8)
    addBits ("tachoEnabled",   8,  60, 0, 0, 0, 1, "TachoOutput");
    addBits ("tachoOutput",    8,  61, 0, 2, 0, 7, "TachoOutput");
    addConst("tachoMult",      8,  62, "U08", 1.0, 0.0, 1, 8, "", "TachoOutput");
    addConst("tachoDiv",       8,  63, "U08", 1.0, 0.0, 1, 8, "", "TachoOutput");
    addConst("tachoPulseMs",   8,  64, "U08", 0.1, 0.0, 0.5, 10.0, "ms", "TachoOutput");
    
    // SHIFT LIGHT (Page 8)
    addBits ("shiftEnabled",   8,  70, 0, 0, 0, 1, "ShiftLight");
    addConst("shiftRpm",       8,  71, "U16", 1.0, 0.0, 1000, 15000, "RPM", "ShiftLight");
    addBits ("shiftOutput",    8,  73, 0, 2, 0, 7, "ShiftLight");
    addBits ("shiftInvert",    8,  74, 0, 0, 0, 1, "ShiftLight");
    
    // TCC (Page 8)
    addBits ("tccEnabled",     8,  80, 0, 0, 0, 1, "TCC");
    addBits ("tccOutput",      8,  81, 0, 2, 0, 7, "TCC");
    addConst("tccMinSpeed",    8,  82, "U08", 1.0, 0.0, 20, 100, "km/h", "TCC");
    addConst("tccMinTps",      8,  83, "U08", 1.0, 0.0, 0, 50, "%", "TCC");
    addConst("tccMinRpm",      8,  84, "U16", 1.0, 0.0, 1000, 4000, "RPM", "TCC");
    addConst("tccUnlockTps",   8,  86, "U08", 1.0, 0.0, 50, 100, "%", "TCC");
    
    // BAROMETRIC (Page 5)
    addBits ("baroMode",       5,  80, 0, 1, 0, 3, "Baro");
    addBits ("baroSensor",     5,  81, 0, 2, 0, 7, "Baro");
    addConst("baroCorrPct",    5,  82, "S08", 1.0, 0.0, -20, 20, "%", "Baro");
    addConst("baroUpdateSec",  5,  83, "U08", 1.0, 0.0, 0, 60, "sec", "Baro");
    
    // MAP/IAT SAMPLE (Page 5)
    addConst("mapSampleCount", 5,  90, "U08", 1.0, 0.0, 1, 32, "", "MapIat");
    addConst("iatSampleCount", 5,  91, "U08", 1.0, 0.0, 1, 32, "", "MapIat");
    addBits ("mapSampleMethod",5,  92, 0, 1, 0, 3, "MapIat");
    addBits ("iatSampleMethod",5,  93, 0, 1, 0, 3, "MapIat");
    addConst("sampleIntervalMs",5, 94, "U08", 1.0, 0.0, 1, 100, "ms", "MapIat");
    addBits ("mapSensorType",  5, 100, 0, 3, 0, 15, "MapIat");
    addConst("mapAt0V",        5, 101, "U08", 0.5, 0.0, 0, 50, "kPa", "MapIat");
    addConst("mapAt5V",        5, 102, "U16", 1.0, 0.0, 100, 400, "kPa", "MapIat");
    addConst("mapSampleWindow",5, 104, "U16", 1.0, 0.0, 0, 720, "° BTDC", "MapIat");
    
    addConst("altMaxDuty",     8, 106, "U08", 1.0, 0.0, 50, 100, "%", "Alternator");
    
    // ========== TUNING WIDGET SETTINGS (Page 9) ==========
    
    // IDLE CONTROL
    addConst("idleRPM",         9,   0, "U16", 1.0, 0.0, 500, 1500, "RPM", "IdleControl");
    addConst("idlePGain",       9,   2, "U08", 0.001, 0.0, 0, 10, "", "IdleControl");
    addConst("idleIGain",       9,   3, "U08", 0.001, 0.0, 0, 10, "", "IdleControl");
    addConst("idleDGain",       9,   4, "U08", 0.001, 0.0, 0, 10, "", "IdleControl");
    addConst("idleColdAdder",   9,   5, "U16", 1.0, 0.0, 0, 500, "RPM", "IdleControl");
    addConst("idleAcAdder",     9,   7, "U16", 1.0, 0.0, 0, 300, "RPM", "IdleControl");
    addBits ("idleDashpotEn",   9,   9, 0, 0, 0, 1, "IdleControl");
    addConst("idleDecayRate",   9,  10, "U08", 1.0, 0.0, 1, 20, "%/s", "IdleControl");
    addConst("idleMinDuty",     9,  11, "U08", 1.0, 0.0, 0, 100, "%", "IdleControl");
    addConst("idleMaxDuty",     9,  12, "U08", 1.0, 0.0, 0, 100, "%", "IdleControl");
    
    // COLD START & WARMUP
    addConst("crankPwCold",     9,  20, "U08", 0.1, 0.0, 0, 25.5, "ms", "ColdStart");
    addConst("crankPwHot",      9,  21, "U08", 0.1, 0.0, 0, 25.5, "ms", "ColdStart");
    addConst("asePctCold",      9,  22, "U08", 1.0, 0.0, 0, 100, "%", "ColdStart");
    addConst("asePctHot",       9,  23, "U08", 1.0, 0.0, 0, 100, "%", "ColdStart");
    addConst("aseCountCold",    9,  24, "U08", 1.0, 0.0, 0, 255, "cycles", "ColdStart");
    addConst("aseCountHot",     9,  25, "U08", 1.0, 0.0, 0, 255, "cycles", "ColdStart");
    addConst("wuePctM40",       9,  26, "U08", 1.0, 0.0, 100, 200, "%", "ColdStart");
    addConst("wuePctM20",       9,  27, "U08", 1.0, 0.0, 100, 200, "%", "ColdStart");
    addConst("wuePct0",         9,  28, "U08", 1.0, 0.0, 100, 200, "%", "ColdStart");
    addConst("wuePct20",        9,  29, "U08", 1.0, 0.0, 100, 200, "%", "ColdStart");
    addConst("wuePct40",        9,  30, "U08", 1.0, 0.0, 100, 200, "%", "ColdStart");
    addConst("wuePct60",        9,  31, "U08", 1.0, 0.0, 100, 150, "%", "ColdStart");
    addConst("wuePct80",        9,  32, "U08", 1.0, 0.0, 100, 120, "%", "ColdStart");
    
    // ACCEL ENRICHMENT
    addBits ("aeMode",          9,  40, 0, 1, 0, 3, "AccelEnrich");
    addConst("aeDuration",      9,  41, "U08", 0.01, 0.0, 0, 2.55, "sec", "AccelEnrich");
    addConst("aeThreshold",     9,  42, "U08", 1.0, 0.0, 0, 255, "%/s", "AccelEnrich");
    addConst("aeMultiplier",    9,  43, "U08", 0.01, 0.0, 0, 2.55, "", "AccelEnrich");
    addConst("aeColdPct",       9,  44, "U08", 1.0, 0.0, 100, 200, "%", "AccelEnrich");
    addConst("daeThreshold",    9,  45, "U08", 1.0, 0.0, 0, 255, "%/s", "AccelEnrich");
    addConst("daeFuelCut",      9,  46, "U08", 1.0, 0.0, 0, 100, "%", "AccelEnrich");
    addBits ("wallWetEn",       9,  47, 0, 0, 0, 1, "AccelEnrich");
    addConst("wwTau",           9,  48, "U08", 0.01, 0.0, 0, 1.0, "", "AccelEnrich");
    addConst("wwBeta",          9,  49, "U08", 0.01, 0.0, 0, 1.0, "", "AccelEnrich");
    
    // FUEL & IGNITION TRIMS - per-cylinder
    addConst("fuelTrimCyl1",    9,  60, "S08", 1.0, 0.0, -25, 25, "%", "Trims");
    addConst("fuelTrimCyl2",    9,  61, "S08", 1.0, 0.0, -25, 25, "%", "Trims");
    addConst("fuelTrimCyl3",    9,  62, "S08", 1.0, 0.0, -25, 25, "%", "Trims");
    addConst("fuelTrimCyl4",    9,  63, "S08", 1.0, 0.0, -25, 25, "%", "Trims");
    addConst("fuelTrimCyl5",    9,  64, "S08", 1.0, 0.0, -25, 25, "%", "Trims");
    addConst("fuelTrimCyl6",    9,  65, "S08", 1.0, 0.0, -25, 25, "%", "Trims");
    addConst("fuelTrimCyl7",    9,  66, "S08", 1.0, 0.0, -25, 25, "%", "Trims");
    addConst("fuelTrimCyl8",    9,  67, "S08", 1.0, 0.0, -25, 25, "%", "Trims");
    addConst("ignTrimCyl1",     9,  68, "S08", 0.5, 0.0, -15, 15, "°", "Trims");
    addConst("ignTrimCyl2",     9,  69, "S08", 0.5, 0.0, -15, 15, "°", "Trims");
    addConst("ignTrimCyl3",     9,  70, "S08", 0.5, 0.0, -15, 15, "°", "Trims");
    addConst("ignTrimCyl4",     9,  71, "S08", 0.5, 0.0, -15, 15, "°", "Trims");
    
    // BOOST CONTROL
    addBits ("boostMode",       9, 100, 0, 1, 0, 3, "BoostControl");
    addConst("boostTarget",     9, 101, "U08", 0.5, 0.0, 0, 50, "psi", "BoostControl");
    addConst("boostPGain",      9, 102, "U08", 0.1, 0.0, 0, 25.5, "", "BoostControl");
    addConst("boostIGain",      9, 103, "U08", 0.1, 0.0, 0, 25.5, "", "BoostControl");
    addConst("boostDGain",      9, 104, "U08", 0.1, 0.0, 0, 25.5, "", "BoostControl");
    addConst("boostDutyMin",    9, 105, "U08", 1.0, 0.0, 0, 100, "%", "BoostControl");
    addConst("boostDutyMax",    9, 106, "U08", 1.0, 0.0, 0, 100, "%", "BoostControl");
    addConst("boostByGear1",    9, 107, "U08", 1.0, 0.0, 0, 100, "%", "BoostControl");
    addConst("boostByGear2",    9, 108, "U08", 1.0, 0.0, 0, 100, "%", "BoostControl");
    addConst("boostByGear3",    9, 109, "U08", 1.0, 0.0, 0, 100, "%", "BoostControl");
    addConst("boostByGear4",    9, 110, "U08", 1.0, 0.0, 0, 100, "%", "BoostControl");
    addConst("boostByGear5",    9, 111, "U08", 1.0, 0.0, 0, 100, "%", "BoostControl");
    addConst("boostByGear6",    9, 112, "U08", 1.0, 0.0, 0, 100, "%", "BoostControl");
    
    // KNOCK CONTROL
    addBits ("knockEnabled",    9, 120, 0, 0, 0, 1, "KnockControl");
    addConst("knockThreshold",  9, 121, "U08", 1.0, 0.0, 0, 255, "mV", "KnockControl");
    addConst("knockRetardDeg",  9, 122, "U08", 0.5, 0.0, 0, 15, "°", "KnockControl");
    addConst("knockRecovery",   9, 123, "U08", 0.5, 0.0, 0, 15, "°/sec", "KnockControl");
    addConst("knockMaxRetard",  9, 124, "U08", 0.5, 0.0, 0, 25, "°", "KnockControl");
    addConst("knockCount",      9, 125, "U08", 1.0, 0.0, 1, 10, "", "KnockControl");
    addConst("knockWindowStart",9, 126, "U08", 1.0, 0.0, 0, 90, "° ATDC", "KnockControl");
    addConst("knockWindowEnd",  9, 127, "U08", 1.0, 0.0, 0, 90, "° ATDC", "KnockControl");
    addConst("knockFreqKhz",    9, 128, "U08", 0.1, 0.0, 1, 25.5, "kHz", "KnockControl");
    
    // VVT/VTEC
    addBits ("vvtMode",         9, 140, 0, 1, 0, 3, "VVT");
    addConst("vvtTargetAngle",  9, 141, "U08", 1.0, 0.0, 0, 60, "°", "VVT");
    addConst("vvtPGain",        9, 142, "U08", 0.1, 0.0, 0, 25.5, "", "VVT");
    addConst("vvtIGain",        9, 143, "U08", 0.1, 0.0, 0, 25.5, "", "VVT");
    addConst("vvtDGain",        9, 144, "U08", 0.1, 0.0, 0, 25.5, "", "VVT");
    addBits ("vtecEnabled",     9, 145, 0, 0, 0, 1, "VVT");
    addConst("vtecEngageRpm",   9, 146, "U16", 1.0, 0.0, 2000, 10000, "RPM", "VVT");
    addConst("vtecDisenRpm",    9, 148, "U16", 1.0, 0.0, 1000, 9000, "RPM", "VVT");
    addConst("vtecMinClt",      9, 150, "U08", 1.0, -40.0, 40, 100, "°C", "VVT");
    
    // LIMITERS (extended)
    addConst("overboostCut",    9, 160, "U08", 0.5, 0.0, 0, 50, "psi", "Limiters");
    addBits ("flatShiftEn",     9, 161, 0, 0, 0, 1, "Limiters");
    addConst("flatShiftRpm",    9, 162, "U16", 1.0, 0.0, 0, 2000, "RPM", "Limiters");
    addConst("maxMapLim",       9, 164, "U16", 1.0, 0.0, 100, 400, "kPa", "Limiters");
    addBits ("oilPCutEn",       9, 166, 0, 0, 0, 1, "Limiters");
    addConst("oilPCutKpa",      9, 167, "U08", 1.0, 0.0, 0, 200, "kPa", "Limiters");
    
    // Additional Limiters
    addBits ("speedLimEnabled", 9, 168, 0, 0, 0, 1, "Limiters");
    addConst("speedLimit",      9, 169, "U16", 1.0, 0.0, 0, 300, "km/h", "Limiters");
    addBits ("speedLimAction",  9, 171, 0, 1, 0, 3, "Limiters");
    addConst("speedLimHyst",    9, 172, "U08", 1.0, 0.0, 0, 20, "km/h", "Limiters");
    addBits ("flatShiftEnabled",9, 173, 0, 0, 0, 1, "Limiters");
    addConst("flatShiftRpmDrop",9, 174, "U16", 1.0, 0.0, 0, 2000, "RPM", "Limiters");
    addConst("launchTps",       9, 176, "U08", 1.0, 0.0, 0, 100, "%", "Limiters");
    
    // Additional MAF/MAT
    addConst("mafVoltage1",     9, 177, "U08", 0.02, 0.0, 0, 5.0, "V", "MAF");
    addConst("mafVoltage2",     9, 178, "U08", 0.02, 0.0, 0, 5.0, "V", "MAF");
    addConst("mafVoltage3",     9, 179, "U08", 0.02, 0.0, 0, 5.0, "V", "MAF");
    addConst("mafVoltage4",     9, 180, "U08", 0.02, 0.0, 0, 5.0, "V", "MAF");
    addConst("matCorrBin1",     9, 181, "S08", 1.0, 0.0, -40, 120, "°C", "MAF");
    addConst("matCorrBin2",     9, 182, "S08", 1.0, 0.0, -40, 120, "°C", "MAF");
    addConst("matCorrVal1",     9, 183, "S08", 1.0, 0.0, -50, 50, "%", "MAF");
    addConst("matCorrVal2",     9, 184, "S08", 1.0, 0.0, -50, 50, "%", "MAF");

    // Additional Trims
    addConst("fuelTrimGlobal",  9, 185, "S08", 1.0, 0.0, -25, 25, "%", "Trims");
    addConst("ignTrimGlobal",   9, 186, "S08", 0.5, 0.0, -15, 15, "°", "Trims");

    // Additional Idle    
    addConst("idleTarget",      9, 187, "U16", 1.0, 0.0, 500, 2000, "RPM", "IdleControl");
    addConst("idleKP",          9, 189, "U08", 0.1, 0.0, 0, 25.5, "", "IdleControl");
    addConst("idleKI",          9, 190, "U08", 0.1, 0.0, 0, 25.5, "", "IdleControl");
    addConst("idleKD",          9, 191, "U08", 0.1, 0.0, 0, 25.5, "", "IdleControl");
    addConst("iacCLmulti",      9, 192, "U08", 1.0, 0.0, 1, 10, "", "IdleControl");
    addConst("iacACextra",      9, 193, "U08", 1.0, 0.0, 0, 50, "%", "IdleControl");
    addConst("iacDashpot",      9, 194, "U08", 1.0, 0.0, 0, 50, "%", "IdleControl");
    addConst("iacMinDuty",      9, 195, "U08", 1.0, 0.0, 0, 100, "%", "IdleControl");
    addConst("iacMaxDuty",      9, 196, "U08", 1.0, 0.0, 0, 100, "%", "IdleControl");
    
    // Additional Cold Start
    addConst("primePulse",      9, 197, "U08", 0.1, 0.0, 0, 25.5, "ms", "ColdStart");
    addConst("wueBins",         9, 198, "U08", 1.0, 0.0, 0, 100, "", "ColdStart");
    addConst("wueValues",       9, 199, "U08", 1.0, 0.0, 0, 255, "", "ColdStart");
    addConst("aseMax",          9, 200, "U08", 1.0, 0.0, 0, 255, "%", "ColdStart");
    
    // Additional Accel Enrichment
    addConst("tpsThresh",       9, 201, "U08", 1.0, 0.0, 0, 255, "v/s", "AccelEnrich");
    addConst("tpsEnrich",       9, 202, "U08", 1.0, 0.0, 0, 255, "ms", "AccelEnrich");
    addConst("tpsTime",         9, 203, "U08", 0.1, 0.0, 0, 2.5, "s", "AccelEnrich");
    addConst("tpsColdMod",      9, 204, "U08", 1.0, 0.0, 0, 255, "%", "AccelEnrich");
    addConst("mapThresh",       9, 205, "U08", 1.0, 0.0, 0, 255, "kPa/s", "AccelEnrich");
    addConst("mapEnrich",       9, 206, "U08", 1.0, 0.0, 0, 255, "ms", "AccelEnrich");
    addConst("dfcoDelay",       9, 207, "U08", 0.1, 0.0, 0, 25.5, "s", "AccelEnrich");
    addConst("dfcoHyst",        9, 208, "U08", 10.0, 0.0, 0, 500, "RPM", "AccelEnrich");
    
    // Additional VVT
    addConst("vtecRpm",         9, 209, "U16", 1.0, 0.0, 0, 15000, "RPM", "VVT");
    addConst("vtecHyst",        9, 211, "U08", 10.0, 0.0, 0, 500, "RPM", "VVT");
    addBits ("vvtEnabled",      9, 212, 0, 0, 0, 1, "VVT");
    addConst("vvtMaxAdv",       9, 213, "U08", 1.0, 0.0, 0, 60, "°", "VVT");
    addConst("vvtMinAdv",       9, 214, "U08", 1.0, 0.0, 0, 60, "°", "VVT");
    
    // Additional Boost
    addBits ("boostClEnabled",  9, 215, 0, 0, 0, 1, "BoostControl");
    addConst("boostKP",         9, 216, "U08", 1.0, 0.0, 0, 255, "%", "BoostControl");
    addConst("boostKI",         9, 217, "U08", 1.0, 0.0, 0, 255, "%", "BoostControl");
    addConst("boostKD",         9, 218, "U08", 1.0, 0.0, 0, 255, "%", "BoostControl");
    
    // Additional Knock
    addConst("knockThresh",     9, 219, "U08", 1.0, 0.0, 0, 255, "V", "KnockControl");
    addConst("knockWinStart",   9, 220, "U08", 1.0, 0.0, 0, 90, "°", "KnockControl");
    addConst("knockWinEnd",     9, 221, "U08", 1.0, 0.0, 0, 90, "°", "KnockControl");
    addConst("knockRetStep",    9, 222, "U08", 0.5, 0.0, 0, 15, "°", "KnockControl");
    addConst("knockRecRate",    9, 223, "U08", 0.5, 0.0, 0, 15, "°/s", "KnockControl");
    
    // IO CONFIG
    addBits ("flexFuelEn",      9, 180, 0, 0, 0, 1, "IOConfig");
    addBits ("flexFuelPin",     9, 181, 0, 3, 0, 15, "IOConfig");
    addBits ("wboEnabled",      9, 182, 0, 0, 0, 1, "IOConfig");
    addBits ("wboType",         9, 183, 0, 2, 0, 7, "IOConfig");
    addBits ("canEnabled",      9, 184, 0, 0, 0, 1, "IOConfig");
    addConst("canBusSpeed",     9, 185, "U16", 1.0, 0.0, 125, 1000, "kbps", "IOConfig");
    
    // ========== NEW-001: EGO / O2 Closed-Loop Settings ==========
    // NOTE: page/offset values are illustrative defaults; real values come from loaded INI
    addBits ("egoType",         2,   0, 0, 2, 0, 7, "EGO");       // 0=Off, 1=Simple, 2=PID
    addBits ("egoSensor",       2,   1, 0, 2, 0, 7, "EGO");       // 0=NB, 1=WB Generic, 2=AEM, etc.
    addConst("egoAuthority",    2,   2, "U08", 1.0, 0.0, 0, 100, "%", "EGO");
    addConst("egoStepSize",     2,   3, "U08", 0.1, 0.0, 0, 10, "%", "EGO");
    addConst("egoUpdateRate",   2,   4, "U08", 1.0, 0.0, 10, 200, "ms", "EGO");
    addConst("egoMinClt",       2,   5, "U08", 1.0, -40.0, 40, 100, "°C", "EGO");
    addConst("egoMinRpm",       2,   6, "U16", 1.0, 0.0, 500, 5000, "RPM", "EGO");
    addConst("egoMinTps",       2,   8, "U08", 1.0, 0.0, 0, 50, "%", "EGO");
    addConst("egoMaxMap",       2,   9, "U08", 1.0, 0.0, 50, 250, "kPa", "EGO");
    addConst("egoCorrMin",      2,  10, "U08", 1.0, 0.0, 50, 100, "%", "EGO");
    addConst("egoCorrMax",      2,  11, "U08", 1.0, 0.0, 100, 150, "%", "EGO");
    addConst("egoKP",           2,  12, "U08", 0.1, 0.0, 0, 25, "", "EGO");
    addConst("egoKI",           2,  13, "U08", 0.1, 0.0, 0, 25, "", "EGO");
    addConst("egoKD",           2,  14, "U08", 0.1, 0.0, 0, 25, "", "EGO");
    
    // ========== NEW-002: Injector Dead-Time vs Battery Voltage ==========
    addConst("injDeadBin1",     2,  20, "U08", 0.1, 0.0, 6, 20, "V", "InjDeadTime");
    addConst("injDeadBin2",     2,  21, "U08", 0.1, 0.0, 6, 20, "V", "InjDeadTime");
    addConst("injDeadBin3",     2,  22, "U08", 0.1, 0.0, 6, 20, "V", "InjDeadTime");
    addConst("injDeadBin4",     2,  23, "U08", 0.1, 0.0, 6, 20, "V", "InjDeadTime");
    addConst("injDeadBin5",     2,  24, "U08", 0.1, 0.0, 6, 20, "V", "InjDeadTime");
    addConst("injDeadBin6",     2,  25, "U08", 0.1, 0.0, 6, 20, "V", "InjDeadTime");
    addConst("injDeadVal1",     2,  26, "U08", 0.1, 0.0, 0, 5, "ms", "InjDeadTime");
    addConst("injDeadVal2",     2,  27, "U08", 0.1, 0.0, 0, 5, "ms", "InjDeadTime");
    addConst("injDeadVal3",     2,  28, "U08", 0.1, 0.0, 0, 5, "ms", "InjDeadTime");
    addConst("injDeadVal4",     2,  29, "U08", 0.1, 0.0, 0, 5, "ms", "InjDeadTime");
    addConst("injDeadVal5",     2,  30, "U08", 0.1, 0.0, 0, 5, "ms", "InjDeadTime");
    addConst("injDeadVal6",     2,  31, "U08", 0.1, 0.0, 0, 5, "ms", "InjDeadTime");
    
    // ========== NEW-005: Cranking Advance Settings ==========
    addConst("crankingAdvance",  4,  10, "S08", 1.0, 0.0, -10, 20, "°", "Ignition");
    addBits ("fixedCrankTiming", 4,  11, 0, 0, 0, 1, "Ignition");
    
    // ========== NEW-006: Ignition Trims Cylinders 5-8 ==========
    addConst("ignTrimCyl5",     9,  72, "S08", 0.5, 0.0, -15, 15, "°", "Trims");
    addConst("ignTrimCyl6",     9,  73, "S08", 0.5, 0.0, -15, 15, "°", "Trims");
    addConst("ignTrimCyl7",     9,  74, "S08", 0.5, 0.0, -15, 15, "°", "Trims");
    addConst("ignTrimCyl8",     9,  75, "S08", 0.5, 0.0, -15, 15, "°", "Trims");
    
    return constants;
}

QMap<QString, ECUDefinition::Table> ECUDefinition::getDefaultSpeeduinoTables() {
    QMap<QString, Table> tables;
    
    // VE Table (Page 1, Offset 0, 16x16 bytes)
    Table ve;
    ve.name = "VE Table (Volumetric Efficiency)";
    ve.page = 1;         // Typically page 1
    ve.address = 0;      // Starts at offset 0
    ve.rows = 16;
    ve.cols = 16;
    ve.elementSize = 1;  // 1 byte per cell (U08)
    ve.xLabel = "RPM (rpm)";
    ve.yLabel = "Load (MAP) (kPa)";
    ve.zLabel = "VE (%)";
    ve.scale = 1.0;
    ve.translate = 0.0;
    tables.insert(ve.name, ve);
    
    // Ignition Table (Page 1, Offset 256, 16x16 bytes)
    Table ign;
    ign.name = "Ignition Advance Table";
    ign.page = 1;
    ign.address = 256;   // Typically offset 256
    ign.rows = 16;
    ign.cols = 16;
    ign.elementSize = 1;
    ign.xLabel = "RPM (rpm)";
    ign.yLabel = "Load (MAP) (kPa)";
    ign.zLabel = "Advance (deg)";
    ign.scale = 1.0;     // Often 1.0 or 0.5 depending on firmware
    ign.translate = 0.0;
    tables.insert(ign.name, ign);
    
    // AFR Table (Page 1, Offset 512, 16x16 bytes)
    Table afr;
    afr.name = "AFR Target Table";
    afr.page = 1;
    afr.address = 512;   // Typically offset 512
    afr.rows = 16;
    afr.cols = 16;
    afr.elementSize = 1; 
    afr.xLabel = "RPM (rpm)";
    afr.yLabel = "Load (MAP) (kPa)";
    afr.zLabel = "AFR Target";
    afr.scale = 0.1;     // AFR usually scaled by 0.1
    afr.translate = 0.0;
    tables.insert(afr.name, afr);
    
    return tables;
}

void ECUDefinition::parseTableEditor(QTextStream &in) {
    // Regex for table header: table = id, mapId, "Title", page
    QRegularExpression tableRegex(R"(^\s*table\s*=\s*(\w+)\s*,\s*(\w+)\s*,\s*\"([^\"]+)\"\s*,\s*(\d+))");
    
    // Key/value properties inside table
    QRegularExpression propRegex(R"(^\s*(\w+)\s*=\s*(.*)$)");

    Table currentTable;
    bool inTable = false;

    while (!in.atEnd()) {
        QString rawLine = in.readLine();
        QString line = rawLine.trimmed();
        
        if (line.isEmpty() || line.startsWith(";")) continue;
        if (line.startsWith("[")) {
            break; // Next section — main loop handles it
        }

        if (line.startsWith("#")) {
            processDirective(line);
            continue;
        }
        if (shouldSkipLine()) continue;
        
        line = expandDefines(line);
        
        QRegularExpressionMatch match = tableRegex.match(line);
        if (match.hasMatch()) {
            if (inTable && !currentTable.name.isEmpty()) {
                m_tableDefinitions.insert(currentTable.name, currentTable);
            }
            
            inTable = true;
            currentTable = Table();
            QString title = match.captured(3); // Title from INI
            currentTable.name = title; 
            currentTable.page = match.captured(4).toInt();
            continue;
        }
        
        if (inTable) {
            match = propRegex.match(line);
            if (match.hasMatch()) {
                QString key = match.captured(1);
                QString valueStr = match.captured(2);
                QStringList values = valueStr.split(",", Qt::SkipEmptyParts);
                for (int i=0; i<values.size(); i++) {
                    values[i] = values[i].trimmed();
                    if (values[i].startsWith('"') && values[i].endsWith('"')) {
                        values[i] = values[i].mid(1, values[i].length() - 2);
                    }
                }
                
                if (key == "xBins" && values.size() >= 1) currentTable.xLabel = values[0];
                if (key == "yBins" && values.size() >= 1) currentTable.yLabel = values[0];
                if (key == "zBins" && values.size() >= 1) {
                    QString zConst = values[0];
                    if (m_constants.contains(zConst)) {
                        Constant c = m_constants[zConst];
                        currentTable.address = c.offset;
                        currentTable.rows = c.rows;
                        currentTable.cols = c.cols;
                        currentTable.scale = c.scale;
                        currentTable.translate = c.translate;
                        currentTable.elementSize = c.byteSize();
                        currentTable.zLabel = zConst;
                    }
                }
            }
        }
    }
    
    if (inTable && !currentTable.name.isEmpty()) {
        m_tableDefinitions.insert(currentTable.name, currentTable);
    }
}

// ============================================================================
//  Signature Validation — Safety Gate for ECU Write Operations
// ============================================================================

ECUDefinition::SignatureValidation ECUDefinition::validateSignature(const ECUSignature &receivedSig) const {
    // If no definition signature has been loaded, allow any signature
    // (backward compatibility for simple use cases)
    if (m_signature.isEmpty()) {
        return SignatureValidation(true, "No definition signature constraint configured");
    }
    
    // Check protocol version first
    if (receivedSig.protocolVersion != 2) {
        return SignatureValidation(false, 
            QString("Protocol version mismatch: expected v2, got v%1")
                .arg(receivedSig.protocolVersion));
    }
    
    // Check firmware version string matches definition signature
    // Definition signature format: "Speeduino YYYY.MM or similar"
    // Received format: "Speeduino 2025.01" from ECU
    if (!receivedSig.firmwareVersion.contains(m_signature, Qt::CaseInsensitive)) {
        return SignatureValidation(false,
            QString("Firmware mismatch: definition expects '%1', ECU reports '%2'")
                .arg(m_signature)
                .arg(receivedSig.firmwareVersion));
    }
    
    // Check page count
    if (receivedSig.pageCount != 15) {
        return SignatureValidation(false,
            QString("Page count mismatch: Speeduino expects 15 pages, got %1")
                .arg(receivedSig.pageCount));
    }
    
    // Validation passed
    return SignatureValidation(true,
        QString("Signature validated: %1").arg(receivedSig.toString()));
}


#undef MAKE_CONSTANT
