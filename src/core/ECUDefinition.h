
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
 * @brief ECU Definition Parser for TunerPro
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
      return (static_cast<double>(rawValue) + translate) * scale;
    }
    
    /// Convert user-facing double to raw integer for the ECU.
    /// Uses qRound (banker's rounding) — NOT truncation — so that
    /// 34.6° with scale=1 becomes raw=35, not raw=34.
    int userToRaw(double userValue) const {
      return rawClamp(qRound((userValue / scale) - translate));
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
    QList<QByteArray> commands;  // Chain of commands
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

  ECUDefinition();
  bool load(const QString &filePath);

  // Accessors
  const QMap<QString, OutputChannel> &getOutputChannels() const;
  const QMap<QString, Constant> &getConstants() const;
  const QMap<QString, ControllerCommand> &getControllerCommands() const;
  const QMap<int, Page> &getPages() const;
  
  QString getSignature() const { return m_signature; }
  int getOutputChannelsSize() const { return m_outputChannelsSize; }
  
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
  };
  static QMap<QString, Table> getDefaultSpeeduinoTables();
  
  const QMap<QString, Table> &getTables() const { return m_tableDefinitions; }

  // Directive state (for #if/#set)
  bool isConditionActive(const QString& name) const;
  
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

  // Conditional parsing state
  QMap<QString, bool> m_conditions;       // #set/#unset directives
  QMap<QString, QString> m_defines;       // #define macros
  QStack<bool> m_conditionalStack;        // #if nesting state
  bool m_skipCurrentBlock = false;

  // Parsing methods
  void parseOutputChannels(QTextStream &in);
  void parseConstants(QTextStream &in, int pageId);
  void parseControllerCommands(QTextStream &in);
  void parseTunerStudio(QTextStream &in);
  void parseTableEditor(QTextStream &in);
  
  // Helper methods
  bool processDirective(const QString &line);
  bool shouldSkipLine() const;
  BitField parseBitField(const QString &shapeStr, const QStringList &options);
  QByteArray parseCommandBytes(const QString &cmdStr);
  QString expandDefines(const QString &line);
};

#endif // ECUDEFINITION_H
