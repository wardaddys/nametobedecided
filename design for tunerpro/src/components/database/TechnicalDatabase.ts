// COMPREHENSIVE TECHNICAL DATABASE FOR ECU TUNING SOFTWARE

export interface TechnicalTerm {
  id: string;
  category: string;
  title: string;
  shortDescription: string;
  fullDescription: string;
  characteristics?: string[];
  tuningFocus?: string[];
  tablesRequired?: string[];
  advantages?: string[];
  disadvantages?: string[];
  typicalValues?: { [key: string]: string };
  relatedTerms?: string[];
}

export const TECHNICAL_DATABASE: { [key: string]: TechnicalTerm } = {
  // ENGINE TYPE DEFINITIONS
  'ENG-001-NA': {
    id: 'ENG-001-NA',
    category: 'Engine Configuration',
    title: 'Naturally Aspirated (NA)',
    shortDescription: 'Engine relying solely on atmospheric pressure for air intake',
    fullDescription: 'Engine that relies solely on atmospheric pressure for air intake without forced induction. Features linear power delivery and naturally breathing characteristics without boost pressure.',
    characteristics: [
      'Linear power delivery',
      'Naturally breathing',
      'No boost pressure',
      'Altitude sensitive',
    ],
    tuningFocus: [
      'Volumetric efficiency optimization',
      'Cam timing',
      'VE tables only',
      'Throttle response',
    ],
    tablesRequired: [
      'VE Table (Primary/Secondary)',
      'Ignition Timing',
      'AFR Target',
    ],
    advantages: [
      'Simplicity and reliability',
      'Predictable behavior',
      'Lower heat generation',
      'Minimal complexity',
    ],
    disadvantages: [
      'Lower power density',
      'Altitude sensitivity',
      'Limited tuning range',
    ],
    relatedTerms: ['VE-001', 'IGN-001'],
  },
  
  'ENG-001-TURBO': {
    id: 'ENG-001-TURBO',
    category: 'Engine Configuration',
    title: 'Turbocharged',
    shortDescription: 'Forced induction using exhaust gas energy',
    fullDescription: 'Forced induction system using exhaust gas energy to compress intake air. Features exponential power delivery, boost pressure dependent performance, and turbo lag characteristics.',
    characteristics: [
      'Exponential power delivery',
      'Boost pressure dependent',
      'Turbo lag present',
      'High thermal loads',
    ],
    tuningFocus: [
      'Boost control',
      'Fueling enrichment',
      'Timing retard under boost',
      'Compressor efficiency',
      'Wastegate duty cycle',
    ],
    tablesRequired: [
      'All VE tables',
      'Boost Target',
      'Boost Compensation',
      'Wastegate Duty Cycle',
      'Overboost Protection',
    ],
    advantages: [
      'High power density',
      'Efficiency at cruise',
      'Adjustable power levels',
      'Excellent power-to-weight',
    ],
    disadvantages: [
      'Turbo lag',
      'Heat management complexity',
      'Complex control systems',
    ],
    typicalValues: {
      'Target Boost (Street)': '7-15 PSI',
      'Target Boost (Race)': '20-35 PSI',
      'Max Boost Limit': '10% above target',
      'Wastegate Base Duty': '30-40%',
    },
    relatedTerms: ['BOOST-001', 'WG-001'],
  },

  'ENG-001-SUPER': {
    id: 'ENG-001-SUPER',
    category: 'Engine Configuration',
    title: 'Supercharged',
    shortDescription: 'Mechanically driven forced induction',
    fullDescription: 'Forced induction mechanically driven by engine crankshaft via belt. Features instant boost response, linear power delivery, and parasitic power consumption.',
    characteristics: [
      'Instant boost response',
      'Linear power delivery',
      'Parasitic drag',
      'No lag',
    ],
    tuningFocus: [
      'Drive ratio management',
      'Bypass valve control',
      'IAT management',
      'Belt slip prevention',
    ],
    tablesRequired: [
      'VE tables',
      'Boost Target',
      'Bypass Control',
      'Pulley Ratio Compensation',
    ],
    advantages: [
      'No lag',
      'Linear response',
      'Simple control',
      'Predictable behavior',
    ],
    disadvantages: [
      'Parasitic power loss',
      'Fixed boost curve',
      'Heat generation',
      'Limited boost range',
    ],
    typicalValues: {
      'Typical Boost': '6-12 PSI',
      'Drive Ratio': '1.5:1 to 3.0:1',
      'Bypass Threshold': '70-80% TPS',
    },
  },

  // VALVE TRAIN SYSTEMS
  'VLV-001-VTEC': {
    id: 'VLV-001-VTEC',
    category: 'Valve Train',
    title: 'VTEC (Variable Valve Timing and Lift Electronic Control)',
    shortDescription: 'Honda discrete cam profile switching system',
    fullDescription: 'Hydraulic pin engagement system that switches between two cam profiles for optimized performance across RPM range. Low-speed profile provides economy and emissions, high-speed profile maximizes power.',
    characteristics: [
      'Discrete profile switching',
      'Hydraulic pin actuation',
      'Dual cam lobes',
      'RPM-based engagement',
    ],
    tuningFocus: [
      'Engagement point optimization',
      'Dual map tuning required',
      'Transition smoothness',
      'Oil pressure monitoring',
    ],
    tablesRequired: [
      'Primary VE Table (Low-Speed)',
      'Secondary VE Table (High-Speed)',
      'Low-Speed Ignition Map',
      'High-Speed Ignition Map',
      'Low-Speed AFR Target',
      'High-Speed AFR Target',
    ],
    advantages: [
      'Best of both worlds',
      'Excellent fuel economy at cruise',
      'High power at high RPM',
      'Proven reliability',
    ],
    disadvantages: [
      'Abrupt transition',
      'Requires dual tuning',
      'Oil pressure dependent',
    ],
    typicalValues: {
      'Engagement RPM': '4500-6000 RPM',
      'Hysteresis': '200-400 RPM',
      'Min CLT': '60°C',
      'Min TPS': '70%',
      'Min Oil Pressure': '30-40 PSI',
      'Low Lift': '8-9mm intake',
      'High Lift': '10-12mm intake',
    },
    relatedTerms: ['OIL-001', 'VE-001'],
  },

  'VLV-001-VVT': {
    id: 'VLV-001-VVT',
    category: 'Valve Train',
    title: 'VVT (Variable Valve Timing)',
    shortDescription: 'Continuous cam phaser system',
    fullDescription: 'Oil pressure actuated vane-type phaser that continuously adjusts cam timing. Provides optimized timing across entire RPM range for improved torque curve and emissions.',
    characteristics: [
      'Continuous adjustment',
      'Oil pressure actuated',
      'Vane-type phaser',
      'Smooth operation',
    ],
    tuningFocus: [
      'Target advance table tuning',
      'PID control gains',
      'Slew rate limits',
      'Response optimization',
    ],
    tablesRequired: [
      'Target Advance Table (RPM vs Load)',
      'VVT PID Settings',
    ],
    advantages: [
      'Smooth continuous adjustment',
      'Optimized across all RPM',
      'Better emissions',
      'Improved torque curve',
    ],
    typicalValues: {
      'Adjustment Range': '30-50° crank angle',
      'Response Time': '200-500ms',
      'P Gain': '0.5-2.0',
      'I Gain': '0.05-0.2',
    },
  },

  // FUEL SYSTEM CONFIGURATIONS
  'FUEL-001-PORT-SINGLE': {
    id: 'FUEL-001-PORT-SINGLE',
    category: 'Fuel System',
    title: 'Port Injection (Single Stage)',
    shortDescription: 'Single set of injectors in intake ports',
    fullDescription: 'Single set of fuel injectors located in intake manifold ports, synchronized with intake valve events. Simple, proven system with good atomization.',
    characteristics: [
      'Single injector set',
      'Port mounted',
      'Synchronized timing',
      'PWM control',
    ],
    tuningFocus: [
      'Injector dead time',
      'Pulse width accuracy',
      'Fuel pressure stability',
      'Battery compensation',
    ],
    advantages: [
      'Simple and proven',
      'Good atomization',
      'Self-cooling of intake',
      'Easy to tune',
    ],
    disadvantages: [
      'Wall wetting',
      'Limited mixture control',
    ],
    typicalValues: {
      'Fuel Pressure (Return)': '43.5 PSI / 3 Bar',
      'Fuel Pressure (Returnless)': '58 PSI / 4 Bar',
      'Dead Time @ 13.5V': '0.8-1.2ms',
      'Max Duty Cycle': '85%',
    },
  },

  'FUEL-001-PORT-STAGED': {
    id: 'FUEL-001-PORT-STAGED',
    category: 'Fuel System',
    title: 'Port Injection (Staged)',
    shortDescription: 'Primary and secondary injector sets',
    fullDescription: 'Primary and secondary injector sets activated based on load. Primary injectors active at all times, secondary injectors activated above threshold for high load conditions.',
    characteristics: [
      'Dual injector sets',
      'Load-based activation',
      'Staged transition',
      'Extended capacity',
    ],
    tuningFocus: [
      'Staging point optimization',
      'Transition smoothing',
      'Primary/secondary balance',
      'Duty cycle management',
    ],
    tablesRequired: [
      'Primary Injector VE Table',
      'Secondary Injector VE Table',
      'Staging Threshold Map',
      'Transition Smoothing',
    ],
    advantages: [
      'Extended capacity',
      'Good idle quality',
      'High power capability',
    ],
    disadvantages: [
      'Complex tuning',
      'Transition challenges',
    ],
    typicalValues: {
      'Primary Capacity': 'Idle to 80% duty',
      'Staging Point': '60-80% load',
      'Transition Duration': '0.5-2.0s',
    },
  },

  // CONTROL STRATEGIES
  'CTL-001-CLOSED': {
    id: 'CTL-001-CLOSED',
    category: 'Control Strategy',
    title: 'Closed Loop Fuel Control',
    shortDescription: 'Feedback-based fuel control using O2 sensor',
    fullDescription: 'Feedback control system where ECU continuously adjusts fuel delivery based on measured AFR from wideband O2 sensor to achieve target AFR. Uses PID control algorithm for precise correction.',
    characteristics: [
      'Continuous feedback',
      'O2 sensor based',
      'PID control',
      'Self-correcting',
    ],
    tuningFocus: [
      'PID gain tuning',
      'Authority limits',
      'Enable conditions',
      'Target AFR table',
    ],
    advantages: [
      'Self-correcting',
      'Compensates for drift',
      'Adapts to conditions',
      'Precise AFR control',
    ],
    disadvantages: [
      'Sensor latency (200-500ms)',
      'Cannot handle rapid transients',
      'Requires functioning sensor',
      'Can mask tuning errors',
    ],
    typicalValues: {
      'P Gain': '0.3-0.5',
      'I Gain': '0.05-0.1',
      'D Gain': '0.01-0.05',
      'Authority Limit': '±15-25%',
      'Update Rate': '10-20 Hz',
      'Enable CLT': '>70°C',
      'Enable RPM Range': '800-4000 RPM',
      'Disable TPS': '>70%',
    },
    relatedTerms: ['O2-001', 'CTL-001-OPEN'],
  },

  'CTL-001-OPEN': {
    id: 'CTL-001-OPEN',
    category: 'Control Strategy',
    title: 'Open Loop Fuel Control',
    shortDescription: 'Feed-forward control without feedback',
    fullDescription: 'Feed-forward control system where ECU delivers fuel based solely on programmed tables without sensor feedback. Critical for transient conditions and WOT operation.',
    characteristics: [
      'No feedback correction',
      'Table-based only',
      'Immediate response',
      'Predictable behavior',
    ],
    tuningFocus: [
      'VE table accuracy critical',
      'Temperature compensation',
      'Transient enrichment',
      'All compensations must be tuned',
    ],
    advantages: [
      'Immediate response',
      'Predictable',
      'No sensor delay',
      'Works for all conditions',
    ],
    disadvantages: [
      'No self-correction',
      'Sensitive to accuracy',
      'Requires precise tuning',
      'Aging affects accuracy',
    ],
    relatedTerms: ['VE-001', 'CTL-001-CLOSED'],
  },

  // SENSORS
  'SEN-001-MAP': {
    id: 'SEN-001-MAP',
    category: 'Sensors',
    title: 'MAP (Manifold Absolute Pressure) Sensor',
    shortDescription: 'Measures absolute air pressure in intake manifold',
    fullDescription: 'Piezoelectric or capacitive pressure transducer that measures absolute air pressure in intake manifold. Primary load axis for speed-density tuning strategy.',
    characteristics: [
      'Analog voltage output',
      '3-wire connection',
      'Linear response',
      'Absolute pressure measurement',
    ],
    tuningFocus: [
      'Sensor calibration',
      'Range selection',
      'Primary load axis',
    ],
    advantages: [
      'Direct pressure measurement',
      'Fast response',
      'Reliable',
      'Inexpensive',
    ],
    typicalValues: {
      'NA Range': '10-105 kPa',
      'Turbo Range (2-Bar)': '10-200 kPa',
      'Turbo Range (3-Bar)': '10-300 kPa',
      'Signal Range': '0.5-4.5V',
      'Atmospheric Pressure': '~100 kPa @ sea level',
    },
    relatedTerms: ['VE-001', 'BOOST-001'],
  },

  'SEN-002-TPS': {
    id: 'SEN-002-TPS',
    category: 'Sensors',
    title: 'TPS (Throttle Position Sensor)',
    shortDescription: 'Reports throttle blade angle',
    fullDescription: 'Potentiometer that varies resistance as throttle shaft rotates, providing voltage output proportional to throttle angle. Used for acceleration enrichment, load calculation in Alpha-N tuning, and transient fuel compensation.',
    characteristics: [
      'Potentiometer based',
      'Analog voltage',
      '3-wire connection',
      'Linear response',
    ],
    tuningFocus: [
      'Calibration (closed/WOT)',
      'Acceleration enrichment trigger',
      'Alpha-N load axis',
    ],
    typicalValues: {
      'Closed Throttle': '0.5-0.8V',
      'Wide Open Throttle': '4.2-4.7V',
      'CL Disable Threshold': '>70%',
      'VTEC Enable': '>70%',
    },
    relatedTerms: ['ACCEL-001'],
  },

  'SEN-003-IAT': {
    id: 'SEN-003-IAT',
    category: 'Sensors',
    title: 'IAT (Intake Air Temperature) Sensor',
    shortDescription: 'Measures intake air temperature',
    fullDescription: 'NTC thermistor that measures temperature of intake air. Used for air density compensation - warmer air is less dense and requires less fuel.',
    characteristics: [
      'NTC thermistor',
      'Resistance decreases with temp',
      'Voltage divider circuit',
      '2-wire passive',
    ],
    tuningFocus: [
      'Air density compensation',
      'Ignition timing adjustment',
      'Installation location critical',
    ],
    typicalValues: {
      'Operating Range': '-40°C to 150°C',
      'Compensation': '-0.5% fuel per 5°C above reference',
      'Typical @ 20°C': '~3.5V',
      'Typical @ 80°C': '~1.5V',
    },
    relatedTerms: ['SEN-004-CLT', 'COMP-001'],
  },

  'SEN-004-CLT': {
    id: 'SEN-004-CLT',
    category: 'Sensors',
    title: 'CLT (Coolant Temperature Sensor)',
    shortDescription: 'Measures engine coolant temperature',
    fullDescription: 'NTC thermistor mounted in coolant passage. Critical for cold start enrichment, warmup enrichment, closed-loop enable, fan control, and VTEC engagement enable condition.',
    characteristics: [
      'NTC thermistor',
      'Coolant passage mounted',
      '2-wire passive',
      'Voltage divider circuit',
    ],
    tuningFocus: [
      'Cold start enrichment',
      'Warmup enrichment curve',
      'Ignition compensation',
      'Enable conditions',
    ],
    typicalValues: {
      'Operating Temp': '85-95°C (185-203°F)',
      'CL Enable': '>70°C',
      'VTEC Enable': '>60°C',
      'Overtemp Warning': '105-110°C',
      'Overtemp Shutdown': '115°C',
    },
    relatedTerms: ['COLD-001', 'VLV-001-VTEC'],
  },

  'SEN-005-O2': {
    id: 'SEN-005-O2',
    category: 'Sensors',
    title: 'O2/Lambda Sensor (Wideband)',
    shortDescription: 'Measures exhaust oxygen for AFR calculation',
    fullDescription: 'Wideband oxygen sensor using zirconia cell to measure exhaust gas oxygen content for continuous AFR measurement from 10:1 to 20:1+. Essential for tuning and closed-loop control.',
    characteristics: [
      'Zirconia cell',
      'Continuous measurement',
      'Heated element',
      'Wide range (10-20+ AFR)',
    ],
    tuningFocus: [
      'Real-time AFR feedback',
      'Closed-loop input',
      'Safety monitoring',
      'Dyno tuning',
    ],
    advantages: [
      'Precise AFR measurement',
      'Wide range',
      'Fast response',
      'Essential for tuning',
    ],
    typicalValues: {
      'Stoich Lambda': '1.00 (14.7:1 gasoline)',
      'Idle/Cruise': 'λ 1.00 (14.7:1)',
      'Light Load': 'λ 1.00-1.05 (14.7-15.4:1)',
      'Full Load NA': 'λ 0.85-0.90 (12.5-13.2:1)',
      'Full Load Boost': 'λ 0.80-0.85 (11.8-12.5:1)',
      'High Boost': 'λ 0.75-0.80 (11.0-11.8:1)',
      'Replacement Interval': '30,000-50,000 miles',
    },
    relatedTerms: ['CTL-001-CLOSED', 'AFR-001'],
  },

  'SEN-006-KNOCK': {
    id: 'SEN-006-KNOCK',
    category: 'Sensors',
    title: 'Knock Sensor',
    shortDescription: 'Detects engine detonation via vibration',
    fullDescription: 'Piezoelectric crystal generates voltage when mechanical vibration occurs, tuned to detect 5-10 kHz frequency band associated with engine knock/detonation. Critical safety feature for boosted engines.',
    characteristics: [
      'Piezoelectric crystal',
      'Vibration sensing',
      '5-10 kHz frequency',
      'AC voltage spike output',
    ],
    tuningFocus: [
      'Sensitivity adjustment',
      'Retard amount',
      'Recovery rate',
      'Logging for analysis',
    ],
    advantages: [
      'Prevents catastrophic damage',
      'Automatic protection',
      'Real-time response',
    ],
    typicalValues: {
      'Initial Retard': '2-4°',
      'Max Retard': '8-10°',
      'Recovery Rate': '0.5-1° per cycle',
      'Frequency Range': '5-10 kHz',
    },
    relatedTerms: ['IGN-001', 'BOOST-001'],
  },

  // BOOST CONTROL
  'BOOST-001': {
    id: 'BOOST-001',
    category: 'Boost Control',
    title: 'Boost Control System',
    shortDescription: 'Electronic wastegate control for boost pressure',
    fullDescription: 'Electronic control of wastegate duty cycle to achieve target boost pressure. Uses closed-loop PID control to modulate wastegate position based on measured vs target boost.',
    characteristics: [
      'PWM wastegate control',
      'PID feedback control',
      'Target boost table',
      'Overboost protection',
    ],
    tuningFocus: [
      'Target boost table (RPM vs Gear/Load)',
      'PID gains',
      'Base duty cycle',
      'Overboost limits',
    ],
    typicalValues: {
      'Base Duty': '30-40%',
      'P Gain': '2.0-5.0',
      'I Gain': '0.5-2.0',
      'Overboost': 'Target + 10%',
      'Boost Cut': 'Target + 15%',
    },
    relatedTerms: ['ENG-001-TURBO', 'WG-001'],
  },

  // VE (VOLUMETRIC EFFICIENCY)
  'VE-001': {
    id: 'VE-001',
    category: 'Fuel Tables',
    title: 'VE (Volumetric Efficiency) Table',
    shortDescription: 'Core fuel table for speed-density tuning',
    fullDescription: 'Represents engine\'s ability to move air at different RPM and load points. Foundation of speed-density fuel calculation. Values typically 0-100%+ with boost compensation.',
    characteristics: [
      '3D table (RPM vs MAP/TPS)',
      'Percentage values',
      'Speed-density core',
      'Most critical fuel table',
    ],
    tuningFocus: [
      'Smooth surface optimization',
      'Closed-loop learning',
      'Dyno verification',
      'Transient response',
    ],
    typicalValues: {
      'NA Peak': '85-95%',
      'Turbo + Boost Comp': '100-200%+',
      'Idle': '40-60%',
    },
    relatedTerms: ['SEN-001-MAP', 'CTL-001-CLOSED'],
  },
};

// Helper function to get term by ID
export function getTechnicalTerm(id: string): TechnicalTerm | undefined {
  return TECHNICAL_DATABASE[id];
}

// Helper function to get all terms in a category
export function getTermsByCategory(category: string): TechnicalTerm[] {
  return Object.values(TECHNICAL_DATABASE).filter(term => term.category === category);
}

// Helper function to search terms
export function searchTerms(query: string): TechnicalTerm[] {
  const lowerQuery = query.toLowerCase();
  return Object.values(TECHNICAL_DATABASE).filter(term =>
    term.title.toLowerCase().includes(lowerQuery) ||
    term.shortDescription.toLowerCase().includes(lowerQuery) ||
    term.fullDescription.toLowerCase().includes(lowerQuery)
  );
}

// Get all categories
export function getAllCategories(): string[] {
  const categories = new Set<string>();
  Object.values(TECHNICAL_DATABASE).forEach(term => categories.add(term.category));
  return Array.from(categories).sort();
}
