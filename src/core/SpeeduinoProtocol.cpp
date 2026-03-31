/**
 * @file SpeeduinoProtocol.cpp
 * @brief Speeduino New-Protocol Implementation
 *
 * Complete implementation of:
 *  - CRC32 (polynomial 0xEDB88320 reflected)
 *  - New-protocol framing (wrap/unwrap)
 *  - All 130-byte real-time data parsing per firmware spec
 *  - Command creation for all supported operations
 */

#include "SpeeduinoProtocol.h"
#include <QDebug>
#include <cstring>  // std::memcpy for F32 IEEE 754 extraction

// ============================================================================
//  CRC32 — Standard reflected polynomial 0xEDB88320
// ============================================================================

static const uint32_t crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91B, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBF, 0xE7B82D09, 0x90BF1D9F,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D4E3, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F6B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0D6B, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7822,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA8670955, 0x31648834, 0x4661B802,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F6, 0x1FDA8360,
    0x81BE16C3, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6B70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD706FF, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
};

uint32_t SpeeduinoProtocol::calculateCRC32(const QByteArray &data) {
    uint32_t crc = 0xFFFFFFFF;
    for (int i = 0; i < data.size(); i++) {
        uint8_t byte = static_cast<uint8_t>(data.at(i));
        crc = crc32_table[(crc ^ byte) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

// ============================================================================
//  New Protocol Framing
// ============================================================================

QByteArray SpeeduinoProtocol::wrapNewProtocol(const QByteArray &payload) {
    QByteArray frame;
    uint16_t len = static_cast<uint16_t>(payload.size());
    // Length header: 2 bytes big-endian
    frame.append(static_cast<char>((len >> 8) & 0xFF));
    frame.append(static_cast<char>(len & 0xFF));
    // Payload
    frame.append(payload);
    // CRC32: 4 bytes big-endian
    uint32_t crc = calculateCRC32(payload);
    frame.append(static_cast<char>((crc >> 24) & 0xFF));
    frame.append(static_cast<char>((crc >> 16) & 0xFF));
    frame.append(static_cast<char>((crc >> 8)  & 0xFF));
    frame.append(static_cast<char>(crc & 0xFF));
    return frame;
}

bool SpeeduinoProtocol::unwrapNewProtocol(const QByteArray &frame, QByteArray &payload) {
    if (frame.size() < 6) return false; // Minimum: 2 len + 0 payload + 4 CRC

    // Extract length (2B BE)
    uint16_t len = extractUint16BE(frame, 0);
    if (frame.size() < static_cast<int>(len) + 6) return false;

    // Extract payload
    payload = frame.mid(2, len);

    // Extract and validate CRC32 (4B BE after payload)
    int crcOffset = 2 + len;
    uint32_t receivedCRC = (static_cast<uint8_t>(frame.at(crcOffset))     << 24) |
                           (static_cast<uint8_t>(frame.at(crcOffset + 1)) << 16) |
                           (static_cast<uint8_t>(frame.at(crcOffset + 2)) << 8)  |
                           (static_cast<uint8_t>(frame.at(crcOffset + 3)));

    uint32_t computedCRC = calculateCRC32(payload);
    return receivedCRC == computedCRC;
}

// ============================================================================
//  Extraction Helpers
// ============================================================================

uint16_t SpeeduinoProtocol::extractUint16LE(const QByteArray &data, int offset) {
    if (offset + 1 >= data.size()) return 0;
    return static_cast<uint16_t>(static_cast<uint8_t>(data.at(offset))) |
           (static_cast<uint16_t>(static_cast<uint8_t>(data.at(offset + 1))) << 8);
}

int16_t SpeeduinoProtocol::extractInt16LE(const QByteArray &data, int offset) {
    return static_cast<int16_t>(extractUint16LE(data, offset));
}

uint8_t SpeeduinoProtocol::extractUint8(const QByteArray &data, int offset) {
    if (offset >= data.size()) return 0;
    return static_cast<uint8_t>(data.at(offset));
}

int8_t SpeeduinoProtocol::extractInt8(const QByteArray &data, int offset) {
    if (offset >= data.size()) return 0;
    return static_cast<int8_t>(data.at(offset));
}

uint16_t SpeeduinoProtocol::extractUint16BE(const QByteArray &data, int offset) {
    if (offset + 1 >= data.size()) return 0;
    return (static_cast<uint16_t>(static_cast<uint8_t>(data.at(offset))) << 8) |
           static_cast<uint16_t>(static_cast<uint8_t>(data.at(offset + 1)));
}

uint32_t SpeeduinoProtocol::extractUint32LE(const QByteArray &data, int offset) {
    if (offset + 3 >= data.size()) return 0;
    return static_cast<uint32_t>(static_cast<uint8_t>(data.at(offset)))
         | (static_cast<uint32_t>(static_cast<uint8_t>(data.at(offset + 1))) << 8)
         | (static_cast<uint32_t>(static_cast<uint8_t>(data.at(offset + 2))) << 16)
         | (static_cast<uint32_t>(static_cast<uint8_t>(data.at(offset + 3))) << 24);
}

int32_t SpeeduinoProtocol::extractInt32LE(const QByteArray &data, int offset) {
    return static_cast<int32_t>(extractUint32LE(data, offset));
}

// ============================================================================
//  Constructor / Definition
// ============================================================================

SpeeduinoProtocol::SpeeduinoProtocol(QObject *parent) : QObject(parent) {}

bool SpeeduinoProtocol::loadDefinition(const QString &filePath) {
    if (m_ecuDefinition.load(filePath)) {
        qDebug() << "ECU Definition loaded. Output Channels:"
                 << m_ecuDefinition.getOutputChannels().size();
        return true;
    }
    return false;
}

// ============================================================================
//  Command Creation
// ============================================================================

QByteArray SpeeduinoProtocol::createRealTimeDataRequest() {
    QByteArray cmd;
    cmd.append(SpeeduinoCommands::CMD_REALTIME_DATA); // 'A'
    return cmd;
}

QByteArray SpeeduinoProtocol::createSignatureRequest() {
    QByteArray cmd;
    cmd.append(SpeeduinoCommands::CMD_SIGNATURE); // 'S'
    return cmd;
}

QByteArray SpeeduinoProtocol::createProtocolVersionRequest() {
    QByteArray cmd;
    cmd.append(SpeeduinoCommands::CMD_PROTO_VERSION); // 'F'
    return cmd;
}

QByteArray SpeeduinoProtocol::createCapabilitiesRequest() {
    QByteArray cmd;
    cmd.append(SpeeduinoCommands::CMD_CAPABILITIES); // 'f'
    return cmd;
}

QByteArray SpeeduinoProtocol::createTestCommsRequest() {
    QByteArray cmd;
    cmd.append(SpeeduinoCommands::CMD_TEST_COMMS); // 'C'
    return cmd;
}

QByteArray SpeeduinoProtocol::createReadPageRequest(uint8_t pageNumber) {
    // Default: read full page from offset 0
    static const uint16_t pageSizes[] = {
        0, 128, 288, 288, 128, 288, 128, 240, 384, 192, 192, 288, 192, 128, 288, 256
    };
    uint16_t size = (pageNumber >= 1 && pageNumber <= 15) ? pageSizes[pageNumber] : PacketSizes::MAX_PAGE_SIZE;
    return createReadPageRequest(pageNumber, 0, size);
}

QByteArray SpeeduinoProtocol::createReadPageRequest(uint8_t page,
                                                    uint16_t offset,
                                                    uint16_t size) {
    QByteArray cmd;
    cmd.append(SpeeduinoCommands::CMD_READ_PAGE); // 'p'
    cmd.append(static_cast<char>(0));             // tsCanId (0 for local)
    cmd.append(static_cast<char>(page));
    // Offset: 2 bytes LITTLE-endian
    cmd.append(static_cast<char>(offset & 0xFF));
    cmd.append(static_cast<char>((offset >> 8) & 0xFF));
    // Length: 2 bytes LITTLE-endian
    cmd.append(static_cast<char>(size & 0xFF));
    cmd.append(static_cast<char>((size >> 8) & 0xFF));
    return cmd;
}

QByteArray SpeeduinoProtocol::createWritePageRequest(uint8_t page,
                                                     uint16_t offset,
                                                     const QByteArray &data) {
    QByteArray cmd;
    cmd.append(SpeeduinoCommands::CMD_WRITE_PAGE); // 'M'
    cmd.append(static_cast<char>(0));              // tsCanId
    cmd.append(static_cast<char>(page));
    // Offset: 2 bytes LE
    cmd.append(static_cast<char>(offset & 0xFF));
    cmd.append(static_cast<char>((offset >> 8) & 0xFF));
    // Length: 2 bytes LE
    uint16_t len = static_cast<uint16_t>(data.size());
    cmd.append(static_cast<char>(len & 0xFF));
    cmd.append(static_cast<char>((len >> 8) & 0xFF));
    // Data
    cmd.append(data);
    return cmd;
}

QByteArray SpeeduinoProtocol::createBurnPageRequest(uint8_t page) {
    QByteArray cmd;
    cmd.append(SpeeduinoCommands::CMD_BURN_PAGE); // 'b'
    cmd.append(static_cast<char>(0));             // unused
    cmd.append(static_cast<char>(page));
    return cmd;
}

QByteArray SpeeduinoProtocol::createPageCRCRequest(uint8_t page) {
    QByteArray cmd;
    cmd.append(SpeeduinoCommands::CMD_PAGE_CRC); // 'd'
    cmd.append(static_cast<char>(0));            // unused
    cmd.append(static_cast<char>(page));
    return cmd;
}

QByteArray SpeeduinoProtocol::createButtonCommand(uint16_t commandId) {
    QByteArray cmd;
    cmd.append(SpeeduinoCommands::CMD_BUTTON); // 'E'
    cmd.append(static_cast<char>((commandId >> 8) & 0xFF)); // cmdHi
    cmd.append(static_cast<char>(commandId & 0xFF));        // cmdLo
    return cmd;
}

QByteArray SpeeduinoProtocol::createToothLogRequest() {
    QByteArray cmd;
    cmd.append(SpeeduinoCommands::CMD_TOOTH_LOG); // 'T'
    return cmd;
}

// ============================================================================
//  parseRealTimeData — Complete 130-byte parser
//
//  AUTHORITATIVE offset map from Speeduino firmware 2025.01.
//  Data payload: 130 bytes AFTER the return-code byte.
//  All multi-byte data values are LITTLE-ENDIAN.
// ============================================================================

RealTimeData SpeeduinoProtocol::parseRealTimeData(const QByteArray &data) {
    if (data.size() < PacketSizes::REALTIME_DATA_SIZE) {
        throw QString("Packet too short: %1 bytes (expected %2)")
                  .arg(data.size())
                  .arg(PacketSizes::REALTIME_DATA_SIZE);
    }

    RealTimeData rt;

    // Byte 0-3: Timing, status, sync
    rt.secl             = extractUint8(data, 0);
    rt.status1          = extractUint8(data, 1);
    rt.engine           = extractUint8(data, 2);
    rt.syncLossCounter  = extractUint8(data, 3);

    // Byte 4-13: Primary sensors + corrections
    rt.map              = extractUint16LE(data, 4);
    rt.iat              = extractUint8(data, 6);
    rt.coolant          = extractUint8(data, 7);
    rt.batCorrection    = extractUint8(data, 8);
    rt.battery10        = extractUint8(data, 9);
    rt.o2               = extractUint8(data, 10);
    rt.egoCorrection    = extractUint8(data, 11);
    rt.iatCorrection    = extractUint8(data, 12);
    rt.wueCorrection    = extractUint8(data, 13);

    // Byte 14-31: Engine operation
    rt.rpm              = extractUint16LE(data, 14);
    rt.aeAmount         = extractUint8(data, 16);
    rt.corrections      = extractUint16LE(data, 17);
    rt.ve1              = extractUint8(data, 19);
    rt.ve2              = extractUint8(data, 20);
    rt.afrTarget        = extractUint8(data, 21);
    rt.tpsDOT           = extractInt16LE(data, 22);
    rt.advance          = extractInt8(data, 24);
    rt.tps              = extractUint8(data, 25);
    rt.loopsPerSecond   = extractUint16LE(data, 26);
    rt.freeRAM          = extractUint16LE(data, 28);
    rt.boostTarget      = extractUint8(data, 30);
    rt.boostDuty        = extractUint8(data, 31);

    // Byte 32-41: Status2, rates, flex, idle
    rt.status2          = extractUint8(data, 32);
    rt.rpmDOT           = extractInt16LE(data, 33);
    rt.ethanolPct       = extractUint8(data, 35);
    rt.flexCorrection   = extractUint8(data, 36);
    rt.flexIgnCorrection= extractInt8(data, 37);
    rt.idleLoad         = extractUint8(data, 38);
    rt.testOutputs      = extractUint8(data, 39);
    rt.o2_2             = extractUint8(data, 40);
    rt.baro             = extractUint8(data, 41);

    // Byte 42-73: CAN input channels (16 × U16LE)
    for (int i = 0; i < 16; i++) {
        rt.canin[i] = extractUint16LE(data, 42 + i * 2);
    }

    // Byte 74-75: TPS ADC, error
    rt.tpsADC           = extractUint8(data, 74);
    rt.errorByte        = extractUint8(data, 75);

    // Byte 76-83: Injector pulse widths
    rt.pw1              = extractUint16LE(data, 76);
    rt.pw2              = extractUint16LE(data, 78);
    rt.pw3              = extractUint16LE(data, 80);
    rt.pw4              = extractUint16LE(data, 82);

    // Byte 84-101: Status3, protect, loads, dwell, VVT1
    rt.status3          = extractUint8(data, 84);
    rt.engineProtectStatus = extractUint8(data, 85);
    rt.fuelLoad         = extractInt16LE(data, 86);
    rt.ignLoad          = extractInt16LE(data, 88);
    rt.dwell            = extractUint16LE(data, 90);
    rt.CLIdleTarget     = extractUint8(data, 92);
    rt.mapDOT           = extractInt16LE(data, 93);
    rt.vvt1Angle        = extractInt16LE(data, 95);
    rt.vvt1TargetAngle  = extractUint8(data, 97);
    rt.vvt1Duty         = extractUint8(data, 98);
    rt.flexBoostCorrection = extractInt16LE(data, 99);
    rt.baroCorrection   = extractUint8(data, 101);

    // Byte 102-109: VE, ASE, VSS, gear, pressures, WMI
    rt.ve               = extractUint8(data, 102);
    rt.ASEValue         = extractUint8(data, 103);
    rt.vss              = extractUint16LE(data, 104);
    rt.gear             = extractUint8(data, 106);
    rt.fuelPressure     = extractUint8(data, 107);
    rt.oilPressure      = extractUint8(data, 108);
    rt.wmiPW            = extractUint8(data, 109);

    // Byte 110-129: Status4, VVT2, outputs, temps, advance, EMAP, fan, A/C, dwell, knock
    rt.status4          = extractUint8(data, 110);
    rt.vvt2Angle        = extractInt16LE(data, 111);
    rt.vvt2TargetAngle  = extractUint8(data, 113);
    rt.vvt2Duty         = extractUint8(data, 114);
    rt.outputsStatus    = extractUint8(data, 115);
    rt.fuelTemp         = extractUint8(data, 116);
    rt.fuelTempCorrection = extractUint8(data, 117);
    rt.advance1         = extractInt8(data, 118);
    rt.advance2         = extractInt8(data, 119);
    rt.TS_SD_Status     = extractUint8(data, 120);
    rt.EMAP             = extractInt16LE(data, 121);
    rt.fanDuty          = extractUint8(data, 123);
    rt.airConStatus     = extractUint8(data, 124);
    rt.actualDwell      = extractUint16LE(data, 125);
    rt.status5          = extractUint8(data, 127);
    rt.knockCount       = extractUint8(data, 128);
    rt.knockRetard      = extractUint8(data, 129);

    rt.timestamp = QDateTime::currentDateTime();
    return rt;
}

// ============================================================================
//  Signature Parsing
// ============================================================================

ECUSignature SpeeduinoProtocol::parseSignature(const QByteArray &data) {
    QString sigStr = QString::fromLatin1(data);

    ECUSignature sig;
    sig.firmwareVersion = "Unknown";
    sig.boardType = "Unknown";
    sig.protocolVersion = 0;
    sig.pageCount = 15; // Speeduino always has 15 pages

    if (sigStr.startsWith("Speeduino")) {
        sig.firmwareVersion = sigStr.mid(10).trimmed();
        sig.boardType = "Generic";
        sig.protocolVersion = 2; // New protocol
    } else {
        sig.firmwareVersion = sigStr.trimmed();
    }

    return sig;
}

// ============================================================================
//  Validation Helpers
// ============================================================================

bool SpeeduinoProtocol::isAckResponse(const QByteArray &data) {
    return data.size() >= 1 && data.at(0) == SpeeduinoCommands::RESPONSE_ACK;
}

bool SpeeduinoProtocol::isNakResponse(const QByteArray &data) {
    return data.size() >= 1 && data.at(0) == SpeeduinoCommands::RESPONSE_NAK;
}

// ============================================================================
//  [CRIT-1] INI-Driven Real-Time Data Parser
//
//  Dynamically extracts output channel values using the offset, type, scale,
//  and translate metadata parsed from the INI file. This replaces the
//  hardcoded offset table with a data-driven approach matching TunerStudio.
//
//  For backward compatibility, also populates the legacy RealTimeData struct
//  by mapping well-known channel names to struct members.
// ============================================================================

SpeeduinoProtocol::DynamicRTData SpeeduinoProtocol::parseRealTimeDataDynamic(
    const QByteArray &payload,
    const ECUDefinition &def)
{
    DynamicRTData result;
    result.legacy = RealTimeData();  // Zero-initialized
    result.legacy.timestamp = QDateTime::currentDateTime();

    const auto &channels = def.getOutputChannels();
    const int payloadSize = payload.size();

    for (auto it = channels.constBegin(); it != channels.constEnd(); ++it) {
        const ECUDefinition::OutputChannel &ch = it.value();

        // Skip expression-based channels (no evaluator yet)
        if (ch.type == "EXPR") {
            continue;
        }

        // Bounds check
        const int typeSize = def.getTypeSize(ch.type);
        if (ch.offset < 0 || ch.offset + typeSize > payloadSize) {
            continue;
        }

        double userValue = 0.0;

        if (ch.isBits) {
            // ---- Bit-field extraction ----
            uint8_t raw = extractUint8(payload, ch.offset);
            int width = ch.bitField.highBit - ch.bitField.lowBit + 1;
            uint8_t mask = static_cast<uint8_t>(((1 << width) - 1) << ch.bitField.lowBit);
            int bitVal = (raw & mask) >> ch.bitField.lowBit;
            userValue = static_cast<double>(bitVal);
        } else if (ch.type == "U08") {
            int rawVal = extractUint8(payload, ch.offset);
            userValue = (static_cast<double>(rawVal) + ch.translate) * ch.scale;
        } else if (ch.type == "S08") {
            int rawVal = extractInt8(payload, ch.offset);
            userValue = (static_cast<double>(rawVal) + ch.translate) * ch.scale;
        } else if (ch.type == "U16") {
            int rawVal = extractUint16LE(payload, ch.offset);
            userValue = (static_cast<double>(rawVal) + ch.translate) * ch.scale;
        } else if (ch.type == "S16") {
            int rawVal = extractInt16LE(payload, ch.offset);
            userValue = (static_cast<double>(rawVal) + ch.translate) * ch.scale;
        } else if (ch.type == "U32") {
            uint32_t rawVal = extractUint32LE(payload, ch.offset);
            userValue = (static_cast<double>(rawVal) + ch.translate) * ch.scale;
        } else if (ch.type == "S32") {
            int32_t rawVal = extractInt32LE(payload, ch.offset);
            userValue = (static_cast<double>(rawVal) + ch.translate) * ch.scale;
        } else if (ch.type == "F32") {
            uint32_t bits = extractUint32LE(payload, ch.offset);
            float fval;
            std::memcpy(&fval, &bits, sizeof(float));
            userValue = static_cast<double>(fval);  // F32 is already in user units
        } else {
            continue;  // Unknown type
        }

        // Store in dynamic map
        result.channels.insert(ch.name, userValue);

        // ---- Legacy struct mapping ----
        // Map well-known INI channel names to RealTimeData fields.
        // This keeps existing widgets working without modification.
        const QString &n = ch.name;
        RealTimeData &rt = result.legacy;

        if      (n == "secl")            rt.secl = extractUint8(payload, ch.offset);
        else if (n == "status1")         rt.status1 = extractUint8(payload, ch.offset);
        else if (n == "engine")          rt.engine = extractUint8(payload, ch.offset);
        else if (n == "syncLossCounter") rt.syncLossCounter = extractUint8(payload, ch.offset);
        else if (n == "map")             rt.map = extractUint16LE(payload, ch.offset);
        else if (n == "iat")             rt.iat = extractUint8(payload, ch.offset);
        else if (n == "clt")             rt.coolant = extractUint8(payload, ch.offset);
        else if (n == "batCorrection")   rt.batCorrection = extractUint8(payload, ch.offset);
        else if (n == "battery10")       rt.battery10 = extractUint8(payload, ch.offset);
        else if (n == "O2")              rt.o2 = extractUint8(payload, ch.offset);
        else if (n == "egoCorrection")   rt.egoCorrection = extractUint8(payload, ch.offset);
        else if (n == "iatCorrection")   rt.iatCorrection = extractUint8(payload, ch.offset);
        else if (n == "wueCorrection")   rt.wueCorrection = extractUint8(payload, ch.offset);
        else if (n == "rpm")             rt.rpm = extractUint16LE(payload, ch.offset);
        else if (n == "aeAmount")        rt.aeAmount = extractUint8(payload, ch.offset);
        else if (n == "corrections")     rt.corrections = extractUint16LE(payload, ch.offset);
        else if (n == "ve")              rt.ve = extractUint8(payload, ch.offset);
        else if (n == "afrTarget")       rt.afrTarget = extractUint8(payload, ch.offset);
        else if (n == "pw1")             rt.pw1 = extractUint16LE(payload, ch.offset);
        else if (n == "tpsDOT")          rt.tpsDOT = extractInt16LE(payload, ch.offset);
        else if (n == "advance")         rt.advance = extractInt8(payload, ch.offset);
        else if (n == "tps")             rt.tps = extractUint8(payload, ch.offset);
        else if (n == "loopsPerSecond")  rt.loopsPerSecond = extractUint16LE(payload, ch.offset);
        else if (n == "freeRAM")         rt.freeRAM = extractUint16LE(payload, ch.offset);
        else if (n == "boostTarget")     rt.boostTarget = extractUint8(payload, ch.offset);
        else if (n == "boostDuty")       rt.boostDuty = extractUint8(payload, ch.offset);
        else if (n == "spark")           rt.status2 = extractUint8(payload, ch.offset);
        else if (n == "rpmDOT")          rt.rpmDOT = extractInt16LE(payload, ch.offset);
        else if (n == "ethanolPct")      rt.ethanolPct = extractUint8(payload, ch.offset);
        else if (n == "flexCorrection")  rt.flexCorrection = extractUint8(payload, ch.offset);
        else if (n == "baro")            rt.baro = extractUint8(payload, ch.offset);
        else if (n == "tpsADC")          rt.tpsADC = extractUint8(payload, ch.offset);
        else if (n == "errorByte")       rt.errorByte = extractUint8(payload, ch.offset);
        else if (n == "dwell")           rt.dwell = extractUint16LE(payload, ch.offset);
        else if (n == "CLIdleTarget")    rt.CLIdleTarget = extractUint8(payload, ch.offset);
        else if (n == "vss")             rt.vss = extractUint16LE(payload, ch.offset);
        else if (n == "gear")            rt.gear = extractUint8(payload, ch.offset);
        else if (n == "fuelPressure")    rt.fuelPressure = extractUint8(payload, ch.offset);
        else if (n == "oilPressure")     rt.oilPressure = extractUint8(payload, ch.offset);
        else if (n == "fanDuty")         rt.fanDuty = extractUint8(payload, ch.offset);
        else if (n == "knockCount")      rt.knockCount = extractUint8(payload, ch.offset);
        else if (n == "knockRetard")     rt.knockRetard = extractUint8(payload, ch.offset);
    }

    return result;
}
