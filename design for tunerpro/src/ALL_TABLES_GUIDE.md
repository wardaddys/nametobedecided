# All Tables System - Comprehensive ECU Tuning Tables

## Overview

The **All Tables** system provides a unified, Hondata SManager-style interface for managing all ECU tuning tables with:

- ✅ **Working 3D/2D graphs** with Hondata-style wireframe surface plots
- ✅ **Conditional visibility** based on engine configuration (boost, VTEC, VVT, staged injection)
- ✅ **Dynamic axis labels** that update per table (RPM, MAP, TPS, ECT, etc.)
- ✅ **Dual cam profiles** for VTEC engines (Low/High speed profiles)
- ✅ **Live operating point markers** during tuning
- ✅ **Bulk editing tools** for selected cells
- ✅ **15+ comprehensive tuning tables**

## Table Categories

### 🔥 Fuel Tables
- **VE Table** - Volumetric Efficiency (primary fuel calibration)
- **AFR Target** - Target air-fuel ratio map
- **Fuel Pulse Width** - Base injector pulse width

### ⚡ Ignition Tables
- **Ignition Advance** - Base timing map with cam profile support

### 🚀 Boost Control (Turbocharged/Supercharged Only)
- **Boost Target** - Target boost pressure vs RPM/TPS
- **Wastegate Duty** - Wastegate solenoid duty cycle control

### 🌡️ Compensation Tables
- **Acceleration Enrichment** - Transient fuel during throttle changes
- **Cold Start Enrichment** - Additional fuel during cold starts
- **IAT Compensation** - Fuel correction for intake air temperature

### ⚙️ Valve Timing & Idle
- **VVT Target Advance** - Variable valve timing control (VVT engines only)
- **Idle Speed Target** - Target idle RPM vs coolant temp
- **IACV Duty Cycle** - Idle air control valve control

### 🛡️ Protection & Limiters
- **Rev Limiter** - RPM limits per gear and temperature
- **Launch Control** - Launch RPM limit vs speed/TPS

### 🏁 VTEC Control (VTEC Engines Only)
- **VTEC Engagement** - VTEC switchover point map

## Key Features

### Hondata-Style 3D Visualization
- **Colored surface plots** with gradient (blue=low, green=mid, yellow/orange=high, red=peak)
- **Black wireframe overlay** on colored faces for grid reference
- **Rotatable view** - drag to rotate, scroll to zoom
- **Live operating point** - blue pulsing marker shows current ECU cell
- **Interactive controls** - reset view, zoom in/out, toggle wireframe
- **Dynamic axes** - labels and units automatically update per table type

### 2D Heatmap View
- **Top-down color-coded grid** with same gradient as 3D
- **Cell selection** - click cells for bulk editing
- **Editable values** - direct input with validation
- **Color legend** - shows value range for current table

### Conditional Table Visibility

Tables automatically show/hide based on engine configuration:

| Configuration | Tables Shown |
|--------------|--------------|
| **Turbocharged/Supercharged** | Boost Target, Wastegate Duty |
| **VTEC** | VTEC Engagement + Dual profiles for VE, AFR, Ignition |
| **VVT/VVTI/VANOS** | VVT Target Advance |
| **Staged Injection** | Primary/Secondary fuel tables |
| **NA (Naturally Aspirated)** | No boost tables |

### VTEC Dual Cam Profiles

When VTEC is enabled, applicable tables show **Low Cam** and **High Cam** tabs:
- Separate maps for low-speed and high-speed cam profiles
- Tab switching to edit each profile independently
- Live data shows active profile during tuning
- Smooth transition warnings at VTEC engagement point

### Bulk Editing Tools

1. **Select cells** - Click cells in 2D view or editable grid
2. **Set step value** - Adjust increment/decrement amount
3. **Add/Subtract** - Apply changes to all selected cells
4. **Clear selection** - Deselect all cells

### Live Tuning Mode

When Live Tuning is enabled:
- Blue pulsing marker appears on 3D/2D graphs at current operating point
- Active cell highlighted in editable grid
- Real-time updates as engine conditions change
- Simulated ECU data sweeps across RPM/load ranges

## Usage

### Access All Tables
1. Click **"All Tables"** tab in top navigation
2. Tables are organized by collapsible categories
3. Click category headers to expand/collapse

### Edit Table Values
1. **3D View**: Rotate and inspect surface plot
2. **2D View**: Click cells to select, edit directly in grid below
3. **Bulk Edit**: Select multiple cells, set step, click Add/Subtract
4. **Save**: Click save icon to commit changes

### Switch Between Tables
- Expand categories to see available tables
- Each table shows its own 3D/2D graph and editable grid
- Graphs update in real-time with edits

### VTEC Profile Switching
- For VTEC-equipped engines, see **Low Cam** / **High Cam** tabs
- Click tab to switch between profiles
- Edit each profile independently
- Both profiles stored separately

## Table Axes Reference

### Common X-Axis Types
- **RPM** - Engine speed (500-10000 rpm)
- **ECT** - Coolant temperature (-20-110°C)
- **IAT** - Intake air temperature (-20-80°C)
- **TPS Rate** - Throttle position rate (0-300 %/s)
- **Speed** - Vehicle speed (0-50 km/h for launch control)
- **Time After Start** - Seconds since engine start
- **Gear** - Current gear (1-6)

### Common Y-Axis Types
- **Load (MAP)** - Manifold absolute pressure (20-300 kPa)
- **TPS** - Throttle position (0-100%)
- **ECT** - Coolant temperature
- **Boost Error** - Difference from target boost
- **RPM Error** - Difference from target idle RPM
- **Load Modifier** - Idle load compensation

### Z-Axis (Value) Types
- **VE %** - Volumetric efficiency percentage
- **AFR λ** - Air-fuel ratio (lambda or AFR)
- **° BTDC** - Degrees before top dead center (timing)
- **ms** - Milliseconds (pulse width, dwell time, dead time)
- **kPa** - Kilopascals (boost pressure)
- **%** - Percentage (duty cycle, fuel add)
- **rpm** - Revolutions per minute (limiter, target idle)
- **bool** - Boolean on/off (VTEC engagement)

## Technical Implementation

### Components
- **`AllTables.tsx`** - Main container with category management
- **`HondataSurface3D.tsx`** - 3D wireframe surface renderer
- **`HondataHeatmap2D.tsx`** - 2D color-coded grid view
- **`TableDefinitions.ts`** - Complete table specifications database

### State Management
- Table data stored per table ID with dual profiles (low/high)
- Cell selection tracked per table
- Active cell updated during live tuning
- Changes isolated per profile for VTEC engines

### Performance
- Canvas-based 3D rendering for smooth 60 FPS
- Painter's algorithm for proper depth sorting
- Efficient state updates with Map data structure
- Lazy loading - only active tables rendered

## Integration with Settings

The Settings dropdown (top-right gear icon) controls table visibility:

1. **Engine Type** → Shows/hides Boost tables
2. **Valve Train** → Enables VTEC dual profiles, VVT tables
3. **Fuel System** → Enables staged injection tables
4. **Control Mode** → Affects closed-loop vs open-loop behavior

Changes in Settings immediately update the All Tables view with smooth fade animations.

## Color Gradient Legend

The Hondata-style gradient represents table values:

```
🔵 Blue (Low Values)    ← Minimum
🟢 Green (Medium-Low)   
🟡 Yellow (Medium)      
🟠 Orange (Medium-High) 
🔴 Red (High Values)    ← Maximum
```

Grid lines are black wireframe overlaid on colored faces for easy reference.

## Keyboard Shortcuts

- **Arrow Keys** - Navigate cells (when grid focused)
- **Ctrl+C** - Copy selected cells
- **Ctrl+V** - Paste values
- **Ctrl+Z** - Undo changes
- **+/-** - Quick increment/decrement selected cells
- **Escape** - Clear selection

## Best Practices

1. **Start with base tables** - VE and Ignition first
2. **Use 3D view** to visualize overall shape and smooth transitions
3. **Use 2D view** for precise cell editing
4. **Enable Live Tuning** to see operating point during dyno runs
5. **Save frequently** - use save button after each change
6. **Export tables** for backup before major changes
7. **Compare Low/High cam profiles** for VTEC engines to ensure smooth transition

## Troubleshooting

**Q: Tables not showing?**  
A: Check Settings → Engine Configuration. Some tables require specific features (boost, VTEC, VVT).

**Q: Can't edit values?**  
A: Ensure Live Tuning is NOT enabled for safety, or confirm you want to edit live.

**Q: 3D graph not rotating?**  
A: Click and drag on the graph canvas. Scroll to zoom.

**Q: Active cell not updating?**  
A: Enable Live Tuning mode and verify ECU connection status.

**Q: Missing Low/High Cam tabs?**  
A: VTEC must be selected in Settings → Valve Train Type.

## Files Modified/Created

### New Files
- `/components/tables/AllTables.tsx` - Main All Tables component
- `/components/tables/HondataSurface3D.tsx` - 3D surface plot renderer
- `/components/tables/HondataHeatmap2D.tsx` - 2D heatmap view
- `/components/tables/TableDefinitions.ts` - Complete table database
- `/components/tables/index.ts` - Exports

### Modified Files
- `/components/layout/TabBar.tsx` - Added "All Tables" tab
- `/App.tsx` - Added AllTables route

## Future Enhancements

- [ ] Import/export individual tables (.csv, .json)
- [ ] Table comparison view (before/after)
- [ ] Undo/redo history stack
- [ ] Smoothing/interpolation tools
- [ ] Copy table between profiles
- [ ] Advanced visualization (contour lines, annotations)
- [ ] Table templates library
- [ ] Real-time ECU sync during edits

---

**Built with React, TypeScript, and Canvas API**  
**Styled to match Hondata SManager and AEM Series 2 aesthetics**  
**Optimized for 1366x768 and 1920x1080 displays**
