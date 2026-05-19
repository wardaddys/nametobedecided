# OS Tuner — Onboarding, User Profile & "Unset = Glow" System
**A pre-1.0 release expansion plan**

Author: drafted for Muhammad Saeed Sajid
Project: OS Tuner v0.5.0-alpha → v0.6.0
Status: design only — no code touched
Anchors: `main.cpp`, `TunerProSplashScreen`, `DashboardWidget`, `CalibrationOverlay`, `ECUSettingsManager`, `ProjectWizardOverlay`, `Settings.h`

---

## 0. Who this release is for, and who it isn't

This release targets the **everyday tuner** — the hobbyist running a Speeduino on a project car, the student rebuilding their first turbo Civic, the weekend dyno-day enthusiast. It is **not** targeting performance shops yet. The shop-tier paid plugins you described are a **Year-2 product**, kicked in only after you have active users and a real community — see Section 7. Everything in v0.5.5 → v0.7.0 below is designed to make solo hobbyists succeed. The plugin foundation built along the way is *latent* — present, documented, but not exercised until shops show up.

Keep that framing in mind reading the rest. If a feature feels too consumer-friendly, that's intentional. If a feature feels too thin for a shop, that's also intentional — shops get their own surface area later.

---

## 1. Reading this honestly first

The ruthless take you asked for:

1. **You are pre-1.0 and you don't have a user yet.** A "user database" right now is a liability, not a feature. Email + name + car gives you nothing to act on, no marketing channel (you'd need to set up SES/Mailgun + an unsubscribe flow or you're spamming), and adds data-handling weight to a desktop app that currently has none. **Build the local profile. Skip everything cloud.** That way you still get the data structure, the consent UX, and the foundation — without the legal surface area. When shops + paid plugins arrive in Year 2, that's the moment to ship a real account system.

2. **An onboarding tutorial that explains "what is engine tuning" is two products glued together.** One is a *learn-to-tune curriculum* (educational, evergreen, belongs in Help). The other is a *product tour* ("here is where the VE table lives, here is how to read a gauge"). Conflating them creates a 40-minute first-run experience that no one finishes. **Split them.** Product tour = mandatory-feeling, 90 seconds, skippable. Curriculum = optional deep-dive, lives in Help, gets opened on demand.

3. **The "glow when not calibrated" idea is the single best feature in this brief.** It's the one thing that materially makes tuning easier and makes new users feel safe. Lean into it harder than the tutorial — it teaches *by existing* every time the user opens the app.

4. **Your splash → wizard flow already exists** (`TunerProSplashScreen` → `ProjectWizardOverlay`). Don't bolt a tutorial on top of it as a fourth modal. Insert one *first-run-only* step inside the existing chain.

OK, now expansion.

---

## 2. The First-Run Flow — what actually happens

Right now `main.cpp` does:

> splash → user clicks `OpenLast` / `Browse` / `Exit` → MainWindow → `postInit()` → ProjectWizardOverlay if no project.

The new flow inserts **two** new screens, both gated by `Settings::isFirstRun()`:

```
Splash (existing)
   │
   ▼
[NEW] Welcome / Profile  ─── skip ──┐
   │                                │
   ▼                                │
[NEW] Product Tour       ─── skip ──┤
   │                                │
   ▼                                │
ProjectWizardOverlay  ◄─────────────┘   (existing — unchanged)
   │
   ▼
MainWindow / Dashboard
```

Both new screens set a `firstRunCompleted = true` flag in `Settings` so they never appear again unless the user manually re-opens them from `Help → Replay Onboarding` or `Settings → Profile`.

**Critical design rule:** the Welcome screen has a giant `Skip` button in the top-right from the moment it opens. No "skip" button hidden behind three clicks. New users who *already know how to tune* will hate you forever if you trap them.

---

## 3. Welcome / Profile screen — the "database" part

### 3.1 What you actually collect, and why

| Field            | Required? | Why it exists                                                                  |
|------------------|-----------|--------------------------------------------------------------------------------|
| Display name     | optional  | Greets the user, shows in title bar, used in MSQ author field                  |
| Email            | optional  | Only if user opts in to update emails — **separate checkbox**                   |
| Experience level | required  | "First time tuning" / "I've tuned before" / "Pro" — gates tour depth, defaults |
| Primary car      | optional  | Make/model/year/engine code/forced induction Y/N — pre-fills project wizard    |
| Tuning goal      | optional  | Daily / Track / Drag / Drift / Dyno — picks a sensible default workspace layout |
| Units            | required  | Metric / Imperial — affects every gauge label and table editor                 |
| Send anon usage  | required  | default OFF, opt-in only                                                       |

**Email is opt-in to release notes only, not a login.** No password, no account, no server roundtrip. You are building a desktop app, not a SaaS. Auth comes later when shop plugins ship.

### 3.2 Where the data lives

Two stores:

**A. `Settings.h` (extend the existing class) — for single-user app-global state.**
Add:

```cpp
// === User Profile ===
static bool getFirstRunCompleted();
static void setFirstRunCompleted(bool v);

static QString getProfileName();
static void setProfileName(const QString &name);

static QString getProfileEmail();
static void setProfileEmail(const QString &email);

static int getExperienceLevel();       // 0=beginner, 1=intermediate, 2=pro
static void setExperienceLevel(int l);

static QString getUnits();             // "metric" | "imperial"
static void setUnits(const QString &u);

static bool getNewsletterOptIn();
static void setNewsletterOptIn(bool v);

static bool getAnonUsageOptIn();
static void setAnonUsageOptIn(bool v);
```

**B. A new SQLite file `profile.db` next to `OSTuner.ini` — for the "garage".**

Why SQLite and not just more QSettings keys: a tuner with two project cars (their daily and their track build) wants both saved, both pre-fillable in the project wizard. That's a table, not a flat key/value. Even for hobbyists, multi-car is common — every tuner I've ever known has at least one car they've finished and one car they're working on.

Schema (keep it minimal — you can extend later, you can never un-ship a column):

```sql
CREATE TABLE garage (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    nickname        TEXT NOT NULL,
    make            TEXT,
    model           TEXT,
    year            INTEGER,
    engine_code     TEXT,
    displacement_cc INTEGER,
    cylinders       INTEGER,
    forced_induction TEXT,         -- 'na' | 'turbo' | 'supercharger'
    injector_cc     INTEGER,
    ecu_firmware    TEXT,
    created_at      TEXT NOT NULL,
    last_used_at    TEXT
);

CREATE TABLE profile (
    key             TEXT PRIMARY KEY,
    value           TEXT
);

CREATE TABLE tutorial_progress (
    module_id       TEXT PRIMARY KEY,
    completed_at    TEXT,
    skipped         INTEGER DEFAULT 0
);
```

Three tables, no migrations needed for v0.6, room to grow.

### 3.3 The skip path

If user clicks Skip on Welcome:
- `firstRunCompleted = true`
- `experienceLevel = 1` (assume intermediate so the tour doesn't condescend)
- `units = metric` (default)
- everything else blank
- garage stays empty until they create a project

**The whole skip flow must take one click and one confirmation.** No "are you sure? you'll miss out on…" guilt-trip dialog. Show a small toast: "Profile skipped — set it anytime in Settings → Profile."

### 3.4 What this screen looks like

Three steps, each a single screen with `Back / Skip / Continue`:

1. **Hi, what should we call you?** — name field, optional email with newsletter checkbox below it (default unchecked), big "Skip everything" link.
2. **What are you tuning?** — make/model/year/engine dropdowns *or* "I'll add this later." If they pick a known engine, you can suggest sensible injector size / displacement / cylinder count defaults.
3. **Your experience** — three big radio cards: *First time* / *I've tuned cars before* / *Pro tuner*. Each card shows a one-line description so people don't lie about being a pro to look cool.

Total time if they don't skip: **under 60 seconds.**

---

## 4. The Product Tour — what you call "comprehensive yet concise intro"

You said: *"an indepth comprehensice yet cnsice intro to what engine tuning is."* Those three adjectives are at war with each other. Pick a side. My recommendation:

> **The tour is a product tour. The curriculum is a separate, opt-in deep dive.**

### 4.1 The product tour (every new user gets this)

A 6-step coachmark / spotlight overlay on the actual MainWindow. Not a video. Not slides. Real UI, real gauges, with a pulsing ring and a 2-sentence caption per step.

| # | Anchor (real widget)              | Caption (~2 lines)                                                                   |
|---|-----------------------------------|--------------------------------------------------------------------------------------|
| 1 | Dashboard gauges (TPS/MAP/RPM…)   | "These are your live engine sensors. The numbers come from your ECU 100× a second."  |
| 2 | A single gauge that's *not calibrated* | "If a gauge glows orange, that sensor isn't calibrated yet. Click it to set it up."  |
| 3 | Workspace tabs (Fueling, Ignition, Sensors) | "Tuning happens here. Fueling is how much fuel. Ignition is when it sparks."         |
| 4 | A 3D fuel map (`TunerMap3DWidget`) | "This is a VE table. Y axis = engine load, X = RPM. Each cell tells the ECU how much air the engine is breathing." |
| 5 | Connection / Burn buttons         | "Edits live in memory until you Burn — that writes them to the ECU. Always Burn before you unplug." |
| 6 | Demo Mode toggle (`DemoModeManager` exists) | "No ECU connected? Toggle Demo Mode and explore with fake data. Nothing you do here can hurt anything." |

End screen: "Want the full guide on how tuning actually works? Help → Learn to Tune."

That's the entire tour. Six steps, ~90 seconds, **skippable from step 1**. Beginners get oriented; pros are out in ten seconds.

### 4.2 The curriculum — Help → Learn to Tune

This is the deep-dive you described. It does **not** run on first launch. It lives in a Help menu entry and an icon on the dashboard. It's a left-nav modal with the following modules:

1. **What is engine tuning?** — the why. Air-fuel ratios, why stoich isn't always the target, what "rich" / "lean" mean, what knock is and why it kills engines.
2. **Reading the sensors** — TPS, MAP, MAF, IAT, CLT, O2/wideband, RPM/CKP. For each: what it measures, what a good value looks like idling vs WOT, what a *broken* reading looks like.
3. **Calibrating your sensors** — opens directly into your existing `CalibrationOverlay` flow with a coach voice ("now press the throttle to the floor and click Get Current"). This is the one module that *acts* on the software, not just explains.
4. **Fueling — VE tables, Req Fuel, injector size** — how the ECU decides how long to open the injector. Walks through your existing `FuelingWorkspace`.
5. **Ignition — when the spark fires** — timing, advance, knock, dwell. Walks through `IgnitionWorkspace`.
6. **Cold start, warmup, accel enrichment** — the "drivability" pillar. Why a car that idles fine still hesitates on a cold morning.
7. **Closed-loop vs open-loop** — when the ECU trusts the wideband and trims itself, when it doesn't.
8. **Datalogging and reading a log** — what to record, what a healthy log looks like, what to look for after a drive.
9. **Safety: when to stop tuning and walk away** — knock retard, lean spikes, coolant temp, oil temp. The "if you see this, kill the throttle" page.
10. **Your first tune, step by step** — a 1-hour checklist: connect → calibrate → set req fuel → idle target → cruise VE → power VE → ignition pulls → datalog → review.

Each module:
- Has its own row in the `tutorial_progress` table.
- Shows a green check next to completed modules.
- Has a "Try it now" button that, where possible, jumps you to the relevant workspace in OS Tuner itself.
- Takes 3–8 minutes to read.

**Total curriculum**: maybe 45 minutes if you read everything. **Total first-run experience**: still under 2 minutes, because the curriculum isn't part of first-run.

### 4.3 In-app coach hooks (the dependency on tutorial progress)

This is where the tutorial earns its keep beyond first run:

- User opens the VE table for the first time and hasn't completed "Fueling" module → small non-blocking banner appears at the top of the workspace: **"New to VE tables? 4-minute primer →"**. Dismissable. Stops appearing forever after dismissal or completion.
- User attempts to write to ECU (Burn) and hasn't completed "Safety" module → soft warning: **"Heads up — Burning sends your changes to a live ECU. 90-second safety primer recommended."** with `Read now / Burn anyway / Don't show again`.
- User datalogs for the first time → toast: **"First log saved. Want to learn how to read it?"**

This turns the tutorial from a wall to a thread — woven through the product instead of dumped at the door.

---

## 5. The "Unset = Glow" sensor system — full spec

This is the feature with the biggest UX win per line of code. Here is the full design.

### 5.1 What "unset" actually means, per sensor

Calibration isn't binary across all sensors. Define it explicitly:

| Sensor   | "Unset" condition                                                                                  | Source of truth                                       |
|----------|----------------------------------------------------------------------------------------------------|-------------------------------------------------------|
| TPS      | `tpsClosed` and `tpsFull` both at firmware defaults *and* `firstRunCompleted == true`              | `ECUSettingsManager` constants `tpsMin` / `tpsMax`    |
| MAP      | Sensor type = "Custom" with 0V/5V kPa pair both default                                            | `mapMin` / `mapMax` page 1                            |
| CLT      | Thermistor table is the firmware default Bosch curve *and* user hasn't acknowledged                | `cltCalibrationTable` (page 2)                        |
| IAT      | Same as CLT, separate table                                                                        | `iatCalibrationTable`                                  |
| O2/AFR   | `o2Type == disabled` OR voltage curve is default and wideband checkbox unchecked                   | `o2Type`, `wbType` constants                          |
| Battery  | Never unset — always reads from ECU ADC                                                            | n/a — gauge never glows                               |
| RPM      | Trigger/decoder not configured (toothed wheel teeth = 0 or pattern = "Unset")                       | `triggerPattern`, `numTeeth`                          |
| Injectors| `reqFuel` = 0 OR injector cc/min = 0                                                               | `reqFuel`, `injectorCC`                                |

**Critical:** treat "user has explicitly confirmed default is correct" as a separate state from "user hasn't looked at it yet." Otherwise pros who *want* the default will be nagged forever. Add an `acknowledged` flag per sensor:

```cpp
// in ECUSettingsManager
bool isCalibrationAcknowledged(const QString &sensorId) const;
void acknowledgeCalibration(const QString &sensorId);
```

Stored in `profile.db` `profile` table as `sensor_ack.<id> = 1`. The calibration overlay gets an "I've reviewed this, don't warn me again" checkbox at the bottom.

### 5.2 The glow itself — visual spec

Build a small `CalibrationState` enum and expose it on `TunerGauge`:

```cpp
enum class CalibrationState {
    Calibrated,      // green dot, no glow
    Acknowledged,    // no glow (user reviewed default and accepted)
    Unset,           // amber glow, pulsing
    Invalid          // red glow, steady (e.g., TPS min > max — broken)
};

void TunerGauge::setCalibrationState(CalibrationState s);
```

**Amber glow** (pulsing): a 4px soft outer halo in `#FFB020`, opacity oscillating 0.4 → 1.0 over 2 seconds with `QPropertyAnimation`. Same animation primitive you're already using on the splash loading bar. Pulses **slowly** — fast pulses read as "error" and panic users.

**Red glow** (steady): same halo, color `#FF4040`, no animation. Reserved for *invalid* (min > max, etc.), not just *unset*.

**Tooltip** on hover: "TPS isn't calibrated yet. Click to calibrate (30 seconds)."

**Click behavior**: opens the existing `CalibrationOverlay` directly to that sensor's panel. You already built `showCalibration(const QString &title)` — wire it up.

### 5.3 The "Calibration Center" — a single pane of glass

A new lightweight dashboard panel listing every sensor + its state + a one-click jump to its calibration UI:

```
┌─ Calibration Center ─────────────────────────────┐
│  ●  TPS         Calibrated     2026-05-12        │
│  ⚠  MAP         Not set        Calibrate →       │
│  ●  CLT         Acknowledged   Default Bosch     │
│  ⚠  IAT         Not set        Calibrate →       │
│  ⚠  O2          Not set        Calibrate →       │
│  ●  RPM/CKP     Calibrated     36-1, 2025-12-04  │
│                                                  │
│  3 of 6 sensors calibrated.                      │
└──────────────────────────────────────────────────┘
```

Lives behind a small `Health` button on the dashboard header. This is the single answer to "is my setup ready?" — and it's what you should show a new user at the end of the product tour.

### 5.4 Edge case: live engine running with unset sensors

If `RealTimeData.rpm > 200` AND any critical sensor (TPS, MAP, CLT) is `Unset` → bright persistent banner across the top of the dashboard:

> **⚠ Engine is running with uncalibrated sensors. Readings and fueling may be wrong. Stop the engine and calibrate before tuning.**

Banner dismissable for the current session only. Re-appears on next launch until calibrated. This is the "this software won't let you do the dumb thing" moment that earns trust.

---

## 6. Implementation order — what to build, in what sequence

You said you want to ship after fixing some bugs. Here's the build order so you can ship the feature in **three small releases** instead of one big one. Each release is independently valuable.

### Phase A — v0.5.5 (1–2 days, fully local, no schema)

- Add `Settings::firstRunCompleted` flag.
- Add the 6-step product tour as a `QWidget` overlay on MainWindow.
- Add Help menu entry `Replay onboarding`.
- Ship.

You now have onboarding. No DB, no profile, no risk.

### Phase B — v0.6.0 (3–5 days)

- Add `profile.db` SQLite store + the three tables.
- Add Welcome screen (name / email / experience / units / car).
- Add `CalibrationState` to `TunerGauge` + glow rendering.
- Wire up the unset detection for the top 3 sensors only: **TPS, MAP, CLT**.
- Add Calibration Center panel.
- Ship.

You now have the database, the profile, and the killer "glow" feature for the sensors that matter most.

### Phase C — v0.7.0 (1 week)

- Curriculum modules (the 10-module Help → Learn to Tune).
- In-product coach hooks (banners on first VE table open, first Burn, first datalog).
- Extend glow to IAT, O2, RPM trigger, injectors.
- "Running with uncalibrated sensors" safety banner.
- Garage table → multi-car support, pre-fill project wizard from garage.
- Ship.

### Out of scope (deliberately, for this whole release cycle)

- Cloud sync of profiles.
- Login / accounts / OAuth.
- Newsletter sending (the checkbox is collected; SMTP is not built).
- The shop-tier paid plugin marketplace (see Section 7 — that's Year 2+).

---

## 7. Year 2+ — what the foundation eventually unlocks

This is the section I got wrong before. Re-stating clearly:

The paid plugin marketplace is a **Year-2 or later** play. It only makes sense **after** OS Tuner has an active hobbyist community, a stable v1.0, real download numbers, and at least one shop reaching out asking "can I pay you to make X." Don't pre-build it. Don't pre-market it. Don't ship a "Pro features coming soon" page in v0.6 — it makes the free product feel crippled before it has fans.

**Who the paid plugins are for: performance shops, not everyday tuners.**

Examples of what shop-tier plugins might be (illustrative — don't commit to any of these yet, they're here to show why the foundation matters):

| Plugin idea                    | What it does for a shop                                                                  | What v0.6 foundation it leans on                                       |
|--------------------------------|------------------------------------------------------------------------------------------|------------------------------------------------------------------------|
| **Customer & Job Manager**     | Track customer cars in/out of the shop, who tuned what, what was changed, invoicing tie-in | `garage` table extended to attach customer info; multi-car already there |
| **Dyno Bridge**                | Pull live data from Dynojet / Mainline / Dynocom; overlay runs; auto-annotate logs       | Real-time data pipeline already exists; calibration state tells dyno which sensors are trustworthy |
| **Fleet / Multi-Shop Mode**    | Sync tunes between a chain's locations; central tune library                             | Cloud sync layer added on top of `profile.db` + `garage`               |
| **Tuner Certification**        | Track which staff tuner is signed off on which engine families                            | `experience_level` + per-user profile schema scales to per-tuner       |
| **Customer-Facing Reports**    | Generate a PDF the shop hands the customer: before/after dyno, AFR plots, safety summary | Uses the Calibration Center as the "ready for tune" gate              |
| **Warranty/Liability Pack**    | Require the shop tuner to confirm safety checklist + sensor calibration before they Burn  | Uses the existing acknowledged/calibrated state per sensor             |

Notice the pattern: every one of those reads from infrastructure built in v0.6 (`profile.db`, garage, calibration state, tutorial progress). None of them require you to commit to building anything specific now. You just keep the foundation *clean and documented* so when a shop says "we'd pay $X/month for that," you can move fast.

**What this means concretely for v0.6:**
- Don't expose any "plugin API" surface to users yet. No plugin marketplace page. No "premium" toggle.
- *Do* keep the schema and the `setCalibrationState()` API on `TunerGauge` public and documented internally — so a future plugin (yours or someone else's) can register sensors and gauges without you having to refactor.
- Don't write a public extension contract yet either. You'll need real plugin authors to know what they actually need.

The foundation work is invisible to v0.6 users. That's the right call. Shops aren't using v0.6.

---

## 8. Things I'd ruthlessly cut from the original brief

You said be ruthless, so:

1. **"Take name AND email AND car or they can't use it" — no.** Hard gating a free open-source desktop ECU tuner behind a form is the fastest way to make people pirate or fork it. Soft-collect, skip-friendly, and the app works fully without any of it. The fields you actually need (units, experience level) are 5 seconds of input, not 5 minutes.

2. **A tutorial that "checks calibration of all sensors, fueling, sparking" as a first-run mandatory thing — no.** That's a 30-minute setup wizard and you'd lose 80% of users. Move sensor calibration into the glow system (passive nag — works forever, not just once) and ignition/fueling into curriculum modules they open when ready.

3. **"Choose to see the onboarding as soon as the app opens" as a setting — yes, but only as `Help → Replay`.** Do not put it in a settings tab labeled "Onboarding." Settings tabs are for things people configure regularly. Onboarding is a one-shot.

4. **Don't store email in plaintext in a shipped SQLite file.** If you really collect email, at least keep it isolated to `profile.db` (which lives in the user's AppData, not in the project bundle) and document in a privacy notice that it's local and never transmitted. Better: hash it if you're only using it to dedupe newsletter signups later.

5. **Don't add a "Pro tuner" tier to the experience picker that *unlocks* features.** Tempting for the shop-plugin hook, but you'll create a class system on day one and the people who lie about being pros will hurt their engines. Use the field for *defaults* (more verbose tooltips for beginners, terser UI for pros) — never for gating safety features.

6. **Don't tease paid plugins in v0.6.** No "Coming soon: Pro Plugins" banner. No teaser in Settings. The free hobbyist product needs to feel complete and loved on its own. Year-2 monetization is invisible until it exists.

---

## 9. Open questions for you before I write a single line of code

1. **Email collection: in or out?** If yes, I'll add a small privacy notice + an unsubscribe-by-default model. If no (my recommendation), I'll cut the field entirely and re-add it the day you have a server.
2. **Units default**: metric (recommended — most of your reference docs are in kPa and °C) or imperial (US tuners expect psi and °F)?
3. **Demo Mode in the tour**: should step 6 of the product tour *auto-enable* Demo Mode so the user sees gauges moving immediately, then disable it when they continue? I think yes.
4. **Garage on day one or punt to Phase C**: do you want multi-car from v0.6, or single-active-car only and multi-car later? Punting is fine.
5. **The "running with uncalibrated sensors" banner**: hard banner or soft toast? I argued hard banner. Your call.

---

Sources: `C:\OS Tuner\src\main.cpp`, `C:\OS Tuner\src\widgets\DashboardWidget.h`, `C:\OS Tuner\src\utils\Settings.h`, plus the project survey.
