// Data Logging Types for Speeduino ECU Tuning
// Based on MegaLogViewer, VehiCAL, PC Link best practices

export interface LogChannel {
  id: string;
  name: string;
  unit: string;
  min: number;
  max: number;
  decimals: number;
  color: string;
  enabled: boolean;
  category: 'engine' | 'fuel' | 'ignition' | 'sensors' | 'calculated';
}

export interface LogDataPoint {
  timestamp: number;
  values: { [channelId: string]: number };
}

export interface LogFile {
  id: string;
  name: string;
  timestamp: number;
  duration: number; // seconds
  sampleRate: number; // Hz
  size: number; // bytes
  channels: string[]; // channel IDs
  data: LogDataPoint[];
  metadata: {
    vehicle: string;
    ecu: string;
    notes: string;
    avgAFR?: number;
    maxBoost?: number;
    maxRPM?: number;
  };
  thumbnail?: string; // Base64 preview image
}

export interface MathChannel {
  id: string;
  name: string;
  formula: string;
  unit: string;
  dependencies: string[]; // channel IDs used in formula
  error?: string;
}

export interface CaptureSettings {
  mode: 'live' | 'triggered';
  sampleRate: number; // 10, 20, 50 Hz
  triggerCondition?: {
    channel: string;
    operator: '>' | '<' | '=' | '>=';
    value: number;
  };
  maxDuration: number; // seconds
  autoStop: boolean;
}

export interface AnalysisResult {
  type: 'veAnalyze' | 'anomaly' | 'comparison';
  timestamp: number;
  data: any;
  suggestions: string[];
}

export interface VEAnalysisResult extends AnalysisResult {
  type: 'veAnalyze';
  data: {
    rpmBins: number[];
    mapBins: number[];
    veTable: number[][]; // 2D array of VE values
    coverage: number[][]; // Coverage percentage per cell
    avgError: number;
  };
}

export interface AnomalyResult extends AnalysisResult {
  type: 'anomaly';
  data: {
    channel: string;
    timestamp: number;
    value: number;
    severity: 'warning' | 'critical';
    message: string;
  }[];
}

// Standard logging channels based on Speeduino/MegaSquirt
export const STANDARD_CHANNELS: LogChannel[] = [
  // Engine
  { id: 'rpm', name: 'RPM', unit: 'RPM', min: 0, max: 12000, decimals: 0, color: '#00FF00', enabled: true, category: 'engine' },
  { id: 'load', name: 'Load', unit: '%', min: 0, max: 100, decimals: 1, color: '#00CCFF', enabled: true, category: 'engine' },
  { id: 'speed', name: 'Speed', unit: 'km/h', min: 0, max: 300, decimals: 0, color: '#FFAA00', enabled: false, category: 'engine' },
  
  // Fuel
  { id: 'afr', name: 'AFR', unit: 'AFR', min: 9, max: 18, decimals: 2, color: '#FF00FF', enabled: true, category: 'fuel' },
  { id: 'lambda', name: 'Lambda', unit: 'λ', min: 0.6, max: 1.3, decimals: 3, color: '#FF00AA', enabled: false, category: 'fuel' },
  { id: 'pulseWidth', name: 'Pulse Width', unit: 'ms', min: 0, max: 20, decimals: 2, color: '#00FFFF', enabled: true, category: 'fuel' },
  { id: 'idc', name: 'Injector Duty', unit: '%', min: 0, max: 100, decimals: 1, color: '#FFCC00', enabled: true, category: 'fuel' },
  { id: 'fuelPressure', name: 'Fuel Pressure', unit: 'psi', min: 0, max: 100, decimals: 1, color: '#FF6600', enabled: false, category: 'fuel' },
  
  // Ignition
  { id: 'timing', name: 'Ignition Timing', unit: '°BTDC', min: -10, max: 60, decimals: 1, color: '#FF9900', enabled: true, category: 'ignition' },
  { id: 'dwell', name: 'Dwell Time', unit: 'ms', min: 0, max: 10, decimals: 2, color: '#FFAA66', enabled: false, category: 'ignition' },
  { id: 'knock', name: 'Knock', unit: 'V', min: 0, max: 5, decimals: 2, color: '#FF0000', enabled: true, category: 'ignition' },
  
  // Sensors
  { id: 'map', name: 'MAP', unit: 'kPa', min: 0, max: 250, decimals: 1, color: '#0099FF', enabled: true, category: 'sensors' },
  { id: 'tps', name: 'TPS', unit: '%', min: 0, max: 100, decimals: 1, color: '#66FF00', enabled: true, category: 'sensors' },
  { id: 'iat', name: 'IAT', unit: '°C', min: -30, max: 150, decimals: 1, color: '#00FFAA', enabled: true, category: 'sensors' },
  { id: 'clt', name: 'CLT', unit: '°C', min: -40, max: 130, decimals: 1, color: '#FF6666', enabled: true, category: 'sensors' },
  { id: 'battery', name: 'Battery', unit: 'V', min: 8, max: 18, decimals: 2, color: '#FFFF00', enabled: true, category: 'sensors' },
  { id: 'baro', name: 'Barometric', unit: 'kPa', min: 50, max: 110, decimals: 1, color: '#AAAAFF', enabled: false, category: 'sensors' },
  { id: 'boost', name: 'Boost', unit: 'psi', min: -10, max: 30, decimals: 1, color: '#FF00FF', enabled: true, category: 'sensors' },
  { id: 'oilPressure', name: 'Oil Pressure', unit: 'psi', min: 0, max: 120, decimals: 1, color: '#996633', enabled: false, category: 'sensors' },
  { id: 'oilTemp', name: 'Oil Temp', unit: '°C', min: 0, max: 150, decimals: 1, color: '#CC6600', enabled: false, category: 'sensors' },
  
  // Calculated
  { id: 've', name: 'VE', unit: '%', min: 0, max: 150, decimals: 1, color: '#00AAFF', enabled: false, category: 'calculated' },
  { id: 'targetAFR', name: 'Target AFR', unit: 'AFR', min: 10, max: 18, decimals: 1, color: '#FF77FF', enabled: false, category: 'calculated' },
  { id: 'afrError', name: 'AFR Error', unit: '%', min: -50, max: 50, decimals: 1, color: '#FF3333', enabled: false, category: 'calculated' },
];

// Example math channel formulas (MegaLogViewer style)
export const EXAMPLE_MATH_FORMULAS = [
  {
    name: 'Load Calculation',
    formula: 'MAP / (RPM * 0.5 / 1000)',
    description: 'Estimated engine load from MAP and RPM'
  },
  {
    name: 'Lambda from AFR',
    formula: 'AFR / 14.7',
    description: 'Convert AFR to Lambda (stoich = 1.0)'
  },
  {
    name: 'VE Error',
    formula: '((AFR - TargetAFR) / TargetAFR) * 100',
    description: 'Percentage error in VE tuning'
  },
  {
    name: 'Boost Pressure',
    formula: 'MAP - Baro',
    description: 'Boost as differential pressure'
  },
  {
    name: 'Power Estimate',
    formula: '(RPM * MAP * 0.1) / 100',
    description: 'Rough horsepower estimate'
  }
];
