/**
 * Comprehensive Boost & Forced Induction Database
 * Based on Haltech NSP, MoTeC M1, AEM Infinity, Hondata, Link PCLink, TunerStudio
 */

export type ForcedInductionType = 
  | 'single-turbo'
  | 'parallel-twin'
  | 'sequential-twin'
  | 'supercharger'
  | 'twin-charger'
  | 'staged-boost';

export type BoostControlMode = 'open-loop' | 'closed-loop';

export interface BoostTableDefinition {
  id: string;
  name: string;
  category: string;
  description: string;
  noviceExplanation: string;
  professionalDetails: string;
  xAxis: {
    label: string;
    unit: string;
    values: number[];
  };
  yAxis: {
    label: string;
    unit: string;
    values: number[];
  };
  zAxis: {
    label: string;
    unit: string;
    min: number;
    max: number;
  };
  defaultData: number[][];
  applicableConfigs: ForcedInductionType[];
  warningThresholds?: {
    min?: number;
    max?: number;
    message: string;
  };
  icon: string;
}

export interface BoostParameter {
  id: string;
  name: string;
  category: string;
  type: 'number' | 'boolean' | 'dropdown' | 'table';
  unit?: string;
  min?: number;
  max?: number;
  default: any;
  noviceExplanation: string;
  professionalDetails: string;
  applicableConfigs: ForcedInductionType[];
  icon: string;
  warningIcon?: string;
}

// Configuration-specific defaults and behaviors
export const FORCED_INDUCTION_CONFIGS: Record<ForcedInductionType, {
  name: string;
  noviceAnalogy: string;
  keyDifferences: string;
  defaultBoostTarget: number; // psi
  defaultRampRate: number; // kPa/s
  hasWastegate: boolean;
  hasBypassValve: boolean;
  requiresSwitchover: boolean;
  defaultPID: { p: number; i: number; d: number };
  setupComplexity: 'beginner' | 'intermediate' | 'advanced' | 'expert';
}> = {
  'single-turbo': {
    name: 'Single Turbo',
    noviceAnalogy: 'One big fan pushing air—like a single strong push',
    keyDifferences: 'Balanced spool; focus on lag management',
    defaultBoostTarget: 20,
    defaultRampRate: 10,
    hasWastegate: true,
    hasBypassValve: false,
    requiresSwitchover: false,
    defaultPID: { p: 30, i: 0.2, d: 10 },
    setupComplexity: 'beginner',
  },
  'parallel-twin': {
    name: 'Parallel Twin Turbo',
    noviceAnalogy: 'Two fans side-by-side—like teamwork for quick start',
    keyDifferences: 'Even flow; less lag, but balance cylinders',
    defaultBoostTarget: 18,
    defaultRampRate: 15,
    hasWastegate: true,
    hasBypassValve: false,
    requiresSwitchover: false,
    defaultPID: { p: 30, i: 0.2, d: 10 },
    setupComplexity: 'intermediate',
  },
  'sequential-twin': {
    name: 'Sequential Twin Turbo',
    noviceAnalogy: 'Small fan for quick start, big for power—like gears shifting',
    keyDifferences: 'Switchover tuning critical (lean spikes)',
    defaultBoostTarget: 25,
    defaultRampRate: 20,
    hasWastegate: true,
    hasBypassValve: false,
    requiresSwitchover: true,
    defaultPID: { p: 35, i: 0.15, d: 12 },
    setupComplexity: 'expert',
  },
  'supercharger': {
    name: 'Supercharger',
    noviceAnalogy: 'Belt-driven fan—no exhaust wait, instant push',
    keyDifferences: 'Linear boost; high heat, no wastegate',
    defaultBoostTarget: 15,
    defaultRampRate: 50,
    hasWastegate: false,
    hasBypassValve: true,
    requiresSwitchover: false,
    defaultPID: { p: 25, i: 0.1, d: 5 },
    setupComplexity: 'intermediate',
  },
  'twin-charger': {
    name: 'Twin Charger (Compound)',
    noviceAnalogy: 'Super for low-end kick, turbo for high—like hybrid car',
    keyDifferences: 'Heat stacking; clutch handover',
    defaultBoostTarget: 22,
    defaultRampRate: 15,
    hasWastegate: true,
    hasBypassValve: true,
    requiresSwitchover: true,
    defaultPID: { p: 30, i: 0.2, d: 10 },
    setupComplexity: 'expert',
  },
  'staged-boost': {
    name: 'Staged Boost',
    noviceAnalogy: 'Multi-level push—like overdrive gears',
    keyDifferences: 'Per-stage controls; flow additive',
    defaultBoostTarget: 25,
    defaultRampRate: 10,
    hasWastegate: true,
    hasBypassValve: false,
    requiresSwitchover: true,
    defaultPID: { p: 30, i: 0.2, d: 10 },
    setupComplexity: 'advanced',
  },
};

// Boost Table Definitions
export const BOOST_TABLE_DEFINITIONS: BoostTableDefinition[] = [
  // ===== TARGET & RAMP =====
  {
    id: 'boost-target',
    name: 'Boost Target',
    category: 'Target & Ramp',
    description: 'Desired boost pressure across RPM and throttle position',
    noviceExplanation: 'Your power goal—too fast = engine stress; too slow = weak acceleration. Ramp like easing into highway—start slow, build to max.',
    professionalDetails: 'MAP target above atmospheric. Plot on compressor map to avoid surge. Linear ramp for street, aggressive for track.',
    xAxis: { label: 'RPM', unit: 'rpm', values: [1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000] },
    yAxis: { label: 'TPS', unit: '%', values: [20, 30, 40, 50, 60, 70, 80, 90, 100] },
    zAxis: { label: 'Boost', unit: 'psi', min: -5, max: 40 },
    defaultData: [
      [0, 2, 4, 6, 8, 10, 12, 14, 15, 15],
      [0, 3, 5, 8, 10, 13, 15, 17, 18, 18],
      [0, 4, 6, 10, 12, 15, 17, 19, 20, 20],
      [0, 5, 8, 12, 14, 17, 19, 21, 22, 22],
      [0, 6, 10, 14, 16, 19, 21, 23, 24, 24],
      [0, 8, 12, 16, 18, 21, 23, 25, 26, 26],
      [0, 10, 14, 18, 20, 23, 25, 27, 28, 28],
      [0, 12, 16, 20, 22, 25, 27, 29, 30, 30],
      [0, 14, 18, 22, 24, 27, 29, 31, 32, 32],
    ],
    applicableConfigs: ['single-turbo', 'parallel-twin', 'sequential-twin', 'supercharger', 'twin-charger', 'staged-boost'],
    warningThresholds: { max: 35, message: '⚠️ High boost risk! Verify intercooler capacity and octane rating.' },
    icon: '🎯',
  },
  {
    id: 'boost-ramp-rate',
    name: 'Boost Ramp Rate',
    category: 'Target & Ramp',
    description: 'Speed of boost increase to prevent surge',
    noviceExplanation: 'Speed limit—gradual rise prevents "sneeze" (surge). Like easing off cruise control smoothly.',
    professionalDetails: 'Slew rate limiter in kPa/s. Lower for surge-prone compressors, higher for quick spool.',
    xAxis: { label: 'RPM', unit: 'rpm', values: [1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000] },
    yAxis: { label: 'Gear', unit: '#', values: [1, 2, 3, 4, 5, 6] },
    zAxis: { label: 'Ramp', unit: 'kPa/s', min: 5, max: 50 },
    defaultData: [
      [5, 8, 10, 12, 15, 18, 20, 22, 25, 28],
      [8, 10, 12, 15, 18, 20, 22, 25, 28, 30],
      [10, 12, 15, 18, 20, 22, 25, 28, 30, 32],
      [12, 15, 18, 20, 22, 25, 28, 30, 32, 35],
      [15, 18, 20, 22, 25, 28, 30, 32, 35, 38],
      [18, 20, 22, 25, 28, 30, 32, 35, 38, 40],
    ],
    applicableConfigs: ['single-turbo', 'parallel-twin', 'sequential-twin', 'twin-charger', 'staged-boost'],
    warningThresholds: { max: 40, message: '⚠️ Very fast ramp! May cause compressor surge or turbo overspeed.' },
    icon: '📈',
  },
  {
    id: 'boost-by-gear',
    name: 'Boost by Gear',
    category: 'Target & Ramp',
    description: 'Gear-dependent boost reduction for traction',
    noviceExplanation: 'Lower gears = less boost—like downshifting easy. Prevents wheel spin in 1st/2nd gear.',
    professionalDetails: 'Percentage offset from base target. Negative for traction, positive for high-gear pull.',
    xAxis: { label: 'Gear', unit: '#', values: [1, 2, 3, 4, 5, 6] },
    yAxis: { label: 'RPM', unit: 'rpm', values: [2000, 3000, 4000, 5000, 6000, 7000, 8000] },
    zAxis: { label: 'Offset', unit: '%', min: -50, max: 20 },
    defaultData: [
      [-20, -15, -10, -5, 0, 0],
      [-20, -15, -10, -5, 0, 0],
      [-15, -10, -5, 0, 0, 0],
      [-15, -10, -5, 0, 0, 5],
      [-10, -5, 0, 0, 0, 5],
      [-10, -5, 0, 0, 5, 10],
      [-10, -5, 0, 0, 5, 10],
    ],
    applicableConfigs: ['single-turbo', 'parallel-twin', 'sequential-twin', 'supercharger', 'twin-charger', 'staged-boost'],
    icon: '⚙️',
  },

  // ===== CONTROL & SOLENOID =====
  {
    id: 'wastegate-duty',
    name: 'Wastegate Duty Cycle',
    category: 'Control & Solenoid',
    description: 'Solenoid PWM output to control wastegate position',
    noviceExplanation: 'Valve openness—like dimmer switch for fan speed. Higher duty = more boost.',
    professionalDetails: 'PID output to 3-port MAC valve. Maps boost error to duty cycle. Calibrate actuator spring pressure.',
    xAxis: { label: 'RPM', unit: 'rpm', values: [1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000] },
    yAxis: { label: 'Error', unit: 'kPa', values: [-20, -10, -5, 0, 5, 10, 20, 30, 40, 50] },
    zAxis: { label: 'Duty', unit: '%', min: 0, max: 100 },
    defaultData: [
      [10, 15, 20, 25, 30, 35, 40, 45, 50, 55],
      [15, 20, 25, 30, 35, 40, 45, 50, 55, 60],
      [20, 25, 30, 35, 40, 45, 50, 55, 60, 65],
      [25, 30, 35, 40, 45, 50, 55, 60, 65, 70],
      [30, 35, 40, 45, 50, 55, 60, 65, 70, 75],
      [35, 40, 45, 50, 55, 60, 65, 70, 75, 80],
      [40, 45, 50, 55, 60, 65, 70, 75, 80, 85],
      [45, 50, 55, 60, 65, 70, 75, 80, 85, 90],
      [50, 55, 60, 65, 70, 75, 80, 85, 90, 95],
      [55, 60, 65, 70, 75, 80, 85, 90, 95, 100],
    ],
    applicableConfigs: ['single-turbo', 'parallel-twin', 'sequential-twin', 'twin-charger', 'staged-boost'],
    icon: '🎛️',
  },
  {
    id: 'bypass-valve-duty',
    name: 'Bypass Valve Duty',
    category: 'Control & Solenoid',
    description: 'Supercharger bypass valve control',
    noviceExplanation: 'Relief valve—opens at low throttle to reduce drag. Like cruise control coasting.',
    professionalDetails: 'Electromagnetic bypass for roots/screw. Close at WOT, open at cruise for efficiency.',
    xAxis: { label: 'RPM', unit: 'rpm', values: [1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000] },
    yAxis: { label: 'TPS', unit: '%', values: [10, 20, 30, 40, 50, 60, 70, 80, 90, 100] },
    zAxis: { label: 'Duty', unit: '%', min: 0, max: 100 },
    defaultData: [
      [90, 85, 80, 75, 70, 65, 60, 55],
      [80, 75, 70, 65, 60, 55, 50, 45],
      [70, 65, 60, 55, 50, 45, 40, 35],
      [60, 55, 50, 45, 40, 35, 30, 25],
      [50, 45, 40, 35, 30, 25, 20, 15],
      [40, 35, 30, 25, 20, 15, 10, 5],
      [30, 25, 20, 15, 10, 5, 0, 0],
      [20, 15, 10, 5, 0, 0, 0, 0],
      [10, 5, 0, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0],
    ],
    applicableConfigs: ['supercharger', 'twin-charger'],
    icon: '🚪',
  },

  // ===== ANTI-LAG & LAUNCH =====
  {
    id: 'anti-lag-retard',
    name: 'Anti-Lag Ignition Retard',
    category: 'Anti-Lag & Launch',
    description: 'Ignition timing retard for exhaust heat',
    noviceExplanation: 'Burps exhaust—like rev-matching; risks heat (EGT 1050°C max). Keeps turbo spinning.',
    professionalDetails: 'Delays combustion to exhaust manifold. Combine with +fuel for flame. Monitor EGT closely.',
    xAxis: { label: 'RPM', unit: 'rpm', values: [2000, 3000, 4000, 5000, 6000, 7000, 8000] },
    yAxis: { label: 'TPS', unit: '%', values: [0, 20, 40, 60, 80, 100] },
    zAxis: { label: 'Retard', unit: '°BTDC', min: -30, max: 0 },
    defaultData: [
      [0, 0, -5, -8, -10, -12, -15],
      [0, -2, -5, -8, -10, -12, -15],
      [0, -3, -6, -9, -12, -15, -18],
      [0, -5, -8, -12, -15, -18, -20],
      [0, -8, -12, -15, -18, -20, -22],
      [0, -10, -15, -18, -20, -22, -25],
    ],
    applicableConfigs: ['single-turbo', 'parallel-twin', 'sequential-twin', 'twin-charger'],
    warningThresholds: { min: -25, message: '⚠️ Extreme retard! Monitor EGT—risk of turbo/manifold damage.' },
    icon: '🔥',
  },
  {
    id: 'launch-control',
    name: 'Launch Control RPM Limit',
    category: 'Anti-Lag & Launch',
    description: 'Two-step rev limiter for launch',
    noviceExplanation: 'Drag start hold—like brake-boost; soft cut. Builds boost at standstill.',
    professionalDetails: 'Ignition cut at lower RPM vs main limiter. Activate via clutch+TPS threshold.',
    xAxis: { label: 'Speed', unit: 'km/h', values: [0, 5, 10, 15, 20] },
    yAxis: { label: 'TPS', unit: '%', values: [80, 85, 90, 95, 100] },
    zAxis: { label: 'Limit', unit: 'rpm', min: 3000, max: 6000 },
    defaultData: [
      [4000, 4200, 4500, 5000, 6000],
      [4000, 4200, 4500, 5000, 6000],
      [4000, 4200, 4500, 5000, 6000],
      [4000, 4200, 4500, 5000, 6000],
      [4000, 4200, 4500, 5000, 6000],
    ],
    applicableConfigs: ['single-turbo', 'parallel-twin', 'sequential-twin', 'supercharger', 'twin-charger', 'staged-boost'],
    icon: '🏁',
  },

  // ===== PROTECTIONS =====
  {
    id: 'overboost-protection',
    name: 'Overboost Protection',
    category: 'Protection & Compensation',
    description: 'Fuel/ignition cut on excessive boost',
    noviceExplanation: 'Emergency brake—if over target, cuts to save engine. Like emergency stop button.',
    professionalDetails: 'Threshold in kPa over target. Soft cut (fuel) or hard cut (ignition). Log faults.',
    xAxis: { label: 'RPM', unit: 'rpm', values: [2000, 3000, 4000, 5000, 6000, 7000, 8000] },
    yAxis: { label: 'Load', unit: '%', values: [40, 50, 60, 70, 80, 90, 100] },
    zAxis: { label: 'Threshold', unit: 'kPa', min: 5, max: 30 },
    defaultData: [
      [10, 10, 10, 10, 10, 10, 10],
      [10, 10, 10, 10, 10, 10, 10],
      [10, 10, 10, 10, 10, 10, 10],
      [10, 10, 10, 10, 10, 10, 10],
      [8, 8, 8, 8, 8, 8, 8],
      [8, 8, 8, 8, 8, 8, 8],
      [5, 5, 5, 5, 5, 5, 5],
    ],
    applicableConfigs: ['single-turbo', 'parallel-twin', 'sequential-twin', 'supercharger', 'twin-charger', 'staged-boost'],
    warningThresholds: { max: 15, message: '⚠️ Tight threshold! May cause nuisance cuts on transients.' },
    icon: '🛡️',
  },
  {
    id: 'iat-compensation',
    name: 'IAT Boost Compensation',
    category: 'Protection & Compensation',
    description: 'Reduce boost target based on intake air temperature',
    noviceExplanation: 'Heat adjust—like AC in summer; denser air = more fuel. Hot air = reduce boost for safety.',
    professionalDetails: 'Percentage reduction per °C over threshold. Protects against knock from heat soak.',
    xAxis: { label: 'IAT', unit: '°C', values: [20, 30, 40, 50, 60, 70, 80, 90, 100] },
    yAxis: { label: 'Boost', unit: 'psi', values: [10, 15, 20, 25, 30] },
    zAxis: { label: 'Reduction', unit: '%', min: 0, max: 30 },
    defaultData: [
      [0, 0, 2, 5, 8, 12, 15, 20, 25],
      [0, 0, 3, 6, 10, 14, 18, 23, 28],
      [0, 0, 4, 8, 12, 16, 20, 25, 30],
      [0, 0, 5, 10, 15, 20, 25, 30, 30],
      [0, 0, 6, 12, 18, 24, 30, 30, 30],
    ],
    applicableConfigs: ['single-turbo', 'parallel-twin', 'sequential-twin', 'supercharger', 'twin-charger', 'staged-boost'],
    icon: '🌡️',
  },
  {
    id: 'egt-limit',
    name: 'EGT Boost Limit',
    category: 'Protection & Compensation',
    description: 'Reduce boost when exhaust temperature is excessive',
    noviceExplanation: 'Temp alarm—like oven timer; richen/retard to cool. Prevents turbo meltdown.',
    professionalDetails: 'Pre-turbo probe recommended. Gradual reduction to avoid torque step. Typical limit 950°C.',
    xAxis: { label: 'EGT', unit: '°C', values: [700, 750, 800, 850, 900, 950, 1000, 1050] },
    yAxis: { label: 'Load', unit: '%', values: [50, 60, 70, 80, 90, 100] },
    zAxis: { label: 'Reduction', unit: '%', min: 0, max: 50 },
    defaultData: [
      [0, 0, 0, 5, 10, 20, 35, 50],
      [0, 0, 0, 5, 10, 20, 35, 50],
      [0, 0, 0, 5, 10, 20, 35, 50],
      [0, 0, 0, 8, 15, 25, 40, 50],
      [0, 0, 0, 10, 20, 30, 45, 50],
      [0, 0, 0, 12, 25, 35, 50, 50],
    ],
    applicableConfigs: ['single-turbo', 'parallel-twin', 'sequential-twin', 'twin-charger', 'staged-boost'],
    warningThresholds: { max: 1000, message: '⚠️ Extreme EGT! Verify sensor placement and fuel enrichment.' },
    icon: '🔥',
  },

  // ===== SEQUENTIAL/COMPOUND SPECIFIC =====
  {
    id: 'sequential-switchover',
    name: 'Sequential Turbo Switchover',
    category: 'Advanced Configuration',
    description: 'RPM/Load points for switching from small to large turbo',
    noviceExplanation: 'Gear shift point—when to switch from quick to powerful. Buffer prevents lean spike.',
    professionalDetails: 'Hysteresis critical. Overlap boost +10 kPa to prevent torque hole. Close small WG before opening large.',
    xAxis: { label: 'RPM', unit: 'rpm', values: [2000, 3000, 4000, 5000, 6000, 7000] },
    yAxis: { label: 'Load', unit: '%', values: [40, 50, 60, 70, 80, 90, 100] },
    zAxis: { label: 'Switch', unit: 'bool', min: 0, max: 1 },
    defaultData: [
      [0, 0, 0, 0, 1, 1],
      [0, 0, 0, 0, 1, 1],
      [0, 0, 0, 1, 1, 1],
      [0, 0, 0, 1, 1, 1],
      [0, 0, 1, 1, 1, 1],
      [0, 0, 1, 1, 1, 1],
      [0, 1, 1, 1, 1, 1],
    ],
    applicableConfigs: ['sequential-twin'],
    warningThresholds: { message: '⚠️ Sequential switch: Add +10 kPa buffer or surge/lean spike risk!' },
    icon: '🔄',
  },
  {
    id: 'twincharger-clutch',
    name: 'Twin-Charger Clutch Duty',
    category: 'Advanced Configuration',
    description: 'Electromagnetic clutch engagement for supercharger',
    noviceExplanation: 'Handoff switch—super for low, turbo for high. Clutch connects/disconnects supercharger.',
    professionalDetails: 'PWM control of EM clutch. Ramp engagement to avoid shock load. Disengage when turbo exceeds super output.',
    xAxis: { label: 'RPM', unit: 'rpm', values: [1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000] },
    yAxis: { label: 'Boost', unit: 'psi', values: [0, 5, 10, 15, 20, 25] },
    zAxis: { label: 'Duty', unit: '%', min: 0, max: 100 },
    defaultData: [
      [100, 100, 90, 70, 40, 10, 0, 0],
      [100, 100, 90, 70, 40, 10, 0, 0],
      [100, 100, 90, 70, 40, 10, 0, 0],
      [100, 90, 80, 60, 30, 5, 0, 0],
      [100, 80, 70, 50, 20, 0, 0, 0],
      [90, 70, 50, 30, 10, 0, 0, 0],
    ],
    applicableConfigs: ['twin-charger'],
    icon: '⚡',
  },
];

// Scalar parameters (non-table)
export const BOOST_PARAMETERS: BoostParameter[] = [
  {
    id: 'control-mode',
    name: 'Boost Control Mode',
    category: 'General',
    type: 'dropdown',
    default: 'closed-loop',
    noviceExplanation: 'Open: Set-it-forget; Closed: Auto-adjusts like smart AC. Closed is better for consistency.',
    professionalDetails: 'Open-loop: Direct duty cycle. Closed-loop: PID feedback from MAP sensor. Use closed for varying conditions.',
    applicableConfigs: ['single-turbo', 'parallel-twin', 'sequential-twin', 'supercharger', 'twin-charger', 'staged-boost'],
    icon: '🎚️',
  },
  {
    id: 'pid-p-gain',
    name: 'PID P Gain',
    category: 'PID Tuning',
    type: 'number',
    unit: '',
    min: 0,
    max: 100,
    default: 30,
    noviceExplanation: 'How hard it corrects—like steering sensitivity. Too high = overshoot/oscillation.',
    professionalDetails: 'Proportional gain. Higher = faster response but risk of overshoot. Start conservative, increase until minor oscillation.',
    applicableConfigs: ['single-turbo', 'parallel-twin', 'sequential-twin', 'supercharger', 'twin-charger', 'staged-boost'],
    icon: '🎯',
  },
  {
    id: 'pid-i-gain',
    name: 'PID I Gain',
    category: 'PID Tuning',
    type: 'number',
    unit: '',
    min: 0,
    max: 5,
    default: 0.2,
    noviceExplanation: 'Eliminates slow drift—like cruise control trim. Too high = slow hunting.',
    professionalDetails: 'Integral gain. Removes steady-state error. Low value (0.1-0.5) typical. Windup protection recommended.',
    applicableConfigs: ['single-turbo', 'parallel-twin', 'sequential-twin', 'supercharger', 'twin-charger', 'staged-boost'],
    icon: '📊',
  },
  {
    id: 'pid-d-gain',
    name: 'PID D Gain',
    category: 'PID Tuning',
    type: 'number',
    unit: '',
    min: 0,
    max: 50,
    default: 10,
    noviceExplanation: 'Dampens quick changes—like shock absorbers. Reduces overshoot.',
    professionalDetails: 'Derivative gain. Anticipates future error. Sensitive to noise—filter MAP signal. Typical 5-15.',
    applicableConfigs: ['single-turbo', 'parallel-twin', 'sequential-twin', 'supercharger', 'twin-charger', 'staged-boost'],
    icon: '🔄',
  },
  {
    id: 'solenoid-frequency',
    name: 'Solenoid Frequency',
    category: 'Hardware',
    type: 'number',
    unit: 'Hz',
    min: 10,
    max: 100,
    default: 30,
    noviceExplanation: 'How fast valve pulses—like keyboard repeat rate. Match your valve specs.',
    professionalDetails: 'PWM frequency for wastegate/bypass solenoid. 20-40Hz typical for MAC valves. Check datasheet.',
    applicableConfigs: ['single-turbo', 'parallel-twin', 'sequential-twin', 'supercharger', 'twin-charger', 'staged-boost'],
    icon: '⚡',
  },
  {
    id: 'enable-antilag',
    name: 'Enable Anti-Lag',
    category: 'Anti-Lag & Launch',
    type: 'boolean',
    default: false,
    noviceExplanation: 'Turbo spool helper—very hot! Only for race/rally. Requires EGT monitoring.',
    professionalDetails: 'Retards timing and adds fuel to maintain exhaust energy. Extreme thermal stress—not for street.',
    applicableConfigs: ['single-turbo', 'parallel-twin', 'sequential-twin', 'twin-charger'],
    icon: '🔥',
    warningIcon: '⚠️',
  },
  {
    id: 'enable-launch',
    name: 'Enable Launch Control',
    category: 'Anti-Lag & Launch',
    type: 'boolean',
    default: false,
    noviceExplanation: 'Two-step rev limiter—builds boost at standstill. Activate with clutch+throttle.',
    professionalDetails: 'Secondary RPM limit below main. Soft ignition cut. Combine with boost target for launch boost.',
    applicableConfigs: ['single-turbo', 'parallel-twin', 'sequential-twin', 'supercharger', 'twin-charger', 'staged-boost'],
    icon: '🏁',
  },
];

// Calculate setup completion percentage
export function calculateSetupCompletion(
  config: ForcedInductionType,
  completedTables: string[],
  completedParams: string[]
): number {
  const requiredTables = BOOST_TABLE_DEFINITIONS
    .filter(t => t.applicableConfigs.includes(config))
    .map(t => t.id);
  
  const requiredParams = BOOST_PARAMETERS
    .filter(p => p.applicableConfigs.includes(config))
    .map(p => p.id);

  const tableCompletion = completedTables.length / requiredTables.length;
  const paramCompletion = completedParams.length / requiredParams.length;

  return Math.round(((tableCompletion + paramCompletion) / 2) * 100);
}
