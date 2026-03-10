# All Tables - Quick Start Guide

## 🚀 Quick Access

1. Click **"All Tables"** tab in top navigation bar
2. Browse tables by category (click to expand/collapse)
3. Each table shows:
   - 🎨 Hondata-style 3D/2D graph (toggle with buttons)
   - 📊 Editable grid with real-time updates
   - ⚙️ Tools: Copy, Download, Upload, Reset, Save

## 🎯 Key Features at a Glance

| Feature | Description |
|---------|-------------|
| **3D Surface** | Rotatable wireframe with color gradient (blue=low → red=high) |
| **2D Heatmap** | Top-down color view with cell selection |
| **Live Mode** | Blue marker shows current operating point |
| **Bulk Edit** | Select cells → Set step → Add/Subtract |
| **VTEC Profiles** | Low/High cam tabs for VTEC engines |
| **Auto-Hide** | Tables appear based on engine config |

## 📋 Available Tables (15+)

### Always Available
- VE Table, AFR Target, Fuel Pulse Width
- Ignition Advance
- Accel Enrichment, Cold Start, IAT Compensation
- Idle Speed Target, IACV Duty
- Rev Limiter, Launch Control

### Conditional (Based on Settings)
- **Boost Target, Wastegate Duty** (Turbo/Supercharged)
- **VVT Target Advance** (VVT/VVTI/VANOS engines)
- **VTEC Engagement** (VTEC engines)
- **Dual Profiles** (Low/High cam for VTEC)

## 🎮 Controls

### 3D Graph
- **Drag** = Rotate view
- **Scroll** = Zoom in/out
- **Reset button** = Return to default angle
- **Wireframe toggle** = Show/hide grid lines

### 2D Heatmap
- **Click cell** = Select for editing
- **Ctrl+Click** = Multi-select
- **Edit in grid below** = Direct value entry

### Bulk Editing
1. Select cells (click in 2D or grid)
2. Set step value (e.g., 0.5)
3. Click **+ Add** or **- Subtract**
4. Click **Clear** to deselect

## 🔧 Configuration Integration

Tables visibility controlled by **Settings** (top-right gear icon):

```
Engine Type → Boost tables on/off
Valve Train → VTEC profiles, VVT tables
Fuel System → Staged injection tables
```

Changes apply immediately!

## 💡 Pro Tips

1. **Use 3D for overview** - Spot discontinuities and shape
2. **Use 2D for precision** - Edit exact values
3. **Enable Live Tuning** - See operating point move during dyno
4. **Save after each change** - Click save icon
5. **VTEC engines** - Edit both Low and High profiles
6. **Compare profiles** - Switch tabs to ensure smooth transition

## 🎨 Color Guide

```
🔵 Blue   = Lowest values
🟢 Green  = Low-medium
🟡 Yellow = Medium
🟠 Orange = Medium-high
🔴 Red    = Highest values
```

Black wireframe overlays colored faces (Hondata style).

## ⚡ Quick Workflow

### Initial Setup
1. Go to **Settings** → Configure engine type, valve train, fuel system
2. Go to **All Tables** → Verify correct tables are visible

### Tuning a Table
1. Expand category (e.g., "Fuel Tables")
2. Select table (e.g., "VE Table")
3. Toggle to **3D view** → Inspect overall shape
4. Toggle to **2D view** → Select cells to edit
5. Enter new values in grid OR use bulk adjust
6. Click **Save** icon

### Live Tuning
1. Enable **Live Tuning** toggle (top bar)
2. Watch blue marker move across table
3. Note values at operating point
4. Adjust as needed
5. Save when done

## 📊 Table Axes Examples

| Table | X-Axis | Y-Axis | Z-Axis |
|-------|--------|--------|--------|
| **VE Table** | RPM | MAP (kPa) | VE % |
| **Ignition** | RPM | MAP (kPa) | Degrees BTDC |
| **Boost Target** | RPM | TPS % | Boost kPa |
| **Accel Enrich** | TPS Rate | ECT °C | Fuel % |
| **Cold Start** | Time (s) | ECT °C | Enrich % |
| **VVT Advance** | RPM | MAP (kPa) | Advance ° |
| **Idle Target** | ECT °C | Load % | RPM |
| **Rev Limiter** | Gear # | ECT °C | RPM Limit |

Axes labels update automatically when you switch tables!

## 🆘 Common Issues

| Problem | Solution |
|---------|----------|
| Tables missing | Check Settings → Engine Configuration |
| Can't rotate 3D | Click and drag on canvas area |
| No VTEC tabs | Enable VTEC in Settings → Valve Train |
| No boost tables | Set Engine Type to Turbocharged/Supercharged |
| Active cell not moving | Enable Live Tuning mode |

## 🔗 Related Docs

- `ALL_TABLES_GUIDE.md` - Complete documentation
- `IMPLEMENTATION_SUMMARY.md` - Technical details
- Settings → Technical Database - For injector/sensor specs

---

**Keyboard Shortcuts**: Arrow keys to navigate | Ctrl+C/V to copy/paste | +/- to adjust | Esc to clear selection

**Need Help?** See full guide in `ALL_TABLES_GUIDE.md`
