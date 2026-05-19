// =============================================================================
// G1: Protocol round-trip tests
//
// Locks down the wire-protocol primitives (CRC32, new-protocol frame
// wrap/unwrap, the 130-byte legacy RT parser, signature parsing, and the
// command-builder methods) so that future widget/UI work in Group E cannot
// silently regress them.
//
// Authority: SpeeduinoProtocol.cpp + Speeduino firmware comms.cpp.
// Severity of regression here: any failure means the tuner stops talking to
// the ECU — the highest-priority test target in the project.
// =============================================================================

#include <QtTest/QtTest>

#include "core/SpeeduinoProtocol.h"

class SpeeduinoProtocolTest : public QObject {
  Q_OBJECT

private slots:
  // --- CRC32 known-answer vectors ---
  void crc32_emptyInput();
  void crc32_singleByte();
  void crc32_helloWorld();
  void crc32_realPagePayload();

  // --- New-protocol frame wrap/unwrap ---
  void wrap_singleCommand();
  void wrap_emptyPayload();
  void unwrap_validFrame();
  void unwrap_corruptCrc_rejected();
  void unwrap_truncatedFrame_rejected();
  void unwrap_lengthMismatch_rejected();
  void roundTrip_payloadIntegrity();

  // --- Command builders ---
  void cmd_realTimeData_isSingleByteA();
  void cmd_signature_isSingleByteS();
  void cmd_burnPage_carriesPageNumber();
  void cmd_pageCrc_carriesPageNumber();
  void cmd_button_isBigEndianWordE();
  void cmd_toothLog_isSingleByteT();

  // --- Legacy 130-byte parser ---
  void parseRT_parsesKnownOffsets();
  void parseRT_shortPayload_returnsZero();

  // --- Signature parser ---
  void parseSignature_extractsAsciiText();
  void parseSignature_emptyInput();

  // --- ACK / NAK ---
  void isAck_recognisesRC_OK();
  void isNak_recognisesRC_NAK();
};

// ---------------------------------------------------------------------------
// CRC32
// ---------------------------------------------------------------------------
void SpeeduinoProtocolTest::crc32_emptyInput() {
  QCOMPARE(SpeeduinoProtocol::calculateCRC32(QByteArray()), uint32_t(0));
}

void SpeeduinoProtocolTest::crc32_singleByte() {
  // CRC32 of a single 'A' (0x41) under reflected poly 0xEDB88320, zero seed,
  // xor-out 0xFFFFFFFF — matches the Speeduino FastCRC reference output.
  const uint32_t expected = 0xD3D99E8B;
  QCOMPARE(SpeeduinoProtocol::calculateCRC32(QByteArray(1, 'A')), expected);
}

void SpeeduinoProtocolTest::crc32_helloWorld() {
  // "123456789" — universally cited CRC32 known-answer vector.
  const QByteArray input("123456789");
  const uint32_t expected = 0xCBF43926;
  QCOMPARE(SpeeduinoProtocol::calculateCRC32(input), expected);
}

void SpeeduinoProtocolTest::crc32_realPagePayload() {
  // Stable property: same input -> same output, idempotent. Detects accidental
  // table corruption in the poly-table without depending on an externally
  // sourced fixture vector.
  QByteArray input;
  for (int i = 0; i < 256; ++i) input.append(static_cast<char>(i));
  const uint32_t first  = SpeeduinoProtocol::calculateCRC32(input);
  const uint32_t second = SpeeduinoProtocol::calculateCRC32(input);
  QCOMPARE(first, second);
  // Single-bit flip MUST change the CRC.
  input[42] = input.at(42) ^ 0x01;
  QVERIFY(first != SpeeduinoProtocol::calculateCRC32(input));
}

// ---------------------------------------------------------------------------
// Frame wrap / unwrap
// ---------------------------------------------------------------------------
void SpeeduinoProtocolTest::wrap_singleCommand() {
  const QByteArray payload(1, SpeeduinoCommands::CMD_REALTIME_DATA);
  const QByteArray frame = SpeeduinoProtocol::wrapNewProtocol(payload);
  // [Length 2B BE] [payload] [CRC32 4B BE] -> 1 + 2 + 4 = 7 bytes.
  QCOMPARE(frame.size(), 7);
  // Length BE = 1.
  QCOMPARE(static_cast<uint8_t>(frame.at(0)), uint8_t(0));
  QCOMPARE(static_cast<uint8_t>(frame.at(1)), uint8_t(1));
  // Payload byte present.
  QCOMPARE(frame.at(2), char('A'));
}

void SpeeduinoProtocolTest::wrap_emptyPayload() {
  const QByteArray frame = SpeeduinoProtocol::wrapNewProtocol(QByteArray());
  // Empty payload — header (2) + CRC32 (4) = 6 bytes, length field = 0.
  QCOMPARE(frame.size(), 6);
  QCOMPARE(static_cast<uint8_t>(frame.at(0)), uint8_t(0));
  QCOMPARE(static_cast<uint8_t>(frame.at(1)), uint8_t(0));
}

void SpeeduinoProtocolTest::unwrap_validFrame() {
  const QByteArray payload("Spd");
  const QByteArray frame = SpeeduinoProtocol::wrapNewProtocol(payload);
  QByteArray decoded;
  QVERIFY(SpeeduinoProtocol::unwrapNewProtocol(frame, decoded));
  QCOMPARE(decoded, payload);
}

void SpeeduinoProtocolTest::unwrap_corruptCrc_rejected() {
  QByteArray frame = SpeeduinoProtocol::wrapNewProtocol(QByteArray("ABC"));
  // Flip the lowest CRC byte.
  frame[frame.size() - 1] = frame.at(frame.size() - 1) ^ 0x01;
  QByteArray decoded;
  QVERIFY(!SpeeduinoProtocol::unwrapNewProtocol(frame, decoded));
}

void SpeeduinoProtocolTest::unwrap_truncatedFrame_rejected() {
  QByteArray decoded;
  // Smaller than the 6-byte minimum frame.
  QVERIFY(!SpeeduinoProtocol::unwrapNewProtocol(QByteArray("\x00\x01\x41"), decoded));
}

void SpeeduinoProtocolTest::unwrap_lengthMismatch_rejected() {
  // Claim length 99 but only one payload byte present.
  QByteArray frame;
  frame.append(char(0)).append(char(99)).append('A');
  for (int i = 0; i < 4; ++i) frame.append(char(0));
  QByteArray decoded;
  QVERIFY(!SpeeduinoProtocol::unwrapNewProtocol(frame, decoded));
}

void SpeeduinoProtocolTest::roundTrip_payloadIntegrity() {
  for (int sz : {1, 16, 130, 288, 4096}) {
    QByteArray payload(sz, '\0');
    for (int i = 0; i < sz; ++i) payload[i] = static_cast<char>(i & 0xFF);
    QByteArray decoded;
    QVERIFY(SpeeduinoProtocol::unwrapNewProtocol(
        SpeeduinoProtocol::wrapNewProtocol(payload), decoded));
    QCOMPARE(decoded, payload);
  }
}

// ---------------------------------------------------------------------------
// Command builders
// ---------------------------------------------------------------------------
void SpeeduinoProtocolTest::cmd_realTimeData_isSingleByteA() {
  SpeeduinoProtocol p;
  const QByteArray r = p.createRealTimeDataRequest();
  QCOMPARE(r.size(), 1);
  QCOMPARE(r.at(0), char('A'));
}

void SpeeduinoProtocolTest::cmd_signature_isSingleByteS() {
  SpeeduinoProtocol p;
  const QByteArray r = p.createSignatureRequest();
  QCOMPARE(r.size(), 1);
  QCOMPARE(r.at(0), char('S'));
}

void SpeeduinoProtocolTest::cmd_burnPage_carriesPageNumber() {
  SpeeduinoProtocol p;
  const QByteArray r = p.createBurnPageRequest(7);
  QVERIFY(r.size() >= 2);
  QCOMPARE(r.at(0), char('b'));
  QCOMPARE(static_cast<uint8_t>(r.at(1)), uint8_t(7));
}

void SpeeduinoProtocolTest::cmd_pageCrc_carriesPageNumber() {
  SpeeduinoProtocol p;
  const QByteArray r = p.createPageCRCRequest(3);
  QVERIFY(r.size() >= 2);
  QCOMPARE(r.at(0), char('d'));
  QCOMPARE(static_cast<uint8_t>(r.at(1)), uint8_t(3));
}

void SpeeduinoProtocolTest::cmd_button_isBigEndianWordE() {
  SpeeduinoProtocol p;
  const QByteArray r = p.createButtonCommand(0x1234);
  QVERIFY(r.size() >= 3);
  QCOMPARE(r.at(0), char('E'));
  // TS button-command IDs are big-endian on the wire.
  QCOMPARE(static_cast<uint8_t>(r.at(1)), uint8_t(0x12));
  QCOMPARE(static_cast<uint8_t>(r.at(2)), uint8_t(0x34));
}

void SpeeduinoProtocolTest::cmd_toothLog_isSingleByteT() {
  SpeeduinoProtocol p;
  const QByteArray r = p.createToothLogRequest();
  QCOMPARE(r.size(), 1);
  QCOMPARE(r.at(0), char('T'));
}

// ---------------------------------------------------------------------------
// Legacy 130-byte RT parser
// ---------------------------------------------------------------------------
void SpeeduinoProtocolTest::parseRT_parsesKnownOffsets() {
  // Build a synthetic 130-byte payload with recognizable values.
  QByteArray payload(130, char(0));
  // Authoritative offsets per SpeeduinoProtocol.cpp parser.
  // RPM is U16 LE at offset 14: set 3500.
  payload[14] = char(3500 & 0xFF);
  payload[15] = char((3500 >> 8) & 0xFF);
  // MAP is U16 LE at offset 4: set 95.
  payload[4]  = char(95 & 0xFF);
  payload[5]  = char(0);
  // CLT is U08 at offset 7: set 80 (raw) -> 80 - 40 = 40°C.
  payload[7]  = char(80);
  // TPS is U08 at offset 25: 50%.
  payload[25] = char(50);
  // Battery voltage is U08 at offset 9: raw 137 -> 13.7V.
  payload[9]  = char(137);

  SpeeduinoProtocol p;
  RealTimeData rt = p.parseRealTimeData(payload);

  QCOMPARE(static_cast<int>(rt.rpm),         3500);
  QCOMPARE(static_cast<int>(rt.map),         95);
  QCOMPARE(static_cast<int>(rt.coolant),     80);  // raw byte preserved
  QCOMPARE(static_cast<int>(rt.tps),         50);
  QCOMPARE(static_cast<int>(rt.battery10),   137); // raw byte preserved
}

void SpeeduinoProtocolTest::parseRT_shortPayload_returnsZero() {
  SpeeduinoProtocol p;
  QByteArray tooShort(10, char(0));
  RealTimeData rt = p.parseRealTimeData(tooShort);
  // Out-of-bounds reads must extract zero, not crash or read garbage.
  QCOMPARE(static_cast<int>(rt.rpm), 0);
  QCOMPARE(static_cast<int>(rt.map), 0);
}

// ---------------------------------------------------------------------------
// Signature parser
// ---------------------------------------------------------------------------
void SpeeduinoProtocolTest::parseSignature_extractsAsciiText() {
  SpeeduinoProtocol p;
  const QByteArray sig("speeduino 202501-dev    ");
  ECUSignature parsed = p.parseSignature(sig);
  QVERIFY(parsed.firmwareVersion.contains("speeduino"));
}

void SpeeduinoProtocolTest::parseSignature_emptyInput() {
  SpeeduinoProtocol p;
  ECUSignature parsed = p.parseSignature(QByteArray());
  // No crash; signature comes back empty.
  QVERIFY(parsed.firmwareVersion.isEmpty());
}

// ---------------------------------------------------------------------------
// ACK / NAK detection
// ---------------------------------------------------------------------------
void SpeeduinoProtocolTest::isAck_recognisesRC_OK() {
  SpeeduinoProtocol p;
  QByteArray ack(1, char(SpeeduinoCommands::RESPONSE_ACK));
  QVERIFY(p.isAckResponse(ack));
}

void SpeeduinoProtocolTest::isNak_recognisesRC_NAK() {
  SpeeduinoProtocol p;
  QByteArray nak(1, char(SpeeduinoCommands::RESPONSE_NAK));
  QVERIFY(p.isNakResponse(nak));
}

QTEST_APPLESS_MAIN(SpeeduinoProtocolTest)
#include "SpeeduinoProtocolTest.moc"
