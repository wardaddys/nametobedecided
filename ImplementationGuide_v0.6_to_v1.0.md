# OS Tuner — Implementation Guide
## From v0.5.0-alpha to v1.0
### Onboarding · Profile · Sensor Glow · Firmware Flasher · Community Tune Hub

> Read this top-to-bottom once before writing any code. Every section answers two questions: **what we're building** and **why we're building it that way and not some other way**. The "why" sections are not commentary — they are the part you'll forget in three weeks and the part a contributor needs to make consistent decisions.

> Project: OS Tuner
> Stack: Qt 6.10.2 / C++ / CMake / MinGW (Windows primary, Linux & macOS later)
> Authors: Muhammad Saeed Sajid, Safeerullah Afridi
> Status: design document — implement in phase order

---

## 0. The North Star

Three principles that override everything else in this document. When a decision is unclear, return here.

1. **The hobbyist with one Speeduino on the bench is the user.** Not shops. Not pros. Not paid plugin customers. Every feature is judged by: "does this make a hobbyist's first 30 minutes with OS Tuner better, or worse?"
2. **The app must never silently do something dangerous.** Flashing firmware, burning a tune, downloading a community tune — every one of these can damage hardware. The app makes these actions easy *and* explicit. Easy ≠ silent.
3. **Ship in small, independently-valuable phases.** Every phase below must be releasable on its own. If Phase F never ships, Phase D should still be a hit. No phase depends on a future phase to make sense.

---

## 1. Roadmap at a glance

| Version | Phase | Headline                           | Effort     | Risk    |
|---------|-------|------------------------------------|------------|---------|
| 0.5.5   | A     | Product tour (no DB, no profile)   | 1–2 days   | Low     |
| 0.6.0   | B     | Profile DB + Sensor Glow + Welcome | 3–5 days   | Low     |
| 0.7.0   | C     | Curriculum + coach hooks           | ~1 week    | Low     |
| 0.7.5   | D     | Firmware flasher (Mega 2560, bundled tunes) | ~1 week    | **High** (can brick boards) |
| 0.8.0   | E     | Firmware fetched from Speeduino official releases + STM32 | 3–5 days   | Medium  |
| 0.9.0   | F     | Community Tune Hub (server + accounts to upload) | ~6 weeks   | **Highest** |
| 1.0.0   | —     | Polish + first stable release      | 1 week     | Low     |

**Why this order:** profile/glow before flasher because the profile drives the curriculum, and the curriculum teaches the user why flashing matters. Flasher before community hub because community downloads are meaningless if the user can't actually install firmware first. Community hub last because it requires a server, accounts, moderation, and legal surface area that the other phases don't.

---

## 2. Decisions already made (don't relitigate)

These are settled. Recorded here so you don't go in circles.

| Topic                          | Decision                                            | Why                                                                                  |
|--------------------------------|-----------------------------------------------------|--------------------------------------------------------------------------------------|
| Email on Welcome               | **Cut.** No email field anywhere in onboarding.     | No server yet, nothing to do with it, GDPR-style surface for zero benefit.           |
| Email later                    | **Collected only when user tries to upload a tune.** | At that moment we genuinely need an account. Anonymous browse + download is fine.    |
| Units                          | **Both supported, chosen in Project Wizard.**        | TunerStudio asks this at project creation; per-project is the correct scope, not per-user. |
| Demo Mode                      | **Off by default. Help → Demo Mode toggles it.**     | Real ECUs are the default expectation. Demo is a learning aid, opt-in.               |
| Garage / multi-car table       | **Cut entirely.** Cars live in projects.             | Project Wizard already collects engine config. A separate garage is duplication.     |
| Uncalibrated-engine warning    | **Hard banner**, persistent, dismissable per-session. | Soft toast is too easy to ignore when something can damage an engine.                |
| Paid plugins                   | **Year 2+ only. Shop-tier only. Invisible in v0.6.** | Hobbyist product needs to feel complete on its own first.                            |
| Coming-soon teasers            | **None.** No "Pro features coming" anywhere.         | Crippled-feeling free products lose to whole-feeling free products.                  |
| Speedy Loader                  | **Built into OS Tuner natively** (Phase D+).         | Removes external dependency and powers the community tune hub later.                 |
| Community tune accounts        | **Anonymous download, account required to upload.**  | Lowest-friction discovery + accountability for the people creating risk.             |

---

## 3. The bug to fix before shipping anything

### 3.1 Shutdown assert: `Called object is not of the correct type (class destructor may have already run)`

**Symptom (from your build log):**

```
[INFO] Application shutting down with exit code: 0
[INFO] === OS Tuner Terminated ===
[WARN] ECU connection lost
[INFO] Disconnected from device
ASSERT failure in MainWindow: "Called object is not of the correct type..."
```

**Why it's happening:**
`SerialManager` is emitting `connectionLost` / `disconnected` from inside its destructor. By the time those signals fire, some receiver (most likely `MainWindow` or a child widget) has already been destroyed. Qt's `qobjectdefs_impl.h` line 105 asserts when a queued/direct connection tries to call into a `this` pointer whose vtable is gone.

This is the classic Qt parent-child destruction ordering bug: children are destroyed *after* the parent's destructor body has executed, so a child emitting a signal during its own teardown can hit a parent that's already a husk.

**Fix (in order of preference):**

1. **Best fix — disconnect explicitly in `MainWindow::~MainWindow()` body, first line:**
   ```cpp
   MainWindow::~MainWindow() {
       if (m_serialManager) disconnect(m_serialManager, nullptr, this, nullptr);
       // ... rest of cleanup
   }
   ```
   *Why:* tells Qt's signal machinery to stop routing anything from `SerialManager` to `this` *before* the parent's destructor body has finished — so by the time child destructors run, there are no dangling connections.

2. **Belt-and-braces fix — also blockSignals in `SerialManager::~SerialManager()`:**
   ```cpp
   SerialManager::~SerialManager() {
       blockSignals(true);
       // ... existing teardown that emits connectionLost / disconnected
   }
   ```
   *Why:* even if some other receiver is still wired up, the signals simply don't fire. Safe and idempotent.

3. **Architectural fix (later) — make `SerialManager` not emit during its own destructor.** Move the "I'm disconnecting" emission to an explicit `shutdown()` method that `MainWindow` calls *before* deleting `SerialManager`. *Why:* destructors emitting signals is fragile by nature; ideally destructors only release resources.

**Recommendation:** apply (1) and (2) together for v0.5.5. Plan (3) as cleanup later.

### 3.2 The `mapping.yaml` warning — 41 unmapped names

**Symptom:**

```
WorkspaceMetadata: mapping.yaml references 41 name(s) with no matching Constant, Table, or PC Variable:
aseHold, asePctCold, baroSensor, boostCutEnable, boostCutPsi, canBcastMode, ...
```

**Why it's happening:**
`resources/workspaces/mapping.yaml` references Speeduino setting names that don't exist in the loaded `mainController.ini`. Two possible causes:

1. **Naming drift** — Speeduino renamed the constants between firmware versions, and the yaml is stale.
2. **Section filtering** — some names live in `.ini` sections your parser ignores (`[frontpage]`, `[loggerdefinition]`, `[tools]`, `[referencetables]`, `[megatune]`, `[eventtriggers]`).

The log shows only `19 of 760 constants mapped`, which means workspace tabs are mostly empty. This is the actual user-visible bug.

**Fix approach:**

1. Take 5 missing names — e.g. `injectorCc`, `dwellRunning`, `triggerType`, `mapAt0V`, `softRevLim` — and grep them in `C:\Users\admin\Documents\TunerStudioProjects\MyCar\projectCfg\mainController.ini`.
2. If they're missing entirely → it's a rename. Find the new name in the ini and update yaml.
3. If they're present but in an ignored section → either un-ignore that section or copy the constant to one that's parsed.

**Why this isn't urgent for the crash fix:** unrelated to the assert. But it's the reason your workspaces look empty, so it must be fixed before user-facing release.

**Bonus issue in the same log:** the ECU `.ini` is parsed three times on startup (you can count the `phase-2 [tunerstudio]` blocks). That's a perf bug — a ~700KB ini file shouldn't be loaded thrice. Cache the parsed result after first load. Defer to v0.7.

---

# Phase A — v0.5.5: Product Tour

**Goal:** every new user understands what they're looking at in 90 seconds. Existing users see nothing change.

**Effort:** 1–2 days.

**Why first:** zero new dependencies, zero new schemas, zero risk. Lets you ship something user-visible *now* while we plan the bigger work.

## A.1 What we build

A 6-step coachmark overlay on the existing MainWindow, gated by a single `Settings::firstRunCompleted` flag.

**Files to add:**
- `src/widgets/onboarding/ProductTourOverlay.h/cpp` — full-window semi-transparent overlay that highlights one widget at a time with a pulsing ring.
- `src/widgets/onboarding/TourStep.h` — small struct: anchor widget pointer, title, body, "Next/Skip/Done."

**Files to modify:**
- `src/utils/Settings.h/cpp` — add `getFirstRunCompleted()` / `setFirstRunCompleted()`.
- `src/MainWindow.h/cpp` — in `postInit()`, if `!firstRunCompleted` and no project is loading, show the tour overlay.
- `src/MainWindow.cpp` — add `Help → Replay Onboarding` menu entry.

## A.2 The six steps

| # | Anchor                          | What the user sees                                                                |
|---|---------------------------------|-----------------------------------------------------------------------------------|
| 1 | Dashboard gauges row            | "These are your live engine sensors. Once your ECU is connected, they update 100× a second." |
| 2 | A deliberately-not-calibrated gauge | "If a gauge glows orange, that sensor isn't calibrated yet. Click it to set it up." *(in Phase A this is just an illustration — the glow ships in Phase B)* |
| 3 | Workspace tabs                  | "Fueling, Ignition, Sensors — this is where you tune. Most of your time will be in Fueling." |
| 4 | The 3D fuel map (`TunerMap3DWidget`) | "This is a VE table. Each cell tells the ECU how much air the engine is breathing at that RPM and load." |
| 5 | Burn button                     | "Edits live in memory until you Burn. Burn writes them to the ECU permanently. Always Burn before unplugging." |
| 6 | The Help menu (pointing at "Demo Mode") | "No ECU yet? Help → Demo Mode lets you explore with simulated data — nothing you do here can hurt anything." |

End screen: a single dialog — "Want the full guide on tuning? Help → Learn to Tune. Otherwise, you're ready."

## A.3 Why these six and not eight, twelve, or three

- **Not three:** three steps is too little to orient — you have a complex app, the user needs to know the *shape* of it.
- **Not twelve:** users who clicked Continue four times will skip the fifth. After 6 they're done psychologically. Anything longer must be opt-in (Phase C's curriculum).
- **Why these specific six:** they trace the user's *actual* first tuning session — look at sensors → know how to fix uncalibrated ones → know where to make changes → understand the most important screen → know how to save → know how to explore safely. That's the minimum mental model.

## A.4 Skip behavior

- Skip button in the top-right of every step, always visible.
- Skipping sets `firstRunCompleted = true` and never bothers the user again unless they re-trigger from Help.
- No "are you sure you want to skip?" dialog. **The skip path must be one click.** Anything else is hostile.

## A.5 Demo Mode in the tour — important nuance

You said: don't auto-enable Demo Mode. Correct. Step 6 only *points at* the Help menu entry, doesn't toggle anything.

**Why this matters:** a beginner who sees gauges suddenly start moving will think their ECU is connected when it isn't. Worse, they might Burn nonsense values. Demo Mode must be a deliberate user choice, not a tour side-effect.

---

# Phase B — v0.6.0: Profile, Welcome screen, Sensor Glow

**Goal:** the app knows who you are (lightly), and unconfigured sensors visually nag you until you fix them.

**Effort:** 3–5 days.

## B.1 The Welcome screen — what's left after the cuts

Since we cut email, units, and garage, the Welcome screen is now a **two-step, ~30-second flow**:

### Step 1 — "Hi, what should we call you?"
- Single text field: display name (optional).
- Below it: a checkbox unchecked by default — "Receive update notifications inside the app when a new OS Tuner release is available." *(Note: this is a local check against GitHub releases, not an email subscription.)*
- "Skip everything" link in the top-right.

### Step 2 — "How much tuning experience do you have?"
- Three big radio cards:
  - **First time tuning a car.** "We'll show extra tooltips and link to the curriculum where relevant."
  - **I've tuned cars before.** "Standard UI, fewer hand-holds."
  - **Pro tuner.** "Minimal tooltips, terser warnings, defaults trust your judgment."
- Below the cards: link "What's the difference?" → expandable text explaining what each setting actually changes in the UI.

That's it. Two screens, no email, no car, no units. Continue → into the existing Product Tour.

### Why we kept "experience level" but cut everything else

Experience level is the **only** field that actually changes the product behavior on day one:
- Beginner: tooltips are verbose; first-time warnings (first VE table edit, first Burn) appear.
- Intermediate: standard.
- Pro: tooltips are 1-line; first-time warnings appear *once* and never again, even on a different machine.

Name is cosmetic (title bar greeting, MSQ author field). Everything else we cut had no behavior tied to it — collecting data with no purpose is bad UX and worse data hygiene.

### Why we kept the in-app update check checkbox

It's the only "phone home" the app does, and it's opt-in. Even if the user opts in, it's a single anonymous HTTPS call to GitHub's releases API. No email, no telemetry, no ID. *Why have it at all:* hobbyists often run installs for months without updating and miss safety fixes. A passive check is the safest way to keep them current.

## B.2 The profile database — `profile.db`

A SQLite file living next to `OSTuner.ini` in `C:\Users\<user>\AppData\Roaming\OSTuner\profile.db`.

### Final schema (garage removed)

```sql
CREATE TABLE profile (
    key             TEXT PRIMARY KEY,
    value           TEXT
);

CREATE TABLE tutorial_progress (
    module_id       TEXT PRIMARY KEY,    -- 'fueling_basics', 'safety', etc.
    completed_at    TEXT,
    skipped         INTEGER DEFAULT 0
);

CREATE TABLE sensor_acknowledgements (
    project_path    TEXT NOT NULL,        -- which project this ack belongs to
    sensor_id       TEXT NOT NULL,        -- 'tps', 'map', 'clt', etc.
    acknowledged_at TEXT NOT NULL,
    PRIMARY KEY (project_path, sensor_id)
);
```

### Why these three tables, no more, no fewer

- **`profile`** — flat key/value because the fields we collect are few and unlikely to grow predictably. If we used columns, we'd hit a schema migration the first time we add a field. Key/value never migrates.
- **`tutorial_progress`** — needs per-module rows because we want green-checks per module in the Help menu. Could be jammed into `profile` as `tutorial.fueling_basics = completed`, but it's cleaner to have a real table with a real timestamp.
- **`sensor_acknowledgements`** — *per-project*, not per-user. Why: a user might tune two cars, one of which uses the default Bosch CLT curve (acknowledged) and one of which has a custom GM thermistor (calibrated). Same user, same machine, different ack state per project. Tying it to project path keeps each car's state independent.

### Why no garage table

You said: cars belong to projects. Correct. The Project Wizard already collects make/model/engine/displacement/cylinders/forced-induction. Duplicating that into a "garage" was redundant. The list of projects on disk *is* the garage.

The follow-on for Phase C: the "switch project" dropdown in the dashboard already gives you a multi-car experience for free. We don't need to build it.

## B.3 The Sensor Glow system

This is the headline feature of v0.6. Everything else supports it.

### B.3.1 The four states

```cpp
// In src/widgets/TunerGauge.h
enum class CalibrationState {
    Calibrated,      // user has explicitly set this sensor's calibration
    Acknowledged,    // user reviewed the default and accepted it
    Unset,           // sensor is using firmware default AND user hasn't looked at it
    Invalid          // sensor config is broken (e.g., TPS min >= max)
};

void setCalibrationState(CalibrationState s);
```

### B.3.2 Why four states and not two (calibrated / not calibrated)

A binary system has a fatal flaw: it can't distinguish *"the user wants the default"* from *"the user hasn't looked yet."* If we only had two states, a pro tuner using the stock Bosch CLT curve would see a permanent amber glow on the CLT gauge they intentionally left alone. They'd disable warnings entirely, and we'd lose the safety mechanism for everyone.

The four states cover the real cases:
- **Calibrated** = user actively set values different from defaults.
- **Acknowledged** = user opened the calibration panel, looked at defaults, said "yes those are fine for my setup." Stored in `sensor_acknowledgements`.
- **Unset** = defaults + never opened. This is the only state that glows amber.
- **Invalid** = a real error (min > max, sensor type "Unknown," etc.). Glows red, not amber.

### B.3.3 Visual spec

- **Calibrated** — small green dot in the gauge corner, no animation.
- **Acknowledged** — small grey dot in the gauge corner, no animation.
- **Unset** — 4px soft amber halo (`#FFB020`), opacity pulsing 0.4 ↔ 1.0 on a 2-second `QPropertyAnimation` cycle.
- **Invalid** — 4px steady red halo (`#FF4040`), no pulse.

**Why slow pulse, not fast:** fast pulsing reads as "alarm — something is wrong now." Slow pulsing reads as "this needs your attention sometime." The first panics; the second informs. Speed has psychological weight — pick it carefully.

**Why amber for unset and red for invalid:** color-codes severity. Unset is "you haven't done this yet" — not an error, just incomplete. Invalid is "this configuration cannot work." Different problems, different colors.

### B.3.4 Click-through behavior

Clicking a glowing gauge opens the existing `CalibrationOverlay` directly to that sensor's panel. You already have `CalibrationOverlay::showCalibration(const QString &title)` — wire each gauge to call it with the right title.

Inside the calibration overlay, at the bottom:
- A new checkbox: **"Defaults are fine for my setup — don't warn me about this sensor."**
- When checked and the user clicks Accept, write a row to `sensor_acknowledgements` and set the gauge to `Acknowledged` state.

### B.3.5 The unset-detection logic

Build a small helper class:

```cpp
// src/core/CalibrationChecker.h
class CalibrationChecker {
public:
    static CalibrationState getStateFor(const QString &sensorId,
                                         ECUSettingsManager *settings,
                                         const QString &projectPath);
};
```

For each sensor, the check is:

| Sensor | "Unset" if...                                                                  |
|--------|--------------------------------------------------------------------------------|
| TPS    | `tpsMin == firmware_default_tpsMin && tpsMax == firmware_default_tpsMax`        |
| MAP    | `mapMin == firmware_default && mapMax == firmware_default && mapSensorType == "Custom"` |
| CLT    | CLT thermistor table bytes match the firmware default Bosch curve byte-for-byte |
| IAT    | Same as CLT, separate table                                                    |
| O2     | `o2Type == 0` (disabled) OR wideband curve is the firmware default              |
| RPM    | `numTeeth == 0` OR `triggerPattern == "Missing"` (no decoder picked)             |
| Inj    | `reqFuel == 0` OR `injectorCc == 0`                                            |
| Battery| Always `Calibrated` (no calibration possible)                                  |

Then check `sensor_acknowledgements` — if a row exists, upgrade from `Unset` to `Acknowledged`.

Then check validity — if any field is structurally broken (min ≥ max, negative when positive required), force to `Invalid`.

### B.3.6 The Calibration Center

A small panel accessed from a `Health` button on the dashboard header:

```
┌─ Calibration Center ─────────────────────────────┐
│  ●  TPS         Calibrated     2026-05-12        │
│  ⚠  MAP         Not set        Calibrate →       │
│  ●  CLT         Acknowledged   Default Bosch     │
│  ⚠  IAT         Not set        Calibrate →       │
│  ⚠  O2          Not set        Calibrate →       │
│  ●  RPM/CKP     Calibrated     36-1              │
│                                                  │
│  3 of 6 sensors ready. Engine: not running.      │
└──────────────────────────────────────────────────┘
```

**Why a separate panel and not just rely on the gauges:** the dashboard answer to "am I ready to tune?" should be a single screen, not a visual sweep of six different gauges. The Calibration Center is the explicit answer.

### B.3.7 The hard banner — running with uncalibrated sensors

When `RealTimeData.rpm > 200` AND any of {TPS, MAP, CLT} is `Unset`:

> ⚠ **Engine is running with uncalibrated sensors. Readings and fueling may be wrong. Stop the engine and calibrate before tuning.**

- Bright amber background, dashboard-wide, top of window.
- Dismissable for the current session (small ×).
- Re-appears on next launch until the sensor is `Calibrated` or `Acknowledged`.

**Why hard banner and not toast:** a running engine with bad sensors can throw a rod. A toast that fades after 3 seconds is the wrong response to that risk. The banner persists until the user has explicitly engaged with the problem.

**Why only TPS/MAP/CLT and not all six:** those three are the sensors fueling actually depends on for AFR calculation at every cycle. The others matter, but bad O2 or IAT degrades the tune over time, not in the first 30 seconds. We rank the safety check by what blows up engines fastest.

---

# Phase C — v0.7.0: Curriculum + coach hooks

**Goal:** the user goes from "knows the UI" to "knows tuning." Voluntary, lives in Help.

**Effort:** ~1 week (mostly writing, not coding).

## C.1 The ten modules

Each is a single `QWebEngineView` (or `QTextBrowser`) page with embedded screenshots from OS Tuner itself.

1. **What is engine tuning?** AFR, stoich, rich/lean, knock. *Why first:* you can't read sensors meaningfully without the vocabulary.
2. **Reading the sensors.** TPS, MAP, MAF, IAT, CLT, O2/wideband, RPM. What each measures, what good values look like at idle and WOT, what a broken reading looks like. *Why second:* sensor literacy before sensor calibration.
3. **Calibrating your sensors.** Walks through the existing `CalibrationOverlay` with a "Try it now" button that opens it.
4. **Fueling.** VE tables, Req Fuel, injector size. Walks through `FuelingWorkspace`.
5. **Ignition.** Timing, advance, knock, dwell. Walks through `IgnitionWorkspace`.
6. **Cold start, warmup, accel enrichment.** Drivability. Why a car can idle fine and still hesitate.
7. **Closed-loop vs open-loop.** When the ECU trims itself, when it doesn't.
8. **Datalogging.** What to record, what to look for. Existing `LoggingWidget`.
9. **Safety: when to stop.** Knock retard, lean spikes, coolant/oil temp. *Why so late:* doesn't make sense until you understand sensors and fueling.
10. **Your first tune, step by step.** Hour-long checklist. Synthesis of everything above.

### Why ten and not five or twenty

- Fewer than 10 forces topics to be compressed past the point of usefulness.
- More than 10 starts to look intimidating in the Help index — the user sees "module 18 of 22" and bounces.
- Ten fits on screen as a single scrollable list with green checks beside completed modules. It's the upper bound of "I can do this in a weekend."

### Why each module gets a "Try it now" button

Reading is the worst way to learn tuning. Reading then *doing* in the actual UI is dramatically better. Each module that maps to a real OS Tuner screen has a button that opens it — Demo Mode is auto-enabled if no ECU is connected, then disabled when the module ends.

This is the **one place** Demo Mode is auto-toggled. Justified because the user is explicitly opting into a tutorial; nothing happens silently behind their back.

## C.2 Coach hooks (the curriculum in disguise)

Curriculum modules sitting in a Help menu nobody opens are wasted. The coach hooks pull users *into* the curriculum exactly when they need it.

| Trigger                                            | Surface                                                                | Why                                                  |
|----------------------------------------------------|------------------------------------------------------------------------|------------------------------------------------------|
| User opens VE table, has not completed "Fueling"   | Non-blocking banner: "New to VE tables? 4-min primer →"                | Just-in-time learning beats prebuilt lessons.        |
| User clicks Burn, has not completed "Safety"       | Soft modal: "Burning writes to a live ECU. 90-sec safety primer?" with Read / Burn anyway / Don't ask again | The single highest-risk action deserves a friction-point. |
| User saves first datalog                           | Toast: "First log saved. Want to learn how to read it?"                | Praise + invitation. Logs are intimidating to read alone. |
| User connects to a real ECU for the first time     | Toast: "Connected. Run the Calibration Center to check your sensors first." | Pull users toward calibration before they tune.      |
| User opens Ignition workspace for the first time   | Banner: "Ignition timing primer recommended before changing values."   | Ignition is more dangerous than fueling to get wrong. |

**Each banner/toast is dismissable with a "Don't show this again" option** that writes to `tutorial_progress` with `skipped = 1`. We don't nag forever.

---

# Phase D — v0.7.5: Firmware flasher (the "Speedy Loader" replacement)

**Goal:** brand-new user goes from "bare Speeduino board" to "firmware installed and starter tune loaded" without ever leaving OS Tuner.

**Effort:** ~1 week.

**Risk:** High. A bug here bricks user hardware.

## D.1 Why we're building this at all

Today's flow for a beginner with a fresh Mega 2560:
1. Buy Speeduino kit.
2. Open OS Tuner, get "no firmware detected" error.
3. Google "how to flash Speeduino."
4. Download Josh's Speedy Loader.
5. Run Speedy Loader, flash firmware.
6. Come back to OS Tuner, finally connect.

We can collapse steps 2–6 into one button. That's a huge first-run win. It's also a prerequisite for the community tune hub — you can't have "download this tune and run it" if the user hasn't flashed firmware yet.

## D.2 What we ship in Phase D specifically

- Bundled `avrdude.exe` inside the OS Tuner installer (Mega 2560 / ATmega2560 support only).
- 3–5 official Speeduino firmware `.hex` files, bundled in `resources/firmware/`.
- 3 hand-picked **starter tunes** for known engines, bundled in `resources/tunes/`. *Suggestion (pending your validation): a generic 4-cyl NA 1.6L, a 4-cyl turbo 2.0L, a 4-cyl NA 2.0L. These are placeholder picks — replace with engines you've actually validated.*
- UI: a new "Flash Firmware" entry that's accessible from three places — *but they all open the same dialog:*
  1. Project Wizard, when no firmware is detected on the connected board.
  2. `Tools → Flash Firmware…` menu.
  3. `ConnectionDialog` — new "Flash Firmware" button next to "Connect."

### Why all three entry points open the same dialog

A user might arrive at the need to flash firmware from three different mental contexts. We meet them at each, and once they're in the dialog the experience is identical. No "wait, this is different from the dialog I saw last time."

## D.3 Why bundle `avrdude` instead of writing native flashing

`avrdude` is the standard tool, ~500KB, GPL+exception licensing that's compatible with our distribution. Writing native STK500 in Qt is 1–2 weeks of work and we'd be debugging serial-flash edge cases that avrdude already handles. Not the place to be original.

For STM32 (Phase E), we'll likely use `stm32flash` or DFU — different tool, same wrap-it pattern.

## D.4 Why bundle firmware instead of fetching from GitHub on first launch

For Phase D, the installer is the source of truth. Predictable, offline-capable, easy to test. Phase E adds the fetch-latest behavior on top.

**Why this is the right order:** flashing is high-risk, networks are unreliable. A user with no internet still needs to be able to flash. Get the offline path working perfectly first, then add the online path.

## D.5 The flash dialog UX — Tesla-style for hobbyists

```
┌─ Flash Speeduino Firmware ────────────────────────────────┐
│                                                            │
│  Board:        Arduino Mega 2560   ▾                       │
│  Port:         COM4 (Arduino Mega 2560)   ▾   [Refresh]    │
│  Firmware:     Speeduino 202501-stable   ▾                 │
│                                                            │
│  Optional starter tune:                                    │
│  ⦿  None — I'll tune from scratch                          │
│  ○  4-cyl NA 1.6L generic                                  │
│  ○  4-cyl Turbo 2.0L generic                               │
│  ○  4-cyl NA 2.0L generic                                  │
│                                                            │
│  ⚠ Flashing will erase any existing firmware on the board. │
│    Make sure the board is plugged in and not connected     │
│    to a running engine.                                    │
│                                                            │
│  ☐ I understand this can damage hardware if used wrong.    │
│                                                            │
│      [ Cancel ]            [ Flash Now (disabled) ]        │
└────────────────────────────────────────────────────────────┘
```

The Flash button is disabled until the safety checkbox is ticked. The checkbox resets every time the dialog opens — **not** a "remember my choice" preference. Friction is the point.

### Why progressive disclosure matters here

A pro can fly through: pick board, pick firmware, check box, flash. ~10 seconds. A beginner gets time to read the warning. Same UI, two speeds.

## D.6 Failure modes and what we do about them

| Failure                                          | What we show                                                                 | Why                                                |
|--------------------------------------------------|------------------------------------------------------------------------------|----------------------------------------------------|
| Wrong COM port                                   | "Couldn't reach the board. Is it plugged in and on COM4?"                    | Avrdude's raw error is unreadable.                 |
| Antivirus blocks `avrdude.exe`                   | "Your antivirus appears to be blocking the flasher. Add avrdude.exe to allowed apps." (with link to how-to) | Extremely common on Windows.                       |
| User unplugs board mid-flash                     | "Flash interrupted. Your board may be in bootloader-only mode. Reconnect and try again." | Recovery is just re-flashing — we tell them that.  |
| Wrong firmware variant for the board             | Pre-flight check fails → "This firmware is for ATmega2560. Detected: STM32." | Catch before flashing, not after.                  |
| Flash hangs                                      | After 60 sec: "Flash didn't complete. Power-cycle the board and try again."  | Default avrdude timeout is too long.               |
| Successful flash                                 | "Flashed Speeduino 202501. Loading starter tune…"                            | Confirmation matters; silence after a flash feels broken. |

## D.7 The post-flash starter tune

After a successful flash, if the user chose a starter tune, we:
1. Wait 3 seconds for the board to reboot.
2. Connect via serial.
3. Burn the starter tune's pages to the ECU.
4. Open a small modal: "Starter tune loaded. Before starting your engine, run **Help → First Tune Checklist**."

That checklist (module 10 of the curriculum) is the explicit pathway from "flashed board" to "engine running safely."

**Why we always nudge to the checklist after a starter tune:** starter tunes are generic. Every engine is different. A starter tune that runs is *not* a tune that's safe to drive on — it's a tune that probably idles. Making this distinction unambiguous keeps users from melting engines.

## D.8 Where the bundled starter tunes come from — you decide

Three options:

1. **You tune them yourself** (or with a trusted contributor) on real engines.
2. **Pull from Speeduino's public reference tunes**, with credit.
3. **Bundle no starter tunes for Phase D**, ship only the flasher.

Option 3 is the safest. Options 1 and 2 are higher-value but require you to take responsibility for the tune working. **My recommendation: ship Phase D with option 3 — flasher only — and add starter tunes in Phase E once you've personally verified at least three.** Pre-tested tunes are a feature, not table stakes.

---

# Phase E — v0.8.0: STM32 support + firmware updates from official source

**Goal:** the flasher covers modern Speeduino builds and stays current automatically.

**Effort:** 3–5 days.

## E.1 STM32 support

- Add `stm32flash` (or DFU-util) to bundled tools.
- Add board options: STM32F4 Black Pill, STM32 Nucleo, Teensy 3.5/3.6.
- Same flash dialog, different backend per board.

**Why STM32 now and not in Phase D:** STM32 boards are increasingly common in new Speeduino builds, but the bulk of existing hobbyist installs are still on Mega 2560. Mega first gets you 80% of users; STM32 closes the gap.

## E.2 Online firmware updates

- On startup (if the user opted in during Welcome), the app checks GitHub for new official Speeduino releases.
- If newer than what's bundled, prompt: "Speeduino 202503 is available. Download? (~120KB)"
- Downloaded firmware lives in `%AppData%/OSTuner/firmware/cache/` — never overwrites bundled fallback.

### Why we never overwrite the bundled fallback

If GitHub is down, the user's network is flaky, or the new firmware is broken, they always have a known-good version they can re-flash from. The cached version is additive, not destructive.

## E.3 The validated starter tunes (deferred from Phase D)

By v0.8, you should have at least 3 engine configs that you've personally verified on bench-running engines. Ship those as the starter tunes. Each one tagged with:
- Engine code
- Displacement
- Forced induction status
- Injector size
- Validator name + date

The validation metadata previews what we'll need for the community tune hub in Phase F — the schema is intentionally the same.

---

# Phase F — v0.9.0: Community Tune Hub

**Goal:** users can browse, download, and (with an account) upload tunes for other users' engines.

**Effort:** ~6 weeks. **Risk:** highest in the entire roadmap.

> Read Section 0's North Star again before designing this. Every decision below cascades from "the app must never silently do something dangerous."

## F.1 What you commit to when you ship this

A server you operate. Forever. Or until you hand it off. There is no zero-cost, zero-maintenance version of a content-hosting platform with safety implications.

Concretely:
- A hosted API (Cloudflare Workers or a small VPS + Node/Rust/Go).
- An object store for tune files (Cloudflare R2 or S3).
- A database (Postgres or SQLite-on-server with daily backups).
- An account system (email + password or OAuth).
- A moderation queue and at least one human reviewer (you, initially).
- A takedown / abuse reporting flow.
- Privacy policy + Terms of Service documents.
- Monitoring + uptime alerting.

**Estimated ongoing cost at modest scale:** $20–50/month. **Estimated ongoing time:** 2–4 hours/week minimum for moderation alone.

If those numbers don't fit your life right now, **skip Phase F entirely until they do.** Phases A–E are independently complete.

## F.2 The architecture

```
┌─────────────────┐         ┌──────────────────┐         ┌─────────────────┐
│  OS Tuner       │ HTTPS   │  ostuner.dev API │         │  R2 / S3        │
│  (desktop app)  │ ◄─────► │  + Postgres      │ ◄─────► │  tune .msq      │
│                 │         │                  │         │  files          │
└─────────────────┘         └──────────────────┘         └─────────────────┘
                                     │
                                     │ Webhook
                                     ▼
                            ┌──────────────────┐
                            │  Moderation UI   │
                            │  (web, for you)  │
                            └──────────────────┘
```

### Why a real server and not "just GitHub releases"

A GitHub repo of tune files works for ~50 users. At 500 it falls apart: no search by engine code, no ratings, no per-tune metadata, no moderation queue, no upload UX better than "fork the repo and PR." We'd be reinventing all of that on top of a forge that wasn't designed for it.

## F.3 The accounts model — your decision restated correctly

- **Browse: no account.** Anyone can open the Tune Hub inside OS Tuner and see what's available.
- **Download: no account.** Anyone can download. The friction would discourage exactly the users we want to help.
- **Upload: account required.** Email + password, or OAuth via GitHub/Google.
- **Rate / comment: account required.** Accountability for everything that's public.

### Why upload-gated and not browse-gated

Anonymous downloads are the on-ramp. Most users will never upload — they just want to find a tune for their engine. Forcing them to make an account to look around would lose them to forums where they can browse freely. Upload is where accountability matters, because uploaders are the ones generating risk for others.

### What we collect on signup

- Email (required, verified).
- Display name (required, can be a handle, must be unique).
- Password (required, hashed with argon2id).
- Optional: linked GitHub account for verification badges.

That's it. No real name, no address, no payment. **The smallest account possible to assign accountability for uploads.**

## F.4 The tune metadata schema

Every uploaded tune must have:

| Field                     | Required | Notes                                                       |
|---------------------------|----------|-------------------------------------------------------------|
| Engine make/model/year    | Yes      | Free text + suggested dropdown                              |
| Engine code               | Yes      | e.g., B16A, K20A, 4G63                                      |
| Displacement (cc)         | Yes      |                                                             |
| Cylinders                 | Yes      |                                                             |
| Forced induction          | Yes      | None / Turbo / Supercharger                                 |
| Injector size (cc/min)    | Yes      |                                                             |
| ECU firmware version      | Yes      | Speeduino version this tune targets                         |
| Dyno-proven?              | Yes      | Yes / No                                                    |
| Track-only?               | Yes      | Yes / No — gates a stronger warning if yes                  |
| Pump fuel grade           | Optional | 91 / 93 / 98 / E85 / Race                                   |
| Description               | Yes      | 200-character minimum to discourage low-effort uploads      |
| Known limitations         | Optional | "Cold start may be rough below 5°C"                         |
| Author handle             | Auto     | From account                                                |
| `.msq` file               | Yes      | Validated server-side as parseable                          |

**Why so much metadata:** the difference between "good tune" and "tune that ruins an engine" is often whether the downloader's engine matches the uploader's. Forcing structured metadata makes mismatches catchable.

## F.5 Moderation: the part that decides whether this works

### The two-tier model

1. **Community.** Anything uploaded that passes basic file validation. Visible after upload, marked with a yellow "Community — not reviewed" badge. Carries a strong disclaimer at download.
2. **Verified.** Reviewed by a human (you, initially). Marked with a green "OS Tuner Verified" badge. Carries the standard disclaimer.

### What "verified" actually means

A human looked at the tune file in OS Tuner, confirmed:
- It opens without errors.
- The advertised engine metadata matches what's in the tune (e.g., no claimed-NA tune with 25 psi boost target).
- Ignition table has no obvious unsafe values (60° advance at 5000 RPM on pump gas → reject).
- AFR target table is sane (no 18:1 at WOT).
- Cold-start enrichment exists and isn't zero.

A verified tune is **not** "this will run perfectly on your engine." It's **"this is a tune a reasonable tuner could have written."** That distinction must be communicated at every touchpoint.

### Why two tiers and not just one (verified-only)

A verified-only world means nothing publishes until you've reviewed it. That's a bottleneck where every uploader waits on you. The community tier lets the flow continue while still distinguishing reviewed from unreviewed work. Browsers see both; defaults sort by Verified first.

### The "I understand" gate

Every tune download, every time, even verified tunes:

> ⚠ **You are about to download a tune file written by another user.**
>
> Engine tuning is a high-risk activity. A wrong tune can damage your engine, your car, and you. OS Tuner verifies that tune files are well-formed; we do **not** verify they will work on *your specific* engine.
>
> Before flashing this tune to your ECU:
> • Read the tune's description and metadata.
> • Confirm engine code, displacement, injector size, and forced induction match yours.
> • Calibrate your sensors using the Calibration Center.
> • Review fuel and ignition tables in OS Tuner before burning.
>
> ☐ I understand this can damage my engine if applied incorrectly.

Checkbox does **not** persist. Always required. **Why:** familiarity breeds carelessness. Users who download 50 tunes need the same friction as the user downloading their first. This is one of the few times we deliberately don't optimize for convenience.

## F.6 What we deliberately don't build

- **No "flash to ECU directly from the hub"** — download → review → burn, always.
- **No silent auto-updates of installed tunes.**
- **No "recommended for your engine" auto-load.** Suggest in the UI, never auto-apply.
- **No reputation that bypasses safety.** A user with 1000 uploads still triggers the same disclaimers.
- **No comments thread per tune.** Ratings + structured Q&A only. Free-text comments scale into a moderation nightmare.
- **No private tunes.** Everything uploaded is public. If you want private, that's the future paid shop tier.

## F.7 Privacy + legal scaffolding

Day-one requirements:

- **Privacy policy** linked from signup, covering: what's stored, where, who can see it, account deletion rights.
- **Terms of Service** covering: ownership of uploaded tunes (uploader owns it, grants OS Tuner license to distribute), prohibited content (paid-product clones, malware, deliberately destructive tunes), takedown rights.
- **Account deletion flow** — required by GDPR even if you're not in the EU. Deleting an account marks uploads as "by deleted user" but doesn't pull the files (so people who downloaded them aren't suddenly orphaned).
- **Abuse reporting** — every tune page has a Report button. Reports go to a moderation inbox.

You don't need a lawyer for v1, but you should run drafts through one before going public with Phase F.

---

# Phase 1.0 — Polish + first stable

Items that aren't part of any single feature but should land before you call it 1.0:

- **Crash reporter** (opt-in, sends stack traces to a server you control). Same opt-in flag as update checks.
- **Telemetry** (opt-in, anonymous, off by default). What workspaces get opened, how long sessions last, which sensors people calibrate. Nothing identifying.
- **`mapping.yaml` cleanup** — the 41 unmapped names from your build log resolved.
- **Triple-load fix** for the ECU `.ini` parser.
- **Installer signing** — Windows users will see SmartScreen warnings until you sign the installer. ~$80/year for a basic code-signing cert. Not optional for a public release.
- **macOS + Linux builds** — Qt makes this almost free; ship them.
- **One real bug-bash week** before tagging 1.0. Test everything end-to-end with a clean install.

---

# Appendix A — File map of every change

| Phase | File / module                                          | Action              |
|-------|--------------------------------------------------------|---------------------|
| Bug   | `src/MainWindow.cpp`                                   | Add explicit disconnect in dtor |
| Bug   | `src/core/SerialManager.cpp`                           | `blockSignals(true)` in dtor |
| Bug   | `resources/workspaces/mapping.yaml`                    | Fix 41 unmapped names |
| A     | `src/widgets/onboarding/ProductTourOverlay.h/cpp`      | New                 |
| A     | `src/widgets/onboarding/TourStep.h`                    | New                 |
| A     | `src/utils/Settings.h/cpp`                             | Add firstRunCompleted |
| A     | `src/MainWindow.cpp`                                   | Hook tour into postInit, add Help menu |
| B     | `src/core/UserProfile.h/cpp`                           | New — SQLite layer  |
| B     | `src/widgets/onboarding/WelcomeOverlay.h/cpp`          | New                 |
| B     | `src/utils/Settings.h/cpp`                             | Add profile getters/setters |
| B     | `src/core/CalibrationChecker.h/cpp`                    | New                 |
| B     | `src/widgets/TunerGauge.h/cpp`                         | Add CalibrationState + paint glow |
| B     | `src/widgets/CalibrationCenterPanel.h/cpp`             | New                 |
| B     | `src/widgets/DashboardWidget.cpp`                      | Add Health button, hard banner |
| B     | `src/widgets/DashboardWidget.cpp` (CalibrationOverlay) | Add acknowledge checkbox |
| C     | `src/widgets/curriculum/CurriculumWindow.h/cpp`        | New                 |
| C     | `resources/curriculum/*.html`                          | New — 10 modules    |
| C     | `src/widgets/CoachHookManager.h/cpp`                   | New                 |
| C     | Various workspaces                                     | Add coach hook callsites |
| D     | `src/widgets/flasher/FlashFirmwareDialog.h/cpp`        | New                 |
| D     | `src/core/AvrdudeRunner.h/cpp`                         | New — wraps avrdude |
| D     | `resources/firmware/*.hex`                             | New                 |
| D     | `installer/avrdude/`                                   | Bundle avrdude.exe  |
| D     | `src/widgets/ProjectWizardOverlay.cpp`                 | Add "flash firmware" branch |
| D     | `src/widgets/ConnectionDialog.cpp`                     | Add flash button    |
| E     | `src/core/Stm32Runner.h/cpp`                           | New                 |
| E     | `src/core/FirmwareUpdateChecker.h/cpp`                 | New                 |
| E     | `resources/tunes/*.msq`                                | Add validated starter tunes |
| F     | `src/widgets/hub/TuneHubWindow.h/cpp`                  | New                 |
| F     | `src/core/HubClient.h/cpp`                             | New — HTTPS to API  |
| F     | `src/widgets/hub/AccountDialog.h/cpp`                  | New                 |
| F     | (server, separate repo)                                | API + moderation UI |

---

# Appendix B — Open questions remaining

1. **Starter tunes in Phase D — option 1, 2, or 3 from Section D.8?** My recommendation: option 3, defer tunes to Phase E.
2. **Does the in-app update check checkbox default to checked or unchecked on the Welcome screen?** I drafted it as unchecked. If you prefer checked-by-default, change before shipping.
3. **For Phase F: are you willing to commit ~3 hours/week to moderation indefinitely?** If no, Phase F gets cut from the 1.0 plan and becomes 1.x.
4. **Code-signing cert in time for 1.0?** Need this decided before the Phase D installer is built.
5. **Domain name for the server (Phase F)?** `ostuner.dev` is referenced in `main.cpp` (`QApplication::setOrganizationDomain`). Confirm we own/will-own it.

---

*End of guide. Treat this as the source of truth; update it as decisions change. Don't write code before re-reading the relevant phase section.*

Sources: project survey of `C:\OS Tuner`; user's build log dated 2026-05-18; design decisions in conversation with Muhammad Saeed Sajid.
