/**
 * @file SpeeduinoProtocol.h
 * @brief Speeduino New-Protocol Implementation
 *
 * Implements the complete Speeduino new-protocol serial communication:
 *   Request:  [Length 2B BE] [Payload: cmd + data...] [CRC32 4B BE]
 *   Response: [Length 2B BE] [Payload: RC + data...]  [CRC32 4B BE]
 *
 * CRC32: Standard polynomial 0xEDB88320 (reflected).
 */

#ifndef SPEEDUINOPROTOCOL_H
#define SPEEDUINOPROTOCOL_H

#include "ECUData.h"
#include "ECUDefinition.h"
#include "SpeeduinoConstants.h"
#include <QByteArray>
#include <QObject>

/**
 * @brief Speeduino protocol command bytes
 */
namespace SpeeduinoCommands {
  const char CMD_REALTIME_DATA = 'A'; ///< Request real-time data (130 bytes)
  const char CMD_SIGNATURE     = 'S'; ///< Request ECU signature (resets secl)
  const char CMD_READ_PAGE     = 'p'; ///< Read page bytes
  const char CMD_WRITE_PAGE    = 'M'; ///< Write page bytes (RAM only)
  const char CMD_BURN_PAGE     = 'b'; ///< Burn page to EEPROM
  const char CMD_BURN_COMPAT   = 'B'; ///< Burn (compat, longer defer)
  const char CMD_TEST_COMMS    = 'C'; ///< Test comms → [0x00, 0xFF]
  const char CMD_PROTO_VERSION = 'F'; ///< Protocol version → "002"
  const char CMD_CAPABILITIES  = 'f'; ///< Capabilities (blocking factors)
  const char CMD_CODE_VERSION  = 'Q'; ///< Code version string
  const char CMD_CAN_ID        = 'I'; ///< CAN ID → [0x00, 0x00]
  const char CMD_PAGE_CRC      = 'd'; ///< Page CRC32 verification
  const char CMD_TOOTH_LOG     = 'T'; ///< Read tooth log
  const char CMD_START_TOOTH   = 'H'; ///< Start tooth logger
  const char CMD_STOP_TOOTH    = 'h'; ///< Stop tooth logger
  const char CMD_START_COMP    = 'J'; ///< Start composite logger
  const char CMD_STOP_COMP     = 'j'; ///< Stop composite logger
  const char CMD_BUTTON        = 'E'; ///< 16-bit button command
  const char CMD_RESET         = 'U'; ///< Reset controller
  const char CMD_CALIB_UPLOAD  = 't'; ///< Upload calibration
  const char CMD_CALIB_CRC     = 'k'; ///< Calibration CRC
  const char CMD_PARTIAL_READ  = 'r'; ///< Multi-purpose read

  const char RESPONSE_ACK = 0x06;
  const char RESPONSE_NAK = 0x15;
} // namespace SpeeduinoCommands

/**
 * @brief Packet size constants
 */
namespace PacketSizes {
  const int REALTIME_DATA_SIZE = 130; ///< 130-byte real-time data payload
  const int SIGNATURE_SIZE     = 20;  ///< Minimum signature response
  const int MAX_PAGE_SIZE      = 288; ///< Maximum page size
} // namespace PacketSizes

/**
 * @brief Timing constants
 */
namespace ProtocolTiming {
  const int TIMEOUT_MS             = 1000;
  const int RETRY_COUNT            = 3;
  const int MIN_REALTIME_INTERVAL  = 10;
  const int HEARTBEAT_INTERVAL_MS  = 1000;
  const int DTR_SETTLE_MS          = 500;  ///< Wait after port open for DTR byte
  const int BURN_TIMEOUT_MS        = 3000; ///< Burn can take longer
  const int BUSY_RETRY_DELAY_MS    = 500;  ///< Wait between BUSY retries
  const int BUSY_MAX_RETRIES       = 10;
} // namespace ProtocolTiming

/**
 * @brief Command type enum for tracking pending responses
 */
enum class CommandType {
    Unknown,
    ProtocolVersion,   ///< 'F' → "002"
    Signature,         ///< 'S' → product string
    Capabilities,      ///< 'f' → [RC, ver, blockFact, tblBlockFact]
    TestComms,         ///< 'C' → [0x00, 0xFF]
    RealTimeData,      ///< 'A' → 130 bytes
    PartialRead,       ///< 'r' → variable
    ReadPage,          ///< 'p' → page data
    WritePage,         ///< 'M' → RC only
    BurnPage,          ///< 'b' → RC_BURN_OK
    PageCRC,           ///< 'd' → RC + CRC32
    ButtonCommand,     ///< 'E' → RC
    CalibUpload,       ///< 't' → RC
    CalibCRC,          ///< 'k' → RC + CRC32
    ToothLog,          ///< 'T' → tooth data
};

/**
 * @brief Speeduino Serial Protocol Handler
 */
class SpeeduinoProtocol : public QObject {
  Q_OBJECT

public:
  explicit SpeeduinoProtocol(QObject *parent = nullptr);

  bool loadDefinition(const QString &filePath);

  // === Command Creation Methods ===
  QByteArray createRealTimeDataRequest();    ///< 'A'
  QByteArray createSignatureRequest();       ///< 'S'
  QByteArray createProtocolVersionRequest(); ///< 'F'
  QByteArray createCapabilitiesRequest();    ///< 'f'
  QByteArray createTestCommsRequest();       ///< 'C'
  QByteArray createReadPageRequest(uint8_t pageNumber);
  QByteArray createReadPageRequest(uint8_t page, uint16_t offset, uint16_t size);
  QByteArray createWritePageRequest(uint8_t page, uint16_t offset, const QByteArray &data);
  QByteArray createBurnPageRequest(uint8_t page);                 ///< 'b'
  QByteArray createPageCRCRequest(uint8_t page);                  ///< 'd'
  QByteArray createButtonCommand(uint16_t commandId);             ///< 'E'
  QByteArray createToothLogRequest();                             ///< 'T'

  // === Response Parsing ===
  RealTimeData parseRealTimeData(const QByteArray &data);
  ECUSignature parseSignature(const QByteArray &data);

  // === CRC32 (New Protocol) ===
  static uint32_t calculateCRC32(const QByteArray &data);
  static QByteArray wrapNewProtocol(const QByteArray &payload);
  static bool unwrapNewProtocol(const QByteArray &frame, QByteArray &payload);

  // === Validation ===
  bool isAckResponse(const QByteArray &data);
  bool isNakResponse(const QByteArray &data);

private:
  ECUDefinition m_ecuDefinition;

  // Extraction helpers
  static uint16_t extractUint16LE(const QByteArray &data, int offset);
  static int16_t  extractInt16LE(const QByteArray &data, int offset);
  static uint8_t  extractUint8(const QByteArray &data, int offset);
  static int8_t   extractInt8(const QByteArray &data, int offset);
  static uint16_t extractUint16BE(const QByteArray &data, int offset);
};

#endif // SPEEDUINOPROTOCOL_H
