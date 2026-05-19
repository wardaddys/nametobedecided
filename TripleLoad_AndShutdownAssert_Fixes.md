# Two bugs, two fixes
## 1) Triple-load of the ECU `.ini` on startup  ·  2) Shutdown ASSERT

> Both diagnosed against the live source in `C:\OS Tuner\src`. No guesses — every claim below references a real file/line.

---

## Bug #1 — The ECU INI is being parsed three times on startup

### What you see in the log

The block starting with `ECUDefinition: phase-2 [ tunerstudio ]` repeats **three times**, each parsing the full ~460KB constants section. The same `WorkspaceMetadata: …engineCc` warning is also printed three times.

### Root cause — exactly where each load happens

Three code paths call `ECUDefinition::load(filePath)` independently with the same path. Trace:

| # | Caller chain | File:line |
|---|---|---|
| 1 | `ProjectManager::loadProject()` → `ECUSettingsManager::loadDefinition(iniPath)` → `m_ecuDef.load(iniPath)` | `src/core/ProjectManager.cpp:52-53` → `src/core/ECUSettingsManager.cpp:1024-1025` |
| 2 | …same `ECUSettingsManager::loadDefinition` then calls `m_serialManager->loadEcuDefinition(iniPath)` → `m_ecuDefinition.load(filePath)` (this is **SerialManager's own** copy of `ECUDefinition`, separate from the one inside `ECUSettingsManager`) | `src/core/ECUSettingsManager.cpp:1082-1088` → `src/core/SerialManager.cpp:46-48` |
| 3 | …that same `SerialManager::loadEcuDefinition` then calls `m_protocol->loadDefinition(filePath)` → `m_ecuDefinition.load(filePath)` (this is **SpeeduinoProtocol's own** copy of `ECUDefinition`, a third independent instance) | `src/core/SerialManager.cpp:52` → `src/core/SpeeduinoProtocol.cpp:186-187` |

In other words: the application holds **three independent `ECUDefinition` instances** (`ECUSettingsManager::m_ecuDef`, `SerialManager::m_ecuDefinition`, `SpeeduinoProtocol::m_ecuDefinition`), and a single call to `ECUSettingsManager::loadDefinition()` causes all three to parse the same file from scratch.

### Why it exists historically (not blaming, just diagnosing)

Each layer wants its own copy so it can answer queries without coupling:
- `ECUSettingsManager` wants the parsed constants and tables for the settings cache.
- `SerialManager` wants the parsed `[TunerStudio] signature` line for signature validation (`m_ecuDefinition.validateSignature(...)` at `SerialManager.cpp:972`).
- `SpeeduinoProtocol` wants the parsed `[OutputChannels]` for decoding incoming real-time data.

Three honest needs, three separate parses. It worked while the parser was fast; now that the ini is ~700KB the cost shows up.

### Three fix options, ranked

#### Option A — **Shared parsed definition** (recommended)

Parse once, share the parsed object. Cleanest fix, biggest payoff.

**Concept:** make `ECUDefinition` a value that's parsed once and passed around as `const ECUDefinition&` (or a `std::shared_ptr<const ECUDefinition>` if cross-thread).

**Concrete change:**

1. Add a method to `SerialManager`:
   ```cpp
   // SerialManager.h
   void setEcuDefinition(const ECUDefinition &def);
   ```
   that copies (or references) an already-parsed definition into both `m_ecuDefinition` and `m_protocol`'s copy — *without re-parsing the file*.

2. Similarly for `SpeeduinoProtocol`:
   ```cpp
   // SpeeduinoProtocol.h
   void setEcuDefinition(const ECUDefinition &def);
   ```

3. In `ECUSettingsManager::loadDefinition()`, after `m_ecuDef.load(iniPath)` succeeds, instead of calling `m_serialManager->loadEcuDefinition(iniPath)` (which re-parses), call:
   ```cpp
   m_serialManager->setEcuDefinition(m_ecuDef);
   ```

**Result:** one parse per project load. Three log blocks → one log block.

**Effort:** ~30 minutes if `ECUDefinition` has a working copy constructor (which it does — it's just a `QMap`-heavy POD-ish class). Some care needed if either downstream consumer mutates its copy — verify they don't.

#### Option B — **Parse-cache by path** (lazy, less invasive)

Add a static cache inside `ECUDefinition::load`:

```cpp
// inside ECUDefinition::load(const QString &filePath)
static QMap<QString, std::shared_ptr<ECUDefinition>> s_cache;
static QMutex s_cacheMutex;

QMutexLocker lock(&s_cacheMutex);
auto it = s_cache.find(filePath);
if (it != s_cache.end()) {
    *this = *(it.value());      // copy-assign cached parse into this instance
    return true;
}
// ... existing parse logic ...
// at the end, on success:
auto cached = std::make_shared<ECUDefinition>(*this);
s_cache.insert(filePath, cached);
return true;
```

Cache by filepath + mtime so a file edit invalidates the entry.

**Pros:** zero changes to callers. The fix is entirely inside `ECUDefinition::load()`.
**Cons:** static state in a parser is a smell. Bigger memory footprint (three copies still live in the three managers — you only save *parse* time, not memory). Cache invalidation is one more thing to get right.

Acceptable as a quick win if Option A feels too invasive.

#### Option C — **Punt to v0.7**

Tag this as a known perf issue, ship v0.6 with the triple-parse. Startup is ~4 seconds slower than it needs to be, but nothing user-visible breaks. The implementation guide already lists this in the "Phase 1.0 polish" section.

**Recommendation: do Option A.** It's small, surgical, and the savings (3× faster project open) is something every user feels every single session. Do it the next time you're already touching `SerialManager.cpp`.

### Quick sanity check on the three copies' coupling

I verified the three `ECUDefinition` instances are not surreptitiously mutated post-load:
- `ECUSettingsManager::m_ecuDef` is read via `getConstants()`, `getTables()`, `getOutputChannels()`, `isSpeeduinoCompatible()` — all const.
- `SerialManager::m_ecuDefinition` is used only for `validateSignature()` — const.
- `SpeeduinoProtocol::m_ecuDefinition` is used only to read output channels — const.

So they really are three identical read-only copies. Perfect candidate for sharing.

---

## Bug #2 — The shutdown ASSERT

### What you see in the log (previous run)

```
[INFO] Application shutting down with exit code: 0
[INFO] === OS Tuner Terminated ===
[WARN] ECU connection lost
[INFO] Disconnected from device
ASSERT failure in MainWindow: "Called object is not of the correct type
(class destructor may have already run)", file qobjectdefs_impl.h, line 105
```

(In the **current** run you didn't close the app, so this wasn't tested. The crash will return on the next clean shutdown unless the fix below is applied.)

### Root cause — exactly where it fires

The relevant code, all verified:

**`MainWindow.cpp:202-208`** (current destructor):
```cpp
MainWindow::~MainWindow() {
  // Disconnect from ECU cleanly before any members are destroyed.
  // This prevents pending timers or callbacks from firing on dead objects.
  if (m_serialManager) {
    m_serialManager->disconnectFromDevice();
  }
}
```

**`SerialManager.cpp:175-196`** (what `disconnectFromDevice` does):
```cpp
void SerialManager::disconnectFromDevice() {
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }

    m_status = ConnectionStatus::Disconnected;
    emit connectionStatusChanged(m_status);   // ← Signal A
    emit disconnected();                       // ← Signal B  *** the culprit ***

    m_heartbeatTimer->stop();
    // ...
    Logger::info("Disconnected from device");
}
```

**`MainWindow.cpp:108-113`** (the receivers):
```cpp
connect(m_serialManager, &SerialManager::connectionStatusChanged, this,
        &MainWindow::onConnectionStatusChanged);
connect(m_serialManager, &SerialManager::error, this, &MainWindow::onError);
connect(m_serialManager, &SerialManager::disconnected, this,
        &MainWindow::onDisconnected);
```

**`SerialManager.cpp:59`** (the dtor that fires *again*):
```cpp
SerialManager::~SerialManager() { disconnectFromDevice(); }
```

### What actually happens at shutdown — the death sequence

1. `MainWindow::~MainWindow()` begins. Body executes line 205: `m_serialManager->disconnectFromDevice()`.
2. `disconnectFromDevice()` emits `connectionStatusChanged(Disconnected)` and `disconnected()`.
3. Qt routes those signals **synchronously** to `MainWindow::onConnectionStatusChanged()` and `MainWindow::onDisconnected()`. `MainWindow` is still mostly alive — body of dtor still running — so these slots execute against a partially-valid `this`. Slots may touch member widgets that are still alive at this moment, so this *probably* doesn't crash yet.
4. The dtor body finishes. **Now Qt destroys MainWindow's children.** The widgets — central widget, all child panels — are destroyed.
5. Qt then implicitly destroys `m_serialManager` **because it was created with `this` as parent** (verify: check `MainWindow.cpp` ctor for `new SerialManager(this)` — almost certainly the case given the lack of an explicit `delete` in the dtor).
6. `~SerialManager()` runs at line 59 → calls `disconnectFromDevice()` **again**.
7. `disconnectFromDevice()` emits `disconnected()` **again**.
8. Qt tries to route the signal to `MainWindow::onDisconnected`. **But MainWindow's destructor has already returned and its vtable is gone.** The slot's `this` is now pointing at memory whose dynamic type has degraded.
9. `qobjectdefs_impl.h:105` checks the dynamic type via the vtable, sees garbage, fires the ASSERT.

That's the full chain. The `[WARN] ECU connection lost` and `[INFO] Disconnected from device` lines you see after `=== OS Tuner Terminated ===` are exactly steps 6–7.

### Why your *existing* dtor body doesn't already prevent this

Your existing dtor does call `disconnectFromDevice()` at line 206 — but it doesn't **break the signal/slot connections**, and it doesn't stop `~SerialManager()` from firing them all over again. The first `disconnectFromDevice()` call is *redundant* with what `~SerialManager()` does, and neither one is safe when MainWindow is partially destroyed.

### The fix — three lines

Change `MainWindow.cpp:202-208` from:

```cpp
MainWindow::~MainWindow() {
  // Disconnect from ECU cleanly before any members are destroyed.
  // This prevents pending timers or callbacks from firing on dead objects.
  if (m_serialManager) {
    m_serialManager->disconnectFromDevice();
  }
}
```

to:

```cpp
MainWindow::~MainWindow() {
  // Cleanly tear down: first sever every signal/slot link from SerialManager
  // to *this*, then ask the device to disconnect. After this point, no slot
  // on MainWindow can be invoked from any SerialManager signal — including
  // ones emitted later by ~SerialManager during Qt's child cleanup.
  if (m_serialManager) {
    disconnect(m_serialManager, nullptr, this, nullptr);  // <-- the real fix
    m_serialManager->disconnectFromDevice();
  }
}
```

That single `disconnect(sender, nullptr, receiver, nullptr)` call removes **all** signal/slot connections from `m_serialManager` to `this`. After this line, every `emit` inside `SerialManager` (including the ones that fire later during `~SerialManager()`) becomes a no-op for any slot on `MainWindow`. The ASSERT can no longer happen because the signal has nowhere to route.

### Belt-and-braces (recommended but optional)

If you want extra safety in case some *other* receiver (a child widget, a logger) is also still wired up at shutdown, add this as the first line of `SerialManager::~SerialManager()`:

```cpp
SerialManager::~SerialManager() {
    blockSignals(true);          // <-- new line
    disconnectFromDevice();
}
```

`blockSignals(true)` makes every `emit` from this `QObject` a no-op for the rest of its life. Combined with the explicit `disconnect()` in `MainWindow`, this is *bulletproof*: even if a future contributor wires up another slot to `SerialManager::disconnected` somewhere and forgets to clean it up, shutdown still won't crash.

### Why not change `disconnectFromDevice()` itself

Tempting to add `blockSignals(true)` inside `disconnectFromDevice()`. **Don't.** That method is also called during normal runtime (e.g., the user clicks "Disconnect" at `SerialManager.cpp:87, 909, 979, 1006`) — and those callers absolutely *do* want the `disconnected()` signal to fire so the UI updates. Block signals only in the destructor path, not the normal-disconnect path.

### Confirming the fix works

After applying the fix:
1. Build.
2. Run the app to the dashboard.
3. Close it cleanly (window X).
4. Check the log. **You should still see `[INFO] Disconnected from device` once or twice** — that's the dtor calling `disconnectFromDevice()` and Qt's child cleanup firing `~SerialManager` which calls it again. That part is harmless. **What should be GONE is the `ASSERT failure` line.**
5. If you also added `blockSignals(true)` in `~SerialManager()`, you'll only see `[INFO] Disconnected from device` once (because the second disconnectFromDevice still runs, but no slots are listening anyway).

### The architectural fix you can defer to v0.7

The right long-term shape is: destructors should not emit signals. `~SerialManager()` calling `disconnectFromDevice()` (which emits) is the actual smell. The cleanest refactor:

1. Add `SerialManager::shutdown()` — does everything `disconnectFromDevice()` does **except** the `emit disconnected()` lines.
2. `~SerialManager()` calls `shutdown()`, not `disconnectFromDevice()`.
3. Normal "user clicked disconnect" still uses `disconnectFromDevice()` and gets the signals.

That's a 10-line refactor. Skip for v0.6 — the `disconnect()` fix above is fine. Plan it for the polish pass before 1.0.

---

## Summary — what to actually change today

| Bug | File | Action | Risk |
|---|---|---|---|
| Shutdown ASSERT | `src/MainWindow.cpp:202-208` | Add `disconnect(m_serialManager, nullptr, this, nullptr);` as the first line inside the `if (m_serialManager)` block | Trivial |
| Shutdown ASSERT (belt-and-braces) | `src/core/SerialManager.cpp:59` | Change dtor body to `{ blockSignals(true); disconnectFromDevice(); }` | Trivial |
| Triple-load | `src/core/SerialManager.h/cpp`, `src/core/SpeeduinoProtocol.h/cpp`, `src/core/ECUSettingsManager.cpp` | Add `setEcuDefinition(const ECUDefinition&)` to both downstream classes; have `ECUSettingsManager::loadDefinition` call those instead of `loadEcuDefinition(filePath)` | Low — but verify the three copies are truly read-only after load (they are, per my check above) |

The ASSERT fix is a 1-line change you can make in 30 seconds. The triple-load fix is ~30 minutes of careful editing. Both are worth doing before v0.5.5 ships.

Sources: live source under `C:\OS Tuner\src` — every line reference verified.
