# Speeduino Serial Protocol

## Overview
TunerPro implements the Speeduino Secondary Serial Protocol.
- **Baud Rate**: 115200 (default)
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1

## Commands

### 'A' (0x41) - Realtime Data
Requests the full status packet.
- **Request**: `0x41`
- **Response**: 75 bytes of data + 1 byte checksum (optional/legacy dependent).
- **Structure**:
  - Byte 0: Seconds (secl)
  - Byte 1: Status 1
  - Byte 2: Engine Status
  - ...
  - Byte 16-17: RPM (Little Endian)
  - ...

### 'S' (0x53) - Signature
Requests firmware version.
- **Request**: `0x53`
- **Response**: String (e.g., "Speeduino 202310")

## Checksum
Simple additive checksum:
`Sum(all bytes) & 0xFF`
