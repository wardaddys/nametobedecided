// Comprehensive table definitions for ECU tuning application
// All table types with axes, units, ranges, and default data

export type TableCategory = 'fuel' | 'ignition' | 'boost' | 'compensation' | 'valve-idle' | 'protection' | 'vtec';

export interface AxisDefinition {
  label: string;
  unit: string;
  values: number[];
  min: number;
  max: number;
}

export interface TableDefinition {
  id: string;
  name: string;
  category: TableCategory;
  description: string;
  xAxis: AxisDefinition;
  yAxis: AxisDefinition;
  zAxis: {
    label: string;
    unit: string;
    min: number;
    max: number;
  };
  defaultData: number[][];
  requiresFeature?: 'hasBoostControl' | 'hasVTECControl' | 'hasStagedInjection' | 'hasVVTControl';
  profileSpecific?: boolean; // True for tables that need Low/High VTEC versions
}

// Helper function to generate 2D array with default values
function generateTable(rows: number, cols: number, defaultValue: number): number[][] {
  return Array.from({ length: rows }, () => Array(cols).fill(defaultValue));
}

// Helper function to generate gradient table
function generateGradientTable(
  rows: number, 
  cols: number, 
  minVal: number, 
  maxVal: number,
  rowBias: number = 0.5,
  colBias: number = 0.5
): number[][] {
  return Array.from({ length: rows }, (_, r) =>
    Array.from({ length: cols }, (_, c) => {
      const rowFactor = r / (rows - 1);
      const colFactor = c / (cols - 1);
      const blended = rowFactor * rowBias + colFactor * colBias;
      return minVal + (maxVal - minVal) * blended;
    })
  );
}

// =====================================================================
// FUEL TABLES
// =====================================================================

export const VE_TABLE: TableDefinition = {
  id: 've-table',
  name: 'VE Table (Volumetric Efficiency)',
  category: 'fuel',
  description: 'Primary fuel calibration table - defines engine breathing efficiency',
  xAxis: {
    label: 'RPM',
    unit: 'rpm',
    values: [500, 1000, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 7000, 8000, 9000],
    min: 500,
    max: 10000,
  },
  yAxis: {
    label: 'Load (MAP)',
    unit: 'kPa',
    values: [20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 140, 160, 180, 200, 250, 300],
    min: 20,
    max: 300,
  },
  zAxis: {
    label: 'VE',
    unit: '%',
    min: 0,
    max: 120,
  },
  defaultData: generateGradientTable(17, 12, 40, 95, 0.6, 0.4),
  profileSpecific: true,
};

export const AFR_TARGET_TABLE: TableDefinition = {
  id: 'afr-target',
  name: 'AFR Target Table',
  category: 'fuel',
  description: 'Target air-fuel ratio map for closed-loop fuel control',
  xAxis: {
    label: 'RPM',
    unit: 'rpm',
    values: [500, 1000, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 7000, 8000, 9000],
    min: 500,
    max: 10000,
  },
  yAxis: {
    label: 'Load (MAP)',
    unit: 'kPa',
    values: [20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 140, 160, 180, 200, 250, 300],
    min: 20,
    max: 300,
  },
  zAxis: {
    label: 'AFR',
    unit: 'λ',
    min: 10.0,
    max: 16.0,
  },
  defaultData: generateGradientTable(17, 12, 11.5, 14.7, 0.7, 0.3),
  profileSpecific: true,
};

export const FUEL_PULSE_WIDTH_TABLE: TableDefinition = {
  id: 'fuel-pulse-width',
  name: 'Fuel Pulse Width',
  category: 'fuel',
  description: 'Base injector pulse width calibration',
  xAxis: {
    label: 'RPM',
    unit: 'rpm',
    values: [500, 1000, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 7000, 8000, 9000],
    min: 500,
    max: 10000,
  },
  yAxis: {
    label: 'Load (MAP)',
    unit: 'kPa',
    values: [20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 140, 160, 180, 200, 250, 300],
    min: 20,
    max: 300,
  },
  zAxis: {
    label: 'Pulse Width',
    unit: 'ms',
    min: 0,
    max: 20,
  },
  defaultData: generateGradientTable(17, 12, 1.5, 12.0, 0.7, 0.3),
};

// =====================================================================
// IGNITION TABLES
// =====================================================================

export const IGNITION_ADVANCE_TABLE: TableDefinition = {
  id: 'ignition-advance',
  name: 'Ignition Advance Table',
  category: 'ignition',
  description: 'Base ignition timing map',
  xAxis: {
    label: 'RPM',
    unit: 'rpm',
    values: [500, 1000, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 7000, 8000, 9000],
    min: 500,
    max: 10000,
  },
  yAxis: {
    label: 'Load (MAP)',
    unit: 'kPa',
    values: [20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 140, 160, 180, 200, 250, 300],
    min: 20,
    max: 300,
  },
  zAxis: {
    label: 'Advance',
    unit: '° BTDC',
    min: -10,
    max: 45,
  },
  defaultData: generateGradientTable(17, 12, 10, 38, 0.3, 0.5),
  profileSpecific: true,
};

// =====================================================================
// BOOST TABLES (Only shown for turbocharged/supercharged)
// =====================================================================

export const BOOST_TARGET_TABLE: TableDefinition = {
  id: 'boost-target',
  name: 'Boost Target Table',
  category: 'boost',
  description: 'Target boost pressure map',
  xAxis: {
    label: 'RPM',
    unit: 'rpm',
    values: [1000, 1500, 2000, 2500, 3000, 3500, 4000, 5000, 6000, 7000, 8000],
    min: 1000,
    max: 9000,
  },
  yAxis: {
    label: 'TPS',
    unit: '%',
    values: [0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100],
    min: 0,
    max: 100,
  },
  zAxis: {
    label: 'Boost',
    unit: 'kPa',
    min: -20,
    max: 200,
  },
  defaultData: generateGradientTable(11, 11, 0, 150, 0.5, 0.6),
  requiresFeature: 'hasBoostControl',
};

export const WASTEGATE_DUTY_TABLE: TableDefinition = {
  id: 'wastegate-duty',
  name: 'Wastegate Duty Cycle',
  category: 'boost',
  description: 'Wastegate solenoid duty cycle control',
  xAxis: {
    label: 'RPM',
    unit: 'rpm',
    values: [1000, 1500, 2000, 2500, 3000, 3500, 4000, 5000, 6000, 7000, 8000],
    min: 1000,
    max: 9000,
  },
  yAxis: {
    label: 'Boost Error',
    unit: 'kPa',
    values: [-50, -40, -30, -20, -10, 0, 10, 20, 30, 40, 50],
    min: -50,
    max: 50,
  },
  zAxis: {
    label: 'Duty',
    unit: '%',
    min: 0,
    max: 100,
  },
  defaultData: generateGradientTable(11, 11, 20, 80, 0.5, 0.5),
  requiresFeature: 'hasBoostControl',
};

export const BOOST_RAMP_RATE_TABLE: TableDefinition = {
  id: 'boost-ramp-rate',
  name: 'Boost Ramp Rate',
  category: 'boost',
  description: 'Speed of boost increase to prevent surge',
  xAxis: {
    label: 'RPM',
    unit: 'rpm',
    values: [1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000],
    min: 1000,
    max: 10000,
  },
  yAxis: {
    label: 'Gear',
    unit: '#',
    values: [1, 2, 3, 4, 5, 6],
    min: 1,
    max: 6,
  },
  zAxis: {
    label: 'Ramp Rate',
    unit: 'kPa/s',
    min: 5,
    max: 50,
  },
  defaultData: [
    [5, 8, 10, 12, 15, 18, 20, 22, 25, 28],
    [8, 10, 12, 15, 18, 20, 22, 25, 28, 30],
    [10, 12, 15, 18, 20, 22, 25, 28, 30, 32],
    [12, 15, 18, 20, 22, 25, 28, 30, 32, 35],
    [15, 18, 20, 22, 25, 28, 30, 32, 35, 38],
    [18, 20, 22, 25, 28, 30, 32, 35, 38, 40],
  ],
  requiresFeature: 'hasBoostControl',
};

export const BOOST_BY_GEAR_TABLE: TableDefinition = {
  id: 'boost-by-gear',
  name: 'Boost by Gear',
  category: 'boost',
  description: 'Gear-dependent boost reduction for traction',
  xAxis: {
    label: 'Gear',
    unit: '#',
    values: [1, 2, 3, 4, 5, 6],
    min: 1,
    max: 6,
  },
  yAxis: {
    label: 'RPM',
    unit: 'rpm',
    values: [2000, 3000, 4000, 5000, 6000, 7000, 8000],
    min: 2000,
    max: 8000,
  },
  zAxis: {
    label: 'Offset',
    unit: '%',
    min: -50,
    max: 20,
  },
  defaultData: [
    [-20, -15, -10, -5, 0, 0],
    [-20, -15, -10, -5, 0, 0],
    [-15, -10, -5, 0, 0, 0],
    [-15, -10, -5, 0, 0, 5],
    [-10, -5, 0, 0, 0, 5],
    [-10, -5, 0, 0, 5, 10],
    [-10, -5, 0, 0, 5, 10],
  ],
  requiresFeature: 'hasBoostControl',
};

export const ANTILAG_RETARD_TABLE: TableDefinition = {
  id: 'antilag-retard',
  name: 'Anti-Lag Ignition Retard',
  category: 'boost',
  description: 'Ignition timing retard for exhaust heat to maintain turbo spool',
  xAxis: {
    label: 'RPM',
    unit: 'rpm',
    values: [2000, 3000, 4000, 5000, 6000, 7000, 8000],
    min: 2000,
    max: 8000,
  },
  yAxis: {
    label: 'TPS',
    unit: '%',
    values: [0, 20, 40, 60, 80, 100],
    min: 0,
    max: 100,
  },
  zAxis: {
    label: 'Retard',
    unit: '°BTDC',
    min: -30,
    max: 0,
  },
  defaultData: [
    [0, 0, -5, -8, -10, -12, -15],
    [0, -2, -5, -8, -10, -12, -15],
    [0, -3, -6, -9, -12, -15, -18],
    [0, -5, -8, -12, -15, -18, -20],
    [0, -8, -12, -15, -18, -20, -22],
    [0, -10, -15, -18, -20, -22, -25],
  ],
  requiresFeature: 'hasBoostControl',
};

export const OVERBOOST_PROTECTION_TABLE: TableDefinition = {
  id: 'overboost-protection',
  name: 'Overboost Protection',
  category: 'boost',
  description: 'Fuel/ignition cut threshold on excessive boost',
  xAxis: {
    label: 'RPM',
    unit: 'rpm',
    values: [2000, 3000, 4000, 5000, 6000, 7000, 8000],
    min: 2000,
    max: 8000,
  },
  yAxis: {
    label: 'Load',
    unit: '%',
    values: [40, 50, 60, 70, 80, 90, 100],
    min: 40,
    max: 100,
  },
  zAxis: {
    label: 'Threshold',
    unit: 'kPa',
    min: 5,
    max: 30,
  },
  defaultData: generateGradientTable(7, 7, 10, 8, 0.3, 0.2),
  requiresFeature: 'hasBoostControl',
};

export const IAT_BOOST_COMP_TABLE: TableDefinition = {
  id: 'iat-boost-comp',
  name: 'IAT Boost Compensation',
  category: 'boost',
  description: 'Reduce boost target based on intake air temperature',
  xAxis: {
    label: 'IAT',
    unit: '°C',
    values: [20, 30, 40, 50, 60, 70, 80, 90, 100],
    min: 20,
    max: 100,
  },
  yAxis: {
    label: 'Boost',
    unit: 'psi',
    values: [10, 15, 20, 25, 30],
    min: 10,
    max: 30,
  },
  zAxis: {
    label: 'Reduction',
    unit: '%',
    min: 0,
    max: 30,
  },
  defaultData: [
    [0, 0, 2, 5, 8, 12, 15, 20, 25],
    [0, 0, 3, 6, 10, 14, 18, 23, 28],
    [0, 0, 4, 8, 12, 16, 20, 25, 30],
    [0, 0, 5, 10, 15, 20, 25, 30, 30],
    [0, 0, 6, 12, 18, 24, 30, 30, 30],
  ],
  requiresFeature: 'hasBoostControl',
};

export const EGT_BOOST_LIMIT_TABLE: TableDefinition = {
  id: 'egt-boost-limit',
  name: 'EGT Boost Limit',
  category: 'boost',
  description: 'Reduce boost when exhaust temperature is excessive',
  xAxis: {
    label: 'EGT',
    unit: '°C',
    values: [700, 750, 800, 850, 900, 950, 1000, 1050],
    min: 700,
    max: 1050,
  },
  yAxis: {
    label: 'Load',
    unit: '%',
    values: [50, 60, 70, 80, 90, 100],
    min: 50,
    max: 100,
  },
  zAxis: {
    label: 'Reduction',
    unit: '%',
    min: 0,
    max: 50,
  },
  defaultData: [
    [0, 0, 0, 5, 10, 20, 35, 50],
    [0, 0, 0, 5, 10, 20, 35, 50],
    [0, 0, 0, 5, 10, 20, 35, 50],
    [0, 0, 0, 8, 15, 25, 40, 50],
    [0, 0, 0, 10, 20, 30, 45, 50],
    [0, 0, 0, 12, 25, 35, 50, 50],
  ],
  requiresFeature: 'hasBoostControl',
};

// =====================================================================
// COMPENSATION TABLES
// =====================================================================

export const ACCEL_ENRICHMENT_TABLE: TableDefinition = {
  id: 'accel-enrichment',
  name: 'Acceleration Enrichment',
  category: 'compensation',
  description: 'Transient fuel enrichment during throttle changes',
  xAxis: {
    label: 'TPS Rate',
    unit: '%/s',
    values: [0, 10, 20, 30, 50, 75, 100, 150, 200, 300],
    min: 0,
    max: 300,
  },
  yAxis: {
    label: 'ECT',
    unit: '°C',
    values: [-20, 0, 20, 40, 60, 80, 100],
    min: -20,
    max: 110,
  },
  zAxis: {
    label: 'Fuel Add',
    unit: '%',
    min: 0,
    max: 50,
  },
  defaultData: generateGradientTable(7, 10, 0, 35, 0.3, 0.7),
};

export const COLD_START_ENRICHMENT_TABLE: TableDefinition = {
  id: 'cold-start-enrichment',
  name: 'Cold Start Enrichment',
  category: 'compensation',
  description: 'Additional fuel during cold engine start',
  xAxis: {
    label: 'Time After Start',
    unit: 's',
    values: [0, 1, 2, 3, 5, 7, 10, 15, 20, 30],
    min: 0,
    max: 30,
  },
  yAxis: {
    label: 'ECT',
    unit: '°C',
    values: [-20, -10, 0, 10, 20, 30, 40, 60, 80],
    min: -20,
    max: 80,
  },
  zAxis: {
    label: 'Enrichment',
    unit: '%',
    min: 0,
    max: 100,
  },
  defaultData: generateGradientTable(9, 10, 5, 80, 0.8, 0.6),
};

export const IAT_COMPENSATION_TABLE: TableDefinition = {
  id: 'iat-compensation',
  name: 'IAT Compensation',
  category: 'compensation',
  description: 'Fuel compensation based on intake air temperature',
  xAxis: {
    label: 'IAT',
    unit: '°C',
    values: [-20, -10, 0, 10, 20, 30, 40, 50, 60, 70, 80],
    min: -20,
    max: 80,
  },
  yAxis: {
    label: 'Load (MAP)',
    unit: 'kPa',
    values: [20, 40, 60, 80, 100, 120, 150, 200],
    min: 20,
    max: 200,
  },
  zAxis: {
    label: 'Fuel Modifier',
    unit: '%',
    min: -20,
    max: 20,
  },
  defaultData: generateGradientTable(8, 11, -10, 10, 0.4, 0.3),
};

// =====================================================================
// VALVE TIMING & IDLE TABLES
// =====================================================================

export const VVT_TARGET_TABLE: TableDefinition = {
  id: 'vvt-target',
  name: 'VVT Target Advance',
  category: 'valve-idle',
  description: 'Variable valve timing target advance',
  xAxis: {
    label: 'RPM',
    unit: 'rpm',
    values: [500, 1000, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 7000, 8000],
    min: 500,
    max: 9000,
  },
  yAxis: {
    label: 'Load (MAP)',
    unit: 'kPa',
    values: [20, 30, 40, 60, 80, 100, 120, 150, 200],
    min: 20,
    max: 200,
  },
  zAxis: {
    label: 'VVT Advance',
    unit: '°',
    min: 0,
    max: 50,
  },
  defaultData: generateGradientTable(9, 11, 5, 40, 0.3, 0.5),
  requiresFeature: 'hasVVTControl',
};

export const IDLE_SPEED_TARGET_TABLE: TableDefinition = {
  id: 'idle-speed-target',
  name: 'Idle Speed Target',
  category: 'valve-idle',
  description: 'Target idle speed vs coolant temperature',
  xAxis: {
    label: 'ECT',
    unit: '°C',
    values: [-20, -10, 0, 10, 20, 30, 40, 60, 80, 100],
    min: -20,
    max: 100,
  },
  yAxis: {
    label: 'Load Modifier',
    unit: '%',
    values: [0, 10, 20, 30, 40, 50],
    min: 0,
    max: 50,
  },
  zAxis: {
    label: 'Target RPM',
    unit: 'rpm',
    min: 600,
    max: 1800,
  },
  defaultData: generateGradientTable(6, 10, 1200, 1600, 0.2, 0.7),
};

export const IACV_DUTY_TABLE: TableDefinition = {
  id: 'iacv-duty',
  name: 'IACV Duty Cycle',
  category: 'valve-idle',
  description: 'Idle air control valve duty cycle',
  xAxis: {
    label: 'RPM Error',
    unit: 'rpm',
    values: [-300, -200, -100, -50, -25, 0, 25, 50, 100, 200, 300],
    min: -300,
    max: 300,
  },
  yAxis: {
    label: 'ECT',
    unit: '°C',
    values: [-20, 0, 20, 40, 60, 80, 100],
    min: -20,
    max: 100,
  },
  zAxis: {
    label: 'Duty',
    unit: '%',
    min: 0,
    max: 100,
  },
  defaultData: generateGradientTable(7, 11, 20, 80, 0.3, 0.5),
};

// =====================================================================
// PROTECTION & LIMITERS
// =====================================================================

export const REV_LIMITER_TABLE: TableDefinition = {
  id: 'rev-limiter',
  name: 'Rev Limiter',
  category: 'protection',
  description: 'RPM limit based on coolant temperature and gear',
  xAxis: {
    label: 'Gear',
    unit: '#',
    values: [1, 2, 3, 4, 5, 6],
    min: 1,
    max: 6,
  },
  yAxis: {
    label: 'ECT',
    unit: '°C',
    values: [0, 20, 40, 60, 80, 100],
    min: 0,
    max: 100,
  },
  zAxis: {
    label: 'RPM Limit',
    unit: 'rpm',
    min: 5000,
    max: 9500,
  },
  defaultData: generateGradientTable(6, 6, 7500, 8800, 0.6, 0.2),
};

export const LAUNCH_CONTROL_TABLE: TableDefinition = {
  id: 'launch-control',
  name: 'Launch Control',
  category: 'protection',
  description: 'Launch control RPM limit based on speed and throttle',
  xAxis: {
    label: 'Speed',
    unit: 'km/h',
    values: [0, 5, 10, 15, 20, 30, 40],
    min: 0,
    max: 50,
  },
  yAxis: {
    label: 'TPS',
    unit: '%',
    values: [0, 50, 75, 90, 95, 100],
    min: 0,
    max: 100,
  },
  zAxis: {
    label: 'RPM Limit',
    unit: 'rpm',
    min: 3000,
    max: 8000,
  },
  defaultData: generateGradientTable(6, 7, 4000, 7000, 0.4, 0.5),
};

// =====================================================================
// VTEC SPECIFIC TABLES
// =====================================================================

export const VTEC_ENGAGEMENT_TABLE: TableDefinition = {
  id: 'vtec-engagement',
  name: 'VTEC Engagement',
  category: 'vtec',
  description: 'VTEC engagement point based on RPM and load',
  xAxis: {
    label: 'RPM',
    unit: 'rpm',
    values: [3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000],
    min: 3000,
    max: 8500,
  },
  yAxis: {
    label: 'Load (MAP)',
    unit: 'kPa',
    values: [40, 50, 60, 70, 80, 90, 100, 120, 150, 200],
    min: 40,
    max: 200,
  },
  zAxis: {
    label: 'Engage',
    unit: 'bool',
    min: 0,
    max: 1,
  },
  defaultData: generateGradientTable(10, 11, 0, 1, 0.4, 0.6),
  requiresFeature: 'hasVTECControl',
};

// =====================================================================
// TABLE REGISTRY
// =====================================================================

export const ALL_TABLE_DEFINITIONS: TableDefinition[] = [
  // Fuel Tables
  VE_TABLE,
  AFR_TARGET_TABLE,
  FUEL_PULSE_WIDTH_TABLE,
  
  // Ignition Tables
  IGNITION_ADVANCE_TABLE,
  
  // Boost Tables (Consolidated from database)
  BOOST_TARGET_TABLE,
  BOOST_RAMP_RATE_TABLE,
  BOOST_BY_GEAR_TABLE,
  WASTEGATE_DUTY_TABLE,
  ANTILAG_RETARD_TABLE,
  OVERBOOST_PROTECTION_TABLE,
  IAT_BOOST_COMP_TABLE,
  EGT_BOOST_LIMIT_TABLE,
  
  // Compensation Tables
  ACCEL_ENRICHMENT_TABLE,
  COLD_START_ENRICHMENT_TABLE,
  IAT_COMPENSATION_TABLE,
  
  // Valve & Idle Tables
  VVT_TARGET_TABLE,
  IDLE_SPEED_TARGET_TABLE,
  IACV_DUTY_TABLE,
  
  // Protection Tables
  REV_LIMITER_TABLE,
  LAUNCH_CONTROL_TABLE,
  
  // VTEC Tables
  VTEC_ENGAGEMENT_TABLE,
];

export function getVisibleTables(features: {
  hasBoostControl: boolean;
  hasVTECControl: boolean;
  hasStagedInjection: boolean;
  hasVVTControl: boolean;
}): TableDefinition[] {
  return ALL_TABLE_DEFINITIONS.filter(table => {
    if (!table.requiresFeature) return true;
    return features[table.requiresFeature];
  });
}

export function getTablesByCategory(category: TableCategory, features: any): TableDefinition[] {
  return getVisibleTables(features).filter(table => table.category === category);
}
