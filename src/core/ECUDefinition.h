
#ifndef ECUDEFINITION_H
#define ECUDEFINITION_H

#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>
#include <QStack>
#include <QtMath>       // qRound — used by fixed-point scaling
#include <algorithm>    // std::clamp
#include "ECUData.h"

/**
 * @brief ECU Definition Parser for OS Tuner
 * 
 * Parses INI files with support for:
 * - [OutputChannels] section with scalar/bits types
 * - [Constants] section with scalar/bits/array types
 * - [ControllerCommands] section
 * - Bit field notation [lowBit:highBit] with "INVALID" suppression
 * - Conditional parsing (#if, #else, #elif, #endif)
 * - Directives (#set, #unset, #define)
 */
class ECUDefinition {
public:
  // Bit field definition
  struct BitField {
    int lowBit;
    int highBit;
    int offset;              // +1 offset modifier e.g., [0:4+1]
    QStringList options;     // String options for dropdown
    QVector<int> validValues; // Non-INVALID value indices
  };

  // Output channel (runtime data)
  struct OutputChannel {
    QString name;
    QString type;       // U08, S08, U16, S16, U32, S32, F32
    int offset;
    QString units;
    double scale;
    double translate;
    bool isBits = false;
    BitField bitField;
    QString expression; // For formula-based channels
    bool hidden = false;
  };

  // Constant (tunable parameter)
  struct Constant {
    QString name;
    QString paramClass;  // scalar, bits, array, string
    QString type;        // U08, S08, U16, S16, U32, S32, F32, ASCII
    int page = 0;        // Memory page number (added for settings management)
    int offset;
    QString units;
    double scale = 1.0;
    double translate = 0.0;
    double min = 0.0;
    double max = 255.0;
    int digits = 0;
    QString category;    // Category for grouping (e.g., "Engine", "Limiters")
    
    // Workspace presentation metadata
    QString primaryWorkspace;        // e.g. "fueling"
    QStringList secondaryWorkspaces; // workspaces that show this read-only
    QString subsection;              // e.g. "injector_hardware"
    int displayOrder = 0;            // for ordering within subsection
    QString humanLabel;              // user-facing label
    QString helpText;                // tooltip / contextual help
    
    // TunerStudio Extensions
    bool requiresPowerCycle = false;
    
    // For bits type
    BitField bitField;
    
    // For array type
    int cols = 1;
    int rows = 1;
    
    // Flags
    bool noMsqSave = false;
    bool controllerPriority = false;
    bool readOnly = false;
    
    // ================================================================
    //  Fixed-Point Scaling — "No-Bullshit" Accuracy Protocol
    //
    //  The Arduino Mega only stores raw integer bytes. Every value
    //  displayed in the UI MUST be the exact result of:
    //      userValue = (rawByte + translate) * scale
    //  And every value written back MUST round correctly:
    //      rawByte   = clamp( round( user/scale - translate ) )
    //
    //  Three guarantees:
    //    1. userToRaw uses qRound — no silent truncation
    //    2. rawClamp enforces wire-type limits (U08 → 0-255, etc.)
    //    3. quantize round-trips user→raw→user so the UI always
    //       shows the EXACT value the ECU will store
    // ================================================================

    /// Convert raw integer from ECU page data to user-facing double.
    double rawToUser(int rawValue) const {
      return (static_cast<double>(rawValue) * scale) + translate;
    }
    
    /// Convert user-facing double to raw integer for the ECU.
    /// Uses qRound (banker's rounding) — NOT truncation — so that
    /// 34.6° with scale=1 becomes raw=35, not raw=34.
    int userToRaw(double userValue) const {
      return rawClamp(qRound((userValue - translate) / scale));
    }
    
    /// Clamp a raw value to the valid range for this constant's wire type.
    /// Prevents byte overflow when writing to ECU page memory.
    int rawClamp(int raw) const {
      if (type == "U08")  return std::clamp(raw, 0, 255);
      if (type == "S08")  return std::clamp(raw, -128, 127);
      if (type == "U16")  return std::clamp(raw, 0, 65535);
      if (type == "S16")  return std::clamp(raw, -32768, 32767);
      // U32/S32/F32 — no practical clamp needed at int range
      return raw;
    }
    
    /// Quantize a user value to the nearest value representable by
    /// the ECU's raw integer storage.  UI MUST call this after every
    /// edit so the display matches what the ECU will actually store.
    ///
    ///   User types 34.6° → quantize → rawToUser(userToRaw(34.6))
    ///     = rawToUser(35)  = (35 + 0) * 1.0 = 35.0°
    ///   UI snaps to 35.0° — no desync.
    double quantize(double userValue) const {
      return rawToUser(userToRaw(userValue));
    }
    
    /// Byte size of the raw wire type (used by table element size inference).
    int byteSize() const {
      if (type == "U08" || type == "S08") return 1;
      if (type == "U16" || type == "S16") return 2;
      if (type == "U32" || type == "S32" || type == "F32") return 4;
      if (paramClass == "bits") return 1;
      return 1;
    }
  };

  // Controller command
  struct ControllerCommand {
    QString name;
    QList<QByteArray> commands;  // Chain of commands (parse-time best-effort bytes)
    // B1 fix: preserve raw templates and referenced variables so a runtime
    // resolver (with live page-cache values) can produce the correct bytes
    // when the command fires. Each entry parallels `commands[]`.
    QList<QString> commandTemplates;
    QList<QStringList> commandVariables;  // referenced \$varNames per template
  };

  // Page definition
  struct Page {
    int id;
    QString name;
    int size;
    int parameterStartOffset = 0;
    QByteArray pageReadCommand;
    QByteArray burnCommand;
    QByteArray pageChunkWrite;
  };

  // ===================================================================
  // Group C: data models for the new INI sections.
  // Each struct is additive — nothing existing depends on it. UI code
  // can ignore these maps until corresponding widgets are wired in.
  // ===================================================================

  // [SettingGroups] — variant selectors (e.g. Narrowband vs Wideband EGO).
  struct SettingGroupOption {
      QString name;       ///< Option name as used by #if (e.g. NARROW_BAND_EGO)
      QString label;      ///< User-facing label
  };
  struct SettingGroup {
      QString referenceName;    ///< Group ID
      QString displayName;
      QVector<SettingGroupOption> options;
      QString selectedOption;   ///< Default = first non-DEFAULT entry
  };

  // [CurveEditor] — 1D curves (WUE, AE, knock thresh, MAF, etc.)
  struct CurveAxis {
      QString arrayConstant;
      QString trackedChannel;
      bool    readOnly = false;
      QString activeExpr;
      QString lineLabel;
  };
  struct CurveEditor {
      QString name;
      QString title;
      QString xLabel;
      QString yLabel;
      QString xMinExpr;
      QString xMaxExpr;
      QString yMinExpr;
      QString yMaxExpr;
      int     xDivs = 0;
      int     yDivs = 0;
      CurveAxis xBins;
      QVector<CurveAxis> yBins;
      QString gauge;
      QString helpUrl;
      QString sizeExpr;
      bool    showTextValues = false;
      bool    suppressGraph = false;
  };

  // [GaugeConfigurations] — dashboard gauge templates.
  struct GaugeTemplate {
      QString name;
      QString channel;
      QString title;
      QString unitsExpr;
      QString loExpr;
      QString hiExpr;
      QString loDExpr;     ///< Low-danger threshold
      QString loWExpr;     ///< Low-warning threshold
      QString hiWExpr;     ///< High-warning threshold
      QString hiDExpr;     ///< High-danger threshold
      int     valueDecimals = 0;
      int     labelDecimals = 0;
      QString activeExpr;
      QString category;
  };

  // [Datalog] — datalog channel definitions.
  struct DatalogField {
      QString channel;
      QString label;
      QString format;       ///< printf-style or tag ("onOff", "yesNo", "hex", ...)
      QString enabledExpr;
      QString lagExpr;
      QString category;
      int     order = 0;
  };

  // [Menu] — navigation tree.
  struct MenuItem {
      enum Kind { SubMenu, GroupMenu, GroupChildMenu, Separator };
      Kind    kind = SubMenu;
      QString target;       ///< target dialog / curve / table / std_*
      QString label;
      int     page = 0;
      QString enableExpr;
      QString visibleExpr;
      QVector<MenuItem> children;
  };
  struct MenuDef {
      QString label;
      QString enableExpr;
      QString visibleExpr;
      QVector<MenuItem> items;
  };
  struct MenuDialog {
      QString name;
      QVector<MenuDef> menus;
  };

  // [VeAnalyze] and [WueAnalyze]
  struct AnalyzeFilter {
      QString name;
      QString label;
      QString channel;
      QString op;
      QString value;
      QString activeExpr;
      bool enabledByDefault = true;
  };
  struct VeAnalyzeMap {
      QString veTable;
      QString targetTable;
      QString lambdaChannel;
      QString egoCorrectionChannel;
      QString activeCondition;
      QStringList lambdaTargetTables;
      QVector<AnalyzeFilter> filters;
  };
  struct WueAnalyzeMap {
      QString wueCurveName;
      QString afrTempCompensationCurve;
      QString targetTable;
      QString lambdaChannel;
      QString coolantTempChannel;
      QString wueChannel;
      QString egoCorrectionChannel;
      QString activeCondition;
      QStringList lambdaTargetTables;
      QVector<AnalyzeFilter> filters;
  };

  // [UserDefined] — dialog layouts.
  struct DialogItem {
      enum Kind { Field, Panel, CommandButton, Gauge, Indicator };
      Kind    kind = Field;
      QString label;
      QString target;
      QString enableExpr;
      QString visibleExpr;
      bool    clickOnCloseIfEnabled = false;
  };
  struct Dialog {
      QString name;
      QString title;
      QString layoutAxis;
      QVector<DialogItem> items;
  };

  ECUDefinition();
  bool load(const QString &filePath);

  // Accessors
  const QMap<QString, OutputChannel> &getOutputChannels() const;
  const QMap<QString, Constant> &getConstants() const;
  const QMap<QString, ControllerCommand> &getControllerCommands() const;
  const QMap<int, Page> &getPages() const;
  
  QString getSignature() const { return m_signature; }
  bool isSpeeduinoCompatible() const;
  int getOutputChannelsSize() const { return m_outputChannelsSize; }
  
  // Apply workspace mappings to parsed constants and tables
  void applyWorkspaceMetadata();
  
  /**
   * @brief Validate if a received ECU signature matches this definition
   * 
   * @param receivedSig The signature received from the connected ECU
   * @return Validation result with detailed error message if mismatch
   */
  struct SignatureValidation {
    bool isValid;
    QString message;
    
    SignatureValidation(bool valid = true, const QString &msg = "") 
        : isValid(valid), message(msg) {}
  };
  
  SignatureValidation validateSignature(const ECUSignature &receivedSig) const;

  // Get default Speeduino constants when no INI file is loaded
  static QMap<QString, Constant> getDefaultSpeeduinoConstants();
  
  // Table definition
  struct Table {
      QString name;
      int page;
      int address; // Offset
      int rows;
      int cols;
      int elementSize; // 1 or 2 bytes
      QString xLabel;
      QString yLabel;
      QString zLabel;
      double scale;
      double translate;

      // B2 fix: resolved axis-bin info from [TableEditor]
      // xBins/yBins reference array constants in [Constants]. We now look
      // them up and surface enough info that the UI can show real axis
      // values instead of falling back to hardcoded guesses.
      QString xAxisConstant;     ///< name of the array Constant providing X bins
      QString yAxisConstant;     ///< name of the array Constant providing Y bins
      QString xAxisChannel;      ///< OutputChannel used as the X tracker (e.g. "rpm")
      QString yAxisChannel;      ///< OutputChannel used as the Y tracker (e.g. "map")
      int     xBinCount = 0;     ///< resolved column count for axis-aware UIs
      int     yBinCount = 0;     ///< resolved row count for axis-aware UIs
      double  xAxisScale = 1.0;
      double  yAxisScale = 1.0;
      double  xAxisTranslate = 0.0;
      double  yAxisTranslate = 0.0;
      QString xAxisUnits;
      QString yAxisUnits;

      // Workspace presentation metadata
      QString primaryWorkspace;
      QStringList secondaryWorkspaces;
      QString subsection;
      int displayOrder = 0;
      QString humanLabel;
      QString helpText;
  };
  static QMap<QString, Table> getDefaultSpeeduinoTables();
  
  const QMap<QString, Table> &getTables() const { return m_tableDefinitions; }

  // Alias used by tests and several call sites that predate getTables().
  const QMap<QString, Table> &getTableDefinitions() const { return m_tableDefinitions; }

  // ---- Group C accessors (additive) ----
  const QMap<QString, SettingGroup>  &getSettingGroups() const { return m_settingGroups; }
  const QMap<QString, Constant>      &getPcVariables()   const { return m_pcVariables; }
  const QVector<DatalogField>        &getDatalogFields() const { return m_datalogFields; }
  const QMap<QString, GaugeTemplate> &getGaugeTemplates() const { return m_gaugeTemplates; }
  const QMap<QString, CurveEditor>   &getCurveEditors()  const { return m_curveEditors; }
  const QMap<QString, MenuDialog>    &getMenuDialogs()   const { return m_menuDialogs; }
  const QMap<QString, Dialog>        &getDialogs()       const { return m_dialogs; }
  const QMap<QString, VeAnalyzeMap>  &getVeAnalyzeMaps() const { return m_veAnalyzeMaps; }
  const QMap<QString, WueAnalyzeMap> &getWueAnalyzeMaps()const { return m_wueAnalyzeMaps; }

  // Directive state (for #if/#set)
  bool isConditionActive(const QString& name) const;
  const QMap<QString, bool> &getConditions() const { return m_conditions; }
  
  // Type size helper (public for use by protocol parsers)
  int getTypeSize(const QString &type) const;

private:
  // Data storage
  QMap<QString, OutputChannel> m_outputChannels;
  QMap<QString, Constant> m_constants;
  QMap<QString, ControllerCommand> m_controllerCommands;
  QMap<int, Page> m_pages;
  QMap<QString, Table> m_tableDefinitions; // Added
  QString m_signature;
  int m_outputChannelsSize = 0;

  // Group C storage — populated by the new section parsers.
  QMap<QString, SettingGroup>  m_settingGroups;
  QMap<QString, Constant>      m_pcVariables;
  QVector<DatalogField>        m_datalogFields;
  QMap<QString, GaugeTemplate> m_gaugeTemplates;
  QMap<QString, CurveEditor>   m_curveEditors;
  QMap<QString, MenuDialog>    m_menuDialogs;
  QMap<QString, Dialog>        m_dialogs;
  QMap<QString, VeAnalyzeMap>  m_veAnalyzeMaps;
  QMap<QString, WueAnalyzeMap> m_wueAnalyzeMaps;

  // Conditional parsing state
  QMap<QString, bool> m_conditions;       // #set/#unset directives
  QMap<QString, QString> m_defines;       // #define macros
  QStack<bool> m_conditionalStack;        // #if nesting state (current branch active?)
  // Parallel stack: has ANY branch in the current #if/#elif/#else chain
  // already matched? Used to enforce C-preprocessor "first-true-wins"
  // semantics so a later #elif with a true condition is correctly skipped.
  QStack<bool> m_conditionalMatched;
  bool m_skipCurrentBlock = false;

  // Parsing methods
  void parseOutputChannels(QTextStream &in);
  void parseConstants(QTextStream &in, int pageId);
  void parseControllerCommands(QTextStream &in);
  void parseTunerStudio(QTextStream &in);
  void parseTableEditor(QTextStream &in);

  // Group C parsers (additive).
  void parseSettingGroups(QTextStream &in);
  void parsePcVariables(QTextStream &in);
  void parseDatalog(QTextStream &in);
  void parseGaugeConfigurations(QTextStream &in);
  void parseCurveEditor(QTextStream &in);
  void parseMenu(QTextStream &in);
  void parseUserDefined(QTextStream &in);
  void parseConstantsExtensions(QTextStream &in);
  void parseSettingContextHelp(QTextStream &in);
  void parseVeAnalyze(QTextStream &in);
  void parseWueAnalyze(QTextStream &in);

  // Brace-aware comma splitter used by gauges/curves where values can
  // contain `{ expression , possibly , with , commas }`.
  static QStringList splitTopLevelCommas(const QString& line);
  
  // Helper methods
  bool processDirective(const QString &line);
  bool shouldSkipLine() const;
  BitField parseBitField(const QString &shapeStr, const QStringList &options);
  // B1 fix: capture referenced \$varName tokens (out-param) so a runtime
  // resolver can substitute live values when the command fires.
  QByteArray parseCommandBytes(const QString &cmdStr,
                               QStringList *referencedVariables = nullptr);
  QString expandDefines(const QString &line);
};

#endif // ECUDEFINITION_H
