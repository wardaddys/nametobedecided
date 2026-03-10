# Speeduino ECU Sensor Validation System

## Overview

This application includes a comprehensive sensor validation system based on official Speeduino documentation and industry-standard automotive sensor ranges. The validation system provides real-time monitoring, color-coded alerts, and anomaly detection to prevent incorrect sensor values from causing tuning errors or engine damage.

## Features

### ✅ Real-Time Validation
- All sensor values are validated against Speeduino-derived optimal ranges
- Instant feedback with color-coded indicators (Green/Yellow/Red)
- Context-aware validation (e.g., AFR warnings at WOT)

### 🎯 Sensor Coverage

The system validates the following sensors:

1. **RPM (Engine Speed)**
   - Optimal: 800-8000 RPM
   - Caution: 500-8200 RPM  
   - Critical: 0-12000 RPM
   - Alerts on rev limit exceed (>8200 RPM)

2. **MAP (Manifold Absolute Pressure)**
   - Optimal: 30-100 kPa (NA), up to 200 kPa (boosted)
   - Detects vacuum leaks (<25 kPa)
   - Overboost protection (>150 kPa NA)

3. **TPS (Throttle Position)**
   - Optimal: 0-100%
   - Validates TPS/MAP correlation
   - Detects stuck throttle conditions

4. **IAT (Intake Air Temperature)**
   - Optimal: -20°C to 80°C
   - Heat soak warning (>90°C)
   - Sensor fault detection

5. **CLT (Coolant Temperature)**
   - Optimal: 80-95°C (operating)
   - Overheat warnings (>100°C)
   - Cold running alerts (<70°C)

6. **AFR (Air-Fuel Ratio)**
   - Stoich: 14.7 (Lambda 1.0)
   - WOT: 11-13 (0.75-0.9 Lambda)
   - Critical lean protection at WOT (>15.5 AFR)
   - Displays both AFR and Lambda values

7. **Battery Voltage**
   - Optimal: 12-14V (running)
   - Low voltage warning (<11V)
   - Overcharge protection (>15V)

8. **Injector Duty Cycle**
   - Optimal: 10-80%
   - Max safe: <85%
   - Leanout risk warning (>95%)

9. **Ignition Timing**
   - Optimal: 10-40° BTDC
   - Knock risk (>50°)
   - Timing slip detection

10. **Barometric Pressure**
    - Altitude-adjusted (70-110 kPa)
    - Weather/altitude tracking

11. **Boost Pressure**
    - Differential (MAP - Baro)
    - Wastegate monitoring
    - Overboost cut

12. **Knock Sensor** (if equipped)
    - Baseline: 0-2V
    - Detonation detection (>3V sustained)

### 🔔 Alert Levels

**Normal (Green #4CAF50)**
- Sensor within optimal range
- No action required
- Green indicators

**Caution (Yellow/Orange #FF9800)**
- Sensor outside optimal but within safe limits
- Monitor closely
- Yellow indicators

**Critical (Red #F44336)**
- Sensor in dangerous range
- Immediate action required
- Flashing red indicators + modal alerts

**Fault (Gray)**
- Sensor offline or reading impossible values
- Check wiring and calibration

### 📊 Visual Indicators

#### Dashboard Widgets
- **RPM Gauge**: Color zones (green/yellow/red), digital display with glow effect, critical alert modal
- **AFR Widget**: Dual-scale bar (AFR + Lambda), stoich zone highlighting, validation tooltips
- **Sensor Monitor**: Grid view of all sensors with progress bars, expandable details, formula displays

#### Alert Components
- **ValidationAlert**: Single sensor warning with recommended actions
- **ValidationAlertStack**: Multiple alerts sorted by severity
- **Inline tooltips**: Hover to see calibration formulas and optimal ranges

### 🧮 Validation Formulas

Each sensor includes its calibration formula for reference:

```
MAP: kPa = (V_in / 5) * 100 (GM open-loop)
TPS: % = ((V - 0.5) / 4) * 100
AFR: AFR = 10 + (V_in * 2) (wideband linear)
IAT/CLT: Steinhart-Hart equation for NTC thermistors
Boost: Boost = MAP - Baro (differential)
```

### 🎭 Anomaly Detection

The `AnomalyDetector` class tracks sensor history and flags unusual patterns:

- **RPM**: >1000 RPM jump in <1s (sensor noise)
- **MAP**: >10 kPa change in 1s at idle (throttle issue)
- **TPS**: >80% with MAP <50 kPa (TPS/MAP mismatch)
- **IAT**: >50°C jump in <10s (disconnected sensor)
- **CLT**: No change >20min (stuck sensor)
- **AFR**: Lambda >1.2 at WOT (lean misfire)
- **Boost**: >5 psi/sec ramp (wastegate stuck)

## Usage

### In Components

```tsx
import { validateSensor, sensorRanges } from './validation/SensorValidation';

// Validate a sensor value
const validation = validateSensor('rpm', 8500, { rpm: 8500 });

// Check validation level
if (validation.level === 'critical') {
  // Show alert
  console.log(validation.message);
}

// Get color for display
const color = validation.color; // '#F44336' for critical
```

### With Anomaly Detection

```tsx
import { AnomalyDetector } from './validation/SensorValidation';

const detector = new AnomalyDetector();

// Detect anomalies
const anomaly = detector.detect('rpm', currentRPM);
if (anomaly.detected) {
  console.log(anomaly.message);
}
```

### Context Integration

The `SensorContext` automatically validates sensors when values update:

```tsx
const { updateSensor } = useSensors();

// This automatically validates and stores validation results
updateSensor('rpm-sensor', { value: 8500 });
```

### Dashboard Widgets

Updated widgets automatically use validation:

```tsx
<RPMGauge liveTuning={true} />
<AFRWidget liveTuning={true} tps={currentTPS} />
<SensorMonitor liveTuning={true} />
```

## Implementation Details

### File Structure

```
/components/validation/
├── SensorValidation.ts       # Core validation logic and ranges
└── ValidationAlert.tsx        # Alert UI components

/components/dashboard/widgets/
├── RPMGauge.tsx              # Updated with validation
├── AFRWidget.tsx             # Updated with validation  
└── SensorMonitor.tsx         # New comprehensive monitor

/components/context/
└── SensorContext.tsx         # Auto-validation on updates
```

### Performance

- Validation is lightweight (<1ms per sensor)
- Anomaly detection uses rolling 30s window
- Color calculations are memoized
- Alerts use AnimatePresence for smooth transitions

### Accessibility

- Color-blind friendly: Uses icons + colors
- Keyboard accessible alert dismissal
- ARIA labels on all indicators
- Screen reader compatible messages

## Speeduino Compliance

All ranges and formulas are sourced from:

✅ [Speeduino Sensor Calibration Wiki](https://wiki.speeduino.com/en/configuration/Sensor_Calibration)  
✅ [Speeduino Manual PDF](https://speeduino.com/Speeduino_manual.pdf)  
✅ [Speeduino Tuning Reference](https://wiki.speeduino.com/en/reference/Tuning_reference)  
✅ [MegaSquirt Tuning Guide](https://www.megamanual.com/v22manual/mtune.htm) (compatible ranges)

## Future Enhancements

### Planned Features

1. **Data Logging Integration**
   - Export validation events to logs
   - Replay with validation highlights
   - Statistical analysis of sensor behavior

2. **Custom Ranges**
   - User-definable thresholds per vehicle
   - Engine-type presets (4-cyl NA, V8 turbo, etc.)
   - Save/load validation profiles

3. **Advanced Anomaly Detection**
   - Machine learning patterns
   - Multi-sensor correlation
   - Predictive warnings

4. **Sound Alerts**
   - Audio warnings for critical conditions
   - Configurable alert tones
   - Text-to-speech announcements

5. **Mobile Responsive Alerts**
   - Bottom-sheet alerts on mobile
   - Haptic feedback
   - Simplified mobile views

## Testing

To test validation:

1. Enable Live Tuning mode
2. Watch sensors automatically fluctuate
3. Observe color changes as values approach limits
4. See critical alerts when RPM exceeds 8200
5. Check AFR warnings when value goes lean
6. Click sensor rows in SensorMonitor for details

## Safety Notice

⚠️ **IMPORTANT**: This validation system is designed for prototyping and development. When using with a real ECU:

- Always verify sensor calibrations match your hardware
- Test all validation thresholds on a dyno or controlled environment
- Never rely solely on software validation for safety-critical decisions
- Follow Speeduino official documentation for production tuning
- This tool is not meant for collecting PII or securing sensitive data

## Support

For issues or questions about the validation system:

1. Check sensor ranges in `SensorValidation.ts`
2. Review Speeduino official documentation
3. Verify sensor calibration in I/O Configuration tab
4. Check browser console for validation messages

---

**Version**: 2.0  
**Last Updated**: October 2025  
**Based on**: Speeduino Protocol v2025.10
