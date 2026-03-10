// COMPREHENSIVE SENSOR DATABASE FOR SPEEDUINO & MEGASQUIRT ECUs

export interface TriggerPattern {
  id: string;
  name: string;
  description: string;
  crankTeeth: string;
  camTeeth: string;
  sequential: boolean;
  resolution: string;
  maxRPM: number;
  difficulty: 'Easy' | 'Medium' | 'Advanced';
  speeduino: boolean;
  megasquirt: boolean;
  notes: string;
}

export interface SensorType {
  id: string;
  name: string;
  manufacturer: string;
  partNumber?: string;
  type: 'Hall' | 'VR' | 'Analog' | 'NTC';
  voltage: string;
  range: string;
  notes: string;
}

export interface CalibrationCurve {
  id: string;
  name: string;
  manufacturer: string;
  sensorType: 'CLT' | 'IAT' | 'TPS' | 'MAP' | 'O2';
  points: Array<{ input: number; output: number }>;
  inputUnit: string;
  outputUnit: string;
}

// TRIGGER PATTERNS DATABASE
export const triggerPatterns: TriggerPattern[] = [
  // UNIVERSAL PATTERNS
  {
    id: 'missing-60-2',
    name: '60-2 Missing Tooth',
    description: '60 teeth with 2 missing - Ford standard, excellent resolution',
    crankTeeth: '60 (58 teeth)',
    camTeeth: 'Optional single pulse',
    sequential: true,
    resolution: '6° per tooth',
    maxRPM: 15000,
    difficulty: 'Easy',
    speeduino: true,
    megasquirt: true,
    notes: 'Most popular aftermarket pattern. Excellent for high RPM.'
  },
  {
    id: 'missing-36-1',
    name: '36-1 Missing Tooth',
    description: '36 teeth with 1 missing - Very popular aftermarket',
    crankTeeth: '36 (35 teeth)',
    camTeeth: 'Optional single pulse',
    sequential: true,
    resolution: '10° per tooth',
    maxRPM: 12000,
    difficulty: 'Easy',
    speeduino: true,
    megasquirt: true,
    notes: 'Excellent balance of resolution and reliability.'
  },
  {
    id: 'missing-24-1',
    name: '24-1 Missing Tooth',
    description: '24 teeth with 1 missing - Good for 4-6 cylinder',
    crankTeeth: '24 (23 teeth)',
    camTeeth: 'Optional single pulse',
    sequential: true,
    resolution: '15° per tooth',
    maxRPM: 10000,
    difficulty: 'Easy',
    speeduino: true,
    megasquirt: true,
    notes: 'Adequate resolution for most street applications.'
  },
  {
    id: 'missing-12-1',
    name: '12-1 Missing Tooth',
    description: '12 teeth with 1 missing - Basic applications',
    crankTeeth: '12 (11 teeth)',
    camTeeth: 'Optional single pulse',
    sequential: true,
    resolution: '30° per tooth',
    maxRPM: 7000,
    difficulty: 'Easy',
    speeduino: true,
    megasquirt: true,
    notes: 'Minimum recommended for most engines.'
  },
  {
    id: 'dual-36-1',
    name: 'Dual Wheel 36+1',
    description: '36 evenly-spaced crank teeth + 1 cam pulse',
    crankTeeth: '36 teeth',
    camTeeth: '1 pulse',
    sequential: true,
    resolution: '10° per tooth',
    maxRPM: 12000,
    difficulty: 'Medium',
    speeduino: true,
    megasquirt: true,
    notes: 'No missing teeth on crank wheel. Cam provides sync.'
  },
  
  // OEM FORD PATTERNS
  {
    id: 'ford-edis',
    name: 'Ford EDIS (36-1)',
    description: 'Ford 36-1 with EDIS module',
    crankTeeth: '36-1 VR wheel',
    camTeeth: 'Optional cam',
    sequential: true,
    resolution: '10° per tooth',
    maxRPM: 12000,
    difficulty: 'Easy',
    speeduino: true,
    megasquirt: true,
    notes: 'EDIS module provides clean PIP signal to ECU. Very reliable.'
  },
  
  // OEM GM PATTERNS
  {
    id: 'gm-7x',
    name: 'GM 7X (LT1)',
    description: '7 evenly-spaced teeth + cam sensor',
    crankTeeth: '7 teeth',
    camTeeth: '1 pulse',
    sequential: true,
    resolution: '~51° per tooth',
    maxRPM: 7000,
    difficulty: 'Medium',
    speeduino: true,
    megasquirt: true,
    notes: 'Common on 90s GM V6/V8. Requires DIS module or decoder.'
  },
  {
    id: 'gm-24x',
    name: 'GM 24X (LS1/LS6)',
    description: '24 teeth with pattern + cam pulse',
    crankTeeth: '24 pattern',
    camTeeth: '1 pulse',
    sequential: true,
    resolution: '15° average',
    maxRPM: 10000,
    difficulty: 'Medium',
    speeduino: true,
    megasquirt: true,
    notes: 'LS1/LS6 1997-2005. Can configure as dual wheel 24+1.'
  },
  {
    id: 'gm-58x',
    name: 'GM 58X (LS2+)',
    description: '58 evenly-spaced teeth + 4-tooth cam',
    crankTeeth: '58 teeth',
    camTeeth: '4-tooth pattern',
    sequential: true,
    resolution: '~6° per tooth',
    maxRPM: 12000,
    difficulty: 'Advanced',
    speeduino: false,
    megasquirt: true,
    notes: 'LS2+ Gen IV/V. MS3 only. Complex pattern.'
  },
  
  // OEM NISSAN PATTERNS
  {
    id: 'nissan-360',
    name: 'Nissan 360 (CAS)',
    description: '360 optical slits + variable inner windows',
    crankTeeth: '360 slits',
    camTeeth: 'Pattern 3 (unique windows)',
    sequential: true,
    resolution: '1° per tooth',
    maxRPM: 15000,
    difficulty: 'Advanced',
    speeduino: true,
    megasquirt: true,
    notes: 'SR20, CA18, RB series. Pattern 3 only (unique window sizes).'
  },
  
  // OEM HONDA PATTERNS
  {
    id: 'honda-d17',
    name: 'Honda D17',
    description: '12 crank teeth + 1 longer, 4 cam teeth + 1',
    crankTeeth: '12+1',
    camTeeth: '4+1',
    sequential: true,
    resolution: '30° per tooth',
    maxRPM: 8000,
    difficulty: 'Medium',
    speeduino: true,
    megasquirt: true,
    notes: 'D17 engines 2001-2005. Crank-only mode available.'
  },
  
  // OEM MAZDA PATTERNS
  {
    id: 'mazda-4tooth',
    name: 'Mazda 4-Tooth (CAS)',
    description: '4 teeth with varying spacing in distributor',
    crankTeeth: '4 pattern',
    camTeeth: 'N/A',
    sequential: false,
    resolution: '~90° per tooth',
    maxRPM: 7000,
    difficulty: 'Medium',
    speeduino: true,
    megasquirt: true,
    notes: 'Miata NA/NB, Mazda 323. Wasted spark only.'
  },
  
  // BASIC DISTRIBUTOR
  {
    id: 'basic-distributor',
    name: 'Basic Distributor',
    description: 'Simple pulse per cylinder from distributor',
    crankTeeth: 'N/A',
    camTeeth: '1 pulse per cylinder',
    sequential: false,
    resolution: 'Varies by cylinder count',
    maxRPM: 7000,
    difficulty: 'Easy',
    speeduino: true,
    megasquirt: true,
    notes: 'Batch fire injection only. Wasted spark ignition.'
  },
];

// HALL EFFECT SENSORS
export const hallSensors: SensorType[] = [
  {
    id: 'honeywell-1gt101dc',
    name: 'Honeywell 1GT101DC',
    manufacturer: 'Honeywell',
    partNumber: '1GT101DC',
    type: 'Hall',
    voltage: '5-24V DC',
    range: 'Up to 100kHz',
    notes: 'Gear tooth sensor. Air gap: 0.5-2mm. Temperature: -40°C to 125°C'
  },
  {
    id: 'allegro-a1101',
    name: 'Allegro A1101/A1102',
    manufacturer: 'Allegro',
    partNumber: 'A1101/A1102',
    type: 'Hall',
    voltage: '5V',
    range: 'Digital output',
    notes: 'Common aftermarket. Requires 1kΩ pull-up resistor.'
  },
  {
    id: 'diyautotune-hall',
    name: 'DIYAutoTune Hall Sensor',
    manufacturer: 'DIYAutoTune',
    partNumber: 'Various',
    type: 'Hall',
    voltage: '5-12V',
    range: 'M12x1.0 threaded',
    notes: 'Pre-calibrated for Speeduino/MS. Air gap: 1mm optimal.'
  },
];

// VR SENSORS
export const vrSensors: SensorType[] = [
  {
    id: 'ford-explorer-vr',
    name: 'Ford Explorer VR Sensor',
    manufacturer: 'Ford',
    partNumber: '4.0L SOHC',
    type: 'VR',
    voltage: 'Self-generating (passive)',
    range: '1-100V AC (RPM dependent)',
    notes: 'Very popular aftermarket. Requires VR conditioner (MAX9926). Air gap: 0.25-0.50mm CRITICAL'
  },
  {
    id: 'gm-7x-vr',
    name: 'GM 7X VR Sensor',
    manufacturer: 'GM',
    partNumber: 'LT1/Various',
    type: 'VR',
    voltage: 'Self-generating',
    range: 'Variable with RPM',
    notes: 'Common on 90s GM. Polarity matters! Use with MAX9926 conditioner.'
  },
  {
    id: 'bosch-2pin-vr',
    name: 'Bosch 2-Pin VR',
    manufacturer: 'Bosch',
    partNumber: '0 261 210 XXX',
    type: 'VR',
    voltage: 'Passive',
    range: 'AC sine wave output',
    notes: 'Generic VR sensor. Requires signal conditioning. Check polarity.'
  },
];

// CLT/IAT CALIBRATION CURVES
export const cltCurves: CalibrationCurve[] = [
  {
    id: 'gm-clt',
    name: 'GM IAT/CLT',
    manufacturer: 'GM',
    sensorType: 'CLT',
    inputUnit: '°C',
    outputUnit: 'Ω',
    points: [
      { input: -40, output: 100700 },
      { input: -20, output: 35900 },
      { input: 0, output: 15800 },
      { input: 20, output: 7500 },
      { input: 40, output: 3520 },
      { input: 60, output: 1800 },
      { input: 80, output: 1000 },
      { input: 100, output: 586 },
      { input: 120, output: 355 },
    ]
  },
  {
    id: 'ford-clt',
    name: 'Ford CLT',
    manufacturer: 'Ford',
    sensorType: 'CLT',
    inputUnit: '°C',
    outputUnit: 'Ω',
    points: [
      { input: -40, output: 95000 },
      { input: 0, output: 16150 },
      { input: 20, output: 7900 },
      { input: 40, output: 3840 },
      { input: 80, output: 1180 },
      { input: 100, output: 680 },
    ]
  },
  {
    id: 'bosch-clt',
    name: 'Bosch CLT (European)',
    manufacturer: 'Bosch',
    sensorType: 'CLT',
    inputUnit: '°C',
    outputUnit: 'Ω',
    points: [
      { input: -20, output: 15000 },
      { input: 0, output: 6500 },
      { input: 20, output: 3000 },
      { input: 40, output: 1500 },
      { input: 80, output: 300 },
      { input: 100, output: 177 },
    ]
  },
  {
    id: 'toyota-clt',
    name: 'Toyota CLT',
    manufacturer: 'Toyota',
    sensorType: 'CLT',
    inputUnit: '°C',
    outputUnit: 'Ω',
    points: [
      { input: -20, output: 14600 },
      { input: 0, output: 5900 },
      { input: 20, output: 2500 },
      { input: 40, output: 1150 },
      { input: 80, output: 310 },
      { input: 100, output: 177 },
    ]
  },
];

// MAP SENSOR DATABASE
export const mapSensors: SensorType[] = [
  {
    id: 'gm-1bar',
    name: 'GM 1-Bar MAP (NA Only)',
    manufacturer: 'GM',
    partNumber: '16137039, 25084465',
    type: 'Analog',
    voltage: '0.5-4.5V',
    range: '10-100 kPa',
    notes: 'Naturally aspirated only. Cannot read boost. Highest NA resolution.'
  },
  {
    id: 'gm-2bar',
    name: 'GM 2-Bar MAP',
    manufacturer: 'GM',
    partNumber: '12247571, 16040749',
    type: 'Analog',
    voltage: '0.5-4.5V',
    range: '10-200 kPa (14.7 psi boost)',
    notes: 'Good for moderate boost applications. Turbo Buick standard.'
  },
  {
    id: 'gm-3bar',
    name: 'GM 3-Bar MAP (Most Popular)',
    manufacturer: 'GM',
    partNumber: '12223861, 12614973',
    type: 'Analog',
    voltage: '0.5-4.5V',
    range: '10-300 kPa (29 psi boost)',
    notes: 'Gold standard for aftermarket turbo. Excellent reliability. ~$30-50'
  },
  {
    id: 'speeduino-2.5bar',
    name: 'Speeduino Onboard 2.5-Bar',
    manufacturer: 'Freescale/NXP',
    partNumber: 'MPX4250AP',
    type: 'Analog',
    voltage: '0.5-4.5V',
    range: '10-250 kPa (22 psi boost)',
    notes: 'Integrated on Speeduino PCB. Most versatile for street/performance.'
  },
  {
    id: 'speeduino-4bar',
    name: 'Speeduino Onboard 4-Bar',
    manufacturer: 'Freescale/NXP',
    partNumber: 'MPXH6400A',
    type: 'Analog',
    voltage: '0.5-4.5V',
    range: '10-400 kPa (43.5 psi boost)',
    notes: 'High-boost option. Drag racing and competition builds.'
  },
  {
    id: 'bosch-3.5bar',
    name: 'Bosch 3.5-Bar MAP',
    manufacturer: 'Bosch',
    partNumber: '0 261 230 013',
    type: 'Analog',
    voltage: '0.5-4.5V',
    range: '10-350 kPa (36 psi boost)',
    notes: 'European turbo diesels. High boost capability.'
  },
];

// TPS SENSORS
export const tpsSensors: SensorType[] = [
  {
    id: 'gm-3wire-tps',
    name: 'GM 3-Wire TPS',
    manufacturer: 'GM',
    partNumber: 'Various',
    type: 'Analog',
    voltage: '5V reference',
    range: '0.5-4.5V (0-100%)',
    notes: 'Most popular. 5kΩ potentiometer. Bolts to many throttle bodies.'
  },
  {
    id: 'ford-tps',
    name: 'Ford TPS',
    manufacturer: 'Ford',
    partNumber: '5.0L Mustang',
    type: 'Analog',
    voltage: '5V reference',
    range: '0.5-4.5V',
    notes: 'Similar to GM. Different connector. 5kΩ potentiometer.'
  },
  {
    id: 'bosch-tps',
    name: 'Bosch Universal TPS',
    manufacturer: 'Bosch',
    partNumber: '0 280 122 001',
    type: 'Analog',
    voltage: '5V reference',
    range: '0-5V',
    notes: 'Universal sensor. Quality construction. May need adapter bracket.'
  },
];

// O2/WIDEBAND SENSORS
export const o2Sensors: SensorType[] = [
  {
    id: 'bosch-lsu-4.9',
    name: 'Bosch LSU 4.9',
    manufacturer: 'Bosch',
    partNumber: '0 258 017 025',
    type: 'Analog',
    voltage: 'Controller dependent',
    range: 'Lambda 0.65-1.6 (AFR 9.5-23.5)',
    notes: 'Most common wideband. Requires controller. Fast response. ~$80'
  },
  {
    id: 'bosch-lsu-4.2',
    name: 'Bosch LSU 4.2',
    manufacturer: 'Bosch',
    partNumber: '0 258 007 057',
    type: 'Analog',
    voltage: 'Controller dependent',
    range: 'Lambda 0.7-1.7',
    notes: 'Older wideband. Still reliable. Slightly slower than 4.9.'
  },
  {
    id: 'ntk-l2h2',
    name: 'NTK L2H2',
    manufacturer: 'NTK/NGK',
    partNumber: 'L2H2',
    type: 'Analog',
    voltage: 'Controller dependent',
    range: 'Lambda 0.68-1.36',
    notes: 'Alternative to Bosch. Different controller required. Good quality.'
  },
];

// RECOMMENDED SENSOR COMBINATIONS
export interface SensorConfig {
  name: string;
  description: string;
  difficulty: 'Beginner' | 'Intermediate' | 'Advanced';
  clt: string;
  iat: string;
  tps: string;
  map: string;
  o2: string;
  primaryTrigger: string;
  secondaryTrigger: string;
  notes: string;
}

export const recommendedConfigs: SensorConfig[] = [
  {
    name: 'Street NA (Naturally Aspirated)',
    description: 'Budget-friendly setup for naturally aspirated engines',
    difficulty: 'Beginner',
    clt: 'GM CLT (PN: 12146312)',
    iat: 'GM IAT (same as CLT)',
    tps: 'GM 3-Wire TPS',
    map: 'GM 1-Bar or Speeduino Onboard',
    o2: 'Bosch LSU 4.9 + AEM UEGO',
    primaryTrigger: '36-1 Missing Tooth (Hall)',
    secondaryTrigger: 'Single cam pulse (Hall)',
    notes: 'Total cost ~$200-300 in sensors. Reliable and well-documented.'
  },
  {
    name: 'Street Turbo (Mild Boost)',
    description: 'Street turbo setup for up to 15 psi boost',
    difficulty: 'Intermediate',
    clt: 'GM CLT',
    iat: 'GM IAT (post-intercooler)',
    tps: 'GM TPS',
    map: 'GM 2-Bar or Speeduino 2.5-Bar',
    o2: 'Bosch LSU 4.9 + Controller',
    primaryTrigger: '60-2 Missing Tooth (VR with MAX9926)',
    secondaryTrigger: 'Single cam pulse (Hall)',
    notes: 'Good resolution and boost capability. VR for reliability.'
  },
  {
    name: 'High Performance Turbo',
    description: 'Race/competition turbo setup for 20+ psi',
    difficulty: 'Advanced',
    clt: 'GM CLT',
    iat: 'GM IAT (post-IC, pre-TB)',
    tps: 'GM TPS',
    map: 'GM 3-Bar or Speeduino 4-Bar',
    o2: 'Bosch LSU 4.9 + AEM X-Series',
    primaryTrigger: '60-2 Missing Tooth (Hall, 0.5mm air gap)',
    secondaryTrigger: 'Single cam pulse (Hall)',
    notes: 'Maximum resolution. High boost capable. Professional installation recommended.'
  },
  {
    name: 'OEM Ford EDIS Retrofit',
    description: 'Using existing Ford EDIS system',
    difficulty: 'Beginner',
    clt: 'GM CLT or Ford OEM',
    iat: 'GM IAT',
    tps: 'Ford TPS',
    map: 'GM 2-Bar',
    o2: 'Bosch LSU 4.9',
    primaryTrigger: 'Ford EDIS 36-1 (VR via EDIS module)',
    secondaryTrigger: 'Optional cam sensor',
    notes: 'EDIS module simplifies trigger setup. Very reliable. Good for beginners.'
  },
];
