# TunerPro - Proprietary ECU Tuning Application
**Version**: 1.0.0-alpha
**Authors**: Safeerullah Afridi (FA-22-151), Muhammad Saeed Sajid (FA-22-143)
**Supervisor**: Prof. Dr. Shariq Hussain
**License**: Proprietary (Closed-Source)
## Overview
TunerPro is a professional-grade, cross-platform desktop ECU tuning application designed for Speeduino. It provides real-time data visualization, configuration management, and tuning capabilities.
## Features
- **Real-time Dashboard**: Animated gauges for RPM, Speed, MAP, IAT, CLT, etc.
- **Cross-Platform**: Runs on Windows, Linux, and macOS.
- **Speeduino Support**: Compatible with all official Speeduino board variants.
- **Dark Theme**: Optimized for low-light tuning environments.
## Documentation
Technical documentation, architecture details, and Speeduino reference guides have been moved to the [TunerPro Knowledge Base](https://github.com/tunerpro/tunerpro-knowledge-base) repository.

## Installation & Updates
TunerPro V2 features a native auto-update system.
1. Download the latest installer from the [Releases](https://github.com/tunerpro/tunerpro-v2/releases) page.
2. Install the application.
3. The app will automatically notify you and download updates as they are released.

## Build Instructions
1. Install Qt 6.5+ (MSVC 2019/2022 recommended).
2. Install CMake 3.20+.
3. Run `cmake -B build` and `cmake --build build --config Release`.
4. Generate the installer using `cpack -C Release` (requires NSIS).
