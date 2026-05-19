/**
 * @file ECUSettingsManager.cpp
 * @brief Implementation of ECUSettingsManager
 */

#include "ECUSettingsManager.h"
#include "SerialManager.h"
#include "Ms1ExtraNameMap.h"
#include "../utils/Logger.h"
#include <QDebug>
#include <QRegularExpression>
#include <cstring>  // memcpy for F32 IEEE 754 conversion

ECUSettingsManager::ECUSettingsManager(QObject *parent)
    : QObject(parent)
    , m_serialManager(nullptr)
    , m_isLoaded(false)
    , m_chunksRequested(0)
    , m_chunksReceived(0)
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
    // Equip static default definitions to allow Simulator/Fallback execution
    m_definitions = ECUDefinition::getDefaultSpeeduinoConstants();
    m_tableDefinitions = ECUDefinition::getDefaultSpeeduinoTables();
    m_values.clear();
    m_tableData.clear();
    
    // Set definition block gate to openly allow Simulator queries.
    m_definitionLoaded = true; 
    Logger::info("ECUSettingsManager initialized with fallback Speeduino mappings.");
}

bool ECUSettingsManager::hasDefinitionLoaded() const {
    return m_definitionLoaded && !m_definitions.isEmpty();
}

int ECUSettingsManager::pageSizeFor(quint8 page) const {
    const auto &pages = m_ecuDef.getPages();
    if (pages.contains(page) && pages.value(page).size > 0) {
        return pages.value(page).size;
    }

    // Safe fallback for Speeduino page map when INI page metadata is absent.
    static const int kFallbackPageSizes[] = {
        0, 768, 288, 288, 128, 288, 128, 240, 384, 288, 192, 288, 192, 128, 288, 256
    };
    if (page >= 1 && page <= 15) {
        return kFallbackPageSizes[page];
    }

    return MAX_PAGE_SIZE;
}

int ECUSettingsManager::constantByteSize(const ECUDefinition::Constant &def) const {
    if (def.paramClass == "bits") {
        return 1;
    }

    int typeSize = 0;
    if (def.type == "U08" || def.type == "S08") typeSize = 1;
    else if (def.type == "U16" || def.type == "S16") typeSize = 2;
    else if (def.type == "U32" || def.type == "S32" || def.type == "F32") typeSize = 4;

    if (typeSize == 0) return 0;

    if (def.paramClass == "array") {
        return typeSize * def.cols * def.rows;
    }

    return typeSize;
}

bool ECUSettingsManager::validateConstantBounds(const ECUDefinition::Constant &def, QString *error) const {
    if (def.page < 1 || def.page >= MAX_PAGES) {
        if (error) {
            // Speeduino pages start from 1. Page 0 is often used for metadata/INI info 
            // that is NOT a real ECU setting. We only warn for pages > 0 to avoid floods.
            if (def.page > 0) {
                *error = QString("Invalid page %1 for constant '%2'").arg(def.page).arg(def.name);
            } else {
                return false; // Silently skip page 0
            }
        }
        return false;
    }

    const int size = constantByteSize(def);
    if (size <= 0) {
        if (error) {
            *error = QString("Unsupported type '%1' for constant '%2'").arg(def.type).arg(def.name);
        }
        return false;
    }

    if (def.offset < 0) {
        if (error) {
            *error = QString("Negative offset %1 for constant '%2'").arg(def.offset).arg(def.name);
        }
        return false;
    }

    if (def.paramClass == "bits") {
        if (def.bitField.lowBit < 0 || def.bitField.lowBit > 7 ||
            def.bitField.highBit < 0 || def.bitField.highBit > 7 ||
            def.bitField.lowBit > def.bitField.highBit) {
            if (error) {
                *error = QString("Invalid bit range [%1:%2] for constant '%3'")
                             .arg(def.bitField.lowBit)
                             .arg(def.bitField.highBit)
                             .arg(def.name);
            }
            return false;
        }
    }

    const int pageSize = pageSizeFor(static_cast<quint8>(def.page));
    if ((def.offset + size) > pageSize) {
        if (error) {
            *error = QString("Out-of-bounds constant '%1': page %2 offset %3 size %4 exceeds page size %5")
                         .arg(def.name)
                         .arg(def.page)
                         .arg(def.offset)
                         .arg(size)
                         .arg(pageSize);
        }
        return false;
    }

    return true;
}

bool ECUSettingsManager::validateTableBounds(const ECUDefinition::Table &def, QString *error) const {
    if (def.page <= 0 || def.page >= MAX_PAGES) {
        if (error) {
            *error = QString("Invalid table page %1 for '%2'").arg(def.page).arg(def.name);
        }
        return false;
    }

    if (def.rows <= 0 || def.cols <= 0) {
        if (error) {
            *error = QString("Invalid table dimensions %1x%2 for '%3'")
                         .arg(def.rows).arg(def.cols).arg(def.name);
        }
        return false;
    }

    if (def.elementSize != 1 && def.elementSize != 2) {
        if (error) {
            *error = QString("Invalid table element size %1 for '%2'")
                         .arg(def.elementSize).arg(def.name);
        }
        return false;
    }

    if (def.address < 0) {
        if (error) {
            *error = QString("Negative table address %1 for '%2'")
                         .arg(def.address).arg(def.name);
        }
        return false;
    }

    if (qFuzzyIsNull(def.scale)) {
        if (error) {
            *error = QString("Invalid table scale 0 for '%1'").arg(def.name);
        }
        return false;
    }

    const int byteCount = def.rows * def.cols * def.elementSize;
    const int pageSize = pageSizeFor(static_cast<quint8>(def.page));
    if ((def.address + byteCount) > pageSize) {
        if (error) {
            *error = QString("Out-of-bounds table '%1': page %2 address %3 size %4 exceeds page size %5")
                         .arg(def.name)
                         .arg(def.page)
                         .arg(def.address)
                         .arg(byteCount)
                         .arg(pageSize);
        }
        return false;
    }

    return true;
}

bool ECUSettingsManager::validateConstantCollisions(
    const QMap<QString, ECUDefinition::Constant> &definitions,
    QString *error) const {
    
    struct ConstantRange {
        int start;
        int end;
        QString name;
        QString paramClass;
    };

    QMap<int, QList<ConstantRange>> rangesByPage;

    for (auto it = definitions.constBegin(); it != definitions.constEnd(); ++it) {
        const ECUDefinition::Constant &def = it.value();

        QString validationError;
        if (!validateConstantBounds(def, &validationError)) {
            continue;
        }

        const int size = constantByteSize(def);
        ConstantRange candidate{def.offset, def.offset + size - 1, def.name, def.paramClass};

        QList<ConstantRange> &pageRanges = rangesByPage[def.page];
        for (const ConstantRange &existing : pageRanges) {
            const bool overlaps = candidate.start <= existing.end && existing.start <= candidate.end;
            if (overlaps) {
                // [INTENTIONAL OVERLAP / ALIASING]
                // 1. Exact same range: This is an alias (two names for the same memory).
                // 2. Bit-fields: Allowed to overlap with each other or their parent scalar.
                bool isAlias = (candidate.start == existing.start && candidate.end == existing.end);
                bool isBitOverlap = (candidate.paramClass == "bits" || existing.paramClass == "bits");

                if (isAlias || isBitOverlap) {
                    continue;
                }
                
                // [CRIT-10] Downgrade to warning: Many INIs have intentional or 
                // legacy overlaps that TunerStudio permits. Do not block project load.
                Logger::warning(QString(
                                 "Memory overlap detected on page %1: '%2' [%3..%4] and '%5' [%6..%7]. "
                                 "This is common in some INIs and will be allowed.")
                                 .arg(def.page)
                                 .arg(candidate.name)
                                 .arg(candidate.start)
                                 .arg(candidate.end)
                                 .arg(existing.name)
                                 .arg(existing.start)
                                 .arg(existing.end));
            }
        }

        pageRanges.append(candidate);
    }

    return true;
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
    if (!hasDefinitionLoaded()) {
        Logger::error("ECUSettingsManager: No definition loaded. Blocking ECU page reads.");
        emit errorOccurred("No ECU definition loaded. Load a project INI before connecting.");
        return;
    }

    Logger::info("ECUSettingsManager: Serial connected, reading settings from loaded definition...");
    // [FIX-QUEUE] Do NOT auto-read all pages on connect.
    // Page reads flood the command queue and prevent RT data polling.
    // The user can manually trigger page reads via the toolbar.
    // readAllFromECU();
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
    if (!validateConstantBounds(def)) {
        return 0;
    }

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

    if (!hasDefinitionLoaded()) {
        emit errorOccurred("No ECU definition loaded. Cannot read ECU pages safely.");
        return;
    }
    
    m_chunksRequested = 0;
    m_chunksReceived = 0;
    m_isLoaded = false;
    
    // Determine which pages need to be read based on settings
    QSet<quint8> pagesToRead;
    for (const auto &def : m_definitions) {
        QString validationError;
        if (!validateConstantBounds(def, &validationError)) {
            Logger::warning("Skipping invalid constant during readAllFromECU: " + validationError);
            continue;
        }
        pagesToRead.insert(def.page);
    }
    
    int blockSize = m_serialManager->getSignature().tableBlockingFactor;
    if (blockSize <= 0 || blockSize > 256) blockSize = 121; // Safe Speeduino default
    
    for (quint8 page : pagesToRead) {
        int pageSize = pageSizeFor(page);
        m_chunksRequested += (pageSize + blockSize - 1) / blockSize;
    }
    
    for (quint8 page : pagesToRead) {
        readPageFromECU(page);
    }
    
    emit loadProgress(0);
}

void ECUSettingsManager::readPageFromECU(quint8 page) {
    if (!m_serialManager) return;

    if (!hasDefinitionLoaded()) {
        Logger::error("readPageFromECU blocked: no definition loaded");
        return;
    }
    
    int pageSize = pageSizeFor(page);
    
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
    
    // Safely insert chunk without truncating the rest of the page
    int requiredSize = offset + data.size();
    while (m_pageCache[table].size() < requiredSize) {
        m_pageCache[table].append('\0');
    }
    m_pageCache[table].replace(offset, data.size(), data);
    
    m_pageLoaded[table] = true;
    m_chunksReceived++;
    
    // Log chunk received (using qDebug for verbose logging)
    qDebug() << "Received chunk for page" << table << "(" << data.size() << "bytes)";
    
    // Extract settings from this page
    extractSettingsFromPage(table);
    extractTablesFromPage(table); // Added call
    
    emit pageReadComplete(table);
    
    // Update progress
    if (m_chunksRequested > 0) {
        int progress = (m_chunksReceived * 100) / m_chunksRequested;
        emit loadProgress(progress);
        
        if (m_chunksReceived == m_chunksRequested && !m_isLoaded) {
            m_isLoaded = true;
            Logger::info("All settings loaded from ECU");
            
            // BUG-005 FIX: Also read core tables (VE, Ignite, AFR) on connect
            readTable("VE Table (Volumetric Efficiency)");
            readTable("Ignition Advance Table");
            readTable("AFR Target Table");
            
            emit readComplete();
        }
    }
}

void ECUSettingsManager::extractSettingsFromPage(quint8 page) {
    for (auto it = m_definitions.constBegin(); it != m_definitions.constEnd(); ++it) {
        if (it.value().page == page) {
            QString validationError;
            if (!validateConstantBounds(it.value(), &validationError)) {
                Logger::warning("Skipping invalid constant during extraction: " + validationError);
                continue;
            }
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
        rawValue = static_cast<int>(
            static_cast<quint8>(pageData.at(def.offset)) |
            (static_cast<quint16>(static_cast<quint8>(pageData.at(def.offset + 1))) << 8));  // LE
    } else if (def.type == "S16" && def.offset + 1 < pageData.size()) {
        rawValue = static_cast<qint16>(
            static_cast<quint8>(pageData.at(def.offset)) |
            (static_cast<quint16>(static_cast<quint8>(pageData.at(def.offset + 1))) << 8));  // LE
    } else if (def.type == "U32" && def.offset + 3 < pageData.size()) {
        // [CRIT-4] U32 little-endian extraction
        rawValue = static_cast<int>(
            static_cast<quint32>(static_cast<quint8>(pageData.at(def.offset)))
            | (static_cast<quint32>(static_cast<quint8>(pageData.at(def.offset + 1))) << 8)
            | (static_cast<quint32>(static_cast<quint8>(pageData.at(def.offset + 2))) << 16)
            | (static_cast<quint32>(static_cast<quint8>(pageData.at(def.offset + 3))) << 24));
    } else if (def.type == "S32" && def.offset + 3 < pageData.size()) {
        // [CRIT-4] S32 little-endian extraction
        quint32 uval = static_cast<quint32>(static_cast<quint8>(pageData.at(def.offset)))
                     | (static_cast<quint32>(static_cast<quint8>(pageData.at(def.offset + 1))) << 8)
                     | (static_cast<quint32>(static_cast<quint8>(pageData.at(def.offset + 2))) << 16)
                     | (static_cast<quint32>(static_cast<quint8>(pageData.at(def.offset + 3))) << 24);
        rawValue = static_cast<qint32>(uval);
    } else if (def.type == "F32" && def.offset + 3 < pageData.size()) {
        // [CRIT-4] F32 IEEE 754 little-endian extraction — bypass rawToUser
        quint32 bits = static_cast<quint32>(static_cast<quint8>(pageData.at(def.offset)))
                     | (static_cast<quint32>(static_cast<quint8>(pageData.at(def.offset + 1))) << 8)
                     | (static_cast<quint32>(static_cast<quint8>(pageData.at(def.offset + 2))) << 16)
                     | (static_cast<quint32>(static_cast<quint8>(pageData.at(def.offset + 3))) << 24);
        float fval;
        std::memcpy(&fval, &bits, sizeof(float));
        return QVariant(static_cast<double>(fval));  // F32 already in user units
    } else if (def.paramClass == "bits") {
        quint8 byte = static_cast<quint8>(pageData.at(def.offset));
        quint8 mask = static_cast<quint8>(
            ((1 << (def.bitField.highBit - def.bitField.lowBit + 1)) - 1) << def.bitField.lowBit);
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

    if (!hasDefinitionLoaded()) {
        emit errorOccurred("No ECU definition loaded. Write blocked for safety.");
        return;
    }
    
    if (!m_definitions.contains(settingName) || !m_values.contains(settingName)) {
        emit errorOccurred("Unknown setting: " + settingName);
        return;
    }
    
    const auto &def = m_definitions[settingName];

    QString validationError;
    if (!validateConstantBounds(def, &validationError)) {
        emit errorOccurred("Write blocked by definition guardrail: " + validationError);
        Logger::error("Write blocked by definition guardrail: " + validationError);
        return;
    }

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
    
    // [CRIT-4] F32 encoding path — uses memcpy, bypasses integer rawValue
    if (def.type == "F32") {
        float fval = static_cast<float>(value.toDouble());
        quint32 bits;
        std::memcpy(&bits, &fval, sizeof(float));
        data.append(static_cast<char>(bits & 0xFF));
        data.append(static_cast<char>((bits >> 8) & 0xFF));
        data.append(static_cast<char>((bits >> 16) & 0xFF));
        data.append(static_cast<char>((bits >> 24) & 0xFF));
        return data;
    }
    
    int rawValue = def.userToRaw(value.toDouble());
    
    if (def.type == "U08" || def.type == "S08") {
        data.append(static_cast<char>(rawValue & 0xFF));
    } else if (def.type == "U16" || def.type == "S16") {
        data.append(static_cast<char>(rawValue & 0xFF));         // Low byte first (LE)
        data.append(static_cast<char>((rawValue >> 8) & 0xFF));  // High byte second
    } else if (def.type == "U32" || def.type == "S32") {
        // [CRIT-4] U32/S32 little-endian encoding
        data.append(static_cast<char>(rawValue & 0xFF));
        data.append(static_cast<char>((rawValue >> 8) & 0xFF));
        data.append(static_cast<char>((rawValue >> 16) & 0xFF));
        data.append(static_cast<char>((rawValue >> 24) & 0xFF));
    } else if (def.paramClass == "bits") {
        // For bits, we need to read-modify-write
        quint8 currentByte = 0;
        if (m_pageLoaded[def.page] && def.offset < m_pageCache[def.page].size()) {
            currentByte = static_cast<quint8>(m_pageCache[def.page].at(def.offset));
        }
        
        quint8 mask = static_cast<quint8>(
            ((1 << (def.bitField.highBit - def.bitField.lowBit + 1)) - 1) << def.bitField.lowBit);
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

    // BUG-A fix: if chained burn is in progress, surface failure and stop the chain
    if (m_burnAllInProgress) {
        m_burnAllInProgress = false;
        m_burnQueue.clear();
        emit burnAllFailed(m_pendingBurnPage, "Burn verification timed out");
    }
}

void ECUSettingsManager::burnAllDirty() {
    if (m_burnAllInProgress) {
        Logger::warning("burnAllDirty() called while chain burn already in progress — ignoring");
        return;
    }

    // Collect all dirty pages into the burn queue
    m_burnQueue.clear();
    for (int i = 0; i < MAX_PAGES; i++) {
        if (m_pageDirty[i]) {
            m_burnQueue.append(static_cast<quint8>(i));
        }
    }

    if (m_burnQueue.isEmpty()) {
        Logger::info("burnAllDirty: no dirty pages to burn");
        emit burnAllComplete();
        return;
    }

    Logger::info(QString("burnAllDirty: %1 dirty pages queued: %2")
                 .arg(m_burnQueue.size())
                 .arg([this]() {
                     QStringList pages;
                     for (quint8 p : m_burnQueue) pages << QString::number(p);
                     return pages.join(", ");
                 }()));

    m_burnAllInProgress = true;
    advanceBurnQueue();
}

void ECUSettingsManager::advanceBurnQueue() {
    if (m_burnQueue.isEmpty()) {
        // All pages burned successfully
        m_burnAllInProgress = false;
        Logger::info("burnAllDirty: all pages burned successfully");
        emit burnAllComplete();
        return;
    }

    quint8 nextPage = m_burnQueue.first();
    int remaining = m_burnQueue.size();
    int total = remaining; // Approximate — we don't track the original count here

    // For accurate progress, we compute from what's left vs what was originally queued.
    // Since burnPage() clears m_pageDirty, we can't recount. But we emit current position.
    emit burnAllProgress(1, remaining);

    Logger::info(QString("burnAllDirty: burning page %1 (%2 remaining)")
                 .arg(nextPage).arg(remaining));

    // burnPage() will set m_isVerifyingBurn = true and clear m_pageDirty[nextPage]
    burnPage(nextPage);
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

        // BUG-A fix: advance to next page in the chained burn queue
        if (m_burnAllInProgress) {
            m_burnQueue.removeFirst();
            advanceBurnQueue();
        }
    } else {
        Logger::error(QString("Burn verification failed for page %1! (Expected 0x%2, Got 0x%3)")
                      .arg(page).arg(m_expectedPageCRC, 8, 16, QChar('0'))
                      .arg(crc, 8, 16, QChar('0')));
        emit burnVerificationFailed(page, "CRC mismatch after burn");

        // BUG-A fix: stop the chain on CRC failure
        if (m_burnAllInProgress) {
            m_burnAllInProgress = false;
            m_burnQueue.clear();
            emit burnAllFailed(page, "CRC mismatch after burn");
        }
    }
}

void ECUSettingsManager::readTable(const QString &tableName) {
    if (!m_definitionLoaded) {
        Logger::error("Table read blocked: no runtime ECU definition loaded");
        return;
    }
    
    if (!m_tableDefinitions.contains(tableName)) {
        Logger::error("Unknown table: " + tableName);
        return;
    }
    
    ECUDefinition::Table table = m_tableDefinitions[tableName];
    QString validationError;
    if (!validateTableBounds(table, &validationError)) {
        Logger::error("Table read blocked by definition guardrail: " + validationError);
        return;
    }

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
        // Offline mode: prefer injected MSQ data over synthetic dummies.
        // injectMsqData() populates m_tableData when a project is loaded — we
        // must not overwrite that real data with zeroes/estimates here.
        if (m_tableData.contains(tableName)) {
            Logger::info("Offline mode: Re-emitting cached MSQ data for " + tableName);
            emit tableDataReceived(tableName, m_tableData[tableName]);
            return;
        }

        // No MSQ data available — generate dummy data so the UI isn't blank.
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
    QString validationError;
    if (!validateTableBounds(def, &validationError)) {
        Logger::error("Table write blocked by definition guardrail: " + validationError);
        return;
    }
    
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
            // [CRIT-3] Raw = (User - Translate) / Scale (TunerStudio INI spec)
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
    if (!m_definitionLoaded || m_tableDefinitions.isEmpty()) return;
    
    if (page >= MAX_PAGES || !m_pageLoaded[page]) return;
    const QByteArray &pageData = m_pageCache[page];

    for (auto it = m_tableDefinitions.begin(); it != m_tableDefinitions.end(); ++it) {
        // Check if table is on this page
        if (it.value().page == page) {
            ECUDefinition::Table def = it.value();
            QString validationError;
            if (!validateTableBounds(def, &validationError)) {
                Logger::warning("Skipping invalid table during extraction: " + validationError);
                continue;
            }

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
    if (!m_ecuDef.load(iniPath)) {
        const QString msg = QString("INI file could not be read or opened: %1").arg(iniPath);
        Logger::error("ECUSettingsManager: " + msg);
        emit errorOccurred(msg);
        return false;
    }
    {
        m_definitions = m_ecuDef.getConstants();
        m_tableDefinitions = m_ecuDef.getTables();
        m_definitionLoaded = !m_definitions.isEmpty();

        if (!m_definitionLoaded) {
            // Diagnostic context: what *did* the parser find?
            const int outputs = m_ecuDef.getOutputChannels().size();
            const int tables  = m_tableDefinitions.size();
            const QString sig = m_ecuDef.getSignature();
            const QString msg =
                QString("INI parsed but no Constants were extracted.\n"
                        "  signature       : %1\n"
                        "  output channels : %2\n"
                        "  tables          : %3\n\n"
                        "Likely cause: the INI uses #if / #set directives "
                        "guarded by options that aren't set in the loaded "
                        "[SettingGroups] context, or the [Constants] section "
                        "wasn't reached. Open %4 to inspect.")
                    .arg(sig.isEmpty() ? QStringLiteral("(none)") : sig)
                    .arg(outputs).arg(tables).arg(iniPath);
            Logger::error("ECUSettingsManager: " + msg);
            emit errorOccurred(msg);
            return false;
        }

        // Warn if the loaded INI is not a compatible Speeduino/MS2Extra firmware.
        // We continue loading (do not block) so the user can still view tables
        // via the name-mapping layer. The warning surfaces in the status bar and
        // via a non-modal errorOccurred signal so MainWindow can display it.
        if (!m_ecuDef.isSpeeduinoCompatible()) {
            const QString sig = m_ecuDef.getSignature();
            Logger::warning(QString("ECUSettingsManager: Non-Speeduino INI detected "
                                    "(signature: '%1'). MSQ data injection will use "
                                    "name-mapping — table values may be limited.")
                            .arg(sig));
            emit errorOccurred(
                QString("⚠ Non-Speeduino firmware detected: \"%1\"\n\n"
                        "This project uses an MS1/Extra (or other) firmware definition.\n"
                        "Table data will be loaded via name mapping — VE, Ignition and AFR "
                        "tables are supported. Other settings may show default values.")
                .arg(sig));
        }

        QString collisionError;
        if (!validateConstantCollisions(m_definitions, &collisionError)) {
            Logger::error("ECUSettingsManager: Definition guardrail failed - " + collisionError);
            emit errorOccurred(collisionError);
            return false;
        }

        if (m_serialManager) {
            // [PERF] Share the already-parsed definition with SerialManager
            // instead of re-parsing from disk. This eliminates the triple-load
            // bug where the INI was parsed 3 times on every project open.
            m_serialManager->setEcuDefinition(m_ecuDef);
        }

        Logger::info(QString("ECUSettingsManager: Loaded %1 settings and %2 tables from INI")
            .arg(m_definitions.size())
            .arg(m_tableDefinitions.size()));
        emit definitionsLoaded();
        return true;
    }
}

void ECUSettingsManager::injectMsqData(const QMap<QString, QString> &constants) {
    // Detect whether the loaded INI is MS1/Extra so we can apply the name-mapping
    // layer before trying to match against m_definitions / m_tableDefinitions.
    const bool isMs1 = Ms1ExtraNameMap::isMs1ExtraSignature(m_ecuDef.getSignature());

    int injectedScalars = 0;
    int injectedTables  = 0;

    for (auto it = constants.constBegin(); it != constants.constEnd(); ++it) {
        QString name = it.key();
        QString valStr = it.value();

        // --- MS1/Extra name translation -----------------------------------------
        if (isMs1) {
            // Try table name mapping first
            QString mappedTable = Ms1ExtraNameMap::toInternalTableName(name);
            if (!mappedTable.isEmpty()) {
                name = mappedTable;  // Will be matched by the table branch below
            } else {
                // Try scalar/constant name mapping
                QString mappedConst = Ms1ExtraNameMap::toInternalConstantName(name);
                if (!mappedConst.isEmpty()) {
                    name = mappedConst;  // Will be matched by the scalar branch below
                }
                // If neither map has it, fall through — the name might already be
                // compatible (e.g. pcVariables) or it will simply be silently skipped.
            }
        }
        // ------------------------------------------------------------------------

        if (m_definitions.contains(name)) {
            // It's a scalar or bit
            bool ok;
            double val = valStr.toDouble(&ok);
            if (ok) {
                m_values[name] = val;
                emit settingChanged(name, val);
                ++injectedScalars;
            }
        } else if (m_tableDefinitions.contains(name)) {
            // It's a table — valStr holds a grid of numbers separated by whitespace
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
                ++injectedTables;
            } else {
                Logger::warning(QString("ECUSettingsManager: Table '%1' token count %2 < expected %3x%4=%5 — skipped")
                    .arg(name).arg(tokens.size())
                    .arg(tableDef.rows).arg(tableDef.cols)
                    .arg(tableDef.rows * tableDef.cols));
            }
        }
    }
    Logger::info(QString("ECUSettingsManager: Injected %1 scalars and %2 tables from MSQ (MS1 mapping: %3)")
        .arg(injectedScalars).arg(injectedTables).arg(isMs1 ? "active" : "inactive"));
}

// ============================================================================
// F2: Additive const accessors used by MsqParser::save() (F3) and any other
// tool that needs to enumerate the full state without touching the existing
// public API.  All const; none mutate state.
// ============================================================================

QStringList ECUSettingsManager::getAllConstantNames() const {
    return m_definitions.keys();
}

QStringList ECUSettingsManager::getPcVariableNames() const {
    QStringList out;
    for (const QString& n : m_pcVariableNames) out.append(n);
    return out;
}

QVector<QVector<double>>
ECUSettingsManager::getTableData(const QString &tableName) const {
    return m_tableData.value(tableName);
}

QMap<QString, bool> ECUSettingsManager::getActiveConditionalFlags() const {
    return m_ecuDef.getConditions();
}

bool ECUSettingsManager::isPcVariable(const QString &settingName) const {
    return m_pcVariableNames.contains(settingName);
}
