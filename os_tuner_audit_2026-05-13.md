# OS Tuner Audit Report
**Date:** 2026-05-13  
**Auditor:** Antigravity (Claude Opus 4.6)  
**Codebase commit/state:** `52fc68d` (HEAD, 8 total commits) + uncommitted local changes  
**Codebase size:** 96 source files, ~847 KB of C++/H code  

---

## 1. Executive Verdict

OS Tuner (still named "TunerPro" / "TunerStudio OS" inconsistently throughout the codebase) is in **early alpha** state. The architecture is fundamentally sound for a Speeduino-only tuner — serial protocol, INI parsing, page read/write, and real-time telemetry all have real, working implementations with evidence of hard-won bug fixes. However, the project is **Speeduino-only** right now with zero RusEFI or FOME code, the INI parser handles only a subset of real-world INI files, there are no integration tests against real hardware, and the README/docs overclaim relative to actual functionality. The developer is **slightly overestimating** project state — the connection layer works but is fragile, table editing exists but round-trip verification is incomplete, and several "Phase 1" features (datalog replay, sensor calibration, cold-start wizard as a guided flow) don't exist yet. Realistic time to a shippable v1.0 for Speeduino alone is **4-8 months** at current pace. The single biggest risk is the developer building more UI widgets before the connection layer and data pipeline are bulletproof against real hardware.

---

## 2. What Exists vs What's Claimed

| Feature Area | Rating | Evidence |
|---|---|---|
| **Connection layer** | **PARTIAL** | `SerialManager.cpp` (1137 lines): Serial open, DTR handling, signature handshake, command queue with retry/timeout, reconnection logic all implemented. Tested against real Speeduino per conversation history. But: legacy-mode-only ('S' handshake), no new-protocol negotiation tested end-to-end, no connection stability test longer than ad-hoc sessions. |
| **Realtime data feed & gauges** | **PARTIAL** | `SpeeduinoProtocol.cpp:311-423`: Full 130-byte parser with correct offsets. `DashboardWidget.cpp`: RPM/MAP gauges, 8 data cards, status panels, AFR/advance graphs — all wired to live data via `dataReceived` signal. Works in simulation mode. Direct 'A' polling bypass (line 1094-1101) is a real fix. But: gauges are `TunerGauge` custom paint widgets with no smoothing/damping, no configurable gauge layout. |
| **VE/AFR/advance table editing UI** | **PARTIAL** | `AllTablesWidget.cpp` (878 lines), `TableEditor.cpp` (9857 bytes): Table grid with heatmap coloring, cell editing, copy/smooth/interpolate/adjust operations, VTEC dual-cam support. Live tuning writes changes back to ECU. But: axis bins are hardcoded guesses based on table dimensions (lines 620-637), not read from INI/ECU. No undo. |
| **Table 3D visualization** | **PARTIAL** | `TunerMap3DWidget.cpp` (17337 bytes): Exists, conditionally compiled with `HAS_QT_DATAVIS`. Renders 3D surface from table data with selected-cell highlighting. But: requires optional Qt DataVisualization module — won't build without it or gracefully degrade. |
| **Closed-loop visualization** | **ABSENT** | No dedicated closed-loop (O2/EGO correction) visualization widget exists. The `egoCorrection` field is parsed but only shown as a raw number in status bar. No lambda/AFR target vs actual overlay. |
| **Trigger/sync diagnostics** | **PARTIAL** | `ToothLoggerWidget.cpp` (296 lines): UI exists with gap visualizer, tooth time table, capture/clear buttons. `SpeeduinoProtocol::createToothLogRequest()` creates the 'T' command. But: `onToothDataReceived` is never connected to `SerialManager` — the widget receives no data. Lines 115-117 set `m_serialManager` but no signal connection is made. Dead feature. |
| **Datalogging** | **PARTIAL** | `LoggingManager.cpp` (3295 bytes), `LoggingWidget.cpp` (246 lines): Start/stop logging UI, CSV file writing, buffer usage display, record count/size/rate stats. Connected to `dataReceived` signal. But: writes only the legacy `RealTimeData` struct fields, no configurable channel selection, no MSL format compatibility. |
| **Datalog replay** | **ABSENT** | No replay widget, no file-open-for-replay, no playback timeline. Zero code. |
| **Cold-start wizard** | **STUB** | `ColdStartWidget.cpp` (14739 bytes): A settings panel for cranking PW, ASE percentages, WUE curve — this is a configuration page, NOT a guided wizard. It's a form with spinboxes. No step-by-step walkthrough, no "wizard" flow. |
| **Sensor calibration UI** | **ABSENT** | No dedicated sensor calibration widget. MAP sensor type is a bits field in ECU settings, but there's no calibration curve editor, no CLT/IAT thermistor table editor. |
| **Engine protection config** | **PARTIAL** | `LimitersWidget.cpp` (24144 bytes): Rev limiter, boost cut, launch control, DFCO settings all have UI with spinboxes. `engineProtectStatus` bits are decoded in `ECUData.h:370-375`. But: no visual alarm/warning when protections trigger during live tuning. |
| **Session workflow** | **PARTIAL** | `ProjectManager.cpp` (3574 bytes): Loads TunerStudio project directories, finds INI + MSQ files, feeds them to ECUDefinition and MsqParser. `StartupDialog.cpp` provides splash screen with action selection. But: no save-as, no project creation, no diff/compare between tunes. |
| **Bench integration** | **ABSENT** | Zero bench/stimulator code in OS Tuner. The Stim Mega firmware is a separate project. No IPC, no shared protocol, no bench control UI. |
| **Plugin loader** | **ABSENT** | No plugin architecture, no dynamic loading, no extension points. The codebase is fully monolithic. |
| **License key UI** | **ABSENT** | No license key stub, no commercial/free split code. |
| **INI file parsing** | **PARTIAL** | `ECUDefinition.cpp` (1202 lines): Parses `[TunerStudio]`, `[OutputChannels]`, `[Constants]`, `[ControllerCommands]`, `[TableEditor]` sections. Handles `#if/#else/#endif/#set/#define` conditionals, scalar/bits/array types, expression channels (skipped). But: does NOT parse `[CurveEditor]`, `[GaugeConfigurations]`, `[Datalog]`, `[Menu]` sections. Many real Speeduino INI features are silently ignored. Only tested with one INI file. |
| **Tune file save/load** | **PARTIAL** | `MsqParser.cpp` (63 lines): Reads MSQ XML files, extracts `<constant>` and `<pcVariable>` values. But: WRITE is not implemented — there is no `MsqParser::save()`. You can load tunes but not save them to MSQ format. |
| **Documentation** | **OVERCLAIMED** | README claims "Real-time Dashboard," "Cross-Platform," "Speeduino Support," "Dark Theme," "auto-update system" — the auto-update checks a hardcoded GitHub URL (`version.json`) but the download URL points to a non-existent repo. README says "Proprietary (Closed-Source)" while the project goal is open-source. Links to a knowledge base repo that likely doesn't exist. |

---

## 3. Architecture Honest Assessment

### Project Structure
The codebase follows a reasonable Qt/C++ layout: `src/core/` (protocol, data model, settings), `src/widgets/` (UI components), `src/dialogs/`, `src/utils/`, `resources/`, `tests/`. This scales adequately for Phase 1. Concerns are separated — `SerialManager` handles wire protocol, `ECUSettingsManager` handles page cache and value encoding, widgets handle display. The separation is imperfect: `SerialManager.h:75` exposes `m_protocol` as a public member, and `ECUSettingsManager` directly accesses `m_serialManager->m_protocol` (line 716) which breaks encapsulation.

### Tech Stack
- **Qt 6.5+ / C++17 / CMake**: Excellent choice for a cross-platform desktop tuning app. Will age well. Qt SerialPort is the right tool. 
- **Optional deps**: Qt Charts and Qt DataVisualization are optional (good), but the 3D widget won't gracefully degrade if DataVisualization is absent — it'll just be an empty tab.
- **No problematic dependencies.** Pure Qt. License-compatible (LGPL Qt with proprietary app is standard).

### Data Model
Tunes are represented as raw `QByteArray` page caches (`m_pageCache[256]`) in `ECUSettingsManager`, with `ECUDefinition::Constant` structs providing the schema for encoding/decoding. This is the correct approach — it mirrors how TunerStudio works. The `rawToUser`/`userToRaw`/`quantize` methods in `ECUDefinition.h:95-126` are well-implemented with proper rounding. Tables are stored as `QVector<QVector<double>>`. The model will survive Phase 3 if the `ECUDefinition` parser is extended — the page-cache approach is ECU-agnostic.

### Communication Layer
The TunerStudio binary protocol is partially implemented. Legacy mode ('S' signature, raw 'A' polling, 'p'/'M'/'b' page operations) works. New protocol framing (CRC32 wrap/unwrap) is implemented (`wrapNewProtocol`/`unwrapNewProtocol`) but the handshake never negotiates into new-protocol mode — it stays legacy. The protocol is abstracted enough that `CommandType` enum and `processResponse` switch could accommodate other ECUs, but there's no abstract interface — `SpeeduinoProtocol` is hardcoded throughout.

Connection loss is handled via `onErrorOccurred` with auto-reconnect (up to 10 attempts). Partial reads are handled by buffer accumulation in `onReadyRead`. The direct RT polling bypass (line 1094-1101) is a pragmatic fix for queue starvation. CRC errors are counted and retried.

**Critical issue:** `SerialManager::onReadyRead()` lines 335-337 have a bare `catch (...)` that silently swallows all exceptions and discards one byte. This masks protocol parsing bugs. Similarly line 387-388.

### State Management
Application state is spread across: `SerialManager` (connection state, polling state), `ECUSettingsManager` (page cache, dirty flags, values), individual widgets (UI state). There is no centralized state store. This is manageable at current scale but will become painful as features grow. Race conditions: The direct RT polling (`m_waitingForRT`) operates outside the command queue, creating a potential conflict if a queued command response arrives while `m_waitingForRT` is true — the `onReadyRead` method checks `m_waitingForRT` first (line 306) which could consume bytes meant for a queued command's response.

### Testing
One test file exists: `ECUSettingsManagerGuardrailsTest.cpp` (232 lines, 11 test cases). Tests cover constant bounds validation, collision detection, and table bounds validation. These are **unit tests only** — no integration tests, no mock serial port, no protocol round-trip tests. Test coverage is approximately 2-3% of the codebase. The critical paths (serial handshake, page read/write round-trip, real-time data parsing) have zero automated tests.

---

## 4. Code Quality Assessment

### File 1: `SerialManager.cpp` — Quality: 3/5
- **Good:** Command queue with typed routing (`CommandType` enum), retry logic, timeout handling, simulation mode, DTR reset handling with documented bug-fix tags (BUG-003, BUG-007).
- **Bad:** Lines 335, 387: bare `catch(...)` swallows all exceptions silently. Line 362: inconsistent RC_OK detection — direct RT path (line 307-309) correctly avoids checking for 0x06 prefix, but the legacy queue path (line 362) still checks for it, creating two contradictory code paths for the same data format. `m_simulatedMemory[256]` is a 256-element array of `QByteArray` allocated on the stack in the header — this is ~4KB of `QByteArray` objects always present even when not simulating.

### File 2: `ECUDefinition.cpp` — Quality: 3/5
- **Good:** Conditional parsing (#if/#else/#endif) with proper stack semantics (line 112-147). Word-boundary-safe regex for offset keywords (line 349-353, tagged CRIT-5). Robust scalar/bits/array constant parsing.
- **Bad:** `parseOutputChannels` compiles 4 `QRegularExpression` objects on every function call (lines 194-208) — should be `static`. The `load()` method (line 14-72) doesn't reset state before loading — calling `load()` twice accumulates data from both files. `parseTunerStudio` (line 172-189) and sub-parsers all use the pattern of checking for `[` to detect the next section, but they consume the line without returning it to the main parser — if a section header is read by a sub-parser, the main loop never sees it. This works only because the sub-parsers `return` immediately, but it's fragile.

### File 3: `ECUSettingsManager.cpp` — Quality: 3.5/5
- **Good:** Thorough validation: `validateConstantBounds`, `validateTableBounds`, `validateConstantCollisions` all check page ranges, offset bounds, bit ranges, element sizes, zero scales, and out-of-bounds addresses with detailed error messages. Fixed-point scaling (`rawToUser`/`userToRaw`/`quantize`) is mathematically correct. Burn verification via status4 bit polling + CRC check.
- **Bad:** `burnAllDirty()` (line 734-740) calls `burnPage()` in a loop, but `burnPage()` returns immediately if `m_isVerifyingBurn` is true (line 675-679) — so only the first dirty page actually gets burned, the rest are silently skipped. This is a **data-loss bug**.

### File 4: `MainWindow.cpp` — Quality: 2.5/5
- **Good:** Clean signal/slot wiring, proper disconnect-on-destroy (line 149-151), signature validation failure shown to user with actionable dialog.
- **Bad:** Line 676: `m_allTablesWidget->setLiveTuningEnabled(false)` called without null check, but `m_allTablesWidget` null check is done on line 667 in the `if` branch — the `else` branch has no guard, will crash if `m_allTablesWidget` is null. Status bar label text includes the label name in the value (line 424: `"RPM: %1"` sets `m_rpmLabel` which already has a "RPM:" prefix label), causing double-labels like "RPM: RPM: 3000". `onSaveClicked` (line 505) only burns dirty pages — it doesn't save to MSQ file, contradicting the button label "Save Details."

### File 5: `SpeeduinoProtocol.cpp` — Quality: 4/5
- **Good:** CRC32 table is correct (standard reflected polynomial). Extraction helpers handle bounds checking (`offset + 1 >= data.size()` returns 0). 130-byte RT parser covers all documented offsets with authoritative comments. Dynamic INI-driven parser (`parseRealTimeDataDynamic`) is well-structured with proper type dispatch and legacy struct back-mapping.
- **Bad:** `extractUint16LE` (line 143) bounds check uses `>=` instead of `>` — `if (offset + 1 >= data.size())` means a valid last-byte access at `data.size() - 1` would fail if `offset = data.size() - 2`. This is off-by-one: should be `offset + 1 > data.size() - 1` or equivalently `offset + 2 > data.size()`. Same bug in `extractUint16BE` (line 163) and `extractUint32LE` (line 169).

---

## 5. Documentation vs Reality Gap

| Document | Assessment |
|---|---|
| **README.md** | **OVERCLAIMED.** Claims "Proprietary (Closed-Source)" — contradicts stated open-source goal. Claims "Compatible with all official Speeduino board variants" — only tested with one. Claims "auto-update system" — `UpdateChecker.cpp` fetches a JSON from a non-existent GitHub repo. Claims "Cross-Platform: Runs on Windows, Linux, and macOS" — only ever built/tested on Windows. Links to `tunerpro/tunerpro-knowledge-base` repo that almost certainly doesn't exist publicly. |
| **Code comments** | **ACCURATE.** Bug-fix tags (BUG-003, BUG-007, FIX-TX-1, FIX-TX-2, CRIT-1 through CRIT-10) are genuine and match real fixes visible in the code. Offset maps in `ECUData.h` and `SpeeduinoProtocol.cpp` are authoritative and correct per Speeduino firmware 2025.01. |
| **version.json** | **STALE.** Says `1.0.0-alpha` but `CMakeLists.txt` says `2.0.0` and `main.cpp` says `2.0.0-alpha`. Three different version numbers. |
| **In-app help** | **ABSENT.** No help system, no tooltips on settings, no user documentation. |

---

## 6. Critical Bugs — Must Fix Before Any User Testing

### BUG-A: `burnAllDirty()` silently skips pages (DATA LOSS)
**File:** `ECUSettingsManager.cpp:734-740`  
**Severity:** 🔴 CRITICAL  
`burnAllDirty()` loops over all pages and calls `burnPage()` for each dirty page. But `burnPage()` returns immediately if `m_isVerifyingBurn` is true (line 675-679). After the first page burn sets `m_isVerifyingBurn = true`, every subsequent `burnPage()` call in the loop is silently skipped with just a log warning. The user clicks "Save", sees "ECU Saved", but only page N was burned — pages N+1 through N+K are silently lost.  
**Fix:** Queue dirty pages and burn sequentially after each verification completes, or collect dirty pages and burn them in a chain via `onBurnVerifyTimeout`/`onPageCRCReceived` callbacks.

### BUG-B: `MainWindow::onLiveTuningToggled()` null pointer crash
**File:** `MainWindow.cpp:676`  
**Severity:** 🔴 HIGH  
In the `else` branch (disabling live tuning), `m_allTablesWidget->setLiveTuningEnabled(false)` is called without a null check. The `if` branch (line 667) guards with `if (m_allTablesWidget)`, but the `else` branch doesn't. If `m_allTablesWidget` is null, this crashes. The indentation also shows this line is missing a guard — it's indented at 6 spaces (copy-paste artifact) while the surrounding code uses 4.

### BUG-C: Off-by-one in extraction helpers
**File:** `SpeeduinoProtocol.cpp:143, 163, 169`  
**Severity:** 🟡 MEDIUM  
`extractUint16LE` checks `if (offset + 1 >= data.size())` — this means if `offset = data.size() - 2`, the check `(data.size() - 2 + 1 >= data.size())` evaluates to `(data.size() - 1 >= data.size())` which is false, so it passes correctly. **On re-examination, this is actually correct.** The check fails when `offset + 1 == data.size()`, i.e. when the second byte would be at exactly `data.size()` (out of bounds). However, the same pattern in `extractUint32LE` at line 169 (`offset + 3 >= data.size()`) also works correctly. **Downgrading: not a bug.** The bounds checks are correct but use an unusual idiom — `offset + N >= size` is equivalent to `offset + N + 1 > size`.

### BUG-D: `catch(...)` silently swallows exceptions and discards data
**File:** `SerialManager.cpp:335-337, 387-388`  
**Severity:** 🟡 MEDIUM  
Two bare `catch(...)` blocks silently discard one byte from the receive buffer on any exception. This masks protocol parsing bugs and makes serial debugging nearly impossible. At minimum, these should log the exception type and the buffer state before discarding.

### BUG-E: Tooth Logger widget is dead code
**File:** `ToothLoggerWidget.cpp:115-117`  
**Severity:** 🟡 MEDIUM  
`setSerialManager()` stores the pointer but never connects the `onToothDataReceived` slot to any signal from `SerialManager`. The 'T' command (`createToothLogRequest()`) exists in the protocol layer, and the `onCaptureClicked()` method toggles the UI state, but no data ever arrives. The button says "START CAPTURE" but captures nothing.  
**Fix:** Connect `SerialManager::toothDataReceived` signal (which doesn't exist yet) to `ToothLoggerWidget::onToothDataReceived`, and add the actual 'T' command send logic to `onCaptureClicked`.

### BUG-F: Status bar shows double-prefixed labels  
**File:** `MainWindow.cpp:320-341 vs 423-424`  
**Severity:** 🟢 LOW  
The `addStatusField` helper at line 326 creates a prefix label ("RPM:"), then `updateRealtimeData` at line 424 sets the value label to `"RPM: %1"`. Result: the user sees "RPM: RPM: 3000". Same pattern for MAP, AFR, CLT, Boost, Speed, Gear.  
**Fix:** Change `updateRealtimeData` to set only the numeric value, not the prefixed string.

### BUG-G: `ECUDefinition::load()` doesn't reset state
**File:** `ECUDefinition.cpp:14-72`  
**Severity:** 🟢 LOW  
Calling `load()` twice (e.g. switching projects) accumulates constants, tables, and output channels from both files because `m_constants`, `m_outputChannels`, `m_tableDefinitions`, etc. are never cleared at the top of `load()`.

---

## 7. Security & Safety Concerns

| Concern | Assessment |
|---|---|
| **Write protection** | **GOOD.** Signature validation gate blocks all writes (`writeTable`, `sendBurnCommand`) if `m_signatureValidated` is false. The gate is checked in `SerialManager.cpp:258-263` and `277-282`. Can't accidentally write to wrong ECU. |
| **Auto-update** | **DANGEROUS.** `UpdateChecker.cpp:78-111` downloads an arbitrary `.exe` from a URL in the JSON response, saves it to `%TEMP%`, and calls `QProcess::startDetached` to execute it. There is NO hash verification, NO signature check, NO certificate pinning. The URL is fetched over HTTPS but `NoLessSafeRedirectPolicy` allows HTTP downgrade redirects. An attacker who controls the DNS or the GitHub repo can push a trojanized installer. **This must be disabled or hardened before any release.** |
| **INI file injection** | **LOW RISK.** INI parsing doesn't execute arbitrary code — it only populates data structures. The `expandDefines` method does string replacement but can't cause buffer overflows (Qt `QString` is safe). The `#include` directive is logged but ignored. |
| **Serial port access** | **ACCEPTABLE.** No raw memory access beyond the page-cache model. Write operations are bounded by page size validation (`validateConstantBounds`, `validateTableBounds`). The `blockingFactor` chunking prevents oversized writes. |
| **Simulation mode** | **SAFE.** Simulated memory is a fixed 256-element array of `QByteArray`. No external side effects. Can't accidentally trigger real ECU operations. |

---

## 8. Performance Analysis

| Area | Assessment |
|---|---|
| **RT polling rate** | 30Hz (33ms interval, `MainWindow.cpp:374`). Adequate for dashboard gauges. TunerStudio uses 20-50Hz. The direct 'A' bypass (`requestRealTimeData`, line 1094-1101) is an excellent pragmatic optimization that prevents queue starvation. |
| **RT parsing overhead** | `parseRealTimeDataDynamic` iterates every output channel on every poll (605 lines, ~40 channels). Each iteration does a `QString` comparison against well-known names in a 40-entry `if/else if` chain. At 30Hz this is ~1200 string comparisons/second — negligible on desktop. No concern. |
| **INI regex compilation** | `parseOutputChannels` compiles 4 `QRegularExpression` objects on every call (lines 194-208). These should be `static` to avoid recompilation. In practice, `parseOutputChannels` is called once at load time, so this is a code quality issue, not a runtime perf issue. |
| **Table rendering** | `AllTablesWidget.cpp` (878 lines) renders tables using `QTableWidget` with a custom `HeatmapDelegate`. Each cell repaints on data change. For 16×16 tables (256 cells) this is fine. For future 32×32 tables, consider `QTableView` with a model for virtualized rendering. |
| **Memory allocation in simulation** | `m_simulatedMemory[256]` allocates 256 `QByteArray` objects (line 118 of `SerialManager.h`). Each is lazily filled to 4096 bytes on `setSimulationMode(true)`. Total: ~1MB. Always allocated even when not simulating. Not a problem on desktop, but wasteful — could use a `QHash<int, QByteArray>` instead. |
| **Receive buffer growth** | `m_receiveBuffer` grows unboundedly via `append(readAll())` and is only cleared on timeout, error, or successful parse. A fast-sending ECU with a slow parser could accumulate data. The 1024-byte overflow check (line 455-458) provides a safety valve, but 1KB is quite small — could truncate valid data during page reads. Should be at least 4KB. |
| **Signal/slot overhead** | `settingChanged` is emitted for every constant during `extractSettingsFromPage` (line 511). With ~200 constants per page and 15 pages, that's ~3000 signal emissions during a full read. Each emission triggers re-renders in ECUSettingsWidget spinboxes. Consider batching: emit once per page, not per constant. |

---

## 9. Memory Management & Resource Leaks

| Item | Assessment |
|---|---|
| **QObject ownership** | **CORRECT.** All heap-allocated objects (`QTimer`, `QSerialPort`, widgets) are parented to `this` in constructors. Qt's ownership tree handles cleanup. No orphaned allocations found. |
| **Signal disconnection** | **CORRECT.** `MainWindow::~MainWindow` calls `m_serialManager->disconnectFromDevice()` before destruction. `ECUSettingsManager::setSerialManager` disconnects old manager before connecting new one (line 280). |
| **QByteArray copies** | Several hot paths copy `QByteArray` by value: `m_receiveBuffer.left(expectedLen)` at line 316 copies the RT payload every poll cycle. At 130 bytes × 30Hz = 3.9KB/s — negligible. But `m_receiveBuffer.remove(0, N)` at line 327 causes an O(N) memmove of the remaining buffer on every poll. For the typical case (buffer = exactly expectedLen), this is fine since `remove(0, 130)` on a 130-byte buffer is essentially free. |
| **QTableWidgetItem leak in ToothLogger** | `ToothLoggerWidget::onToothDataReceived` (line 268-290) creates new `QTableWidgetItem` objects on every data reception without clearing previous items. `setRowCount` doesn't delete items when growing — only when shrinking. If tooth data arrives continuously, old items accumulate. Should call `clearContents()` before rebuilding. |
| **Network reply cleanup** | `UpdateChecker.cpp:36` calls `reply->deleteLater()` — correct. The download reply at line 90 also uses `deleteLater()`. No network resource leaks. |

---

## 10. INI Parser Coverage Gaps

The `ECUDefinition` parser handles 5 of the ~12 standard TunerStudio INI sections. Here's what's missing and why it matters:

| Section | Status | Impact |
|---|---|---|
| `[TunerStudio]` | ✅ Parsed | Signature extraction works. |
| `[OutputChannels]` | ✅ Parsed | Dynamic RT data works. |
| `[Constants]` | ✅ Parsed | Scalar/bits/array constants extracted. |
| `[ControllerCommands]` | ✅ Parsed | Command bytes decoded. |
| `[TableEditor]` | ✅ Parsed | Table definitions extracted, with fallback dims. |
| `[CurveEditor]` | ❌ Missing | **No 2D curve editing.** WUE curves, VE-by-RPM, injector dead-time vs voltage — all these are `CurveEditor` items in Speeduino INI. Without this, the "ColdStartWidget" WUE spinboxes are manually entering curve points that should be graphically editable. |
| `[GaugeConfigurations]` | ❌ Missing | Gauge ranges, warn/danger thresholds, and units are hardcoded. A proper INI-driven gauge setup would automatically configure the dashboard. |
| `[Datalog]` | ❌ Missing | The logging widget writes a fixed set of fields. A data-driven logger needs the `[Datalog]` section to know which channels to record and their labels. |
| `[Menu]` | ❌ Missing | The UI menu structure is hardcoded. TunerStudio INI files define menus dynamically — this is needed for FOME/RusEFI support. |
| `[SettingGroups]` | ❌ Missing | Settings grouping is manual. |
| `[UserDefined]` | ❌ Missing | Custom pages won't render. |
| `[PcVariables]` | ❌ Missing | PC-side variables (e.g., units preference) not parsed. |

**Verdict:** The parser is adequate for a hardcoded Speeduino-only UI but cannot become a "universal tuner" without `[CurveEditor]` and `[GaugeConfigurations]` at minimum.

---

## 11. Build System Assessment

**CMakeLists.txt:** Well-structured. Qt6 required components are correct. Optional `Charts` and `DataVisualization` are gracefully handled with `find_package(QUIET)` and compile definitions. CPack/NSIS installer config is present.

| Item | Assessment |
|---|---|
| **Minimum CMake version** | 3.20 — good. Qt6 requires 3.16+. |
| **C++ standard** | C++17 — appropriate for Qt6. |
| **AUTOMOC/AUTORCC/AUTOUIC** | All enabled — correct for Qt. |
| **UI files** | 3 `.ui` files listed (`MainWindow.ui`, `DashboardWidget.ui`, `ConnectionDialog.ui`) but the code constructs UI programmatically in many places. These `.ui` files may be stale/unused — `MainWindow::setupUi()` builds everything in C++. |
| **Test target** | `TunerProCoreTests` links against `SerialManager.cpp` + `SpeeduinoProtocol.cpp` — this drags in `QSerialPort` as a dependency for unit tests. Tests should use a mock serial port, not the real one. |
| **Missing source files in CMake** | `src/core/Ms1ExtraNameMap.h` is `#include`d by `ECUSettingsManager.cpp` but doesn't appear in the `HEADERS` list. Qt MOC won't process it (it's not a QObject so that's fine), but it won't appear in IDE project views. |
| **Install target** | `install(TARGETS ... RUNTIME DESTINATION .)` installs the exe to the root of the install tree. No `windeployqt` integration — the installer will ship without Qt DLLs. Need a post-install step or CPack `fixup_bundle`. |
| **Cross-platform** | README claims Linux/macOS support. The build system technically supports it (CMake + Qt6), but there are no CI configs, no platform-specific #ifdefs, no macOS bundle config, no `.desktop` file for Linux. "Cross-platform" is aspirational. |

---

## 12. Testing Strategy Assessment

**Current state:** 1 test file, 11 test cases, ~2-3% coverage.

| Test Category | Status | What's Needed |
|---|---|---|
| **Unit tests (data model)** | 🟡 Minimal | `ECUSettingsManagerGuardrailsTest` covers bounds validation and collision detection. Needs: `ECUDefinition` parser tests against real INI files, `MsqParser` tests, `ExpressionParser` tests. |
| **Unit tests (protocol)** | ❌ None | Zero tests for `SpeeduinoProtocol`. Need: CRC32 known-answer tests, `wrapNewProtocol`/`unwrapNewProtocol` round-trip tests, `parseRealTimeData` with known byte arrays, `parseSignature` with edge cases. |
| **Integration tests (serial)** | ❌ None | No mock serial port. Need: `QIODevice`-based mock that simulates ECU responses, end-to-end handshake test, page read/write round-trip, burn verification flow. |
| **Integration tests (INI)** | ❌ None | No tests loading real Speeduino `.ini` files and verifying constant counts, table dimensions, output channel offsets. The project includes `speeduino.ini` (1313 bytes) but it's a stub, not the real 200KB+ INI. |
| **UI tests** | ❌ None | No widget tests. Acceptable for alpha, but need smoke tests before beta. |
| **Hardware-in-loop** | ❌ None | The Stim Mega project is separate. No automated HIL integration. |

**Recommendation:** Before v1.0, minimum required:
1. Protocol round-trip tests (CRC, parse, encode) — 20 tests, ~2 days
2. INI parser tests against the real `speeduino-2025.01.ini` — 10 tests, ~1 day
3. Mock serial port + handshake test — 5 tests, ~2 days
4. `burnAllDirty` chain test (verifying all dirty pages actually burn) — 3 tests, ~1 day

---

## 13. Naming & Branding Cleanup

The codebase has an identity crisis. All of these names appear in different places:

| Name Used | Where |
|---|---|
| **TunerPro** | `TunerProColors.h`, `TunerProSplashScreen.cpp`, `TunerProCoreTests`, logging path `Documents/TunerPro/Logs`, `TunerPro_Update.exe` |
| **TunerStudio OS** | `MainWindow.cpp:37` (`setWindowTitle`), `AboutDialog`, README, CPack |
| **TunerStudio_OS** | `CMakeLists.txt` project name, `CPACK_PACKAGE_NAME` |
| **OS Tuner** | Workspace directory name |
| **Speeduino** | Protocol code, ECU definition — correct usage (this is the ECU type) |

**Must do before release:** Pick one name and grep-replace the rest. "TunerStudio" is Phil Tobin's trademark — using it in your project name risks a cease-and-desist. Recommend a unique name like "OpenTuner", "SpeedTune", or "TunerOS" (check trademark availability).

---

## 14. Development Roadmap — Realistic v1.0

### Phase 1: Stabilization (Weeks 1-4)
| Task | Priority | Effort |
|---|---|---|
| Fix `burnAllDirty` page-skip bug (BUG-A) | 🔴 P0 | 1 day |
| Fix null crash in `onLiveTuningToggled` (BUG-B) | 🔴 P0 | 15 min |
| Fix status bar double-prefix (BUG-F) | 🟢 P2 | 30 min |
| Fix `ECUDefinition::load()` state reset (BUG-G) | 🟡 P1 | 30 min |
| Add logging to `catch(...)` blocks (BUG-D) | 🟡 P1 | 30 min |
| Disable or harden auto-update (Security) | 🔴 P0 | 1 day |
| Connect ToothLogger to real 'T' command (BUG-E) | 🟡 P1 | 2 days |
| Write protocol unit tests (CRC, parse, encode) | 🔴 P0 | 2 days |
| Write INI parser tests against real Speeduino INI | 🔴 P0 | 1 day |
| Resolve naming/branding (Section 13) | 🟡 P1 | 1 day |
| Fix version number inconsistency (3 different versions) | 🟡 P1 | 30 min |

### Phase 2: Feature Completion (Weeks 5-10)
| Task | Priority | Effort |
|---|---|---|
| Implement `MsqParser::save()` (write MSQ files) | 🔴 P0 | 2 days |
| Parse `[CurveEditor]` section, build curve editor widget | 🟡 P1 | 5 days |
| Undo/redo for table and setting changes | 🟡 P1 | 3 days |
| Parse `[GaugeConfigurations]` for INI-driven gauges | 🟢 P2 | 3 days |
| Configurable datalog channel selection via `[Datalog]` | 🟡 P1 | 2 days |
| Add gauge smoothing/damping to `TunerGauge` | 🟢 P2 | 1 day |
| Live table cell highlight (current RPM/MAP crosshair) | 🟡 P1 | 2 days |
| Add connection stability test (30-min soak test) | 🟡 P1 | 2 days |
| Encapsulate `m_protocol` (currently public member) | 🟢 P2 | 1 day |
| Add real Speeduino INI + MSQ as test fixtures | 🔴 P0 | 1 day |

### Phase 3: Polish (Weeks 11-16)
| Task | Priority | Effort |
|---|---|---|
| Cross-platform build CI (GitHub Actions: Win/Linux/macOS) | 🟡 P1 | 2 days |
| `windeployqt` integration in CMake/CPack | 🔴 P0 | 1 day |
| User documentation / in-app help tooltips | 🟡 P1 | 3 days |
| Closed-loop (EGO correction) visualization overlay | 🟢 P2 | 2 days |
| Datalog replay with playback timeline | 🟢 P2 | 5 days |
| Remove or replace stale `.ui` files | 🟢 P3 | 1 day |
| Performance profiling under sustained 50Hz polling | 🟡 P1 | 1 day |
| Beta test with 3+ different Speeduino board variants | 🔴 P0 | 2 weeks |

**Total estimated effort to shippable v1.0 (Speeduino only): 14-18 weeks** at single-developer pace.

---

## 15. Final Verdict

**The bones are real.** This is not a UI mockup pretending to be a tuning app. The serial protocol, page cache model, INI parser, RT data pipeline, and table editor all contain evidence of real hardware debugging (BUG-003, BUG-007, FIX-TX-1/2, CRIT-1 through CRIT-10). The data model (`ECUSettingsManager` + `ECUDefinition` page-cache approach) is architecturally sound and mirrors how TunerStudio actually works.

**The biggest risk is not technical debt — it's premature feature expansion.** The codebase has 29 widget files, 9 tuning sub-widgets, and 3D visualization support, but the `burnAllDirty` function can't actually burn all dirty pages. The ToothLogger tab has beautiful gap visualization code but receives zero data. The update checker downloads unsigned executables. The priority must be: **make the existing features bulletproof before adding new ones.**

**Three things that must happen before anyone uses this on a real engine:**
1. Fix `burnAllDirty()` — this is a data-loss bug that will corrupt tunes
2. Disable the auto-updater — it's a remote code execution vector
3. Run a 1-hour connected soak test against a real Speeduino — the serial layer has never been stress-tested

If those three are fixed, you have a genuinely usable Speeduino tuner for confident users who don't need hand-holding. The path to v1.0 is clear — it's just about discipline: fix before feature, test before ship.

---

*End of audit report.*
