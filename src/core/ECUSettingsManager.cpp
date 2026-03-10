/**
 * @file ECUSettingsManager.cpp
 * @brief Implementation of ECUSettingsManager
 */

#include "ECUSettingsManager.h"
#include "SerialManager.h"
#include "../utils/Logger.h"
#include <QDebug>
#include <QRegularExpression>

ECUSettingsManager::ECUSettingsManager(QObject *parent)
    : QObject(parent)
    , m_serialManager(nullptr)
    , m_isLoaded(false)
    , m_pagesRequested(0)
    , m_pagesReceived(0)
    , m_isVerifyingBurn(false)
    , m_expectedPageCRC(0)
{
    // Initialize page state
    for (int i = 0; i < MAX_PAGES; i++) {
        m_pageDirty[i] = false;
        m_pageLoaded[i] = false;
        m_pageCache[i].clear();
    }
    
    // Load setting definitions
    initializeSettings();
    
    // Burn delay timer - wait 1000ms (ECU_DEFER_DELAY) after last write before burning
    m_burnDelayTimer = new QTimer(this);
    m_burnDelayTimer->setSingleShot(true);
    m_burnDelayTimer->setInterval(1000); 
    connect(m_burnDelayTimer, &QTimer::timeout, this, [this]() {
        if (m_pendingBurnPage > 0) {
            burnPage(m_pendingBurnPage);
        }
    });

    // Burn verification timeout - 2s max per spec
    m_burnVerifyTimeoutTimer = new QTimer(this);
    m_burnVerifyTimeoutTimer->setSingleShot(true);
    m_burnVerifyTimeoutTimer->setInterval(2000);
    connect(m_burnVerifyTimeoutTimer, &QTimer::timeout, this, &ECUSettingsManager::onBurnVerifyTimeout);
}

ECUSettingsManager::~ECUSettingsManager() {}

void ECUSettingsManager::initializeSettings() {
    m_definitions = ECUDefinition::getDefaultSpeeduinoConstants();
    Logger::info("Loaded " + QString::number(m_definitions.size()) + " setting definitions");
}

void ECUSettingsManager::setSerialManager(SerialManager *serialManager) {
    if (m_serialManager) {
        disconnect(m_serialManager, nullptr, this, nullptr);
    }
    
    m_serialManager = serialManager;
    
    if (m_serialManager) {
        connect(m_serialManager, &SerialManager::tableResponseReceived,
                this, &ECUSettingsManager::onTableResponseReceived);
        connect(m_serialManager, &SerialManager::connected,
                this, [this](const ECUSignature &) { onSerialConnected(); });
        connect(m_serialManager, &SerialManager::disconnected,
                this, &ECUSettingsManager::onSerialDisconnected);
        connect(m_serialManager, &SerialManager::dataReceived,
                this, &ECUSettingsManager::onDataReceived);
        connect(m_serialManager, &SerialManager::pageCRCReceived,
                this, &ECUSettingsManager::onPageCRCReceived);
    }
}

void ECUSettingsManager::onSerialConnected() {
    Logger::info("ECUSettingsManager: Serial connected, reading settings...");
    readAllFromECU();
}

void ECUSettingsManager::onSerialDisconnected() {
    m_isLoaded = false;
    for (int i = 0; i < MAX_PAGES; i++) {
        m_pageLoaded[i] = false;
    }
}

// ========== Value Access ==========

QVariant ECUSettingsManager::getValue(const QString &settingName) const {
    if (m_values.contains(settingName)) {
        return m_values[settingName];
    }
    return QVariant();
}

int ECUSettingsManager::getRawValue(const QString &settingName) const {
    if (!m_definitions.contains(settingName)) {
        return 0;
    }
    
    const auto &def = m_definitions[settingName];
    if (!m_pageLoaded[def.page] || m_pageCache[def.page].isEmpty()) {
        return 0;
    }
    
    const QByteArray &page = m_pageCache[def.page];
    if (def.offset >= page.size()) {
        return 0;
    }
    
    // Extract raw value based on type
    if (def.type == "U08") {
        return static_cast<quint8>(page.at(def.offset));
    } else if (def.type == "S08") {
        return static_cast<qint8>(page.at(def.offset));
    } else if (def.type == "U16" && def.offset + 1 < page.size()) {
        return static_cast<quint8>(page.at(def.offset)) |
               (static_cast<quint8>(page.at(def.offset + 1)) << 8);  // LE: low byte first
    } else if (def.type == "S16" && def.offset + 1 < page.size()) {
        qint16 val = static_cast<quint8>(page.at(def.offset)) |
                     (static_cast<quint8>(page.at(def.offset + 1)) << 8);  // LE: low byte first
        return val;
    } else if (def.paramClass == "bits") {
        quint8 byte = static_cast<quint8>(page.at(def.offset));
        quint8 mask = ((1 << (def.bitField.highBit - def.bitField.lowBit + 1)) - 1) << def.bitField.lowBit;
        return (byte & mask) >> def.bitField.lowBit;
    }
    
    return 0;
}

void ECUSettingsManager::setValue(const QString &settingName, const QVariant &value, bool autoWrite) {
    if (!m_definitions.contains(settingName)) {
        Logger::warning("Unknown setting: " + settingName);
        return;
    }
    
    m_values[settingName] = value;
    emit settingChanged(settingName, value);
    
    const auto &def = m_definitions[settingName];
    markPageDirty(def.page);
    
    if (autoWrite && m_serialManager && m_serialManager->isConnected()) {
        writeToECU(settingName);
    }
}

ECUDefinition::Constant ECUSettingsManager::getDefinition(const QString &settingName) const {
    return m_definitions.value(settingName);
}

bool ECUSettingsManager::hasSetting(const QString &settingName) const {
    return m_definitions.contains(settingName);
}

// ========== ECU Operations ==========

void ECUSettingsManager::readAllFromECU() {
    if (!m_serialManager || !m_serialManager->isConnected()) {
        emit errorOccurred("Not connected to ECU");
        return;
    }
    
    m_pagesRequested = 0;
    m_pagesReceived = 0;
    m_isLoaded = false;
    
    // Determine which pages need to be read based on settings
    QSet<quint8> pagesToRead;
    for (const auto &def : m_definitions) {
        pagesToRead.insert(def.page);
    }
    
    m_pagesRequested = pagesToRead.size();
    
    for (quint8 page : pagesToRead) {
        readPageFromECU(page);
    }
    
    emit loadProgress(0);
}

void ECUSettingsManager::readPageFromECU(quint8 page) {
    if (!m_serialManager) return;
    
    // Use actual page sizes from protocol spec
    static const uint16_t pageSizes[] = {
        0, 128, 288, 288, 128, 288, 128, 240, 384, 192, 192, 288, 192, 128, 288, 256
    };
    int pageSize = (page >= 1 && page <= 15) ? pageSizes[page] : MAX_PAGE_SIZE;
    
    // BUG-006 FIX: Use tableBlockingFactor for page reads
    int blockSize = m_serialManager->getSignature().tableBlockingFactor;
    if (blockSize <= 0 || blockSize > 256) blockSize = 121; // Safe Speeduino default
    
    for (int offset = 0; offset < pageSize; offset += blockSize) {
        int chunkSize = qMin(blockSize, pageSize - offset);
        m_serialManager->readTable(page, offset, chunkSize);
    }
}

void ECUSettingsManager::onTableResponseReceived(quint8 table, quint16 offset, const QByteArray &data) {
    if (table >= MAX_PAGES) {
        Logger::warning("Received data for invalid page: " + QString::number(table));
        return;
    }
    
    // Store page data
    if (offset == 0) {
        m_pageCache[table] = data;
    } else {
        // Append if offset != 0 (chunked read)
        while (m_pageCache[table].size() < offset) {
            m_pageCache[table].append('\0');
        }
        m_pageCache[table].replace(offset, data.size(), data);
    }
    
    m_pageLoaded[table] = true;
    m_pagesReceived++;
    
    // Log page received (using qDebug for verbose logging)
    qDebug() << "Received page" << table << "(" << data.size() << "bytes)";
    
    // Extract settings from this page
    extractSettingsFromPage(table);
    extractTablesFromPage(table); // Added call
    
    emit pageReadComplete(table);
    
    // Update progress
    if (m_pagesRequested > 0) {
        int progress = (m_pagesReceived * 100) / m_pagesRequested;
        emit loadProgress(progress);
        
        if (m_pagesReceived >= m_pagesRequested) {
            m_isLoaded = true;
            Logger::info("All settings loaded from ECU");
            
            // BUG-005 FIX: Also read core tables (VE, Ignite, AFR) on connect
            readTable("veMap");
            readTable("ignMap");
            readTable("afrMap");
            
            emit readComplete();
        }
    }
}

void ECUSettingsManager::extractSettingsFromPage(quint8 page) {
    for (auto it = m_definitions.constBegin(); it != m_definitions.constEnd(); ++it) {
        if (it.value().page == page) {
            QVariant value = decodeValue(it.value(), m_pageCache[page]);
            m_values[it.key()] = value;
            emit settingChanged(it.key(), value);
        }
    }
}

QVariant ECUSettingsManager::decodeValue(const ECUDefinition::Constant &def, const QByteArray &pageData) const {
    if (def.offset >= pageData.size()) {
        return QVariant(0.0);
    }
    
    int rawValue = 0;
    
    if (def.type == "U08") {
        rawValue = static_cast<quint8>(pageData.at(def.offset));
    } else if (def.type == "S08") {
        rawValue = static_cast<qint8>(pageData.at(def.offset));
    } else if (def.type == "U16" && def.offset + 1 < pageData.size()) {
        rawValue = static_cast<quint8>(pageData.at(def.offset)) |
                   (static_cast<quint8>(pageData.at(def.offset + 1)) << 8);  // LE: low byte first
    } else if (def.type == "S16" && def.offset + 1 < pageData.size()) {
        rawValue = static_cast<qint16>(
            static_cast<quint8>(pageData.at(def.offset)) |
            (static_cast<quint8>(pageData.at(def.offset + 1)) << 8));  // LE: low byte first
    } else if (def.paramClass == "bits") {
        quint8 byte = static_cast<quint8>(pageData.at(def.offset));
        quint8 mask = ((1 << (def.bitField.highBit - def.bitField.lowBit + 1)) - 1) << def.bitField.lowBit;
        rawValue = (byte & mask) >> def.bitField.lowBit;
    }
    
    // Convert to user value
    double userValue = def.rawToUser(rawValue);
    return QVariant(userValue);
}

void ECUSettingsManager::writeToECU(const QString &settingName) {
    if (!m_serialManager || !m_serialManager->isConnected()) {
        emit errorOccurred("Not connected to ECU");
        return;
    }
    
    if (!m_definitions.contains(settingName) || !m_values.contains(settingName)) {
        emit errorOccurred("Unknown setting: " + settingName);
        return;
    }
    
    const auto &def = m_definitions[settingName];
    QByteArray encoded = encodeValue(def, m_values[settingName]);
    
    // BUG-006 FIX: Use blockingFactor for individual setting writes
    int blockSize = m_serialManager->getSignature().blockingFactor;
    if (blockSize <= 0 || blockSize > 256) blockSize = 121;
    
    for (int offset = 0; offset < encoded.size(); offset += blockSize) {
        int chunkSize = qMin(blockSize, int(encoded.size()) - offset);
        m_serialManager->writeTable(def.page, def.offset + offset, encoded.mid(offset, chunkSize));
    }
    
    qDebug() << "Writing" << settingName << "to page" << def.page << "offset" << def.offset;
    
    // Update local cache so bits/etc work correctly on next write
    if (def.offset + encoded.size() <= m_pageCache[def.page].size()) {
        m_pageCache[def.page].replace(def.offset, encoded.size(), encoded);
    }

    emit writeComplete(settingName);
    
    // Schedule burn after delay
    m_pendingBurnPage = def.page;
    m_burnDelayTimer->start();
}

QByteArray ECUSettingsManager::encodeValue(const ECUDefinition::Constant &def, const QVariant &value) const {
    QByteArray data;
    
    int rawValue = def.userToRaw(value.toDouble());
    
    if (def.type == "U08" || def.type == "S08") {
        data.append(static_cast<char>(rawValue & 0xFF));
    } else if (def.type == "U16" || def.type == "S16") {
        data.append(static_cast<char>(rawValue & 0xFF));         // Low byte first (LE)
        data.append(static_cast<char>((rawValue >> 8) & 0xFF));  // High byte second
    } else if (def.paramClass == "bits") {
        // For bits, we need to read-modify-write
        quint8 currentByte = 0;
        if (m_pageLoaded[def.page] && def.offset < m_pageCache[def.page].size()) {
            currentByte = static_cast<quint8>(m_pageCache[def.page].at(def.offset));
        }
        
        quint8 mask = ((1 << (def.bitField.highBit - def.bitField.lowBit + 1)) - 1) << def.bitField.lowBit;
        currentByte = (currentByte & ~mask) | ((rawValue << def.bitField.lowBit) & mask);
        data.append(static_cast<char>(currentByte));
    }
    
    return data;
}

void ECUSettingsManager::markPageDirty(quint8 page) {
    if (page < MAX_PAGES) {
        m_pageDirty[page] = true;
    }
}

void ECUSettingsManager::burnPage(quint8 page) {
    if (!m_serialManager || !m_serialManager->isConnected()) {
        return;
    }
    
    if (m_isVerifyingBurn) {
        // Enqueue or skip? For now, we only support one burn at a time
        Logger::warning("Burn already in progress, skipping burn for page " + QString::number(page));
        return;
    }

    Logger::info("Sending burn command for page " + QString::number(page));
    
    // 1. Calculate and store expected CRC32 for verification later
    if (page >= MAX_PAGES || m_pageCache[page].isEmpty()) {
        Logger::error("Cannot verify burn: page cache empty for page " + QString::number(page));
    } else {
        m_expectedPageCRC = SpeeduinoProtocol::calculateCRC32(m_pageCache[page]);
    }

    // 2. Send the burn command
    m_serialManager->sendBurnCommand(page);
    
    // 3. Start polling for status4 bit 4
    m_isVerifyingBurn = true;
    m_pendingBurnPage = page;
    m_burnVerifyTimeoutTimer->start();
    
    m_pageDirty[page] = false;
}

void ECUSettingsManager::onDataReceived(const RealTimeData &data) {
    if (!m_isVerifyingBurn) return;

    // Check status4 bit 4 (BIT_STATUS4_BURNPENDING)
    bool isPending = (data.status4 & 0x10);
    
    if (!isPending) {
        // Burn completed on ECU
        m_isVerifyingBurn = false;
        m_burnVerifyTimeoutTimer->stop();
        
        Logger::info("Burn verified on status4 bit 4. Requesting sync CRC...");
        
        // Final verification: Get page CRC from ECU
        SerialCommand crcCmd;
        crcCmd.data = m_serialManager->m_protocol->createPageCRCRequest(m_pendingBurnPage);
        crcCmd.type = CommandType::PageCRC;
        crcCmd.expectedResponse = -1;
        crcCmd.timeoutMs = ProtocolTiming::TIMEOUT_MS;
        crcCmd.retryCount = 3;
        crcCmd.pageNum = m_pendingBurnPage;
        m_serialManager->queueCommand(crcCmd);
    }
}

void ECUSettingsManager::onBurnVerifyTimeout() {
    if (!m_isVerifyingBurn) return;
    
    m_isVerifyingBurn = false;
    Logger::error("Burn verification timed out for page " + QString::number(m_pendingBurnPage));
    emit burnVerificationFailed(m_pendingBurnPage, "ECU timed out responding to burn");
}

void ECUSettingsManager::burnAllDirty() {
    for (int i = 0; i < MAX_PAGES; i++) {
        if (m_pageDirty[i]) {
            burnPage(static_cast<quint8>(i));
        }
    }
}

QList<quint8> ECUSettingsManager::getDirtyPages() const {
    QList<quint8> dirty;
    for (int i = 0; i < MAX_PAGES; i++) {
        if (m_pageDirty[i]) {
            dirty.append(static_cast<quint8>(i));
        }
    }
    return dirty;
}

void ECUSettingsManager::onPageCRCReceived(quint8 page, uint32_t crc) {
    if (page != m_pendingBurnPage) return;

    if (crc == m_expectedPageCRC) {
        Logger::info("Burn verification successful for page " + QString::number(page));
        m_pendingBurnPage = 0;
        emit burnComplete(page);
    } else {
        Logger::error(QString("Burn verification failed for page %1! (Expected 0x%2, Got 0x%3)")
                      .arg(page).arg(m_expectedPageCRC, 8, 16, QChar('0'))
                      .arg(crc, 8, 16, QChar('0')));
        emit burnVerificationFailed(page, "CRC mismatch after burn");
    }
}

void ECUSettingsManager::readTable(const QString &tableName) {
    if (!m_tableDefinitions.contains(tableName)) {
        // Init definitions if needed (lazy load)
        if (m_tableDefinitions.isEmpty()) {
             m_tableDefinitions = ECUDefinition::getDefaultSpeeduinoTables();
        }
    }
    
    if (!m_tableDefinitions.contains(tableName)) {
        Logger::error("Unknown table: " + tableName);
        return;
    }
    
    ECUDefinition::Table table = m_tableDefinitions[tableName];
    int size = table.rows * table.cols * table.elementSize;
    
    Logger::info(QString("Reading table %1 (Page %2, Offset %3, Size %4)")
                 .arg(tableName).arg(table.page).arg(table.address).arg(size));
                 
    if (m_serialManager && m_serialManager->isConnected()) {
        int blockSize = m_serialManager->getSignature().tableBlockingFactor;
        if (blockSize <= 0) blockSize = 64;
        
        for (int offset = 0; offset < size; offset += blockSize) {
            int chunkSize = qMin(blockSize, size - offset);
            m_serialManager->readTable(table.page, table.address + offset, chunkSize);
        }
    } else {
        // Offline mode: Generate dummy data so the UI doesn't look broken
        Logger::info("Offline mode: Generating dummy data for " + tableName);
        QVector<QVector<double>> dummyData(table.rows, QVector<double>(table.cols, 0.0));
        
        // Make it look somewhat realistic based on the table
        for (int r = 0; r < table.rows; ++r) {
            for (int c = 0; c < table.cols; ++c) {
                if (tableName.contains("VE")) {
                    dummyData[r][c] = 40.0 + (r * 2) + (c * 2); // 40-100 range
                } else if (tableName.contains("Ignition")) {
                    dummyData[r][c] = 10.0 + (c * 1.5) - (r * 0.5); // 10-35 range
                } else if (tableName.contains("AFR")) {
                    dummyData[r][c] = 14.7 - (r * 0.1) - (c * 0.1); // 11-15 range
                } else {
                    dummyData[r][c] = 0.0;
                }
            }
        }
        
        m_tableData[tableName] = dummyData;
        emit tableDataReceived(tableName, dummyData);
    }
}

void ECUSettingsManager::writeTable(const QString &tableName, const QVector<QVector<double>> &data) {
    if (!m_tableDefinitions.contains(tableName)) return;
    
    ECUDefinition::Table def = m_tableDefinitions[tableName];
    
    // Validate dimensions
    if (data.size() != def.rows || (data.size() > 0 && data[0].size() != def.cols)) {
        Logger::error("Table dimension mismatch for " + tableName);
        return;
    }
    
    // Flatten data
    QByteArray bytes;
    for (int r = 0; r < def.rows; ++r) {
        for (int c = 0; c < def.cols; ++c) {
            double userVal = data[r][c];
            // Raw = (User / Scale) - Translate
            double rawValDouble = (userVal - def.translate) / def.scale;
            int rawVal = static_cast<int>(qRound(rawValDouble));
            
            // Apply limits based on element size
            if (def.elementSize == 1) {
                if (rawVal < 0) rawVal = 0;
                if (rawVal > 255) rawVal = 255;
                bytes.append(static_cast<char>(rawVal));
            } else {
                // U16 Little Endian
                if (rawVal < 0) rawVal = 0; 
                if (rawVal > 65535) rawVal = 65535;
                bytes.append(static_cast<char>(rawVal & 0xFF));
                bytes.append(static_cast<char>((rawVal >> 8) & 0xFF));
            }
        }
    }
    
    if (m_serialManager && m_serialManager->isConnected()) {
        int blockSize = m_serialManager->getSignature().blockingFactor; // tables use writeBlock size usually, or page write
        if (blockSize <= 0) blockSize = 64;
        
        for (int offset = 0; offset < bytes.size(); offset += blockSize) {
            int chunkSize = qMin(blockSize, int(bytes.size()) - offset);
            m_serialManager->writeTable(def.page, def.address + offset, bytes.mid(offset, chunkSize));
        }
        
        // Schedule burn
        m_pendingBurnPage = def.page;
        m_burnDelayTimer->start();
    }
    
    // Update local cache
    m_tableData[tableName] = data;
    
    // Update pageCache so that individual settings don't overwrite table changes
    if (def.page < MAX_PAGES && m_pageLoaded[def.page] && def.address + bytes.size() <= m_pageCache[def.page].size()) {
        m_pageCache[def.page].replace(def.address, bytes.size(), bytes);
    }
}

void ECUSettingsManager::extractTablesFromPage(quint8 page) {
    if (m_tableDefinitions.isEmpty()) {
         m_tableDefinitions = ECUDefinition::getDefaultSpeeduinoTables();
    }
    
    if (page >= MAX_PAGES || !m_pageLoaded[page]) return;
    const QByteArray &pageData = m_pageCache[page];

    for (auto it = m_tableDefinitions.begin(); it != m_tableDefinitions.end(); ++it) {
        // Check if table is on this page
        if (it.value().page == page) {
            ECUDefinition::Table def = it.value();
            int size = def.rows * def.cols * def.elementSize;
            
            // Allow partial pages if we have enough data for this table
            if (def.address + size <= pageData.size()) {
                 // Parse data
                QVector<QVector<double>> parsedData(def.rows, QVector<double>(def.cols));
                int byteIdx = def.address;
                
                for (int r = 0; r < def.rows; ++r) {
                    for (int c = 0; c < def.cols; ++c) {
                        int rawVal = 0;
                        if (def.elementSize == 1) {
                            rawVal = static_cast<quint8>(pageData[byteIdx]);
                            byteIdx++;
                        } else {
                            // U16 LE
                            rawVal = static_cast<quint8>(pageData[byteIdx]) | (static_cast<quint8>(pageData[byteIdx+1]) << 8);
                            byteIdx += 2;
                        }
                        
                        double userVal = (rawVal * def.scale) + def.translate;
                        parsedData[r][c] = userVal;
                    }
                }
                
                m_tableData[it.key()] = parsedData;
                emit tableDataReceived(it.key(), parsedData);
            }
        }
    }
}

QVariant ECUSettingsManager::getTableValue(const QString &tableName, int row, int col) {
    if (m_tableData.contains(tableName)) {
        if (row < m_tableData[tableName].size() && col < m_tableData[tableName][row].size()) {
            return m_tableData[tableName][row][col];
        }
    }
    return QVariant();
}

QStringList ECUSettingsManager::getTableNames() const {
    return m_tableDefinitions.keys();
}

bool ECUSettingsManager::loadDefinition(const QString &iniPath) {
    if (m_ecuDef.load(iniPath)) {
        m_definitions = m_ecuDef.getConstants();
        m_tableDefinitions = m_ecuDef.getTables();
        Logger::info(QString("ECUSettingsManager: Loaded %1 settings and %2 tables from INI")
            .arg(m_definitions.size())
            .arg(m_tableDefinitions.size()));
        emit definitionsLoaded();
        return true;
    }
    return false;
}

void ECUSettingsManager::injectMsqData(const QMap<QString, QString> &constants) {
    for (auto it = constants.constBegin(); it != constants.constEnd(); ++it) {
        QString name = it.key();
        QString valStr = it.value();
        
        if (m_definitions.contains(name)) {
            // It's a scalar or bit
            bool ok;
            double val = valStr.toDouble(&ok);
            if (ok) {
                m_values[name] = val;
                emit settingChanged(name, val);
            }
        } else if (m_tableDefinitions.contains(name)) {
            // It's a table, valStr holds a grid of numbers separated by whitespace
            ECUDefinition::Table tableDef = m_tableDefinitions[name];
            QStringList tokens = valStr.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (tokens.size() >= tableDef.rows * tableDef.cols) {
                QVector<QVector<double>> tableData(tableDef.rows, QVector<double>(tableDef.cols));
                int idx = 0;
                for (int r = 0; r < tableDef.rows; ++r) {
                    for (int c = 0; c < tableDef.cols; ++c) {
                        tableData[r][c] = tokens[idx++].toDouble();
                    }
                }
                m_tableData[name] = tableData;
                emit tableDataReceived(name, tableData);
            }
        }
    }
    Logger::info("ECUSettingsManager: Injected MSQ values into cache");
}
