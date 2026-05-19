# OS Tuner — Implementation Plan

**Date:** 2026-05-13  
**Starting Phase:** Phase 0 (Audit Response)  
**Reason:** The audit identified 7 bugs, a security vulnerability, naming chaos, and version inconsistency. None of Phase 1's user-facing features can be trusted until these are resolved. Sequential execution, one fix at a time.

## Phase 0 — Commit Sequence

| # | Item | Files Touched | Status |
|---|---|---|---|
| 0.1a | BUG-A: `burnAllDirty()` chained burn | `ECUSettingsManager.h`, `ECUSettingsManager.cpp` | ✅ |
| 0.1b | BUG-B: null guard in `onLiveTuningToggled` | `MainWindow.cpp` | ✅ |
| 0.1c | BUG-D: replace bare `catch(...)` with logged catch | `SerialManager.cpp` | ✅ |
| 0.1d | BUG-E: wire ToothLogger to serial 'T' command | `SerialManager.h`, `SerialManager.cpp`, `ToothLoggerWidget.cpp`, `MainWindow.cpp` | ✅ |
| 0.1e | BUG-F: status bar double-prefix | `MainWindow.cpp` | ✅ |
| 0.1f | BUG-G: `ECUDefinition::load()` state reset | `ECUDefinition.cpp` | ✅ |
| 0.2 | Neuter auto-updater | `UpdateChecker.h`, `UpdateChecker.cpp` | ✅ |
| 0.3 | Naming cleanup → "OS Tuner" | `main.cpp`, `MainWindow.cpp`, `CMakeLists.txt`, `AboutDialog.cpp`, `ECUDefinition.h`, `LoggingManager.cpp`, `LoggingWidget.cpp`, `version.json`, `OSTunerColors.h` (alias) | ✅ |
| 0.4 | Version consolidation → `0.5.0-alpha` | `version.h` (new), `main.cpp`, `MainWindow.cpp`, `AboutDialog.cpp`, `CMakeLists.txt`, `version.json` | ✅ |
| 0.5 | README honesty pass | `README.md` | ✅ |

## Remaining work in naming cleanup

The `TunerProColors` namespace is used by 15+ widget files. An `OSTunerColors.h` alias header was created (`namespace OSTunerColors = TunerProColors`). Full migration of `TunerProColors::` → `OSTunerColors::` across all widgets is deferred as it's mechanical and doesn't affect functionality or user-facing branding. Same for renaming `TunerProSplashScreen.h` → `OSTunerSplashScreen.h`.

## Build environment note

The existing toolchain (MinGW 13.1.0 + Qt 6.10.2) has a known compatibility issue with `std::allocator_traits` that prevents the main app target from compiling. This is a pre-existing issue, not caused by Phase 0 changes. The test target (`OSTunerCoreTests`) compiles successfully.

## Next step

Phase 1.1 — Project creation wizard (requires a working build first; toolchain upgrade may be needed).

## Questions filed

See `QUESTIONS_FOR_SAEED.md` for decisions needed:
1. GitHub repo URL for update checker
2. License file timing
3. TunerProColors.h full rename approach
