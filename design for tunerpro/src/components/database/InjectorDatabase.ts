// COMPREHENSIVE INJECTOR DATABASE - FUEL-002
// Covers US, Metric, and Global brands with flow rates, impedance, and dead time data

export interface InjectorSpec {
  id: string;
  brand: string;
  model: string;
  flowRateCCMin: number; // cc/min @ 3 bar (43.5 PSI)
  flowRateLbHr: number; // lb/hr @ 3 bar
  impedance: number; // Ohms
  deadTimeOffset14V: number; // milliseconds @ 14V
  deadTimeOffset12V?: number; // milliseconds @ 12V
  deadTimeOffset10V?: number; // milliseconds @ 10V
  pressure: number; // Base pressure in bar
  category: 'street' | 'performance' | 'race' | 'extreme';
  notes: string;
  manufacturer: string;
}

export const INJECTOR_DATABASE: InjectorSpec[] = [
  // BOSCH - OEM & Performance
  {
    id: 'BOSCH-0280158110',
    brand: 'Bosch',
    model: '0280158110',
    flowRateCCMin: 210,
    flowRateLbHr: 19.9,
    impedance: 12,
    deadTimeOffset14V: 0.85,
    deadTimeOffset12V: 1.05,
    deadTimeOffset10V: 1.35,
    pressure: 3.0,
    category: 'street',
    notes: 'OEM Mitsubishi Evo',
    manufacturer: 'Bosch',
  },
  {
    id: 'BOSCH-0280158036',
    brand: 'Bosch',
    model: '0280158036',
    flowRateCCMin: 260,
    flowRateLbHr: 24.8,
    impedance: 12,
    deadTimeOffset14V: 0.90,
    deadTimeOffset12V: 1.10,
    deadTimeOffset10V: 1.40,
    pressure: 3.0,
    category: 'street',
    notes: 'OEM Subaru WRX',
    manufacturer: 'Bosch',
  },
  {
    id: 'BOSCH-0445110247',
    brand: 'Bosch',
    model: '0445110247',
    flowRateCCMin: 1200,
    flowRateLbHr: 114.3,
    impedance: 14,
    deadTimeOffset14V: 0.68,
    deadTimeOffset12V: 0.85,
    deadTimeOffset10V: 1.15,
    pressure: 3.0,
    category: 'race',
    notes: 'E85 compatible, high flow turbo',
    manufacturer: 'Bosch',
  },

  // DENSO - Toyota/Lexus OEM
  {
    id: 'DENSO-23250-66030',
    brand: 'Denso',
    model: '23250-66030',
    flowRateCCMin: 310,
    flowRateLbHr: 29.5,
    impedance: 12,
    deadTimeOffset14V: 0.75,
    deadTimeOffset12V: 0.92,
    deadTimeOffset10V: 1.22,
    pressure: 3.0,
    category: 'street',
    notes: 'Toyota Supra 2JZ-GTE',
    manufacturer: 'Denso',
  },
  {
    id: 'DENSO-23209-74020',
    brand: 'Denso',
    model: '23209-74020',
    flowRateCCMin: 440,
    flowRateLbHr: 41.9,
    impedance: 12,
    deadTimeOffset14V: 0.78,
    deadTimeOffset12V: 0.95,
    deadTimeOffset10V: 1.25,
    pressure: 3.0,
    category: 'performance',
    notes: 'Toyota 2JZ upgrade',
    manufacturer: 'Denso',
  },

  // INJECTOR DYNAMICS - Popular Aftermarket
  {
    id: 'ID-725',
    brand: 'Injector Dynamics',
    model: 'ID725',
    flowRateCCMin: 685,
    flowRateLbHr: 65.3,
    impedance: 15,
    deadTimeOffset14V: 0.65,
    deadTimeOffset12V: 0.80,
    deadTimeOffset10V: 1.05,
    pressure: 3.0,
    category: 'performance',
    notes: 'Popular NA/mild turbo, excellent atomization',
    manufacturer: 'Injector Dynamics',
  },
  {
    id: 'ID-1000',
    brand: 'Injector Dynamics',
    model: 'ID1000',
    flowRateCCMin: 1045,
    flowRateLbHr: 99.5,
    impedance: 15,
    deadTimeOffset14V: 0.70,
    deadTimeOffset12V: 0.88,
    deadTimeOffset10V: 1.18,
    pressure: 3.0,
    category: 'race',
    notes: 'High HP turbo, 600+ hp capable',
    manufacturer: 'Injector Dynamics',
  },
  {
    id: 'ID-1300X',
    brand: 'Injector Dynamics',
    model: 'ID1300X',
    flowRateCCMin: 1340,
    flowRateLbHr: 127.6,
    impedance: 14,
    deadTimeOffset14V: 0.68,
    deadTimeOffset12V: 0.85,
    deadTimeOffset10V: 1.15,
    pressure: 3.0,
    category: 'race',
    notes: 'E85 compatible, 800+ hp',
    manufacturer: 'Injector Dynamics',
  },
  {
    id: 'ID-2000',
    brand: 'Injector Dynamics',
    model: 'ID2000',
    flowRateCCMin: 2200,
    flowRateLbHr: 209.5,
    impedance: 13,
    deadTimeOffset14V: 0.60,
    deadTimeOffset12V: 0.75,
    deadTimeOffset10V: 1.00,
    pressure: 3.0,
    category: 'extreme',
    notes: '1000+ hp drag/roll racing',
    manufacturer: 'Injector Dynamics',
  },

  // PRECISION TURBO - High Flow
  {
    id: 'PREC-1600',
    brand: 'Precision',
    model: '1600cc',
    flowRateCCMin: 1680,
    flowRateLbHr: 160.0,
    impedance: 11,
    deadTimeOffset14V: 0.55,
    deadTimeOffset12V: 0.70,
    deadTimeOffset10V: 0.95,
    pressure: 3.0,
    category: 'extreme',
    notes: 'Big turbo applications',
    manufacturer: 'Precision Turbo',
  },

  // FUEL INJECTOR CLINIC - Race Spec
  {
    id: 'FIC-2150',
    brand: 'Fuel Injector Clinic',
    model: 'FIC2150',
    flowRateCCMin: 2265,
    flowRateLbHr: 215.7,
    impedance: 13,
    deadTimeOffset14V: 0.60,
    deadTimeOffset12V: 0.78,
    deadTimeOffset10V: 1.05,
    pressure: 3.0,
    category: 'extreme',
    notes: 'Drag racing, 1200+ hp',
    manufacturer: 'Fuel Injector Clinic',
  },
  {
    id: 'FIC-1650',
    brand: 'Fuel Injector Clinic',
    model: 'FIC1650',
    flowRateCCMin: 1732,
    flowRateLbHr: 165.0,
    impedance: 12,
    deadTimeOffset14V: 0.62,
    deadTimeOffset12V: 0.80,
    deadTimeOffset10V: 1.08,
    pressure: 3.0,
    category: 'race',
    notes: 'High flow race injector',
    manufacturer: 'Fuel Injector Clinic',
  },

  // KEIHIN - Honda OEM
  {
    id: 'KEIHIN-16600-PND-A01',
    brand: 'Keihin',
    model: '16600-PND-A01',
    flowRateCCMin: 550,
    flowRateLbHr: 52.4,
    impedance: 12,
    deadTimeOffset14V: 0.80,
    deadTimeOffset12V: 0.98,
    deadTimeOffset10V: 1.28,
    pressure: 3.0,
    category: 'performance',
    notes: 'Honda NSX OEM',
    manufacturer: 'Keihin',
  },
  {
    id: 'KEIHIN-16450-RBB-003',
    brand: 'Keihin',
    model: '16450-RBB-003',
    flowRateCCMin: 310,
    flowRateLbHr: 29.5,
    impedance: 12,
    deadTimeOffset14V: 0.82,
    deadTimeOffset12V: 1.00,
    deadTimeOffset10V: 1.30,
    pressure: 3.0,
    category: 'street',
    notes: 'Honda S2000 OEM',
    manufacturer: 'Keihin',
  },

  // MAGNETI MARELLI - European
  {
    id: 'MM-630',
    brand: 'Magneti Marelli',
    model: '630cc',
    flowRateCCMin: 630,
    flowRateLbHr: 60.0,
    impedance: 14,
    deadTimeOffset14V: 0.72,
    deadTimeOffset12V: 0.90,
    deadTimeOffset10V: 1.20,
    pressure: 3.0,
    category: 'performance',
    notes: 'Ferrari/Alfa Romeo performance',
    manufacturer: 'Magneti Marelli',
  },

  // SIEMENS DEKA - GM/LS Platform
  {
    id: 'DEKA-60',
    brand: 'Siemens Deka',
    model: '60 lb/hr',
    flowRateCCMin: 630,
    flowRateLbHr: 60.0,
    impedance: 12,
    deadTimeOffset14V: 0.92,
    deadTimeOffset12V: 1.15,
    deadTimeOffset10V: 1.48,
    pressure: 3.0,
    category: 'performance',
    notes: 'LS swap popular choice',
    manufacturer: 'Siemens Deka',
  },
  {
    id: 'DEKA-80',
    brand: 'Siemens Deka',
    model: '80 lb/hr',
    flowRateCCMin: 840,
    flowRateLbHr: 80.0,
    impedance: 12,
    deadTimeOffset14V: 0.88,
    deadTimeOffset12V: 1.10,
    deadTimeOffset10V: 1.42,
    pressure: 3.0,
    category: 'race',
    notes: 'Boosted LS applications',
    manufacturer: 'Siemens Deka',
  },

  // DEATSCHWERKS - Popular Aftermarket
  {
    id: 'DW-1000',
    brand: 'DeatschWerks',
    model: 'DW1000',
    flowRateCCMin: 1050,
    flowRateLbHr: 100.0,
    impedance: 14,
    deadTimeOffset14V: 0.70,
    deadTimeOffset12V: 0.88,
    deadTimeOffset10V: 1.18,
    pressure: 3.0,
    category: 'race',
    notes: 'High flow, E85 compatible',
    manufacturer: 'DeatschWerks',
  },
  {
    id: 'DW-1300',
    brand: 'DeatschWerks',
    model: 'DW1300',
    flowRateCCMin: 1365,
    flowRateLbHr: 130.0,
    impedance: 13,
    deadTimeOffset14V: 0.68,
    deadTimeOffset12V: 0.85,
    deadTimeOffset10V: 1.15,
    pressure: 3.0,
    category: 'race',
    notes: '800-1000 hp capable',
    manufacturer: 'DeatschWerks',
  },
];

// Helper functions
export function getInjectorById(id: string): InjectorSpec | undefined {
  return INJECTOR_DATABASE.find(inj => inj.id === id);
}

export function getInjectorsByCategory(category: InjectorSpec['category']): InjectorSpec[] {
  return INJECTOR_DATABASE.filter(inj => inj.category === category);
}

export function getInjectorsByManufacturer(manufacturer: string): InjectorSpec[] {
  return INJECTOR_DATABASE.filter(inj => inj.manufacturer === manufacturer);
}

export function searchInjectors(query: string): InjectorSpec[] {
  const lowerQuery = query.toLowerCase();
  return INJECTOR_DATABASE.filter(inj =>
    inj.brand.toLowerCase().includes(lowerQuery) ||
    inj.model.toLowerCase().includes(lowerQuery) ||
    inj.notes.toLowerCase().includes(lowerQuery)
  );
}

export function getAllManufacturers(): string[] {
  const manufacturers = new Set<string>();
  INJECTOR_DATABASE.forEach(inj => manufacturers.add(inj.manufacturer));
  return Array.from(manufacturers).sort();
}

// Calculate required flow rate for target HP
export function calculateRequiredFlow(
  targetHP: number,
  bsfc: number = 0.5, // lb/hp-hr, 0.5 for NA, 0.55-0.65 for boost
  numInjectors: number = 4,
  maxDutyCycle: number = 0.85
): number {
  // Required flow = (HP × BSFC × 0.5) / (Number of Injectors × Max DC)
  const requiredLbHr = (targetHP * bsfc * 0.5) / (numInjectors * maxDutyCycle);
  const requiredCCMin = requiredLbHr * 10.5; // 1 lb/hr = 10.5 cc/min @ 3 bar
  return Math.round(requiredCCMin);
}

// Suggest injectors for target HP
export function suggestInjectorsForHP(
  targetHP: number,
  numInjectors: number = 4,
  isBoosted: boolean = false
): InjectorSpec[] {
  const bsfc = isBoosted ? 0.6 : 0.5;
  const requiredFlow = calculateRequiredFlow(targetHP, bsfc, numInjectors);
  
  return INJECTOR_DATABASE
    .filter(inj => inj.flowRateCCMin >= requiredFlow * 0.9 && inj.flowRateCCMin <= requiredFlow * 1.5)
    .sort((a, b) => Math.abs(a.flowRateCCMin - requiredFlow) - Math.abs(b.flowRateCCMin - requiredFlow))
    .slice(0, 5);
}

// Calculate flow at different pressure
export function calculateFlowAtPressure(
  baseFlow: number,
  basePressure: number,
  targetPressure: number
): number {
  // Flow ∝ √Pressure
  return baseFlow * Math.sqrt(targetPressure / basePressure);
}

// Dead time interpolation
export function interpolateDeadTime(
  injector: InjectorSpec,
  voltage: number
): number {
  if (voltage >= 14) return injector.deadTimeOffset14V;
  if (voltage >= 12 && injector.deadTimeOffset12V) {
    if (voltage === 12) return injector.deadTimeOffset12V;
    // Linear interpolation between 12V and 14V
    const t = (voltage - 12) / 2;
    return injector.deadTimeOffset12V + t * (injector.deadTimeOffset14V - injector.deadTimeOffset12V);
  }
  if (voltage >= 10 && injector.deadTimeOffset10V) {
    if (voltage === 10) return injector.deadTimeOffset10V;
    if (injector.deadTimeOffset12V) {
      const t = (voltage - 10) / 2;
      return injector.deadTimeOffset10V + t * (injector.deadTimeOffset12V - injector.deadTimeOffset10V);
    }
  }
  // Fallback to 14V value
  return injector.deadTimeOffset14V;
}
