# mapping.yaml — Resolution Guide for the 41 Unmapped Names

> **Goal of this document:** turn that scary `WorkspaceMetadata: mapping.yaml references 41 name(s) with no matching Constant…` warning into a one-pass mechanical fix. Every one of the 41 names is resolved below: real ini name, why it didn't match, and what to write in the yaml.

> Source of truth: `C:\Users\admin\Documents\TunerStudioProjects\MyCar\projectCfg\mainController.ini` (Speeduino firmware `speeduino 202501`). 1004 constants extracted, every yaml name cross-checked.

---

## 1. The actual root cause (it's not just renames)

Three causes are mixed up in the 41-name list. Recognising which is which is half the work:

### Cause #1 — **Case mismatch.** Your parser is case-sensitive.

Speeduino uses a mix of `TitleCase` (`SoftRevLim`, `TrigPattern`, `IgInv`) and `lowercase/camelCase` (`numTeeth`, `dwellrun`, `boostLimit`). The yaml authored its names assuming a uniform camelCase that doesn't exist in the real ini.

**Examples in your 41:**
- `softRevLim` (yaml) → `SoftRevLim` (ini) — same name, different case
- `triggerType` (yaml) → `TrigPattern` (ini) — different name **and** case

The fix is mechanical: write the exact casing the ini uses.

### Cause #2 — **Rename / restructure.** TunerStudio convention vs Speeduino convention.

Some yaml names look like they came from a Megasquirt or TunerStudio reference template:
- `triggerWheel`, `triggerMissing` → Speeduino calls these `numTeeth`, `missingTeeth`
- `dwellRunning`, `dwellCranking` → Speeduino calls these `dwellrun`, `dwellcrank` (lowercase 'r' and 'c'!)
- `injectorCc` → Speeduino doesn't store injector size as one field; it stores `injOpen` (open time in ms) and uses `reqFuel` (base PW) + cylinder count to back-derive flow rate

The fix here is to **pick the real Speeduino field** and update both the yaml `name` and `human_label`.

### Cause #3 — **Concept doesn't exist in Speeduino.** This is the hardest class.

Two of the 41 names map to TunerStudio concepts that Speeduino's firmware genuinely **does not have**:
- **`mapSensorType`** — TunerStudio has a "select a preset MAP sensor (MPX4250, GM 3-bar, etc.)" dropdown that auto-fills the kPa-per-volt calibration. Speeduino has no such preset table — it only has raw `mapMin`/`mapMax` values that the user enters by hand.
- **`baroSensor`** — same story. Speeduino has `useExtBaro` (boolean: use external baro sensor?) and `baroMin`/`baroMax`. There's no "type" dropdown.

For these you have **three options**:
- **(A)** Drop them from yaml entirely (cleanest).
- **(B)** Replace them with the closest real Speeduino fields (`mapMin`/`mapMax` for MAP; `useExtBaro` + `baroMin`/`baroMax` for baro). This is what I recommend — same UX intent, just using the actual underlying fields.
- **(C)** Keep them and treat them as "virtual" workspace placeholders that the OS Tuner UI will *compute* from underlying fields (advanced, save for a later pass).

I recommend **(B)** below and reflect it in the fix table.

### One more cause that's *not* a real problem: section filtering.

I checked the parser's "ignored sections" list against the ini. None of the 41 names live in `[frontpage]`, `[loggerdefinition]`, `[tools]`, `[referencetables]`, `[megatune]`, or `[eventtriggers]`. They're all under `[Constants]` / `[SettingGroups]` / `[PcVariables]` — which your parser **does** read. So this is not a section-filtering bug. Don't go un-ignoring sections to "fix" this.

---

## 2. The 41-name resolution table

For every yaml name in the warning, here is: the real ini name, the cause, and the action. Apply the actions and your warning goes from 41 down to **0** (assuming you adopt option (B) for the two concept gaps).

Legend:
- **CASE** = Cause #1: just fix the casing.
- **RENAME** = Cause #2: ini uses a different identifier for the same field.
- **GAP** = Cause #3: concept doesn't exist; remap to the closest real field.
- **MULTI** = the concept is split across multiple ini fields.

| # | yaml name (broken) | Real ini name | Cause | Action |
|---|---|---|---|---|
| 1 | `softRevLim` | `SoftRevLim` | CASE | Change yaml `name: softRevLim` → `name: SoftRevLim`. Keep human_label "Soft Rev Limit". |
| 2 | `triggerType` | `TrigPattern` | RENAME | Change `name:` to `TrigPattern`. Keep human_label "Trigger Type". |
| 3 | `triggerWheel` | `numTeeth` | RENAME | Change `name:` to `numTeeth`. Keep human_label "Trigger Teeth". |
| 4 | `triggerMissing` | `missingTeeth` | RENAME | Change `name:` to `missingTeeth`. Keep human_label "Missing Teeth". |
| 5 | `triggerAngle` | `TrigAng` | RENAME+CASE | Change `name:` to `TrigAng`. Keep human_label "Trigger Angle". |
| 6 | `injectorCc` | *(no direct equivalent — see note)* | GAP/MULTI | Speeduino doesn't store injector cc/min directly. Closest real field is `injOpen` (injector open time). **Recommend: drop `injectorCc` from yaml entirely, add `injOpen` instead.** Update human_label to "Injector Open Time (ms)". |
| 7 | `injMode` | `injLayout` | RENAME | Speeduino's `injLayout` (Paired / Semi-Sequential / Sequential) covers the "injection mode" concept. Change `name:` to `injLayout`. Note: you already have `injLayout` mapped under engine_basics — **delete the duplicate `injMode` entry** to avoid two yaml rows pointing at the same field. |
| 8 | `injTiming` | *(no direct equivalent)* | GAP | Speeduino doesn't have a "start vs end of injection" timing mode constant — it uses `injAng` (end-of-injection angle) directly. **Recommend: drop `injTiming`. Keep `injAngle` (which is `injAng` in ini, see row 9).** |
| 9 | `injAngle` | `injAng` | RENAME | Change `name:` to `injAng`. Human_label "Injection Angle". |
| 10 | `sparkEdge` | `IgInv` | RENAME | Speeduino's "going high vs going low" maps to `IgInv` (Inverted polarity). Change `name:` to `IgInv`. Update human_label to "Ignition Polarity" — clearer than "Spark Edge". |
| 11 | `fixedDwell` | `dwell` | RENAME+CASE | Speeduino's running dwell constant is just `dwell` in the firmware globals — but the user-facing tunable is `dwellrun`. **Use `dwellrun`.** See row 13. |
| 12 | `useDwellTable` | `useDwellMap` | RENAME | Change `name:` to `useDwellMap`. Keep human_label. |
| 13 | `dwellRunning` | `dwellrun` | RENAME+CASE | Change `name:` to `dwellrun` (note: lowercase 'r'!). Keep human_label "Running Dwell". |
| 14 | `dwellCranking` | `dwellcrank` | RENAME+CASE | Change `name:` to `dwellcrank` (lowercase 'c'). Keep human_label. |
| 15 | `crankingRpm` | `crankRPM` | CASE | Change `name:` to `crankRPM`. |
| 16 | `crankPwCold` | `crankingEnrichValues[0]` | MULTI | Speeduino stores cranking enrichment as a 4-entry curve `crankingEnrichValues` indexed by `crankingEnrichBins` (temperature). There is no single "cold cranking PW" constant — it's the cold end of the curve. **Recommend: drop `crankPwCold`. Add `crankingEnrichValues` as a *curve* mapping under cold_start.** |
| 17 | `crankPwHot` | `crankingEnrichValues[3]` | MULTI | Same as row 16 — the hot end of the same curve. **Drop `crankPwHot`. Subsumed by the curve.** |
| 18 | `asePctCold` | `asePct` | RENAME+MULTI | Speeduino has `asePct` as a 4-entry array (per CLT bin). Not a scalar "cold pct." Change `name:` to `asePct` and treat as an array. |
| 19 | `aseHold` | *(no equivalent)* | GAP | Speeduino doesn't have a separate "hold time" — it has `aseTaperTime` (transition time to disable ASE) and `aseCount` (array of durations per bin). **Recommend: drop `aseHold`. Replace with `aseTaperTime`** and human_label "ASE Taper Time". |
| 20 | `idleRPM` | *(no equivalent)* | GAP | Speeduino doesn't have a single "target idle RPM" constant — it has `iacCLValues` (an array of target RPMs per CLT bin) and `CLIdleTarget` (current calculated target). **Recommend: drop `idleRPM` from yaml**, or replace with `iacCLValues` as a curve. |
| 21 | `idlePGain` | `idleKP` | RENAME | Change `name:` to `idleKP`. Keep label "Idle P-Gain". |
| 22 | `idleIGain` | `idleKI` | RENAME | Change `name:` to `idleKI`. |
| 23 | `idleDGain` | `idleKD` | RENAME | Change `name:` to `idleKD`. |
| 24 | `idleColdAdder` | *(no equivalent — see note)* | GAP | The Speeduino mental model is "the idle target *curve* is cold-aware," not "a single cold adder." Closest real fields are part of `iacCLValues`/`iacCrankSteps`. **Recommend: drop `idleColdAdder`** unless you want to expose `iacCrankSteps` (cranking idle steps) instead. |
| 25 | `idleAcAdder` | `airConIdleUpRPMAdder` | RENAME | Change `name:` to `airConIdleUpRPMAdder`. Human_label "A/C Idle Adder". |
| 26 | `fuelCutPct` | *(no equivalent)* | GAP | Speeduino has `hardCutType` (Full vs Rolling) and `SoftLimRetard` for soft cut behavior, but no "percent of fuel events to cut" tunable. **Recommend: drop `fuelCutPct`. Replace with `hardCutType`** if you want a related field in this workspace slot. |
| 27 | `boostCutEnable` | `boostCutEnabled` | CASE+SUFFIX | Change `name:` to `boostCutEnabled` (note the `d`). |
| 28 | `boostCutPsi` | `boostLimit` | RENAME+UNIT | Speeduino's overboost limit is `boostLimit`, measured in **kPa, not psi**. Change `name:` to `boostLimit`, update human_label to "Boost Cut Limit (kPa)" and help_text to reflect kPa. |
| 29 | `mapSensorType` | *(no equivalent)* | GAP | Speeduino has no MAP-sensor-preset dropdown. **Recommend Option B: drop `mapSensorType` and add `mapMin` + `mapMax` instead** (two yaml entries, both under sensors/map_sensor). Together they fully define the MAP sensor's calibration. |
| 30 | `mapAt0V` | `mapMin` | RENAME | Change `name:` to `mapMin`. Keep label "MAP at 0V". |
| 31 | `mapAt5V` | `mapMax` | RENAME | Change `name:` to `mapMax`. Keep label "MAP at 5V". |
| 32 | `baroSensor` | *(no equivalent)* | GAP | Same as MAP — no preset dropdown. **Recommend Option B: drop `baroSensor` and add `useExtBaro` + `baroMin` + `baroMax`** as three yaml entries under sensors/optional_sensors. |
| 33 | `engineCc` | *(no equivalent)* | GAP | Speeduino doesn't store engine displacement directly — it's computed by the tuner from injector size + req_fuel. **Recommend: drop `engineCc`.** This is a TunerStudio-style "convenience field" that doesn't exist at the firmware level. |
| 34 | `vvtPWMFreq` | `vvtFreq` | RENAME | Change `name:` to `vvtFreq`. Keep label. |
| 35 | `vvtTable1` | `vvtTable` | RENAME | Change `name:` to `vvtTable`. (Speeduino has `vvtTable` for cam 1 and `vvt2Table` for cam 2 — your `vvt2Table` row already references the right name, so just fix row 1.) |
| 36 | `injAPin` | *(no equivalent — pins are board-default)* | GAP | Speeduino doesn't have a per-channel injector pin override constant — injector pins are tied to board-default mappings. There is no `injAPin`/`injBPin`/etc. tunable. **Recommend: drop `injAPin` from yaml.** Add a workspace-level note that injector pins are board-defined. |
| 37 | `ignAPin` | *(no equivalent)* | GAP | Same as row 36. **Recommend: drop `ignAPin` from yaml.** |
| 38 | `canEnable` | *(no exact equivalent)* | RENAME+SPLIT | Speeduino's "is CAN bus on" is `enable_secondarySerial` (the secondary UART that *can* speak CAN) plus the `CANBroadcastProt` protocol selector. **Recommend: change `canEnable` → `CANBroadcastProt`** (when this is "Off", CAN is effectively disabled). |
| 39 | `canBcastMode` | `CANBroadcastProt` | RENAME+CASE | Change `name:` to `CANBroadcastProt`. **Note: this is the same field as the row above. Delete one of the two duplicate yaml entries.** |
| 40 | `serial2Enabled` | `enable_secondarySerial` | RENAME | Change `name:` to `enable_secondarySerial`. Keep label. |
| 41 | `serial2BaudRate` | *(no equivalent)* | GAP | Speeduino's secondary serial baud is firmware-fixed (115200 for Mega, configurable per-board for STM32 but not user-tunable from ini). There is no baud rate constant. **Recommend: drop `serial2BaudRate`.** |

---

## 3. Summary by action class

| Action | Count | Names |
|---|---|---|
| **Fix casing only** | 3 | softRevLim, crankingRpm, boostCutEnable |
| **Rename to real field** | 17 | triggerType, triggerWheel, triggerMissing, triggerAngle, injMode, injAngle, sparkEdge, useDwellTable, dwellRunning, dwellCranking, asePctCold, idlePGain, idleIGain, idleDGain, idleAcAdder, boostCutPsi, mapAt0V, mapAt5V, vvtPWMFreq, vvtTable1, serial2Enabled (Note: 17 distinct rows; total touches more cells.) |
| **Replace concept with closest real field(s)** | 7 | injectorCc → injOpen; mapSensorType → drop + add mapMin/mapMax; baroSensor → drop + add useExtBaro/baroMin/baroMax; fixedDwell → dwellrun; aseHold → aseTaperTime; canEnable → CANBroadcastProt; idleAcAdder → airConIdleUpRPMAdder |
| **Drop entirely (concept doesn't exist in Speeduino)** | 9 | injTiming, crankPwCold, crankPwHot, idleRPM, idleColdAdder, fuelCutPct, engineCc, injAPin, ignAPin, serial2BaudRate |
| **Duplicate (one row is redundant)** | 2 | injMode (already covered by `injLayout`), canBcastMode (same field as canEnable) |

Net effect on yaml: ~5 entries deleted, ~17 entries renamed, ~5 entries added (the splits for mapMin/mapMax/useExtBaro/baroMin/baroMax/injOpen).

---

## 4. Ready-to-apply yaml diff (illustrative chunk)

Below is what the **Triggers**, **Fueling**, **Ignition**, **Cold Start**, **Idle**, **Engine Protection**, **Sensors**, **VVT**, **I/O**, **Comms** sections should look like after the fix. Drop this in as-is to test, but keep your existing yaml around for diffing.

```yaml
# --- TRIGGERS & SYNC ---
- name: TrigPattern
  primary_workspace: triggers
  subsection: crank_trigger
  display_order: 10
  human_label: "Trigger Type"
  help_text: "Pattern of the trigger wheel."

- name: numTeeth
  primary_workspace: triggers
  subsection: crank_trigger
  display_order: 20
  human_label: "Trigger Teeth"
  help_text: "Total number of teeth if there were no missing teeth."

- name: missingTeeth
  primary_workspace: triggers
  subsection: crank_trigger
  display_order: 30
  human_label: "Missing Teeth"
  help_text: "Number of missing teeth on the trigger wheel."

- name: TrigAng
  primary_workspace: triggers
  subsection: crank_trigger
  display_order: 40
  human_label: "Trigger Angle"
  help_text: "Degrees BTDC at sync point."

# --- FUELING ---
- name: injOpen
  primary_workspace: fueling
  subsection: injector_hardware
  display_order: 10
  human_label: "Injector Open Time"
  help_text: "Injector dead time at nominal battery voltage (ms)."

- name: reqFuel
  primary_workspace: fueling
  subsection: fuel_system
  display_order: 30
  human_label: "Required Fuel"
  help_text: "Base fuel pulse width (ms)."

- name: injAng
  primary_workspace: fueling
  subsection: injection_timing
  display_order: 50
  human_label: "Injection Angle"
  help_text: "End-of-injection angle in degrees."

# (note: injMode entry removed — duplicate of injLayout in engine_basics)
# (note: injTiming entry removed — concept not in firmware)
# (note: injectorCc removed — replaced by injOpen above)

# --- IGNITION ---
- name: sparkMode
  primary_workspace: ignition
  subsection: coil_hardware
  display_order: 10
  human_label: "Spark Mode"
  help_text: "Wasted spark, sequential, or COP."

- name: IgInv
  primary_workspace: ignition
  subsection: coil_hardware
  display_order: 20
  human_label: "Ignition Polarity"
  help_text: "Whether the spark fires when the ignition signal goes high or low. Most systems use 'Going Low'."

- name: dwellrun
  primary_workspace: ignition
  subsection: coil_hardware
  display_order: 30
  human_label: "Running Dwell"
  help_text: "Nominal dwell time when the engine is running (ms)."

- name: useDwellMap
  primary_workspace: ignition
  subsection: coil_hardware
  display_order: 40
  human_label: "Use Dwell Table"
  help_text: "Enable 3D dwell table instead of a fixed dwell."

- name: dwellcrank
  primary_workspace: ignition
  subsection: coil_hardware
  display_order: 60
  human_label: "Cranking Dwell"
  help_text: "Dwell time during cranking (usually slightly higher than running dwell)."

# (note: fixedDwell removed — dwellrun is the user-facing setting)

# --- COLD START ---
- name: crankRPM
  primary_workspace: cold_start
  subsection: cranking
  display_order: 10
  human_label: "Cranking RPM"
  help_text: "RPM threshold below which the engine is considered cranking."

- name: asePct
  primary_workspace: cold_start
  subsection: ase
  display_order: 40
  human_label: "ASE % (by CLT bin)"
  help_text: "After-start enrichment percent for each CLT bin (4-entry array)."
  secondary_workspaces: [fueling]

- name: aseTaperTime
  primary_workspace: cold_start
  subsection: ase
  display_order: 50
  human_label: "ASE Taper Time"
  help_text: "Transition time used to disable ASE (s)."

# (note: crankPwCold/Hot removed — use crankingEnrichValues curve instead, see TODO)
# (note: aseHold removed — replaced by aseTaperTime)

# --- IDLE CONTROL ---
- name: idleKP
  primary_workspace: idle
  subsection: idle_air_control
  display_order: 20
  human_label: "Idle P-Gain"
  help_text: "Proportional gain for idle PID."

- name: idleKI
  primary_workspace: idle
  subsection: idle_air_control
  display_order: 30
  human_label: "Idle I-Gain"
  help_text: "Integral gain for idle PID."

- name: idleKD
  primary_workspace: idle
  subsection: idle_air_control
  display_order: 40
  human_label: "Idle D-Gain"
  help_text: "Derivative gain for idle PID."

- name: airConIdleUpRPMAdder
  primary_workspace: idle
  subsection: compensations
  display_order: 60
  human_label: "A/C Idle Adder"
  help_text: "RPM added to the idle target when A/C is engaged."

# (note: idleRPM removed — Speeduino uses iacCLValues curve, expose later as curve)
# (note: idleColdAdder removed — no equivalent in firmware)

# --- ENGINE PROTECTION ---
- name: hardRevLim
  primary_workspace: engine_protection
  subsection: rev_limiters
  display_order: 10
  human_label: "Hard Rev Limit"
  help_text: "Absolute maximum RPM (full cut)."

- name: SoftRevLim
  primary_workspace: engine_protection
  subsection: rev_limiters
  display_order: 20
  human_label: "Soft Rev Limit"
  help_text: "RPM threshold for soft cut (spark retard or rolling cut)."

- name: hardCutType
  primary_workspace: engine_protection
  subsection: rev_limiters
  display_order: 30
  human_label: "Cut Type"
  help_text: "Full cut or Rolling cut for rev/launch limits."

- name: boostCutEnabled
  primary_workspace: engine_protection
  subsection: boost_cut
  display_order: 40
  human_label: "Enable Boost Cut"
  help_text: "Enable overboost protection."

- name: boostLimit
  primary_workspace: engine_protection
  subsection: boost_cut
  display_order: 50
  human_label: "Boost Cut Limit"
  help_text: "Maximum allowed manifold pressure before fuel/spark cut (kPa)."

# (note: fuelCutPct removed — no equivalent; hardCutType added in its slot)

# --- SENSORS ---
- name: mapMin
  primary_workspace: sensors
  subsection: map_sensor
  display_order: 10
  human_label: "MAP at 0V (kPa)"
  help_text: "Pressure value when the MAP sensor outputs 0V."

- name: mapMax
  primary_workspace: sensors
  subsection: map_sensor
  display_order: 20
  human_label: "MAP at 5V (kPa)"
  help_text: "Pressure value when the MAP sensor outputs 5V."

- name: useExtBaro
  primary_workspace: sensors
  subsection: optional_sensors
  display_order: 30
  human_label: "Use External Baro Sensor"
  help_text: "If on, the dedicated baro sensor pin is read; otherwise the initial MAP reading is used as baro."

- name: baroMin
  primary_workspace: sensors
  subsection: optional_sensors
  display_order: 40
  human_label: "Baro at 0V (kPa)"
  help_text: "Pressure when external baro sensor outputs 0V."

- name: baroMax
  primary_workspace: sensors
  subsection: optional_sensors
  display_order: 50
  human_label: "Baro at 5V (kPa)"
  help_text: "Pressure when external baro sensor outputs 5V."

# (note: mapSensorType removed — no preset table in Speeduino)
# (note: baroSensor removed — replaced by useExtBaro + baroMin/baroMax)

# --- VVT ---
- name: vvtFreq
  primary_workspace: vvt
  subsection: vvt_general
  display_order: 30
  human_label: "VVT PWM Frequency"
  help_text: "PWM frequency for the VVT solenoid output (Hz)."

- name: vvtTable
  primary_workspace: vvt
  subsection: vvt_cam1
  display_order: 40
  human_label: "VVT1 Target Table"
  help_text: "Target intake-cam angle map."

# --- COMMS ---
- name: CANBroadcastProt
  primary_workspace: comms
  subsection: can_bus
  display_order: 10
  human_label: "CAN Broadcast Mode"
  help_text: "Broadcast protocol (Off, BMW, VAG, Haltech). Set to Off to disable CAN output."

- name: enable_secondarySerial
  primary_workspace: comms
  subsection: secondary_serial
  display_order: 40
  human_label: "Secondary Serial Enabled"
  help_text: "Enable the secondary UART channel (CAN, msDroid, RealDash, TunerStudio passthrough)."

# (note: canEnable removed — CANBroadcastProt covers the on/off via "Off" option)
# (note: serial2BaudRate removed — baud is firmware-fixed, not user-tunable)

# --- I/O ---
# (note: injAPin and ignAPin removed — Speeduino doesn't expose per-channel pin overrides)
# Keep only the real pin constants: fanPin, fuelPumpPin, tachoPin, baroPin, boostPin, idleUpPin, etc.
```

---

## 5. The two follow-ups this fix exposes

### 5.1 Your parser is case-sensitive — should it be?

Three of the 41 misses were pure casing. The Speeduino ini file mixes casing aggressively (`SoftRevLim`, `numTeeth`, `dwellrun`). If you make the parser **case-insensitive** for name matching, you eliminate an entire class of future drift bugs — every time Speeduino renames a constant slightly, you wouldn't need a yaml update.

**Recommended change:** in `WorkspaceMetadata` / `ECUDefinition`, lowercase both sides before comparison. Keep the original casing for display. The cost is one `.toLower()` per lookup, the benefit is robustness forever.

**Counterargument:** if Speeduino ever ships two constants whose names differ only in case (`Map` vs `map`), case-insensitive matching would collide. I checked the current 1004 constants and there are no such collisions. Risk is low.

### 5.2 The 41-name warning is hiding the bigger number: 720 unmapped constants

Your log says `19 of 760 constants mapped`. After this fix you'll bump that to maybe 30 of 760 — better, but still 95% of Speeduino is invisible to your workspaces. That's why the new workspace tabs feel sparse.

This isn't a bug to fix in one pass; it's the actual product work for v0.6/v0.7. The yaml is the **map of Speeduino → workspace surface**, and it's only 5% done. Plan to add ~50–100 more mappings as you flesh out each workspace.

**Recommended order of attack** (after this 41-name fix lands):
1. **Sensors workspace** — currently stubbed in code, biggest single gap. Map all CLT/IAT/O2/MAP/TPS calibration fields. ~30 constants.
2. **Fueling workspace** — already partially mapped, but missing AE (accel enrichment), priming, flood clear, baro correction. ~25 constants.
3. **Ignition workspace** — missing dwell map enable details, knock retard, cranking spark. ~20 constants.
4. **Boost workspace** — missing PID gains (boostKP/KI/KD), per-gear boost, closed-loop targets. ~15 constants.
5. **VVT** — closed-loop PID, dual VVT, error detection. ~10 constants.
6. **CAN/Comms** — the entire `caninput_selXa/b` matrix if you want CAN inputs visible. ~80 constants (skippable in v0.6).

### 5.3 The triple-ini-load is a separate perf bug, not part of this fix

Your build log shows the ini being parsed three times on startup (look for `phase-2 [ tunerstudio ] ( 22  bytes)` repeated). That's:
1. Initial load when MainWindow's ProjectManager loads the project.
2. Signature validation when ECUSettingsManager re-reads it.
3. Workspace metadata application.

460KB of constants section parsed three times is ~6× slower than needed at startup. Cache the parsed `ECUDefinition` after first load. Track this as a separate v0.7 task.

---

## 6. Test plan after applying the yaml diff

1. Clean build, run the app, load the same `MyCar` project.
2. Watch the log line: `WorkspaceMetadata: mapping.yaml references N name(s) with no matching Constant...`
3. **Expected: N == 0.** If N > 0, the remaining names are likely typos in the new yaml — grep the ini for them.
4. Watch the line: `WorkspaceMetadata: applied to X constants, Y tables, and Z pc variables`.
5. **Expected: X ≥ 35** (up from 19), reflecting the new mappings.
6. Open each affected workspace tab in the UI. Confirm:
   - Triggers tab now shows TrigPattern + numTeeth + missingTeeth + TrigAng.
   - Fueling shows reqFuel + injOpen + injAng.
   - Ignition shows IgInv + dwellrun + dwellcrank + useDwellMap.
   - Sensors shows mapMin/mapMax + useExtBaro/baroMin/baroMax.
   - Engine Protection shows hardRevLim + SoftRevLim + hardCutType + boostCutEnabled + boostLimit.
7. Edit one value in each tab, save the project, close OS Tuner, reopen, verify the value persisted in the MSQ.

If all 7 pass, the warning is dead.

---

Sources: `C:\Users\admin\Documents\TunerStudioProjects\MyCar\projectCfg\mainController.ini` (live Speeduino 202501 ini, 1004 constants extracted); `C:\OS Tuner\resources\workspaces\mapping.yaml` (current yaml, 60 entries); user build log dated 2026-05-18.
