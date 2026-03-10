// Speeduino ECU Sensor Validation System
// Based on official Speeduino documentation and Megasquirt tuning references

export type ValidationLevel = 'normal' | 'caution' | 'critical' | 'fault';

export interface ValidationResult {
  level: ValidationLevel;
  message: string;
  color: string;
  shouldAlert: boolean;
}

export interface SensorRange {
  optimal: { min: number; max: number; description: string };
  caution: { min: number; max: number };
  critical: { min: number; max: number };
  formula?: string;
  calibration: string;
  unit: string;
  anomalyThreshold?: {
    deltaValue: number;
    deltaTime: number; // in seconds
    description: string;
  };
}

// Sensor validation ranges based on Speeduino specs
export const sensorRanges: Record<string, SensorRange> = {
  rpm: {
    optimal: { min: 800, max: 8000, description: 'Idle 800-1200, Cruise 2000-4000, Max Safe 6500-8000' },
    caution: { min: 500, max: 8200 },
    critical: { min: 0, max: 12000 },
    calibration: 'Trigger-based (e.g., 36-1 wheel, 0-10000 RPM scale)',
    unit: 'RPM',
    anomalyThreshold: {
      deltaValue: 1000,
      deltaTime: 1,
      description: 'Sudden >1000 RPM jump in <1s indicates sensor noise'
    }
  },
  map: {
    optimal: { min: 30, max: 100, description: 'Idle 30-100 kPa, Cruise 50-90 kPa, Boost 100-200 kPa' },
    caution: { min: 10, max: 150 },
    critical: { min: 0, max: 250 },
    formula: 'kPa = (V_in / 5) * 100 (GM open-loop)',
    calibration: '0-5V linear, piecewise for boosted',
    unit: 'kPa',
    anomalyThreshold: {
      deltaValue: 10,
      deltaTime: 1,
      description: '>10 kPa change in 1s at idle indicates throttle issue'
    }
  },
  tps: {
    optimal: { min: 0, max: 100, description: 'Idle 0-5%, Cruise 10-60%, WOT 95-100%' },
    caution: { min: 0, max: 98 },
    critical: { min: 0, max: 100 },
    formula: '% = ((V - 0.5) / 4) * 100',
    calibration: '0.5-4.5V linear',
    unit: '%',
    anomalyThreshold: {
      deltaValue: 80,
      deltaTime: 0.5,
      description: 'TPS >80% with MAP <50 kPa indicates mismatch'
    }
  },
  iat: {
    optimal: { min: -20, max: 80, description: 'Cruise -20-80°C, Cold Start <20°C, Hot >60°C' },
    caution: { min: -40, max: 90 },
    critical: { min: -50, max: 150 },
    formula: '1/T = A + B*ln(R) + C*(ln(R))^3 (Steinhart-Hart)',
    calibration: 'NTC thermistor with Steinhart-Hart equation',
    unit: '°C',
    anomalyThreshold: {
      deltaValue: 50,
      deltaTime: 10,
      description: '>50°C jump in <10s indicates sensor disconnect'
    }
  },
  clt: {
    optimal: { min: 80, max: 95, description: 'Operating 80-95°C, Warmup 40-80°C' },
    caution: { min: 40, max: 100 },
    critical: { min: 0, max: 120 },
    formula: 'Resistance table (e.g., GM: 177Ω @100°C)',
    calibration: 'NTC, voltage divider 0-5V',
    unit: '°C',
    anomalyThreshold: {
      deltaValue: 0.5,
      deltaTime: 1200,
      description: 'No change >20min indicates stuck sensor'
    }
  },
  afr: {
    optimal: { min: 14.2, max: 14.9, description: 'Stoich 14.7 (Lambda 1.0), WOT 11-13, Idle 14.2-14.9' },
    caution: { min: 10.5, max: 15.5 },
    critical: { min: 9, max: 18 },
    formula: 'AFR = 10 + (V_in * 2) (linear approx wideband)',
    calibration: 'Wideband 0-5V, narrowband 0-1V switch @0.45V',
    unit: 'AFR',
    anomalyThreshold: {
      deltaValue: 1.5,
      deltaTime: 2,
      description: 'Lambda >1.2 at WOT indicates lean misfire'
    }
  },
  batteryVoltage: {
    optimal: { min: 12, max: 14, description: '12-14V running, 11.5-12.5V crank' },
    caution: { min: 8, max: 15 },
    critical: { min: 0, max: 18 },
    calibration: 'Direct analog 0-20V scaled to 0-5V input',
    unit: 'V',
    anomalyThreshold: {
      deltaValue: 2,
      deltaTime: 1,
      description: '<10V during crank indicates starter draw'
    }
  },
  injectorDutyCycle: {
    optimal: { min: 10, max: 80, description: 'Idle 10-20%, Cruise 20-50%, Max <80%' },
    caution: { min: 5, max: 85 },
    critical: { min: 0, max: 95 },
    calibration: 'PWM % from ECU output',
    unit: '%',
    anomalyThreshold: {
      deltaValue: 80,
      deltaTime: 5,
      description: 'IDC >80% at <4000 RPM indicates oversized injectors'
    }
  },
  ignitionTiming: {
    optimal: { min: 10, max: 40, description: 'Idle 10-20° BTDC, Cruise 30-40°, WOT 25-35°' },
    caution: { min: 0, max: 50 },
    critical: { min: -10, max: 60 },
    calibration: 'Crank trigger reference, advance table lookup',
    unit: '° BTDC',
    anomalyThreshold: {
      deltaValue: 10,
      deltaTime: 1,
      description: '>10° jump indicates timing slip'
    }
  },
  baro: {
    optimal: { min: 70, max: 110, description: '70-110 kPa (altitude-adjusted)' },
    caution: { min: 40, max: 120 },
    critical: { min: 20, max: 130 },
    formula: 'kPa = (V_in / 5) * 101.3',
    calibration: '0-5V linear',
    unit: 'kPa',
    anomalyThreshold: {
      deltaValue: 5,
      deltaTime: 60,
      description: '>5 kPa/min change indicates weather/vehicle motion'
    }
  },
  boost: {
    optimal: { min: 0, max: 20, description: 'NA 0 psi, Turbo 5-20 psi' },
    caution: { min: -5, max: 25 },
    critical: { min: -10, max: 30 },
    formula: 'Boost = MAP - Baro (differential)',
    calibration: 'MAP - Baro differential',
    unit: 'psi',
    anomalyThreshold: {
      deltaValue: 5,
      deltaTime: 1,
      description: '>5 psi/sec ramp indicates wastegate stuck'
    }
  },
  knock: {
    optimal: { min: 0, max: 2, description: '0-5V baseline, spikes <2V' },
    caution: { min: 0, max: 3 },
    critical: { min: 0, max: 5 },
    calibration: 'Frequency-tuned (e.g., 6-8kHz)',
    unit: 'V',
    anomalyThreshold: {
      deltaValue: 1,
      deltaTime: 2,
      description: '>1V >2s indicates detonation'
    }
  },
  oilPressure: {
    optimal: { min: 20, max: 80, description: 'Idle 20-40 psi, Running 40-80 psi' },
    caution: { min: 10, max: 100 },
    critical: { min: 0, max: 120 },
    calibration: '0-5V linear, sensor-specific',
    unit: 'psi'
  },
  oilTemp: {
    optimal: { min: 80, max: 110, description: 'Operating 80-110°C' },
    caution: { min: 60, max: 130 },
    critical: { min: 0, max: 150 },
    calibration: 'NTC thermistor',
    unit: '°C'
  },
  fuelPressure: {
    optimal: { min: 40, max: 60, description: 'Base 40-45 psi, 1:1 boost referenced' },
    caution: { min: 30, max: 80 },
    critical: { min: 0, max: 100 },
    calibration: '0-5V linear',
    unit: 'psi'
  }
};

// Validation function
export function validateSensor(
  sensorType: string,
  value: number,
  context?: { rpm?: number; map?: number; tps?: number }
): ValidationResult {
  const range = sensorRanges[sensorType];
  
  if (!range) {
    return {
      level: 'normal',
      message: 'Unknown sensor',
      color: '#4CAF50',
      shouldAlert: false
    };
  }

  // Check for critical range violations
  if (value < range.critical.min || value > range.critical.max) {
    return {
      level: 'critical',
      message: `CRITICAL: ${sensorType.toUpperCase()} out of range (${value}${range.unit})`,
      color: '#F44336',
      shouldAlert: true
    };
  }

  // Check for caution range violations
  if (value < range.caution.min || value > range.caution.max) {
    return {
      level: 'caution',
      message: `WARNING: ${sensorType.toUpperCase()} approaching limits (${value}${range.unit})`,
      color: '#FF9800',
      shouldAlert: true
    };
  }

  // Context-specific validations
  if (context) {
    // TPS vs MAP mismatch
    if (sensorType === 'tps' && context.tps && context.map) {
      if (context.tps > 80 && context.map < 50) {
        return {
          level: 'caution',
          message: 'TPS/MAP mismatch: High TPS with low MAP',
          color: '#FF9800',
          shouldAlert: true
        };
      }
    }

    // AFR at WOT validation
    if (sensorType === 'afr' && context.tps && context.tps > 90) {
      if (value > 15) {
        return {
          level: 'critical',
          message: 'DANGER: Lean condition at WOT - risk of engine damage',
          color: '#F44336',
          shouldAlert: true
        };
      }
    }

    // Rev limiter
    if (sensorType === 'rpm' && context.rpm) {
      if (context.rpm > 8200) {
        return {
          level: 'critical',
          message: 'REV LIMIT EXCEEDED - Fuel/spark cut active',
          color: '#F44336',
          shouldAlert: true
        };
      }
      if (context.rpm > 7500) {
        return {
          level: 'caution',
          message: 'Approaching rev limit',
          color: '#FF9800',
          shouldAlert: true
        };
      }
    }
  }

  // Check if within optimal range
  if (value >= range.optimal.min && value <= range.optimal.max) {
    return {
      level: 'normal',
      message: `${sensorType.toUpperCase()} normal (${value}${range.unit})`,
      color: '#4CAF50',
      shouldAlert: false
    };
  }

  // Outside optimal but within caution range
  return {
    level: 'caution',
    message: `${sensorType.toUpperCase()} outside optimal range (${value}${range.unit})`,
    color: '#FF9800',
    shouldAlert: false
  };
}

// Anomaly detection
export class AnomalyDetector {
  private history: Map<string, Array<{ value: number; timestamp: number }>> = new Map();

  detect(sensorType: string, value: number): { detected: boolean; message: string } {
    const range = sensorRanges[sensorType];
    if (!range?.anomalyThreshold) {
      return { detected: false, message: '' };
    }

    const now = Date.now();
    const sensorHistory = this.history.get(sensorType) || [];
    
    // Add current reading
    sensorHistory.push({ value, timestamp: now });
    
    // Keep only recent history (last 30 seconds)
    const recentHistory = sensorHistory.filter(h => now - h.timestamp < 30000);
    this.history.set(sensorType, recentHistory);

    // Check for anomalies
    if (recentHistory.length >= 2) {
      const previous = recentHistory[recentHistory.length - 2];
      const timeDelta = (now - previous.timestamp) / 1000; // Convert to seconds
      const valueDelta = Math.abs(value - previous.value);

      if (
        valueDelta >= range.anomalyThreshold.deltaValue &&
        timeDelta <= range.anomalyThreshold.deltaTime
      ) {
        return {
          detected: true,
          message: `ANOMALY: ${range.anomalyThreshold.description}`
        };
      }
    }

    return { detected: false, message: '' };
  }

  reset(sensorType?: string) {
    if (sensorType) {
      this.history.delete(sensorType);
    } else {
      this.history.clear();
    }
  }
}

// Helper function to get color based on validation level
export function getValidationColor(level: ValidationLevel): string {
  switch (level) {
    case 'normal':
      return '#4CAF50';
    case 'caution':
      return '#FF9800';
    case 'critical':
      return '#F44336';
    case 'fault':
      return '#9E9E9E';
    default:
      return '#4CAF50';
  }
}

// Helper to format sensor value with validation
export function formatSensorValue(sensorType: string, value: number): string {
  const range = sensorRanges[sensorType];
  if (!range) return value.toFixed(1);

  // Clamp value to critical range
  const clamped = Math.max(
    range.critical.min,
    Math.min(range.critical.max, value)
  );

  // Format based on sensor type
  switch (range.unit) {
    case 'RPM':
      return Math.round(clamped).toString();
    case '%':
      return clamped.toFixed(1);
    case '°C':
    case 'kPa':
    case 'psi':
      return clamped.toFixed(1);
    case 'V':
      return clamped.toFixed(2);
    case '° BTDC':
      return clamped.toFixed(1);
    case 'AFR':
      return clamped.toFixed(2);
    default:
      return clamped.toFixed(1);
  }
}
