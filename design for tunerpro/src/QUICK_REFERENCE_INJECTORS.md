# Injector Settings - Quick Reference Card

## 🎯 Where to Find It
**Settings (⚙️) → Fuel & Injectors Tab**

---

## 📋 Quick Setup (2 Minutes)

### Step 1: Calculate Required Size
```
Click "Show Calculator"
Enter your target HP: [____]
Number of injectors: [4]
Engine type: [Boosted/NA]
→ See required flow + suggestions
```

### Step 2: Select Injector
```
Manufacturer: [Choose brand ▼]
Model: [Choose model ▼]
→ Specs auto-fill
```

### Step 3: Load Dead Time
```
Scroll to "Dead Time Calibration Table"
Click: [Load from Brand Name]
→ Table auto-fills
✓ Done!
```

---

## 🔢 Common Injector Sizes

| Target HP | Engine | Suggested Size | Example Models |
|-----------|--------|----------------|----------------|
| 300-400 | NA | 440-550cc | Bosch 0280, ID725 |
| 400-500 | Turbo | 550-725cc | ID725, Keihin 550 |
| 500-700 | Turbo | 850-1050cc | ID1000, DW1000 |
| 700-1000 | Turbo | 1300-1650cc | ID1300X, FIC1650 |
| 1000+ | Race | 2000+cc | ID2000, FIC2150 |

---

## ⚙️ Key Settings Explained

### Flow Rate
**What it is:** Amount of fuel injector can deliver per minute  
**Units:** cc/min or lb/hr (1 lb/hr = 10.5 cc/min)  
**Example:** 1000cc = 95 lb/hr  

### Impedance
**What it is:** Electrical resistance of injector coil  
**Typical:** 12-15 Ohms (high-Z), <12 Ohms (low-Z)  
**⚠️ Warning:** Low-Z may need resistor box  

### Dead Time
**What it is:** Delay from ECU signal to fuel spray  
**Typical:** 0.6-1.0ms @ 14V  
**Why it matters:** 1ms dead time = 100% error at 1ms pulse width!  

### Fuel Pressure
**What it is:** Pressure in fuel rail  
**Standard:** 43.5 PSI (3.0 bar)  
**Returnless:** 58 PSI (4.0 bar)  
**Effect:** Higher pressure = more flow (Flow ∝ √Pressure)  

---

## 🎨 Color Guide (Dead Time Table)

| Color | Latency | Meaning |
|-------|---------|---------|
| 🟢 Green | <0.5ms | Excellent - fast response |
| 🟡 Yellow | 0.8-1.0ms | Moderate - acceptable |
| 🔴 Red | >1.5ms | High - may affect idle |

---

## 🔧 Staged Injection (Advanced)

### When to Use:
- Need >1000cc total flow
- Want good idle quality
- Running E85 with high power

### How to Enable:
1. Settings → Engine Configuration
2. Fuel System: **Port Injection (Staged)**
3. Return to Fuel & Injectors tab
4. ⭐ Secondary section appears

### Typical Setup:
- **Primary:** 550cc (idle to mid-load)
- **Secondary:** 1600cc (high load)
- **Total:** 2150cc (800+ HP capable)

---

## 🧮 Quick Calculations

### Required Flow:
```
Flow (cc/min) = (HP × BSFC × 0.5) / (# Injectors × Max DC)

Where:
- BSFC = 0.5 (NA), 0.6 (Turbo)
- Max DC = 0.85 (85%)
```

**Example:** 600 HP turbo, 4 injectors
```
Flow = (600 × 0.6 × 0.5) / (4 × 0.85)
     = 180 / 3.4
     = 53 lb/hr
     = 556 cc/min
→ Select 600-725cc injectors
```

### Pressure Adjustment:
```
New Flow = Base Flow × √(New Pressure / Base Pressure)
```

**Example:** 1000cc @ 3.0 bar → 4.0 bar
```
New Flow = 1000 × √(4.0/3.0)
         = 1000 × 1.155
         = 1155 cc/min (+15.5%)
```

---

## ⚠️ Common Warnings

### "Low Impedance Warning"
**Cause:** Injector has <12Ω resistance  
**Fix:** 
- Check ECU supports low-Z injectors
- Add resistor box if needed
- Or select high-Z injectors

### "High Dead Time"
**Cause:** Latency >1.5ms  
**Effect:** Poor idle, lean at low PW  
**Fix:**
- Consider faster injectors
- Or tune dead time compensation carefully

### "Duty Cycle >90%"
**Cause:** Injectors too small for HP target  
**Effect:** Fuel starvation, lean condition  
**Fix:** Select larger injectors immediately

---

## 💡 Pro Tips

1. **Size for 80-85% duty max** - leaves safety margin
2. **E85 needs +30% flow** vs gasoline
3. **Test dead time at idle** - most critical
4. **Match all 4 injectors** - same brand/model
5. **Higher pressure** = better atomization + more flow

---

## 🚨 Troubleshooting

| Problem | Likely Cause | Quick Fix |
|---------|--------------|-----------|
| Lean idle | Dead time too low | Increase by 0.1ms |
| Rich idle | Dead time too high | Decrease by 0.1ms |
| Hunting idle | Injectors oversized | Check duty cycle >10% |
| Lean WOT | Duty cycle >90% | Need larger injectors |
| Hard start | Dead time wrong | Load from database |

---

## 📞 Quick Help

**Can't find injector in database?**
- Use calculator to find similar flow rate
- Manually enter specs
- Or contact injector manufacturer for data

**Calculator shows no suggestions?**
- HP target may be too high/low
- Adjust BSFC (0.5 NA, 0.6 turbo)
- Browse database manually

**Dead time table confusing?**
- Just click "Load from [Brand]"
- Default values work for most applications
- Fine-tune later if needed

---

## 🎓 Learn More

- **V3_IMPLEMENTATION_SUMMARY.md** - Full technical details
- **INJECTOR_QUICK_START.md** - Step-by-step walkthroughs
- **INJECTOR_SETTINGS_FINAL.md** - Complete implementation guide
- **Settings → Technical Database** - In-app reference

---

## ✅ Pre-Flight Checklist

Before tuning:
- [ ] Injector selected and specs confirmed
- [ ] Flow rate appropriate for HP target
- [ ] Dead time loaded/verified
- [ ] Fuel pressure entered (if not 3.0 bar)
- [ ] Max HP calculated >target HP
- [ ] Duty cycle <85% at max load
- [ ] Low-Z warning addressed (if any)
- [ ] Staged injection configured (if applicable)

---

*Quick Reference v1.0 - Print or save for quick access!*
