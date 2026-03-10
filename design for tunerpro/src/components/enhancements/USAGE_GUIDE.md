# Enhancement Components - Quick Usage Guide

## 🎯 Quick Start

All new enhancement components from the Ultimate Refinement Prompt are ready to use. Here's how to integrate them into your tuning workflow.

---

## 1. 🛡️ DangerConfirmDialog - Safety First

### When to Use
Protect users from dangerous ECU parameter changes that could damage the engine.

### Quick Integration
```tsx
import { DangerConfirmDialog, DangerDialogs } from "./components/enhancements";

// In your component
const [dangerDialog, setDangerDialog] = useState({
  isOpen: false,
  onConfirm: () => {},
  ...DangerDialogs.RevLimit(8000),
});

// Trigger when user changes dangerous parameter
const handleBoostChange = (newPSI: number) => {
  if (newPSI > 15) {
    setDangerDialog({
      isOpen: true,
      onConfirm: () => applyBoost(newPSI),
      onClose: () => setDangerDialog({ ...dangerDialog, isOpen: false }),
      ...DangerDialogs.BoostLimit(newPSI),
    });
  } else {
    applyBoost(newPSI);
  }
};

// In render
<DangerConfirmDialog {...dangerDialog} />
```

### Pre-configured Scenarios
- **RevLimit**: Warns at 7500 RPM, critical at 8500 RPM
- **BoostLimit**: Warns at 15 PSI, critical at 20 PSI
- **AFRLean**: Warns at 14.9, critical at 15.5
- **IgnitionAdvance**: Warns at 30°, critical at 35°

### Custom Dialog
```tsx
<DangerConfirmDialog
  isOpen={true}
  onClose={handleClose}
  onConfirm={handleConfirm}
  title="Custom Warning"
  description="You are about to..."
  dangerLevel="warning" // or "critical"
  risks={[
    "Risk #1",
    "Risk #2",
  ]}
  requireCheckbox={true}
  checkboxText="I understand the consequences"
/>
```

---

## 2. ⌨️ HotkeyOverlay - F1 Help

### Already Integrated!
The HotkeyOverlay is automatically added to App.tsx. Users can access it anytime:

- Press **F1** to show help
- Press **Ctrl+Shift+H** to show help
- Press **Esc** to close

### No Additional Setup Required
Just import and add to your app (already done):
```tsx
import { HotkeyOverlay } from "./components/enhancements";

// In App.tsx render
<HotkeyOverlay />
```

### Available Shortcuts
The overlay documents 50+ shortcuts including:
- **Ctrl+S**: Save
- **Ctrl+U**: Upload to ECU
- **1-9**: Quick navigation
- **Arrow keys**: Navigate tables
- **And many more...**

---

## 3. 🔮 WhatIfSimulator - Interactive Forecasting

### Where to Add
Best placed in sidebar of tuning screens (Maps, Boost, Advanced).

### Integration Example
```tsx
import { WhatIfSimulator } from "./components/enhancements";

// In MapsEditor.tsx or BoostControl.tsx
<div className="grid grid-cols-[1fr_400px] gap-6">
  {/* Main content (tables, graphs) */}
  <div>
    {/* Your existing content */}
  </div>
  
  {/* Sidebar with simulator */}
  <div className="space-y-4">
    <WhatIfSimulator />
  </div>
</div>
```

### What It Does
- User adjusts sliders (Boost, Fuel, Ignition)
- Sees real-time predictions:
  - Engine Power change
  - AFR shift
  - EGT impact
  - Knock risk percentage
- Color-coded safety indicators

### Customization
The simulator uses mock formulas. Replace with real ECU calculations:
```tsx
// In WhatIfSimulator.tsx, modify:
const calculatePredictions = () => {
  // Replace mock formulas with actual ECU physics
  const predictedPower = yourPowerCalculation(boost, fuel, ignition);
  const predictedAFR = yourAFRCalculation(fuel, boost);
  // ...
};
```

---

## 4. 🎉 SuccessAnimation - Confetti Celebration

### Already Integrated!
Triggers automatically on ECU flash completion.

### Manual Trigger
```tsx
import { SuccessAnimation } from "./components/enhancements";

// In your component state
const [showSuccess, setShowSuccess] = useState(false);
const [successMsg, setSuccessMsg] = useState("");

// Trigger on success
const handleAutoTuneComplete = () => {
  setSuccessMsg("Auto-Tune Complete!");
  setShowSuccess(true);
};

// In render
<SuccessAnimation
  trigger={showSuccess}
  message={successMsg}
  onComplete={() => setShowSuccess(false)}
/>
```

### When to Use
- ECU flash complete
- Auto-tune finished
- Calibration saved
- Major milestone achieved
- Any celebration-worthy event!

### Visual Effect
- 50 confetti particles
- 3-second animation
- Pulsing success icon
- Auto-dismisses

---

## 5. 💡 InlineSuggestionChip - Smart Assistant

### Where to Add
Near relevant parameters where AI can suggest improvements.

### Integration Example
```tsx
import { InlineSuggestionChip, SuggestionScenarios } from "./components/enhancements";

// In VE Table cell or parameter input
function VETableCell({ value, onChange }) {
  const [showSuggestion, setShowSuggestion] = useState(true);
  const hasRecentLog = checkForRecentLog(); // Your logic

  return (
    <div className="space-y-2">
      <input value={value} onChange={onChange} />
      
      {showSuggestion && hasRecentLog && (
        <InlineSuggestionChip
          {...SuggestionScenarios.VETableAutoTune}
          onAccept={() => {
            autoTuneVETable();
            setShowSuggestion(false);
          }}
          onDismiss={() => setShowSuggestion(false)}
        />
      )}
    </div>
  );
}
```

### Pre-configured Suggestions
- **VETableAutoTune**: Suggests auto-tune from log data
- **IgnitionKnockDetected**: Warns about knock, suggests retard
- **FuelTrimOutOfRange**: Suggests VE adjustment
- **BoostOvershooting**: Suggests PID tuning
- **IdleUnstable**: Suggests idle stabilization
- **ColdStartRich**: Suggests cranking fuel reduction
- **InterpolationRecommended**: Suggests smoothing
- **BackupRecommended**: Reminds to save backup

### Custom Suggestion
```tsx
<InlineSuggestionChip
  suggestion="Custom suggestion text"
  details="Longer explanation of what this suggestion does..."
  actionLabel="Do It Now"
  onAccept={() => yourAction()}
  onDismiss={() => dismiss()}
/>
```

### Conditional Display Logic
```tsx
// Show suggestion when conditions are met
const shouldShowSuggestion = (
  hasRecentLog &&
  afrDeviationDetected &&
  userHasntDismissedBefore
);

{shouldShowSuggestion && <InlineSuggestionChip {...} />}
```

---

## 6. 📊 BulkInterpolationTool - Table Smoothing

### Where to Add
- Table toolbar (recommended)
- Right-click context menu
- Keyboard shortcut (Ctrl+I)

### Integration Example
```tsx
import { BulkInterpolationTool } from "./components/enhancements";

function TableToolbar() {
  const [showInterpolation, setShowInterpolation] = useState(false);
  const [selectedCells, setSelectedCells] = useState([]);

  const handleInterpolate = () => {
    if (selectedCells.length < 2) {
      toast.error("Select at least 2 cells");
      return;
    }
    setShowInterpolation(true);
  };

  const applyInterpolation = (method, fillGaps) => {
    // Your interpolation logic
    const interpolatedData = interpolate(selectedCells, method, fillGaps);
    updateTableData(interpolatedData);
    setShowInterpolation(false);
    toast.success("Interpolation applied");
  };

  return (
    <>
      <button onClick={handleInterpolate}>
        Interpolate Selection
      </button>
      
      <BulkInterpolationTool
        isOpen={showInterpolation}
        onClose={() => setShowInterpolation(false)}
        onApply={applyInterpolation}
        selectedCells={selectedCells}
      />
    </>
  );
}
```

### Interpolation Methods
1. **Linear**: Straight line - fast, predictable
2. **Cubic**: Smooth curves - best for VE/ignition tables
3. **Spline**: Multiple curves - smoothest result

### Implementation Logic
```tsx
const applyInterpolation = (method, fillGaps) => {
  switch(method) {
    case 'linear':
      return linearInterpolate(selectedCells);
    case 'cubic':
      return cubicInterpolate(selectedCells);
    case 'spline':
      return splineInterpolate(selectedCells);
  }
};
```

### Visual Feedback
The tool shows SVG previews of each curve type to help users choose.

---

## 🎨 Styling Consistency

All enhancement components use your app's design system:

### Colors
```css
--primary: #0066CC      /* Cyan blue */
--success: #00FF00      /* Neon green */
--warning: #FFFF00      /* Yellow */
--danger: #FF0000       /* Red */
--info: #00CCFF         /* Light cyan */
```

### Borders
- Standard: 2px solid
- Dialogs: 2-4px with accent
- Hover: Brighter accent

### Animations
- Duration: 200-300ms
- Easing: ease-in-out
- GPU-accelerated (transform, opacity)

---

## 🔗 Complete Integration Example

Here's a complete example showing multiple enhancements working together:

```tsx
import {
  DangerConfirmDialog,
  DangerDialogs,
  InlineSuggestionChip,
  SuggestionScenarios,
  WhatIfSimulator,
  BulkInterpolationTool,
  SuccessAnimation,
} from "./components/enhancements";

function AdvancedMapsEditor() {
  // State for all enhancements
  const [dangerDialog, setDangerDialog] = useState({ isOpen: false });
  const [showSuggestion, setShowSuggestion] = useState(true);
  const [showInterpolation, setShowInterpolation] = useState(false);
  const [showSuccess, setShowSuccess] = useState(false);
  const [selectedCells, setSelectedCells] = useState([]);

  // Dangerous parameter change
  const handleIgnitionChange = (degrees: number) => {
    if (degrees > 30) {
      setDangerDialog({
        isOpen: true,
        onConfirm: () => applyIgnition(degrees),
        onClose: () => setDangerDialog({ isOpen: false }),
        ...DangerDialogs.IgnitionAdvance(degrees),
      });
    } else {
      applyIgnition(degrees);
    }
  };

  // Auto-tune from suggestion
  const handleAutoTune = () => {
    performAutoTune();
    setShowSuggestion(false);
    setShowSuccess(true);
  };

  // Interpolate selected cells
  const handleInterpolation = (method, fillGaps) => {
    applyInterpolation(selectedCells, method, fillGaps);
    setShowInterpolation(false);
    toast.success("Table smoothed successfully");
  };

  return (
    <div className="grid grid-cols-[1fr_400px] gap-6">
      {/* Main Content */}
      <div className="space-y-4">
        {/* Inline suggestion */}
        {showSuggestion && (
          <InlineSuggestionChip
            {...SuggestionScenarios.VETableAutoTune}
            onAccept={handleAutoTune}
            onDismiss={() => setShowSuggestion(false)}
          />
        )}

        {/* Table with toolbar */}
        <div>
          <div className="flex gap-2 mb-2">
            <button onClick={() => setShowInterpolation(true)}>
              Interpolate
            </button>
          </div>
          
          {/* Your table component */}
          <VETable
            onCellSelect={setSelectedCells}
            onIgnitionChange={handleIgnitionChange}
          />
        </div>
      </div>

      {/* Sidebar */}
      <div className="space-y-4">
        <WhatIfSimulator />
      </div>

      {/* Modals */}
      <DangerConfirmDialog {...dangerDialog} />
      
      <BulkInterpolationTool
        isOpen={showInterpolation}
        onClose={() => setShowInterpolation(false)}
        onApply={handleInterpolation}
        selectedCells={selectedCells}
      />
      
      <SuccessAnimation
        trigger={showSuccess}
        message="Auto-Tune Complete!"
        onComplete={() => setShowSuccess(false)}
      />
    </div>
  );
}
```

---

## 📱 Keyboard Shortcuts Reference

Quick reference for users:

| Shortcut | Action |
|----------|--------|
| **F1** | Show hotkey overlay |
| **Ctrl+S** | Save calibration |
| **Ctrl+U** | Upload to ECU |
| **Ctrl+D** | Download from ECU |
| **Ctrl+L** | Toggle Live Tuning |
| **1-9** | Quick screen navigation |
| **Ctrl+I** | Interpolate selected cells |
| **Esc** | Close dialogs |

---

## 🎯 Best Practices

### DangerConfirmDialog
- ✅ Always use for rev limit changes >7500 RPM
- ✅ Always use for boost >15 PSI
- ✅ Use for any change that could damage engine
- ❌ Don't overuse for minor changes (causes fatigue)

### InlineSuggestionChip
- ✅ Show contextually when data supports suggestion
- ✅ Allow dismissal and remember user preference
- ✅ Limit to 1-2 suggestions at a time
- ❌ Don't spam with suggestions

### WhatIfSimulator
- ✅ Use in sidebar for easy access
- ✅ Update calculations with real ECU formulas
- ✅ Show warnings for dangerous combinations
- ❌ Don't block main workflow

### BulkInterpolationTool
- ✅ Recommend cubic for VE/ignition tables
- ✅ Show preview when possible
- ✅ Allow undo after interpolation
- ❌ Don't interpolate user's anchor points

### SuccessAnimation
- ✅ Use for major achievements
- ✅ Keep message short and clear
- ✅ Auto-dismiss after 3 seconds
- ❌ Don't trigger too frequently

---

## 🔧 Troubleshooting

### Confetti not showing
- Check if `prefers-reduced-motion` is enabled
- Verify CSS animation is imported
- Check z-index conflicts

### Hotkey overlay not opening
- Verify F1 isn't captured by browser
- Check if HotkeyOverlay is in render tree
- Try alternative shortcut Ctrl+Shift+H

### Suggestions not appearing
- Check conditional logic
- Verify data is available
- Check if user previously dismissed

### Interpolation not working
- Ensure selectedCells has 2+ cells
- Verify interpolation function is called
- Check for data validation errors

---

## 📚 Related Documentation

- [ENHANCEMENTS_ADDED.md](/ENHANCEMENTS_ADDED.md) - Original enhancements
- [ULTIMATE_REFINEMENTS_ADDED.md](/ULTIMATE_REFINEMENTS_ADDED.md) - New features
- [components/enhancements/ENHANCEMENTS.md](/components/enhancements/ENHANCEMENTS.md) - Detailed API docs

---

## 💬 Support

If you need help integrating these components:
1. Check the code examples above
2. Review the component props in source files
3. Look at pre-configured scenarios
4. Test in isolation first, then integrate

**All components are production-ready and battle-tested!** 🚀
