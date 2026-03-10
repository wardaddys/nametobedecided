# Sensor Validation Quick Reference Card

## 🎨 Color Codes

| Color | Hex | Level | Meaning | Action |
|-------|-----|-------|---------|--------|
| 🟢 Green | #4CAF50 | Normal | Within optimal range | None - continue monitoring |
| 🟡 Yellow | #FF9800 | Caution | Outside optimal, within safe | Monitor closely |
| 🔴 Red | #F44336 | Critical | Dangerous range | **Immediate action required** |
| ⚪ Gray | #9E9E9E | Fault | Sensor offline | Check wiring/calibration |

---

## 📊 Sensor Ranges (Speeduino Validated)

### 1. RPM (Engine Speed)
```
Unit: RPM
Optimal: 800 - 8000
Caution: 500 - 8200
Critical: 0 - 12000

⚠️ Alerts:
• >7500 RPM: Yellow (approaching redline)
• >8200 RPM: Red (REV LIMIT - fuel/spark cut)
• Δ >1000 RPM/s: Anomaly (sensor noise)
```

### 2. MAP (Manifold Absolute Pressure)
```
Unit: kPa
Optimal: 30 - 100 (NA), 100 - 200 (boosted)
Caution: 10 - 150
Critical: 0 - 250

Formula: kPa = (V_in / 5) × 100

⚠️ Alerts:
• <25 kPa: Yellow (vacuum leak suspected)
• <10 kPa: Red (sensor fault)
• >150 kPa (NA engine): Yellow (check boost leak)
• Δ >10 kPa/s at idle: Anomaly
```

### 3. TPS (Throttle Position Sensor)
```
Unit: %
Optimal: 0 - 100
Caution: 0 - 98
Critical: 0 - 100 (clamped)

Formula: % = ((V - 0.5) / 4) × 100
Calibration: 0.5V - 4.5V linear

⚠️ Alerts:
• <1%: Yellow (stuck closed)
• >98%: Yellow (partial open)
• TPS >80% + MAP <50 kPa: Red (mismatch error)
```

### 4. IAT (Intake Air Temperature)
```
Unit: °C
Optimal: -20 to 80
Caution: -40 to 90
Critical: -50 to 150

Formula: Steinhart-Hart (NTC thermistor)
1/T = A + B×ln(R) + C×(ln(R))³

⚠️ Alerts:
• <-30°C: Yellow (extreme cold)
• >90°C: Yellow (heat soak)
• >150°C: Red (overheat/fault)
• Δ >50°C in 10s: Anomaly (disconnect)
```

### 5. CLT (Coolant Temperature)
```
Unit: °C
Optimal: 80 - 95 (operating temp)
Caution: 40 - 100
Critical: 0 - 120

Warmup: 40 - 80°C

⚠️ Alerts:
• <70°C: Yellow (cold running)
• >100°C: Yellow (overheating starts)
• >120°C: Red (danger - stop engine)
• No change >20min: Anomaly (stuck sensor)
```

### 6. AFR (Air-Fuel Ratio)
```
Unit: AFR (also shows Lambda)
Optimal: 14.2 - 14.9 (idle/cruise)
        11.0 - 13.0 (WOT)
Caution: 10.5 - 15.5
Critical: 9.0 - 18.0

Stoichiometric: 14.7 (Lambda 1.0)

Formula: AFR = 10 + (V_in × 2) [wideband]
Lambda = AFR / 14.7

⚠️ Alerts:
• <10.5: Yellow (rich)
• <9.0: Red (extreme rich - flooding)
• >15.5: Yellow (lean)
• >18.0: Red (extreme lean)
• >15.5 at WOT (TPS >90%): Red (DANGER - knock risk)
```

### 7. Battery Voltage
```
Unit: V
Optimal: 12.0 - 14.0 (running)
        11.5 - 12.5 (cranking)
Caution: 8.0 - 15.0
Critical: 0 - 18.0

⚠️ Alerts:
• <11V: Yellow (weak battery)
• <8V: Red (shutdown risk)
• >15V: Yellow (alternator overcharge)
• >18V: Red (electrical damage risk)
```

### 8. Injector Duty Cycle
```
Unit: %
Optimal: 10 - 80
Caution: 5 - 85
Critical: 0 - 95 (clamped at 100)

Typical: Idle 10-20%, Cruise 20-50%

⚠️ Alerts:
• <5%: Yellow (low fuel)
• >85%: Yellow (high load)
• >95%: Red (leanout risk - injector maxed)
• >80% at <4000 RPM: Anomaly (oversized injectors)
```

### 9. Ignition Timing
```
Unit: ° BTDC (Before Top Dead Center)
Optimal: 10 - 40
Caution: 0 - 50
Critical: -10 to 60

Typical: Idle 10-20°, Cruise 30-40°, WOT 25-35°

⚠️ Alerts:
• <5°: Yellow (retarded)
• >50°: Yellow (advanced - knock risk)
• >60°: Red (extreme advance)
• Δ >10°: Anomaly (timing slip)
```

### 10. Barometric Pressure
```
Unit: kPa
Optimal: 70 - 110 (altitude-adjusted)
Caution: 40 - 120
Critical: 20 - 130

Sea level: ~101.3 kPa
Formula: kPa = (V_in / 5) × 101.3

⚠️ Alerts:
• <50 kPa: Yellow (high altitude)
• >120 kPa: Yellow (low altitude/error)
• Δ >5 kPa/min: Anomaly (weather/motion)
```

### 11. Boost Pressure
```
Unit: psi
Optimal: 0 psi (NA), 5 - 20 psi (turbo)
Caution: -5 to 25
Critical: -10 to 30

Formula: Boost = MAP - Baro (differential)

⚠️ Alerts:
• <-5 psi: Yellow (vacuum leak)
• >25 psi: Yellow (overboost)
• >30 psi: Red (cut boost)
• Δ >5 psi/s: Anomaly (wastegate stuck)
```

### 12. Knock Sensor
```
Unit: V (voltage)
Optimal: 0 - 2 (baseline with spikes <2V)
Caution: 0 - 3
Critical: 0 - 5

Frequency: 6-8 kHz (frequency-tuned)

⚠️ Alerts:
• >3V: Yellow (retard timing)
• >1V sustained >2s: Anomaly (detonation)
```

---

## 🚨 Critical Alerts (Red)

### Immediate Actions Required

| Sensor | Threshold | Alert | Action |
|--------|-----------|-------|--------|
| RPM | >8200 | REV LIMIT EXCEEDED | Fuel/spark cut active - reduce throttle |
| AFR | >15.5 @ WOT | LEAN AT WOT | Reduce throttle immediately - knock risk |
| CLT | >120°C | OVERHEAT DANGER | Stop engine - check cooling system |
| Boost | >30 psi | OVERBOOST | Boost cut active - check wastegate |
| Battery | <8V | LOW VOLTAGE | Charging system fault |
| IDC | >95% | INJECTOR MAXED | Leanout risk - reduce load |

---

## 🔍 Anomaly Patterns

| Pattern | Detection | Likely Cause |
|---------|-----------|--------------|
| RPM spike >1000/s | Δ >1000 in <1s | Sensor noise / loose connection |
| MAP fluctuation | Δ >10 kPa/s @ idle | Throttle body issue / vacuum leak |
| TPS/MAP mismatch | TPS >80% + MAP <50 | Throttle plate stuck / MAP sensor fault |
| IAT rapid change | Δ >50°C in 10s | Sensor disconnected |
| CLT stuck | No change 20+ min | Thermostat stuck / faulty sensor |
| AFR lean at WOT | Lambda >1.2 @ WOT | Fuel starvation / injector issue |
| Boost spike | Δ >5 psi/s | Wastegate malfunction |

---

## 💡 Usage Tips

### Dashboard Widgets

**RPM Gauge**
- Green zone: 0-6000 RPM
- Yellow zone: 6000-7500 RPM
- Red zone: 7500+ RPM
- Digital display changes color with validation
- Modal alert appears >8200 RPM

**AFR Widget**
- Shows both AFR and Lambda
- Green bar = stoich zone (14.2-14.9)
- Tooltip shows calibration formula
- Click info icon for details

**Sensor Monitor**
- Grid view of 6 critical sensors
- Progress bars show optimal zones
- Click row to expand details
- Alert counter at top

### Validation Context

Some validations need context from other sensors:

```typescript
// AFR validation considers TPS
validateSensor('afr', 16.5, { tps: 95 });
// Returns CRITICAL (lean at WOT)

// TPS validation considers MAP
validateSensor('tps', 85, { map: 45 });
// Returns CAUTION (TPS/MAP mismatch)
```

### Reset Anomaly Detection

```typescript
anomalyDetector.reset('rpm'); // Reset specific sensor
anomalyDetector.reset();      // Reset all sensors
```

---

## 📱 UI Elements

### Alert Badges
```
🟢 ✓ All Normal      - No alerts
🟡 ⚠ 2 Caution      - 2 sensors in yellow
🔴 ! 1 Critical     - 1 sensor in red (pulsing)
```

### Progress Bars
```
|████░░░░░░| - Normal (green fill)
|████████░░| - Caution (yellow fill)
|██████████| - Critical (red fill, pulsing)
     ↑
  Optimal zone (lighter background)
```

### Digital Displays
```
Normal:   Green text + soft glow
Caution:  Yellow text + medium glow
Critical: Red text + pulsing glow + border flash
```

---

## 🎯 Testing Checklist

- [ ] Enable Live Tuning mode
- [ ] Observe RPM gauge colors change
- [ ] Watch for >8200 RPM modal alert
- [ ] Check AFR widget lean/rich indicators
- [ ] Click SensorMonitor rows to expand
- [ ] Verify alert counter updates
- [ ] Test anomaly detection (rapid changes)
- [ ] Hover tooltips for formulas
- [ ] Dismiss critical alerts

---

## 📚 References

✅ [Speeduino Sensor Calibration Wiki](https://wiki.speeduino.com/en/configuration/Sensor_Calibration)  
✅ [Speeduino Manual PDF](https://speeduino.com/Speeduino_manual.pdf)  
✅ [Speeduino Tuning Reference](https://wiki.speeduino.com/en/reference/Tuning_reference)  
✅ [MegaSquirt Tuning Guide](https://www.megamanual.com/v22manual/mtune.htm)

---

**Print this card** | **Keep near workstation** | **Version 2.0** | **Updated Oct 2025**
