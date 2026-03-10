# Injector Configuration - Quick Start Guide

## 🎯 5-Minute Setup

### Step 1: Select Your Injectors
1. Click ⚙️ **Settings** (top-right gear icon)
2. Go to **Injectors** tab
3. Click **Show Calculator**
4. Enter your target HP → Get recommended injectors
5. Select injector from dropdown

### Step 2: Configure Dead Time
1. Go to **Dead Time** tab
2. Click **Load from [Injector Brand]** button
3. Done! (or manually edit cells if you have bench data)

### Step 3: Adjust Fuel Pressure (if different)
1. Back to **Injectors** tab
2. Find "Actual Fuel Pressure" field
3. Enter your actual pressure (e.g., 3.5 bar)
4. Flow rate auto-updates

---

## 📋 Common Setups

### Street/Mild Turbo (400-500 HP)
**Recommended:** Injector Dynamics ID725 (685 cc/min)
- Excellent atomization
- Street-friendly idle
- E85 compatible
- Flow: 65 lb/hr

**Alternative:** DeatschWerks DW1000 (1050 cc/min)
- More headroom
- Slight overkill but safe
- Flow: 100 lb/hr

### High HP Turbo (600-800 HP)
**Recommended:** Injector Dynamics ID1300X (1340 cc/min)
- E85 rated
- Excellent flow
- Proven reliability

**Alternative:** Fuel Injector Clinic FIC1650 (1732 cc/min)
- Race-spec
- Maximum flow
- 800+ HP capable

### Extreme/Drag (1000+ HP)
**Recommended:** Injector Dynamics ID2000 (2200 cc/min)
- Drag racing proven
- Massive flow
- Low dead time

**Alternative:** Fuel Injector Clinic FIC2150 (2265 cc/min)
- 1200+ HP capable
- Race-only

---

## ⚠️ Important Values

### Dead Time (Latency)
| Injector Type | @ 14V | @ 12V | @ 10V |
|---------------|-------|-------|-------|
| **Excellent** | <0.7ms | <0.9ms | <1.2ms |
| **Good** | 0.7-0.9ms | 0.9-1.1ms | 1.2-1.5ms |
| **Acceptable** | 0.9-1.1ms | 1.1-1.3ms | 1.5-1.8ms |
| **Poor** | >1.1ms | >1.3ms | >1.8ms |

### Fuel Pressure
| System Type | Pressure |
|-------------|----------|
| Return-Style (Vacuum Referenced) | 43.5 PSI / 3.0 bar |
| Returnless | 58 PSI / 4.0 bar |
| Boost-Referenced | Base + Boost (e.g., 3 bar + 1 bar boost = 4 bar total) |

### Max Duty Cycle
- **Street:** 80% max (safety margin, longevity)
- **Performance:** 85% max (balanced)
- **Race:** 90% max (short term only, risk of saturation)

---

## 🔧 Troubleshooting

### ❌ Idle too lean
**Fix:** Increase dead time by 0.1ms @ 13.5V row

### ❌ Idle too rich
**Fix:** Decrease dead time by 0.1ms @ 13.5V row

### ❌ Injectors hitting 90%+ duty cycle
**Fix:** Need larger injectors! Use calculator to size up

### ❌ Idle quality poor (hunting)
**Fix:** Might be oversized injectors. Check that idle duty cycle is >10%

### ❌ "Low Impedance Warning"
**Fix:** 
- Check ECU supports low-Z injectors
- May need resistor box
- Or switch to high-Z injectors (14-16Ω)

---

## 💡 Pro Tips

1. **Size injectors for 80-85% duty at max HP** - leaves safety margin
2. **E85 users: Add 30% flow capacity** - E85 requires more fuel
3. **Dead time affects idle most** - WOT is less sensitive
4. **Staged injection:** Primary sized for cruise, secondary for WOT
5. **Pressure compensation is automatic** - just enter actual pressure

---

## 📞 Quick Reference

**Flow Conversion:**
- 1 lb/hr = 10.5 cc/min @ 3 bar

**BSFC (Brake Specific Fuel Consumption):**
- NA: 0.5 lb/hp-hr
- Turbo: 0.6 lb/hp-hr
- E85: 0.65 lb/hp-hr

**Required Flow Formula:**
```
Flow (cc/min) = (HP × BSFC × 0.5) / (Injectors × Duty)
```

**Example:** 500 HP turbo, 4 injectors, 85% duty
```
Flow = (500 × 0.6 × 0.5) / (4 × 0.85)
     = 150 / 3.4
     = 44.1 lb/hr
     = 463 cc/min per injector
```
→ Select **550cc injectors** (safety margin)

---

## ✅ Setup Checklist

- [ ] Injector selected from database
- [ ] Flow rate confirmed
- [ ] Dead time loaded/verified
- [ ] Fuel pressure entered (if not 3.0 bar)
- [ ] Max HP calculated and sufficient
- [ ] Duty cycle <85% at max HP
- [ ] Dead time table complete (all voltage points)
- [ ] Staged injection configured (if applicable)
- [ ] Low impedance warning addressed (if applicable)

---

## 🎓 Learn More

See **V3_IMPLEMENTATION_SUMMARY.md** for:
- Complete injector database list
- Advanced dead time tuning
- Staged injection workflows
- Technical formulas and theory

---

*Quick Start Guide v1.0*
