// Comprehensive Firing Order Database for Speeduino ECU
// ONLY includes firing orders officially supported by Speeduino firmware
// Reference: Speeduino documentation and official codebase

export interface FiringOrderDefinition {
  id: string;
  cylinders: number;
  layout: 'inline' | 'v' | 'flat' | 'rotary';
  firingOrder: string;
  description: string;
  commonApplications: string[];
  speeduinoSupported: boolean;
}

export const FIRING_ORDER_DATABASE: FiringOrderDefinition[] = [
  // =====================================================================
  // 1-CYLINDER ENGINES
  // =====================================================================
  {
    id: '1cyl-1',
    cylinders: 1,
    layout: 'inline',
    firingOrder: '1',
    description: 'Single cylinder',
    commonApplications: ['Motorcycles', 'Small engines', 'Lawn equipment'],
    speeduinoSupported: true,
  },

  // =====================================================================
  // 2-CYLINDER ENGINES
  // =====================================================================
  {
    id: '2cyl-even',
    cylinders: 2,
    layout: 'inline',
    firingOrder: '1-2',
    description: '2-Cylinder Even Fire (360°)',
    commonApplications: ['Parallel twin motorcycles', 'Fiat TwinAir'],
    speeduinoSupported: true,
  },
  {
    id: '2cyl-270',
    cylinders: 2,
    layout: 'v',
    firingOrder: '1-2',
    description: '2-Cylinder Uneven Fire (270°)',
    commonApplications: ['V-Twin motorcycles', 'Ducati', 'Harley-Davidson'],
    speeduinoSupported: true,
  },

  // =====================================================================
  // 3-CYLINDER ENGINES
  // =====================================================================
  {
    id: '3cyl-123',
    cylinders: 3,
    layout: 'inline',
    firingOrder: '1-2-3',
    description: '3-Cylinder Standard',
    commonApplications: ['Ford 1.0L EcoBoost', 'BMW S1000RR', 'Triumph Triple'],
    speeduinoSupported: true,
  },
  {
    id: '3cyl-132',
    cylinders: 3,
    layout: 'inline',
    firingOrder: '1-3-2',
    description: '3-Cylinder Alternate',
    commonApplications: ['Some European 3-cylinder designs'],
    speeduinoSupported: true,
  },

  // =====================================================================
  // 4-CYLINDER ENGINES (Most Common)
  // =====================================================================
  {
    id: '4cyl-1342',
    cylinders: 4,
    layout: 'inline',
    firingOrder: '1-3-4-2',
    description: 'Inline-4 Standard (Most Common)',
    commonApplications: [
      'Honda K20/K24',
      'Toyota 4A-GE/2JZ',
      'Mazda B-series',
      'Nissan SR20/RB26',
      'Most modern I4 engines',
    ],
    speeduinoSupported: true,
  },
  {
    id: '4cyl-1243',
    cylinders: 4,
    layout: 'inline',
    firingOrder: '1-2-4-3',
    description: 'Inline-4 Alternate',
    commonApplications: ['Yamaha R1', 'Some motorcycle engines'],
    speeduinoSupported: true,
  },
  {
    id: '4cyl-1324',
    cylinders: 4,
    layout: 'inline',
    firingOrder: '1-3-2-4',
    description: 'Inline-4 Rare Configuration',
    commonApplications: ['Some vintage engines'],
    speeduinoSupported: true,
  },
  {
    id: '4cyl-flat-1342',
    cylinders: 4,
    layout: 'flat',
    firingOrder: '1-3-4-2',
    description: 'Flat-4 (Boxer)',
    commonApplications: ['Subaru EJ/FA engines', 'Porsche 914', 'VW Beetle'],
    speeduinoSupported: true,
  },

  // =====================================================================
  // 5-CYLINDER ENGINES
  // =====================================================================
  {
    id: '5cyl-12453',
    cylinders: 5,
    layout: 'inline',
    firingOrder: '1-2-4-5-3',
    description: 'Inline-5 Standard',
    commonApplications: [
      'Audi RS3 2.5T',
      'Volvo T5',
      'VW/Audi 2.5L I5',
      'Honda Acura Vigor',
    ],
    speeduinoSupported: true,
  },

  // =====================================================================
  // 6-CYLINDER ENGINES
  // =====================================================================
  {
    id: '6cyl-153624',
    cylinders: 6,
    layout: 'inline',
    firingOrder: '1-5-3-6-2-4',
    description: 'Inline-6 Standard (Most Common)',
    commonApplications: [
      'BMW M50/S50/N54/B58',
      'Toyota 1JZ/2JZ',
      'Nissan RB25/RB26',
      'Mercedes M104',
    ],
    speeduinoSupported: true,
  },
  {
    id: '6cyl-165432',
    cylinders: 6,
    layout: 'v',
    firingOrder: '1-6-5-4-3-2',
    description: 'V6 Standard',
    commonApplications: [
      'GM 3800 V6',
      'Ford Cologne V6',
      'Chrysler 3.5L V6',
    ],
    speeduinoSupported: true,
  },
  {
    id: '6cyl-142536',
    cylinders: 6,
    layout: 'v',
    firingOrder: '1-4-2-5-3-6',
    description: 'V6 Alternate (60° V6)',
    commonApplications: [
      'Honda J-series V6',
      'Nissan VQ series',
      'Toyota 1MZ/2GR',
    ],
    speeduinoSupported: true,
  },
  {
    id: '6cyl-flat-153624',
    cylinders: 6,
    layout: 'flat',
    firingOrder: '1-5-3-6-2-4',
    description: 'Flat-6 (Boxer)',
    commonApplications: ['Porsche 911', 'Subaru EZ30/EZ36'],
    speeduinoSupported: true,
  },

  // =====================================================================
  // 8-CYLINDER ENGINES
  // =====================================================================
  {
    id: '8cyl-18436572',
    cylinders: 8,
    layout: 'v',
    firingOrder: '1-8-4-3-6-5-7-2',
    description: 'V8 Standard (Most Common)',
    commonApplications: [
      'Chevrolet LS engines (LS1/LS3/LS7)',
      'Ford Coyote 5.0L',
      'Chrysler Hemi',
      'Most American V8s',
    ],
    speeduinoSupported: true,
  },
  {
    id: '8cyl-18726543',
    cylinders: 8,
    layout: 'v',
    firingOrder: '1-8-7-2-6-5-4-3',
    description: 'V8 Ford/Chrysler Alternate',
    commonApplications: [
      'Ford 351 Windsor',
      'Chrysler LA engines',
      'Some vintage Ford V8s',
    ],
    speeduinoSupported: true,
  },
  {
    id: '8cyl-15426378',
    cylinders: 8,
    layout: 'v',
    firingOrder: '1-5-4-2-6-3-7-8',
    description: 'V8 GM/European',
    commonApplications: [
      'GM LT1 (older)',
      'BMW S65 V8',
      'Ferrari V8 engines',
    ],
    speeduinoSupported: true,
  },
  {
    id: '8cyl-flat-18725634',
    cylinders: 8,
    layout: 'flat',
    firingOrder: '1-8-7-2-5-6-3-4',
    description: 'Flat-8 (Boxer)',
    commonApplications: ['Porsche 918 Spyder', 'Vintage racing engines'],
    speeduinoSupported: true,
  },
];

// Helper function to get firing orders by cylinder count
export function getFiringOrdersByCylinders(cylinders: number): FiringOrderDefinition[] {
  return FIRING_ORDER_DATABASE.filter(
    (fo) => fo.cylinders === cylinders && fo.speeduinoSupported
  );
}

// Helper function to get firing orders by cylinder count and layout
export function getFiringOrdersByConfig(
  cylinders: number,
  layout: 'inline' | 'v' | 'flat' | 'rotary'
): FiringOrderDefinition[] {
  return FIRING_ORDER_DATABASE.filter(
    (fo) =>
      fo.cylinders === cylinders &&
      fo.layout === layout &&
      fo.speeduinoSupported
  );
}

// Helper function to get available layouts for cylinder count
export function getAvailableLayouts(cylinders: number): Array<'inline' | 'v' | 'flat' | 'rotary'> {
  const layouts = new Set<'inline' | 'v' | 'flat' | 'rotary'>();
  FIRING_ORDER_DATABASE.filter((fo) => fo.cylinders === cylinders).forEach((fo) =>
    layouts.add(fo.layout)
  );
  return Array.from(layouts);
}

// Cylinder count options supported by Speeduino
export const SUPPORTED_CYLINDER_COUNTS = [1, 2, 3, 4, 5, 6, 8];

// Layout display names
export const LAYOUT_DISPLAY_NAMES: Record<string, string> = {
  inline: 'Inline',
  v: 'V-Configuration',
  flat: 'Flat/Boxer',
  rotary: 'Rotary',
};
