# ECU Tuner Pro - Interactive Prototype

A high-fidelity, interactive prototype for a Windows desktop ECU tuning application, designed for Qt/QML conversion.

## 🎯 Project Overview

This prototype demonstrates a comprehensive ECU tuning application with:
- **Real-time monitoring** dashboard with animated gauges
- **Maps & tables editor** with heatmap visualization and 3D surface view
- **AutoTune** with human-in-the-loop approval workflow
- **Log viewer** with timeline scrubbing and parameter correlation
- **Sensor calibration** with interactive curve editor
- **Engine parameters** configuration with nested tabs

## 🎨 Design System

### Color Palette
- **Dark Theme**: App Background `#0B0F12`, Surface `#111419`
- **Primary Accent**: `#1FB6FF` (Cyan Blue)
- **Status Colors**: Warning `#FFB62A`, Critical `#FF3B30`
- **Heatmap**: Cool `#00A676` → Warm `#FFD166` → Hot `#E24B4B`

### Typography
- **Interface**: Inter (400, 500, 700)
- **Data/Mono**: Roboto Mono (400, 700)

### Spacing
Strict **8pt spacing system**: 4px, 8px, 16px, 24px, 32px, 48px

## ✨ Key Features

### 1. Dashboard (Real-Time Monitoring)
- **Draggable & Resizable Widgets**: Enable Designer Mode to customize layout
- **Animated Gauges**: RPM, Boost, AFR with smooth spring animations
- **Live Data**: Toggle "Live Tuning" to see simulated real-time updates
- **Metric Tiles**: ECT, IAT, Battery, TPS with status indicators

### 2. Maps & Tables Editor
- **Heatmap Table**: 16×16 fuel/timing map with color-coded cells
- **Cell Editing**: Double-click to edit, right-click for context menu
- **Multi-Selection**: Click+drag or Shift+click for range selection
- **2D/3D Visualization**: Toggle between heatmap and 3D surface view
- **Editing Tools**: Smooth, Interpolate, and Apply to ECU
- **Modified Indicators**: Orange flags show pending changes

### 3. AutoTune
- **Live Suggestions**: AI-generated tuning recommendations
- **Approve/Reject**: Individual control over each suggested change
- **Authority Limits**: Configurable maximum change percentage
- **Confirmation Dialog**: Preview changes before applying

### 4. Log Viewer & Graphing
- **Timeline Scrubbing**: Drag slider to navigate through log data
- **Multi-Parameter Display**: Select which sensors to visualize
- **Map Correlation**: Shows active map cell at current timeline position
- **Playback Controls**: Play, pause, skip to start/end

### 5. Sensors & Calibration
- **Live Sensor Values**: Raw voltage and scaled readings
- **Curve Editor**: Drag control points to adjust multi-point scaling
- **Preset Library**: Load common sensor calibrations
- **Numeric Inputs**: Precise coordinate entry for each point

### 6. Engine Parameters & Safeties
- **Nested Tabs**: Engine Constants, Features & Safeties, Outputs
- **Rev Limiter**: Adjustable RPM limit with fuel/ignition cut options
- **Advanced Features**: Launch Control, Boost Control, VTEC, etc.
- **Tooltips**: Contextual help for every parameter

## 🎮 Interactive Demo

### Live Tuning Toggle (Top Bar)
- **OFF** (Default): Static sample values, ECU read-only
- **ON**: Animated gauges, live data updates, enables ECU modifications

### Global Actions (Top Bar)
- **Save**: Saves calibration to file
- **Upload**: Sends changes to ECU (requires Live Tuning ON)
- **Download**: Reads current calibration from ECU
- **Help**: Opens documentation

### Navigation (Left Sidebar)
- Dashboard
- Maps & Tables
- Graphing & Logging
- AutoTune
- Sensors
- Settings (Engine Parameters)

**Tip**: Click the collapse button at the bottom to switch to icon-only view

## 🔧 Technical Implementation

### Built With
- **React** + TypeScript
- **Tailwind CSS v4** for styling
- **Motion** (Framer Motion) for animations
- **Recharts** for data visualization
- **shadcn/ui** components
- **re-resizable** for draggable widgets

### Design Tokens
All colors, spacing, and typography are defined in `/styles/globals.css` as CSS variables for easy Qt/QML conversion.

### Component Architecture
- **Modular Design**: Each screen is a separate component
- **Shared State**: Live tuning toggle affects all screens
- **8px Grid System**: All layouts snap to baseline grid
- **Responsive**: Scales from 1366×768 to 1920×1080

## 📐 Screen Resolution

- **Primary Target**: 1366 × 768
- **Scaled Version**: 1920 × 1080
- **Layout Grid**: 12-column, 16px gutter, 24px margins

## 🚀 Key Interactions

### Dashboard
1. Toggle "Designer Mode" switch
2. Drag widgets to reposition (snaps to 8px grid)
3. Resize widgets using corner handles
4. Right-click widgets for configuration

### Maps Editor
1. Click cells to select (Shift+click for range)
2. Double-click to edit cell value inline
3. Right-click for context menu (+1%, -1%, Lock, etc.)
4. Select cells and use Smooth/Interpolate tools
5. Toggle 2D/3D visualization
6. Drag to rotate 3D surface (prototype mock)
7. "Apply to ECU" requires Live Tuning ON

### AutoTune
1. Enable Live Tuning first
2. Click "Start" to begin AutoTune engine
3. Review each suggestion individually
4. Click "Approve" or "Reject" for each change
5. "Apply Approved" shows confirmation dialog
6. Success toast appears after applying

### Log Viewer
1. Select log file from left panel
2. Use playback controls to navigate
3. Drag timeline slider to scrub
4. Mouse wheel over chart for fine control
5. Check/uncheck parameters to show/hide
6. "Active Map Cell" badge shows correlation

### Sensors
1. Click "Calibrate" on any sensor
2. Drag control points on curve editor
3. Or use numeric inputs for precision
4. Select preset from dropdown
5. Save calibration

## 🎯 Design Decisions

### Glassmorphism
Secondary surfaces use subtle glassmorphism (backdrop blur + semi-transparent background) for visual depth while maintaining readability.

### Heatmap Colors
- **Cool to Hot gradient** matches tuner expectations (safe → caution → critical)
- **High contrast** ensures readability of numeric values on colored backgrounds

### Monospace for Data
All numeric readouts use Roboto Mono for:
- **Alignment**: Digits stack vertically in tables
- **Professionalism**: Industry standard for technical applications
- **Readability**: Clear distinction between similar characters (0 vs O, 1 vs I)

### Live Tuning Safety
Master toggle in top bar acts as safety lock:
- **Prevents accidental ECU writes** when disabled
- **Visual indicators** (pulsing dot, status bar badge) when active
- **Disabled state** on destructive actions when OFF

## 📝 Developer Handoff

See `IMPLEMENTATION_NOTES.md` for detailed Qt/QML conversion guidance, including:
- Component mapping
- Performance considerations
- State management
- 3D visualization requirements
- Testing checklist

## 🔮 Future Enhancements (Not in Prototype)

- **Map Comparison**: Overlay two maps showing cell-by-cell delta
- **Tune Quick View Tray**: Floating action tray for common operations
- **Onboarding Flow**: Animated overlays highlighting key features
- **Real ECU Communication**: Serial/CAN interface in Qt C++ backend
- **File I/O**: Save/load calibration files (.cal format)
- **Backup/Restore**: Automatic backup before ECU writes

## 📄 License

This is a design prototype for demonstration and development purposes.

---

**Note**: This is an interactive prototype. All data is simulated. Production implementation requires Qt/QML conversion with real ECU communication backend.
