# OS Tuner — Open-Source ECU Tuning Application

**Version:** 0.5.0-alpha  
**Status:** Alpha — under active development. Not yet recommended for production tuning.  
**Authors:** Safeerullah Afridi (FA-22-151), Muhammad Saeed Sajid (FA-22-143)  
**Supervisor:** Prof. Dr. Shariq Hussain  
**License:** TBD (AGPL-3.0 recommended — decision pending before v1.0)

## Overview

OS Tuner is a desktop ECU tuning application for the open-source ECU ecosystem (Speeduino, RusEFI, FOME). Built with Qt 6 and C++17.

## What Works Today

- **Serial communication** with Speeduino ECUs (new-protocol framing with CRC32)
- **Real-time dashboard** with animated gauges (RPM, MAP, CLT, IAT, TPS, AFR, etc.)
- **INI definition parsing** for `[Constants]`, `[OutputChannels]`, `[TableEditor]`, `[ControllerCommands]`
- **Table editor** for VE, ignition, and AFR target tables (read/write/burn)
- **Settings management** with page cache, write-back, and CRC burn verification
- **MSQ project loading** (basic constant import)
- **Datalog recording** to CSV
- **Dark theme** optimized for low-light tuning environments
- **ECU signature validation** — blocks writes when firmware/definition mismatch

## What Doesn't Work Yet

- **Cross-platform builds** — currently only tested on Windows (Linux/macOS support planned)
- **Curve editor** — `[CurveEditor]` INI section not yet parsed
- **Gauge configurations** — `[GaugeConfigurations]` INI section not parsed
- **Tooth logger** — UI exists, recently wired to serial backend (needs real-hardware testing)
- **MSQ save** — can read projects but cannot write them back
- **Closed-loop O2 visualization** — no dedicated widget yet
- **Datalog replay** — recording works, playback is not implemented
- **Auto-updater** — disabled for security reasons; check for updates opens GitHub releases page

## Supported Hardware

| ECU | Status |
|-----|--------|
| Speeduino (all variants) | Primary target, actively tested |
| RusEFI | INI parsing capable, untested with hardware |
| FOME | INI parsing capable, untested with hardware |

## Build Instructions

### Requirements
- Qt 6.5+ with QtSerialPort, QtNetwork, QtCharts modules
- CMake 3.20+
- C++17-capable compiler (MSVC 2019/2022 recommended on Windows)

### Build
```bash
cmake -B build -DCMAKE_PREFIX_PATH=<path-to-qt6>
cmake --build build --config Release
```

### Run Tests
```bash
cmake -B build -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release
```

### Package (Windows)
```bash
cpack -C Release   # Requires NSIS
```

## Project Structure

```
src/
├── core/           # Protocol, settings, definition parser, logging
├── widgets/        # Dashboard, table editor, tooth logger, gauges
├── dialogs/        # About, startup, settings
├── utils/          # Logger, settings, expression parser
└── main.cpp        # Application entry point
tests/              # Qt Test-based unit tests
resources/          # QSS themes, fonts, icons
```

## Contributing

This project is in early development. If you're interested in contributing, please open an issue first to discuss the change you'd like to make.

## Acknowledgments

- [Speeduino](https://speeduino.com/) — the open-source engine management system
- [RusEFI](https://rusefi.com/) — open-source engine control unit
- [FOME](https://github.com/FOME-Tech) — fork of rusEFI
