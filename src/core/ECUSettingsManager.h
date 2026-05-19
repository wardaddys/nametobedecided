/**
 * @file ECUSettingsManager.h
 * @brief ECU Settings Manager for reading/writing settings to ECU
 *
 * This class manages the communication between UI settings controls
 * and the actual ECU memory via serial protocol.
 */

#ifndef ECUSETTINGSMANAGER_H
#define ECUSETTINGSMANAGER_H

#include "ECUDefinition.h"
#include "ECUData.h"
#include <QObject>
#include <QHash>
#include <QSet>
#include <QVariant>
#include <QByteArray>
#include <QTimer>

class SerialManager;

/**
 * @brief Manages ECU settings read/write operations
 * 
 * Provides a high-level interface for:
 * - Caching ECU page data
 * - Converting between raw ECU values and user-displayable values
 * - Reading settings from ECU on connect
 * - Writing individual settings on UI change
 * - Burn commands to persist changes to flash
 */
class ECUSettingsManager : public QObject {
    Q_OBJECT

    friend class ECUSettingsManagerGuardrailsTest;

public:
    static const int MAX_PAGES = 16;
    static const int MAX_PAGE_SIZE = 288;
    
    explicit ECUSettingsManager(QObject *parent = nullptr);
    ~ECUSettingsManager();
    
    /**
     * @brief Set the serial manager for communication
     */
    void setSerialManager(SerialManager *serialManager);
    
    /**
     * @brief Check if settings have been loaded from ECU
     */
    bool isLoaded() const { return m_isLoaded; }
    
    // ========== Value Access ==========
    
    /**
     * @brief Get a setting value (user-scaled)
     * @param settingName Setting identifier
     * @return User-displayable value, or invalid QVariant if not found
     */
    QVariant getValue(const QString &settingName) const;
    
    /**
     * @brief Get raw ECU value for a setting
     * @param settingName Setting identifier
     * @return Raw byte value from ECU
     */
    int getRawValue(const QString &settingName) const;
    
    /**
     * @brief Set a setting value (will be written to ECU)
     * @param settingName Setting identifier
     * @param value User-scale value
     * @param autoWrite If true, immediately write to ECU
     */
    void setValue(const QString &settingName, const QVariant &value, bool autoWrite = true);
    
    /**
     * @brief Get the definition for a setting
     */
    ECUDefinition::Constant getDefinition(const QString &settingName) const;

    /**
     * @brief Check if a setting exists
     */
    bool hasSetting(const QString &settingName) const;

    // F2: Additive const accessors used by MsqParser::save() and other tools
    // that need to walk every setting / table without reaching into private
    // members.  None of these change existing behaviour.
    QStringList getAllConstantNames() const;
    QStringList getPcVariableNames() const;
    QVector<QVector<double>> getTableData(const QString &tableName) const;
    QMap<QString, bool> getActiveConditionalFlags() const;
    bool isPcVariable(const QString &settingName) const;
    const ECUDefinition &definition() const { return m_ecuDef; }
    
    // ========== ECU Operations ==========
    
    /**
     * @brief Load settings from a TunerStudio INI file
     */
    bool loadDefinition(const QString &iniPath);
    
    /**
     * @brief Inject parsed MSQ data into the cache
     */
    void injectMsqData(const QMap<QString, QString> &constants);
    
    // ========== ECU Operations ==========
    
    /**
     * @brief Read all settings pages from ECU
     */
    void readAllFromECU();
    
    /**
     * @brief Read a specific page from ECU
     */
    void readPageFromECU(quint8 page);
    
    void writeToECU(const QString &settingName);
    
    // ========== Table Operations ==========
    QStringList getTableNames() const;
    void readTable(const QString &tableName);
    void writeTable(const QString &tableName, const QVector<QVector<double>> &data);
    QVariant getTableValue(const QString &tableName, int row, int col);
    
    /**
     * @brief Burn a page to flash (persist changes)
     */
    void burnPage(quint8 page);
    
    /**
     * @brief Burn all dirty pages to flash
     */
    void burnAllDirty();
    
    /**
     * @brief Get list of pages with pending changes
     */
    QList<quint8> getDirtyPages() const;
    
signals:
    /**
     * @brief Emitted when definitions (INI/MSQ) finish loading
     */
    void definitionsLoaded();
    
    /**
     * @brief Emitted when a setting value changes
     */
    void settingChanged(const QString &name, const QVariant &value);
    
    /**
     * @brief Emitted when all pages have been read
     */
    void readComplete();
    
    /**
     * @brief Emitted when a page read completes
     */
    void pageReadComplete(quint8 page);
    
    /**
     * @brief Emitted when a write operation completes
     */
    void writeComplete(const QString &settingName);
    
    /**
     * @brief Emitted when burn completes
     */
    void burnComplete(quint8 page);

    /**
     * @brief Progress during burnAllDirty chain: current/total pages
     */
    void burnAllProgress(int current, int total);

    /**
     * @brief Emitted when burnAllDirty chain finishes (all pages done)
     */
    void burnAllComplete();

    /**
     * @brief Emitted when burnAllDirty chain fails on a specific page
     */
    void burnAllFailed(quint8 page, const QString &reason);
    
    void errorOccurred(const QString &error);
    
    void tableDataReceived(const QString &tableName, const QVector<QVector<double>> &data);
    
    /**
     * @brief Emitted when loading progress changes (0-100)
     */
    void loadProgress(int percent);

    /**
     * @brief Emitted when burn verification fails
     */
    void burnVerificationFailed(quint8 page, const QString &reason);

private slots:
    void onTableResponseReceived(quint8 table, quint16 offset, const QByteArray &data);
    void onSerialConnected();
    void onSerialDisconnected();
    void onDataReceived(const RealTimeData &data); // For burn polling
    void onBurnVerifyTimeout();
    void onPageCRCReceived(quint8 page, uint32_t crc);

private:
    void initializeSettings();
    bool hasDefinitionLoaded() const;
    int pageSizeFor(quint8 page) const;
    int constantByteSize(const ECUDefinition::Constant &def) const;
    bool validateConstantBounds(const ECUDefinition::Constant &def, QString *error = nullptr) const;
    bool validateTableBounds(const ECUDefinition::Table &def, QString *error = nullptr) const;
    bool validateConstantCollisions(const QMap<QString, ECUDefinition::Constant> &definitions,
                                    QString *error = nullptr) const;
    void extractSettingsFromPage(quint8 page);
    void extractTablesFromPage(quint8 page); // Added
    QByteArray encodeValue(const ECUDefinition::Constant &def, const QVariant &value) const;
    QVariant decodeValue(const ECUDefinition::Constant &def, const QByteArray &pageData) const;
    void markPageDirty(quint8 page);
    
    SerialManager *m_serialManager;
    ECUDefinition m_ecuDef;                      // Parsed INI data
    QMap<QString, ECUDefinition::Constant> m_definitions;
    QMap<QString, ECUDefinition::Table> m_tableDefinitions; // Added
    QHash<QString, QVariant> m_values;           // Cached user-scale values
    QMap<QString, QVector<QVector<double>>> m_tableData; // Added
    QSet<QString> m_pcVariableNames;             // F2: distinguish PC-only vars
    
    QByteArray m_pageCache[MAX_PAGES];
    bool m_pageDirty[MAX_PAGES];                 // Pages with pending writes
    bool m_pageLoaded[MAX_PAGES];                // Pages successfully read
    bool m_isLoaded;
    bool m_definitionLoaded;
    int m_chunksRequested;
    int m_chunksReceived;
    
    QTimer *m_burnDelayTimer;                    // Delay burn after writes
    QTimer *m_burnVerifyTimeoutTimer;            // Timeout for burn verification
    quint8 m_pendingBurnPage;
    bool m_isVerifyingBurn;                      // Is true while polling status4
    quint32 m_expectedPageCRC;                   // Computed CRC to compare against ECU

    // Chained burn-all state (BUG-A fix)
    QList<quint8> m_burnQueue;                   // Ordered list of dirty pages to burn
    bool m_burnAllInProgress = false;            // True while chained burn is running
    void advanceBurnQueue();                     // Process next page in burn chain
};

#endif // ECUSETTINGSMANAGER_H
