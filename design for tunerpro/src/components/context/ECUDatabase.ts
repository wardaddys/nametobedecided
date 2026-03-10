// Comprehensive ECU Database for Speeduino and MegaSquirt ECUs
// Auto-configuration for I/O pins based on selected ECU

export type PinType = 
  | 'injector' 
  | 'ignition' 
  | 'fuel_pump' 
  | 'idle_control' 
  | 'tachometer' 
  | 'fan' 
  | 'boost' 
  | 'vvt' 
  | 'auxiliary'
  | 'crank_trigger'
  | 'cam_trigger'
  | 'map_sensor'
  | 'tps'
  | 'clt'
  | 'iat'
  | 'o2_sensor'
  | 'battery_voltage'
  | 'flex_fuel'
  | 'launch_control'
  | 'knock_sensor'
  | 'spare';

export interface Pin {
  pinNumber: string;
  function: string;
  type: PinType;
  specs: string;
  arduinoPin?: string;
  notes: string;
  enabled: boolean;
}

export interface ECUConfig {
  id: string;
  name: string;
  manufacturer: 'Speeduino' | 'MegaSquirt';
  platform: string;
  connectorType: string;
  outputs: Pin[];
  inputs: Pin[];
  powerGrounds: Pin[];
  features: string[];
  engineSupport: string[];
  mapSize: string;
  recommendedUse: string;
}

export const ECU_DATABASE: ECUConfig[] = [
  // ==================== SPEEDUINO ECUs ====================
  {
    id: 'speeduino-v043',
    name: 'Speeduino v0.4.3c/d (Most Popular)',
    manufacturer: 'Speeduino',
    platform: 'Arduino Mega 2560 R3',
    connectorType: '40-Pin IDC Connector',
    outputs: [
      { pinNumber: '1', function: 'Injector 1 (1/2)', type: 'injector', specs: 'Ground switching, max 6A', arduinoPin: '8', notes: 'Can drive 2 low-Z with resistors', enabled: true },
      { pinNumber: '2', function: 'Injector 2 (1/2)', type: 'injector', specs: 'Ground switching, max 6A', arduinoPin: '9', notes: 'Can drive 2 low-Z with resistors', enabled: true },
      { pinNumber: '3', function: 'Injector 3 (1/2)', type: 'injector', specs: 'Ground switching, max 6A', arduinoPin: '10', notes: 'Can drive 2 low-Z with resistors', enabled: true },
      { pinNumber: '4', function: 'Injector 4 (1/2)', type: 'injector', specs: 'Ground switching, max 6A', arduinoPin: '11', notes: 'Can drive 2 low-Z with resistors', enabled: true },
      { pinNumber: '21', function: 'Injector 1 (2/2)', type: 'injector', specs: 'Ground switching, max 6A', arduinoPin: '8', notes: 'Parallel with pin 1', enabled: false },
      { pinNumber: '22', function: 'Injector 2 (2/2)', type: 'injector', specs: 'Ground switching, max 6A', arduinoPin: '9', notes: 'Parallel with pin 2', enabled: false },
      { pinNumber: '23', function: 'Injector 3 (2/2)', type: 'injector', specs: 'Ground switching, max 6A', arduinoPin: '10', notes: 'Parallel with pin 3', enabled: false },
      { pinNumber: '24', function: 'Injector 4 (2/2)', type: 'injector', specs: 'Ground switching, max 6A', arduinoPin: '11', notes: 'Parallel with pin 4', enabled: false },
      { pinNumber: '5', function: 'Ignition 1', type: 'ignition', specs: 'Logic level 5V, 330mA', arduinoPin: '40', notes: 'Requires igniter/smart coil', enabled: true },
      { pinNumber: '6', function: 'Ignition 2', type: 'ignition', specs: 'Logic level 5V, 330mA', arduinoPin: '38', notes: 'Requires igniter/smart coil', enabled: true },
      { pinNumber: '7', function: 'Ignition 3', type: 'ignition', specs: 'Logic level 5V, 330mA', arduinoPin: '36', notes: 'Requires igniter/smart coil', enabled: true },
      { pinNumber: '8', function: 'Ignition 4', type: 'ignition', specs: 'Logic level 5V, 330mA', arduinoPin: '34', notes: 'Requires igniter/smart coil', enabled: true },
      { pinNumber: '33', function: 'Tachometer Out', type: 'tachometer', specs: 'Logic level', arduinoPin: '49', notes: 'For dashboard tach', enabled: true },
      { pinNumber: '34', function: 'Fuel Pump', type: 'fuel_pump', specs: 'Ground switching', arduinoPin: '37', notes: 'Via onboard relay driver', enabled: true },
      { pinNumber: '35', function: 'Idle Control (PWM)', type: 'idle_control', specs: 'PWM or On/Off', arduinoPin: '45', notes: 'For 2-wire idle valve', enabled: true },
      { pinNumber: '36', function: 'Fan Output', type: 'fan', specs: 'Ground switching', arduinoPin: 'A8', notes: 'Configurable', enabled: true },
      { pinNumber: '37', function: 'VVT Output', type: 'vvt', specs: 'PWM', arduinoPin: '4', notes: 'Optional, configurable', enabled: false },
      { pinNumber: '38', function: 'Boost Control', type: 'boost', specs: 'PWM', arduinoPin: '7', notes: 'Optional, configurable', enabled: false },
    ],
    inputs: [
      { pinNumber: '9', function: 'RPM1 (Crank)', type: 'crank_trigger', specs: 'VR or Hall', arduinoPin: '19', notes: 'Requires VR conditioner for VR', enabled: true },
      { pinNumber: '10', function: 'RPM2 (Cam)', type: 'cam_trigger', specs: 'VR or Hall', arduinoPin: '18', notes: 'Optional, for sequential', enabled: false },
      { pinNumber: '11', function: 'MAP Sensor Signal', type: 'map_sensor', specs: 'Analog 0-5V', arduinoPin: 'A0', notes: 'Onboard sensor included', enabled: true },
      { pinNumber: '12', function: 'TPS (Throttle Position)', type: 'tps', specs: 'Analog 0-5V', arduinoPin: 'A2', notes: '0-5V input', enabled: true },
      { pinNumber: '13', function: 'CLT (Coolant Temp)', type: 'clt', specs: 'Analog', arduinoPin: 'A1', notes: 'Resistive sensor', enabled: true },
      { pinNumber: '14', function: 'IAT (Intake Air Temp)', type: 'iat', specs: 'Analog', arduinoPin: 'A3', notes: 'Resistive sensor', enabled: true },
      { pinNumber: '15', function: 'O2/Lambda Sensor', type: 'o2_sensor', specs: 'Analog 0-5V', arduinoPin: 'A3', notes: 'Wideband controller output', enabled: true },
      { pinNumber: '16', function: 'Battery Voltage', type: 'battery_voltage', specs: 'Analog', arduinoPin: 'A15', notes: 'Voltage divider', enabled: true },
      { pinNumber: '17', function: 'Flex Fuel Sensor', type: 'flex_fuel', specs: 'Frequency', arduinoPin: '20', notes: 'Optional', enabled: false },
      { pinNumber: '39', function: 'Launch Control Input', type: 'launch_control', specs: 'Digital', arduinoPin: 'A15', notes: 'Switch to ground', enabled: false },
      { pinNumber: '40', function: 'Spare Analog Input', type: 'spare', specs: 'Analog', arduinoPin: 'Various', notes: 'Configurable in proto area', enabled: false },
    ],
    powerGrounds: [
      { pinNumber: '18-20, 25-32', function: 'Grounds', type: 'spare', specs: 'Multiple pins', notes: 'For current distribution', enabled: true },
      { pinNumber: 'External', function: '+12V Supply', type: 'spare', specs: 'Main power', notes: 'Separate 2-pin terminal, fused', enabled: true },
    ],
    features: [
      'DRV8825/8824 stepper motor driver support',
      'Dual VR Conditioner (MAX9926)',
      'Prototype area for custom circuits',
      '16x16 Fuel and Ignition maps',
      'Optional VVT and Boost Control outputs',
    ],
    engineSupport: [
      '1-4 cylinders: Full sequential fuel & ignition',
      '6 cylinders: Semi-sequential fuel, wasted spark',
      '8 cylinders: Batch fire, wasted spark',
    ],
    mapSize: '16x16',
    recommendedUse: 'Most popular choice for 4-cylinder projects, cost-effective',
  },

  {
    id: 'speeduino-dropbear-v2',
    name: 'Speeduino Dropbear V2 (Premium)',
    manufacturer: 'Speeduino',
    platform: 'Teensy 4.1 (600MHz ARM Cortex-M7)',
    connectorType: '2x 24-Pin Delphi Sicma',
    outputs: [
      { pinNumber: 'A1', function: 'Injector 1', type: 'injector', specs: 'High-Z, 2A', arduinoPin: '6', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'A2', function: 'Injector 2', type: 'injector', specs: 'High-Z, 2A', arduinoPin: '7', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'A3', function: 'Injector 3', type: 'injector', specs: 'High-Z, 2A', arduinoPin: '8', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'A4', function: 'Injector 4', type: 'injector', specs: 'High-Z, 2A', arduinoPin: '9', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'A5', function: 'Injector 5', type: 'injector', specs: 'High-Z, 2A', arduinoPin: '14', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'A6', function: 'Injector 6', type: 'injector', specs: 'High-Z, 2A', arduinoPin: '15', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'A7', function: 'Injector 7', type: 'injector', specs: 'High-Z, 2A', arduinoPin: '29', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'A8', function: 'Injector 8', type: 'injector', specs: 'High-Z, 2A', arduinoPin: '30', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'B1', function: 'Ignition 1', type: 'ignition', specs: '5V/12V logic, 330mA', arduinoPin: '2', notes: 'Pre-driver included', enabled: true },
      { pinNumber: 'B2', function: 'Ignition 2', type: 'ignition', specs: '5V/12V logic, 330mA', arduinoPin: '3', notes: 'Pre-driver included', enabled: true },
      { pinNumber: 'B3', function: 'Ignition 3', type: 'ignition', specs: '5V/12V logic, 330mA', arduinoPin: '4', notes: 'Pre-driver included', enabled: true },
      { pinNumber: 'B4', function: 'Ignition 4', type: 'ignition', specs: '5V/12V logic, 330mA', arduinoPin: '5', notes: 'Pre-driver included', enabled: true },
      { pinNumber: 'B5', function: 'Ignition 5', type: 'ignition', specs: '5V/12V logic, 330mA', arduinoPin: '10', notes: 'Pre-driver included', enabled: true },
      { pinNumber: 'B6', function: 'Ignition 6', type: 'ignition', specs: '5V/12V logic, 330mA', arduinoPin: '11', notes: 'Pre-driver included', enabled: true },
      { pinNumber: 'B7', function: 'Ignition 7', type: 'ignition', specs: '5V/12V logic, 330mA', arduinoPin: '31', notes: 'Pre-driver included', enabled: true },
      { pinNumber: 'B8', function: 'Ignition 8', type: 'ignition', specs: '5V/12V logic, 330mA', arduinoPin: '32', notes: 'Pre-driver included', enabled: true },
      { pinNumber: 'C1', function: 'Fuel Pump', type: 'fuel_pump', specs: 'Medium current, 2A', arduinoPin: '22', notes: 'Ground switch', enabled: true },
      { pinNumber: 'C2', function: 'Idle Control PWM', type: 'idle_control', specs: 'Medium current, 2A', arduinoPin: '23', notes: 'PWM output', enabled: true },
      { pinNumber: 'C7', function: 'Tachometer Out', type: 'tachometer', specs: 'Logic level', arduinoPin: '12', notes: 'For dash', enabled: true },
      { pinNumber: 'C8', function: 'Fan Output', type: 'fan', specs: 'Medium current, 2A', arduinoPin: '24', notes: 'Ground switch', enabled: true },
      { pinNumber: 'D19', function: 'Boost Control', type: 'boost', specs: 'Medium current, 2A', arduinoPin: '25', notes: 'PWM output', enabled: false },
      { pinNumber: 'D20', function: 'VVT Output', type: 'vvt', specs: 'Medium current, 2A', arduinoPin: '26', notes: 'PWM output', enabled: false },
      { pinNumber: 'D21', function: 'N2O/Aux Output 1', type: 'auxiliary', specs: 'Medium current, 2A', arduinoPin: '27', notes: 'Configurable', enabled: false },
      { pinNumber: 'D22', function: 'Aux Output 2', type: 'auxiliary', specs: 'Medium current, 2A', arduinoPin: '28', notes: 'Configurable', enabled: false },
    ],
    inputs: [
      { pinNumber: 'D1/D2', function: 'Crank Input (VR)', type: 'crank_trigger', specs: 'VR', arduinoPin: '19/20', notes: 'Onboard conditioner', enabled: true },
      { pinNumber: 'D3/D4', function: 'Cam Input (VR)', type: 'cam_trigger', specs: 'VR', arduinoPin: '17/18', notes: 'Onboard conditioner', enabled: false },
      { pinNumber: 'D7', function: 'MAP Sensor', type: 'map_sensor', specs: 'Analog', arduinoPin: 'A0', notes: 'Internal or external', enabled: true },
      { pinNumber: 'D8', function: 'Barometer', type: 'spare', specs: 'Analog', arduinoPin: 'A1', notes: 'Internal sensor', enabled: true },
      { pinNumber: 'D9', function: 'TPS', type: 'tps', specs: 'Analog 0-5V', arduinoPin: 'A2', notes: '0-5V', enabled: true },
      { pinNumber: 'D10', function: 'CLT', type: 'clt', specs: 'Analog', arduinoPin: 'A3', notes: 'Thermistor', enabled: true },
      { pinNumber: 'D11', function: 'IAT', type: 'iat', specs: 'Analog', arduinoPin: 'A4', notes: 'Thermistor', enabled: true },
      { pinNumber: 'D12', function: 'O2/Lambda', type: 'o2_sensor', specs: 'Analog 0-5V', arduinoPin: 'A5', notes: '0-5V wideband', enabled: true },
      { pinNumber: 'D13', function: 'Flex Fuel', type: 'flex_fuel', specs: 'Frequency', arduinoPin: '13', notes: 'Optional', enabled: false },
      { pinNumber: 'D14', function: 'Knock Sensor', type: 'knock_sensor', specs: 'Analog', arduinoPin: 'A17', notes: 'With external controller', enabled: false },
      { pinNumber: 'D15-16', function: 'Spare Analog', type: 'spare', specs: 'Analog', arduinoPin: 'A16, A17', notes: 'Configurable', enabled: false },
      { pinNumber: 'D17-18', function: 'Spare Digital', type: 'spare', specs: 'Digital', arduinoPin: '34, 35', notes: 'Configurable', enabled: false },
    ],
    powerGrounds: [
      { pinNumber: 'Multiple', function: 'Sensor Grounds', type: 'spare', specs: 'Distributed', notes: 'Across both connectors', enabled: true },
      { pinNumber: 'Multiple', function: 'Power Grounds', type: 'spare', specs: 'High current', notes: 'High current paths', enabled: true },
      { pinNumber: 'External', function: '+12V Supply', type: 'spare', specs: 'Main power', notes: 'Main power input', enabled: true },
    ],
    features: [
      'SD Card Logging',
      'USB-C Connection',
      'WiFi and Bluetooth LE',
      'CAN Transceiver (built-in)',
      'Dual VR Conditioner (onboard)',
      'Hardware filters adjustable via switches',
    ],
    engineSupport: [
      '1-8 cylinders: Full sequential fuel & ignition',
      'V12: Semi-sequential fuel, wasted spark',
      '4-rotor Wankel: Leading & trailing ignition',
    ],
    mapSize: '16x16',
    recommendedUse: 'Professional builds, high-performance applications, complex engine setups',
  },

  {
    id: 'speeduino-ua4c',
    name: 'Speeduino UA4C (Under Arduino 4 Channel)',
    manufacturer: 'Speeduino',
    platform: 'Arduino Mega 2560',
    connectorType: '46-Pin IDC Connector',
    outputs: [
      { pinNumber: '1', function: 'Injector 1', type: 'injector', specs: 'Ground switch, 6A', arduinoPin: '8', notes: 'LED indicator', enabled: true },
      { pinNumber: '2', function: 'Injector 2', type: 'injector', specs: 'Ground switch, 6A', arduinoPin: '9', notes: 'LED indicator', enabled: true },
      { pinNumber: '3', function: 'Injector 3', type: 'injector', specs: 'Ground switch, 6A', arduinoPin: '10', notes: 'LED indicator', enabled: true },
      { pinNumber: '4', function: 'Injector 4', type: 'injector', specs: 'Ground switch, 6A', arduinoPin: '11', notes: 'LED indicator', enabled: true },
      { pinNumber: '5', function: 'Ignition 1', type: 'ignition', specs: 'Logic 5V, 330mA', arduinoPin: '40', notes: 'LED indicator', enabled: true },
      { pinNumber: '6', function: 'Ignition 2', type: 'ignition', specs: 'Logic 5V, 330mA', arduinoPin: '38', notes: 'LED indicator', enabled: true },
      { pinNumber: '7', function: 'Ignition 3', type: 'ignition', specs: 'Logic 5V, 330mA', arduinoPin: '36', notes: 'LED indicator', enabled: true },
      { pinNumber: '8', function: 'Ignition 4', type: 'ignition', specs: 'Logic 5V, 330mA', arduinoPin: '34', notes: 'LED indicator', enabled: true },
      { pinNumber: '9', function: 'Fuel Pump', type: 'fuel_pump', specs: 'Low current', arduinoPin: '37', notes: 'LED indicator', enabled: true },
      { pinNumber: '10', function: 'Tachometer', type: 'tachometer', specs: 'Low current', arduinoPin: '49', notes: 'LED indicator', enabled: true },
      { pinNumber: '11', function: 'Fan', type: 'fan', specs: 'Low current', arduinoPin: '47', notes: 'LED indicator', enabled: true },
      { pinNumber: '12', function: 'Fan 2', type: 'fan', specs: 'Low current', arduinoPin: '48', notes: 'LED indicator', enabled: false },
      { pinNumber: '13', function: 'Idle Output', type: 'idle_control', specs: 'High current, 2A', arduinoPin: '45', notes: 'Ground switch', enabled: true },
      { pinNumber: '14', function: 'Idle 2 Output', type: 'idle_control', specs: 'High current, 2A', arduinoPin: '46', notes: 'Ground switch', enabled: false },
      { pinNumber: '15', function: 'Boost Control', type: 'boost', specs: 'High current, 2A', arduinoPin: '7', notes: 'Ground switch', enabled: false },
      { pinNumber: '16', function: 'VVT Output', type: 'vvt', specs: 'High current, 2A', arduinoPin: '4', notes: 'Ground switch', enabled: false },
      { pinNumber: '17-20', function: 'AUX 1-4', type: 'auxiliary', specs: 'Low current', arduinoPin: 'Various', notes: 'Configurable', enabled: false },
    ],
    inputs: [
      { pinNumber: '21', function: 'Crank Trigger (RPM1)', type: 'crank_trigger', specs: 'VR/Hall', arduinoPin: '19', notes: 'Socket for VR conditioner', enabled: true },
      { pinNumber: '22', function: 'Cam Trigger (RPM2)', type: 'cam_trigger', specs: 'VR/Hall', arduinoPin: '18', notes: 'Socket for VR conditioner', enabled: false },
      { pinNumber: '23', function: 'MAP Sensor', type: 'map_sensor', specs: 'Analog', arduinoPin: 'A0', notes: '250kPa or 400kPa option', enabled: true },
      { pinNumber: '24', function: 'Barometer', type: 'spare', specs: 'Analog', arduinoPin: 'A5', notes: 'Separate from MAP', enabled: true },
      { pinNumber: '25', function: 'TPS', type: 'tps', specs: 'Analog 0-5V', arduinoPin: 'A2', notes: '0-5V', enabled: true },
      { pinNumber: '26', function: 'CLT', type: 'clt', specs: 'Analog', arduinoPin: 'A1', notes: 'Thermistor', enabled: true },
      { pinNumber: '27', function: 'IAT', type: 'iat', specs: 'Analog', arduinoPin: 'A3', notes: 'Thermistor', enabled: true },
      { pinNumber: '28', function: 'O2 Sensor', type: 'o2_sensor', specs: 'Analog 0-5V', arduinoPin: 'A4', notes: '0-5V wideband', enabled: true },
      { pinNumber: '29', function: 'Flex Fuel', type: 'flex_fuel', specs: 'Frequency', arduinoPin: '20', notes: 'Optional', enabled: false },
      { pinNumber: '30', function: 'Launch Control (LNCH)', type: 'launch_control', specs: 'Digital', arduinoPin: 'A15', notes: 'Switch to ground', enabled: false },
      { pinNumber: '31', function: 'Knock Sensor', type: 'knock_sensor', specs: 'Analog', arduinoPin: 'Various', notes: 'Future firmware support', enabled: false },
      { pinNumber: '32-33', function: 'Spare Inputs', type: 'spare', specs: 'Analog/Digital', arduinoPin: 'Various', notes: 'Configurable', enabled: false },
      { pinNumber: '34', function: 'Battery Voltage', type: 'battery_voltage', specs: 'Analog', arduinoPin: 'A15', notes: 'Voltage sensing', enabled: true },
    ],
    powerGrounds: [
      { pinNumber: '35-40', function: 'Sensor Grounds', type: 'spare', specs: 'Multiple', notes: 'For noise reduction', enabled: true },
      { pinNumber: '41-44', function: 'Power Grounds', type: 'spare', specs: 'High current', notes: 'High current paths', enabled: true },
      { pinNumber: '45', function: '+12V Switched', type: 'spare', specs: 'Main power', notes: 'Main power', enabled: true },
      { pinNumber: '46', function: '+5V Reference', type: 'spare', specs: 'Sensor power', notes: 'For sensors (TPS, MAP)', enabled: true },
    ],
    features: [
      'SMD technology for compact size',
      'All outputs include LEDs for troubleshooting',
      'Serial ports for Bluetooth/CAN modules',
      '16x16 Fuel and ignition maps',
      'DRV8825 Stepper Driver support',
    ],
    engineSupport: [
      '1, 2 (even fire only), 3, 4 cylinders: Full sequential fuel & ignition',
      '6 (even fire only) and 8 cylinders: Wasted spark, 2 squirts per cycle',
      'Rotaries: Up to 2 rotors',
    ],
    mapSize: '16x16',
    recommendedUse: 'Motorcycles, ATVs, space-limited applications, 4-cylinder projects',
  },

  {
    id: 'speeduino-no2c',
    name: 'Speeduino NO2C (No Overhang Two Channel)',
    manufacturer: 'Speeduino',
    platform: 'Arduino Mega 2560',
    connectorType: '24-Pin Molex Micro-Fit 3.0',
    outputs: [
      { pinNumber: '1', function: 'Injector 1', type: 'injector', specs: 'Ground switch, 6A', arduinoPin: '8', notes: 'Can drive 2 low-Z injectors', enabled: true },
      { pinNumber: '2', function: 'Injector 2', type: 'injector', specs: 'Ground switch, 6A', arduinoPin: '9', notes: 'Can drive 2 low-Z injectors', enabled: true },
      { pinNumber: '3', function: 'Ignition 1', type: 'ignition', specs: 'Logic 5V, 330mA', arduinoPin: '40', notes: 'Requires igniter', enabled: true },
      { pinNumber: '4', function: 'Ignition 2', type: 'ignition', specs: 'Logic 5V, 330mA', arduinoPin: '38', notes: 'Requires igniter', enabled: true },
      { pinNumber: '5', function: 'Fuel Pump', type: 'fuel_pump', specs: 'Ground switch', arduinoPin: '37', notes: 'Via driver', enabled: true },
      { pinNumber: '6', function: 'Idle Control', type: 'idle_control', specs: 'PWM/On-Off', arduinoPin: '45', notes: '2-wire valve', enabled: true },
      { pinNumber: '7', function: 'Fan Output', type: 'fan', specs: 'Ground switch', arduinoPin: '47', notes: 'Optional', enabled: false },
      { pinNumber: '8', function: 'Tachometer Out', type: 'tachometer', specs: 'Logic level', arduinoPin: '49', notes: 'Optional', enabled: false },
    ],
    inputs: [
      { pinNumber: '9', function: 'Crank Input (RPM1)', type: 'crank_trigger', specs: 'VR/Hall', arduinoPin: '19', notes: 'Requires VR cond. for VR', enabled: true },
      { pinNumber: '10', function: 'Cam Input (RPM2)', type: 'cam_trigger', specs: 'VR/Hall', arduinoPin: '18', notes: 'Optional', enabled: false },
      { pinNumber: '11', function: 'MAP Signal', type: 'map_sensor', specs: 'Analog', arduinoPin: 'A0', notes: 'Onboard sensor', enabled: true },
      { pinNumber: '12', function: 'TPS', type: 'tps', specs: 'Analog 0-5V', arduinoPin: 'A2', notes: '0-5V', enabled: true },
      { pinNumber: '13', function: 'CLT', type: 'clt', specs: 'Analog', arduinoPin: 'A1', notes: 'Thermistor', enabled: true },
      { pinNumber: '14', function: 'IAT', type: 'iat', specs: 'Analog', arduinoPin: 'A3', notes: 'Thermistor', enabled: true },
      { pinNumber: '15', function: 'O2 Sensor', type: 'o2_sensor', specs: 'Analog 0-5V', arduinoPin: 'A4', notes: '0-5V wideband', enabled: true },
      { pinNumber: '16', function: 'Battery Voltage', type: 'battery_voltage', specs: 'Analog', arduinoPin: 'A15', notes: 'Voltage sense', enabled: true },
    ],
    powerGrounds: [
      { pinNumber: '17-20', function: 'Grounds', type: 'spare', specs: 'Sensor and power', notes: 'Sensor and power grounds', enabled: true },
      { pinNumber: '21', function: '+12V Switched', type: 'spare', specs: 'Main power', notes: 'Main power', enabled: true },
      { pinNumber: '22', function: '+5V Reference', type: 'spare', specs: 'Sensor power', notes: 'For TPS and MAP', enabled: true },
      { pinNumber: '23-24', function: 'Spare/Optional', type: 'spare', specs: 'Expansion', notes: 'Expansion capability', enabled: false },
    ],
    features: [
      'Very compact design (smaller than a deck of cards)',
      'Molex Micro-Fit 3.0 connectors',
      'Daughter board expansion capability',
      'Can be modified for 4-cylinder sequential injection',
    ],
    engineSupport: [
      '1-2 cylinders: Full sequential',
      '4 cylinders: Wasted spark, batch fire (can be modified for sequential)',
    ],
    mapSize: '16x16',
    recommendedUse: 'Small engines, motorcycles, minimalist builds, distributor replacement',
  },

  {
    id: 'speeduino-c2c',
    name: 'Speeduino C2C (Compact 2 Channel)',
    manufacturer: 'Speeduino',
    platform: 'Onboard Arduino-compatible chip',
    connectorType: '24-pin Microfit Molex connector',
    outputs: [
      { pinNumber: '1', function: 'Injector 1', type: 'injector', specs: 'Ground switch, 6A', notes: 'Standard injector output', enabled: true },
      { pinNumber: '2', function: 'Injector 2', type: 'injector', specs: 'Ground switch, 6A', notes: 'Standard injector output', enabled: true },
      { pinNumber: '3', function: 'Ignition 1', type: 'ignition', specs: 'Logic level', notes: 'Requires igniter', enabled: true },
      { pinNumber: '4', function: 'Ignition 2', type: 'ignition', specs: 'Logic level', notes: 'Requires igniter', enabled: true },
      { pinNumber: '5', function: 'Fuel Pump', type: 'fuel_pump', specs: 'Ground switch', notes: 'Standard output', enabled: true },
      { pinNumber: '6', function: 'Idle Control', type: 'idle_control', specs: 'PWM', notes: 'PWM capable', enabled: true },
      { pinNumber: '7', function: 'Fan', type: 'fan', specs: 'Ground switch', notes: 'Can be repurposed as VVT', enabled: false },
      { pinNumber: '8', function: 'Tachometer', type: 'tachometer', specs: 'Logic level', notes: 'Optional', enabled: false },
    ],
    inputs: [
      { pinNumber: '9', function: 'Crank Input', type: 'crank_trigger', specs: 'VR/Hall', notes: 'Standard sensor inputs', enabled: true },
      { pinNumber: '10', function: 'MAP Sensor', type: 'map_sensor', specs: 'Analog', notes: 'Internal MAP Sensor', enabled: true },
      { pinNumber: '11', function: 'TPS', type: 'tps', specs: 'Analog 0-5V', notes: 'Standard input', enabled: true },
      { pinNumber: '12', function: 'CLT', type: 'clt', specs: 'Analog', notes: 'Standard input', enabled: true },
      { pinNumber: '13', function: 'IAT', type: 'iat', specs: 'Analog', notes: 'Standard input', enabled: true },
      { pinNumber: '14', function: 'O2 Sensor', type: 'o2_sensor', specs: 'Analog', notes: 'Standard input', enabled: true },
      { pinNumber: '15', function: 'Battery Voltage', type: 'battery_voltage', specs: 'Analog', notes: 'Voltage monitoring', enabled: true },
    ],
    powerGrounds: [
      { pinNumber: '16-20', function: 'Grounds', type: 'spare', specs: 'Multiple', notes: 'Power and sensor grounds', enabled: true },
      { pinNumber: '21', function: '+12V', type: 'spare', specs: 'Main power', notes: 'Main power input', enabled: true },
      { pinNumber: '22', function: '+5V Reference', type: 'spare', specs: 'Sensor power', notes: 'For sensors', enabled: true },
    ],
    features: [
      'Case size: 84 x 58 x 25mm (pack of cards size)',
      '24-pin Microfit Molex connector',
      'Pin-compatible with NO2C',
      'Plug-and-play Bluetooth module support',
      'Optional VR Conditioner',
    ],
    engineSupport: [
      'Up to 4 cylinders (fuel and ignition)',
    ],
    mapSize: '16x16',
    recommendedUse: 'Basic applications, motorcycles, small engines',
  },

  // ==================== MEGASQUIRT ECUs ====================
  {
    id: 'megasquirt-ms1',
    name: 'MegaSquirt MS1 (Legacy)',
    manufacturer: 'MegaSquirt',
    platform: 'Motorola MC68HC908 (8-bit)',
    connectorType: 'DB37 Male Connector',
    outputs: [
      { pinNumber: '20', function: 'Injector Bank A', type: 'injector', specs: 'Ground switch, 5A', notes: 'Can drive multiple low-Z with resistor box', enabled: true },
      { pinNumber: '21', function: 'Injector Bank B', type: 'injector', specs: 'Ground switch, 5A', notes: 'Optional second bank', enabled: false },
      { pinNumber: '30', function: 'Fidle (PWM Idle)', type: 'idle_control', specs: 'PWM, 1A', notes: 'Fast idle valve', enabled: true },
      { pinNumber: '36', function: 'Ignition Output', type: 'ignition', specs: '12V trigger', notes: 'With hardware mod for MS1/Extra', enabled: true },
      { pinNumber: '35', function: 'Fuel Pump', type: 'fuel_pump', specs: 'Ground switch, 1A', notes: 'Relay driver', enabled: true },
    ],
    inputs: [
      { pinNumber: '24', function: 'RPM Input (Crank)', type: 'crank_trigger', specs: 'VR or Hall', notes: 'Primary timing signal', enabled: true },
      { pinNumber: '26', function: 'TPS Sensor Vref (+5V)', type: 'spare', specs: 'Power', notes: 'For TPS sensor', enabled: true },
      { pinNumber: '28', function: 'TPS Signal', type: 'tps', specs: 'Analog 0-5V', notes: '0-5V input', enabled: true },
      { pinNumber: '32', function: 'CLT (Coolant Temp)', type: 'clt', specs: 'Analog', notes: 'Resistive sensor', enabled: true },
      { pinNumber: '33', function: 'MAT/IAT (Air Temp)', type: 'iat', specs: 'Analog', notes: 'Resistive sensor', enabled: true },
      { pinNumber: '34', function: 'O2 Sensor', type: 'o2_sensor', specs: 'Analog 0-1V', notes: '0-1V narrowband', enabled: true },
    ],
    powerGrounds: [
      { pinNumber: '1-19', function: 'Grounds', type: 'spare', specs: 'Multiple', notes: 'Most pins are ground (except 3,4,5,6)', enabled: true },
      { pinNumber: '1', function: '+5V Output', type: 'spare', specs: 'Low current', notes: 'For serial Bluetooth (rarely used)', enabled: false },
      { pinNumber: '37', function: '+12V Main Power', type: 'spare', specs: 'Main power', notes: 'Main power supply', enabled: true },
    ],
    features: [
      '12x12 Fuel and Spark tables',
      'Timing resolution: 0.3 degrees',
      'Fuel resolution: 100μs',
      'Compatible with V2.2 or V3 main boards',
    ],
    engineSupport: [
      'Any number of cylinders (batch fire fuel only)',
      'Wasted spark or COP ignition (with MS1/Extra)',
    ],
    mapSize: '12x12',
    recommendedUse: 'Budget builds, fuel-only applications (not recommended for new builds - MS2 preferred)',
  },

  {
    id: 'megasquirt-ms2',
    name: 'MegaSquirt MS2 (w/ MS2/Extra)',
    manufacturer: 'MegaSquirt',
    platform: 'MC9S12C64 (16-bit, 24MHz)',
    connectorType: 'DB37 Male Connector',
    outputs: [
      { pinNumber: '20', function: 'Injector A (Bank 1)', type: 'injector', specs: 'Ground switch, 5A', notes: 'Can drive 2 high-Z or low-Z with resistor', enabled: true },
      { pinNumber: '21', function: 'Injector B (Bank 2)', type: 'injector', specs: 'Ground switch, 5A', notes: 'Can drive 2 high-Z or low-Z with resistor', enabled: true },
      { pinNumber: '22', function: 'Injector C (Channel 3)', type: 'injector', specs: 'Ground switch, 5A', notes: 'With board modification', enabled: false },
      { pinNumber: '23', function: 'Injector D (Channel 4)', type: 'injector', specs: 'Ground switch, 5A', notes: 'With board modification', enabled: false },
      { pinNumber: '30', function: 'Spark Output A (Ign 1)', type: 'ignition', specs: '12V coil driver', notes: 'COP or igniter input', enabled: true },
      { pinNumber: '36', function: 'Spark Output B (Ign 2)', type: 'ignition', specs: '12V coil driver', notes: 'COP or igniter input', enabled: true },
      { pinNumber: '32', function: 'Spark Output C (Ign 3)', type: 'ignition', specs: '12V coil driver', notes: 'With board modification', enabled: false },
      { pinNumber: '33', function: 'Spark Output D (Ign 4)', type: 'ignition', specs: '12V coil driver', notes: 'With board modification', enabled: false },
      { pinNumber: '34', function: 'Spark Output E (Ign 5)', type: 'ignition', specs: '12V coil driver', notes: 'With board modification', enabled: false },
      { pinNumber: '35', function: 'Spark Output F (Ign 6)', type: 'ignition', specs: '12V coil driver', notes: 'With board modification', enabled: false },
      { pinNumber: '25', function: 'IAC1A / JS0 (Stepper A+)', type: 'idle_control', specs: 'Configurable', notes: 'Boost, N2O, Shift Light, Tach', enabled: true },
      { pinNumber: '27', function: 'IAC1B / JS1 (Stepper A-)', type: 'idle_control', specs: 'Configurable', notes: 'Boost, N2O, Fan, Aux', enabled: true },
      { pinNumber: '29', function: 'IAC2A / JS2 (Stepper B+)', type: 'idle_control', specs: 'Configurable', notes: 'Boost, N2O, Fan, Aux', enabled: false },
      { pinNumber: '31', function: 'IAC2B / JS3 (Stepper B-)', type: 'idle_control', specs: 'Configurable', notes: 'Boost, N2O, Fan, Aux', enabled: false },
      { pinNumber: '14', function: 'D14 Output (Fuel Pump)', type: 'fuel_pump', specs: 'Configurable', notes: 'Fuel Pump default', enabled: true },
      { pinNumber: '15', function: 'D15 Output (Tachometer)', type: 'tachometer', specs: 'Configurable', notes: 'Tachometer default', enabled: true },
      { pinNumber: '16', function: 'D16 Output (Fan Control)', type: 'fan', specs: 'Configurable', notes: 'Fan Control default', enabled: true },
    ],
    inputs: [
      { pinNumber: '24', function: 'RPM/Crank Input', type: 'crank_trigger', specs: 'VR or Hall', notes: 'Must be at TDC timing', enabled: true },
      { pinNumber: '25', function: 'Cam/Second Trigger', type: 'cam_trigger', specs: 'VR or Hall', notes: 'Optional, for sequential', enabled: false },
      { pinNumber: '26', function: 'Vref (+5V)', type: 'spare', specs: 'Power output', notes: 'For TPS and MAP sensors', enabled: true },
      { pinNumber: '28', function: 'TPS Signal', type: 'tps', specs: 'Analog 0-5V', notes: 'Throttle position', enabled: true },
      { pinNumber: '32', function: 'CLT Signal', type: 'clt', specs: 'Analog', notes: 'Coolant temperature', enabled: true },
      { pinNumber: '33', function: 'MAT/IAT Signal', type: 'iat', specs: 'Analog', notes: 'Air temperature', enabled: true },
      { pinNumber: '34', function: 'O2 Sensor', type: 'o2_sensor', specs: 'Analog 0-5V', notes: 'Wideband controller output', enabled: true },
    ],
    powerGrounds: [
      { pinNumber: '1', function: 'Ground', type: 'spare', specs: 'Sensor ground', notes: 'Sensor ground', enabled: true },
      { pinNumber: '2, 7-13', function: 'Ground', type: 'spare', specs: 'Sensor grounds', notes: 'Sensor grounds', enabled: true },
      { pinNumber: '14-19', function: 'Ground', type: 'spare', specs: 'Power grounds', notes: 'Power grounds (connect to engine)', enabled: true },
      { pinNumber: '37', function: '+12V Main Power', type: 'spare', specs: 'Main power', notes: 'Fused main supply', enabled: true },
    ],
    features: [
      '16x16 Fuel and Spark tables (with MS2/Extra)',
      'Timing resolution: 0.1 degrees',
      'Fuel resolution: 0.67μs (theoretical)',
      'CAN Communications built-in',
      'Every-tooth wheel decoding',
      'Compatible with V3 or V3.57 main boards',
    ],
    engineSupport: [
      'Up to 4 cylinders: Sequential fuel and COP ignition (with modifications)',
      'Up to 8 cylinders: Batch fire fuel, wasted spark',
    ],
    mapSize: '16x16',
    recommendedUse: 'Intermediate to advanced builds, boosted applications, engines requiring precise control',
  },

  {
    id: 'megasquirt-ms3',
    name: 'MegaSquirt MS3 + MS3X',
    manufacturer: 'MegaSquirt',
    platform: 'MC9S12XEP100 (16-bit, 50MHz with 100MHz RISC core)',
    connectorType: 'Main Board DB37 + MS3X DB37',
    outputs: [
      { pinNumber: 'X1', function: 'Injector 1', type: 'injector', specs: 'High-z, sequential', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'X2', function: 'Injector 2', type: 'injector', specs: 'High-z, sequential', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'X3', function: 'Injector 3', type: 'injector', specs: 'High-z, sequential', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'X4', function: 'Injector 4', type: 'injector', specs: 'High-z, sequential', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'X5', function: 'Injector 5', type: 'injector', specs: 'High-z, sequential', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'X6', function: 'Injector 6', type: 'injector', specs: 'High-z, sequential', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'X7', function: 'Injector 7', type: 'injector', specs: 'High-z, sequential', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'X8', function: 'Injector 8', type: 'injector', specs: 'High-z, sequential', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'X9', function: 'Ignition 1', type: 'ignition', specs: 'Logic level COP', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'X10', function: 'Ignition 2', type: 'ignition', specs: 'Logic level COP', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'X11', function: 'Ignition 3', type: 'ignition', specs: 'Logic level COP', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'X12', function: 'Ignition 4', type: 'ignition', specs: 'Logic level COP', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'X13', function: 'Ignition 5', type: 'ignition', specs: 'Logic level COP', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'X14', function: 'Ignition 6', type: 'ignition', specs: 'Logic level COP', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'X15', function: 'Ignition 7', type: 'ignition', specs: 'Logic level COP', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'X16', function: 'Ignition 8', type: 'ignition', specs: 'Logic level COP', notes: 'Full sequential capable', enabled: true },
      { pinNumber: 'X17', function: 'Nitrous Stage 1', type: 'auxiliary', specs: 'Relay/Solenoid', notes: 'Configurable', enabled: false },
      { pinNumber: 'X18', function: 'Nitrous Stage 2', type: 'auxiliary', specs: 'Relay/Solenoid', notes: 'Configurable', enabled: false },
      { pinNumber: 'X19', function: 'Boost Control', type: 'boost', specs: 'PWM', notes: 'Boost solenoid control', enabled: false },
      { pinNumber: 'X20', function: 'Water Injection', type: 'auxiliary', specs: 'Relay/Solenoid', notes: 'Configurable', enabled: false },
      { pinNumber: 'X21', function: 'Fan Control', type: 'fan', specs: 'Relay', notes: 'Fan relay control', enabled: true },
      { pinNumber: 'X22', function: 'Shift Light', type: 'auxiliary', specs: 'Output', notes: 'Shift light output', enabled: false },
      { pinNumber: 'X23', function: 'Tachometer Output', type: 'tachometer', specs: 'Signal', notes: 'Tach output', enabled: true },
      { pinNumber: 'Main14', function: 'Fuel Pump', type: 'fuel_pump', specs: 'Relay', notes: 'Fuel pump relay', enabled: true },
      { pinNumber: 'Main25-31', function: 'Idle Control (Stepper)', type: 'idle_control', specs: 'Stepper or PWM', notes: 'Stepper IAC control', enabled: true },
    ],
    inputs: [
      { pinNumber: 'Main24', function: 'RPM/Crank Input', type: 'crank_trigger', specs: 'VR or Hall', notes: 'Primary trigger', enabled: true },
      { pinNumber: 'X24', function: 'Cam Input', type: 'cam_trigger', specs: 'VR or Hall', notes: 'Secondary trigger', enabled: false },
      { pinNumber: 'Main26', function: 'Vref (+5V)', type: 'spare', specs: 'Power output', notes: 'Sensor power', enabled: true },
      { pinNumber: 'Main28', function: 'TPS Signal', type: 'tps', specs: 'Analog 0-5V', notes: 'Throttle position', enabled: true },
      { pinNumber: 'Main32', function: 'CLT Signal', type: 'clt', specs: 'Analog', notes: 'Coolant temperature', enabled: true },
      { pinNumber: 'Main33', function: 'MAT/IAT Signal', type: 'iat', specs: 'Analog', notes: 'Air temperature', enabled: true },
      { pinNumber: 'Main34', function: 'O2 Sensor', type: 'o2_sensor', specs: 'Analog 0-5V', notes: 'Wideband controller output', enabled: true },
      { pinNumber: 'X25', function: 'MAP Sensor', type: 'map_sensor', specs: 'Analog', notes: 'Manifold pressure', enabled: true },
      { pinNumber: 'X26', function: 'External MAP', type: 'spare', specs: 'Analog', notes: 'Additional MAP sensor', enabled: false },
      { pinNumber: 'X27', function: 'Pre-Intercooler Pressure', type: 'spare', specs: 'Analog', notes: 'Pressure sensor', enabled: false },
      { pinNumber: 'X28', function: 'Post-Intercooler Pressure', type: 'spare', specs: 'Analog', notes: 'Pressure sensor', enabled: false },
      { pinNumber: 'X29', function: 'Launch Control Switch', type: 'launch_control', specs: 'Digital', notes: 'Switch input', enabled: false },
      { pinNumber: 'X30', function: 'Idle-Up Switch', type: 'spare', specs: 'Digital', notes: 'Switch input', enabled: false },
      { pinNumber: 'X31', function: 'Nitrous Arming Switch', type: 'spare', specs: 'Digital', notes: 'Switch input', enabled: false },
      { pinNumber: 'X32', function: 'Map Switching', type: 'spare', specs: 'Digital', notes: 'Switch input', enabled: false },
    ],
    powerGrounds: [
      { pinNumber: 'Main1-19', function: 'Grounds', type: 'spare', specs: 'Multiple', notes: 'Sensor and power grounds', enabled: true },
      { pinNumber: 'Main37', function: '+12V Main Power', type: 'spare', specs: 'Main power', notes: 'Main power supply', enabled: true },
      { pinNumber: 'X33-37', function: 'MS3X Grounds', type: 'spare', specs: 'Multiple', notes: 'Expansion board grounds', enabled: true },
    ],
    features: [
      '16x16 Fuel and Spark tables',
      'Onboard SD Card logging',
      'Built-in USB-Serial adapter',
      'Per-cylinder fuel trim',
      'Per-cylinder spark trim',
      'Mappable injector timing',
      'CAN Communications',
      'Over 50 different ignition modes supported',
    ],
    engineSupport: [
      'Up to 8 cylinders: Full sequential fuel & spark',
      'V12: Semi-sequential fuel, wasted spark ignition',
      '4-rotor Wankel: Leading & trailing ignition outputs',
    ],
    mapSize: '16x16',
    recommendedUse: 'Professional builds, V8 applications, complex setups, race applications',
  },

  {
    id: 'microsquirt',
    name: 'MicroSquirt',
    manufacturer: 'MegaSquirt',
    platform: 'MC9S12C64 (same as MS2)',
    connectorType: 'Custom connector',
    outputs: [
      { pinNumber: '1', function: 'Injector Output 1', type: 'injector', specs: '5A max each', notes: 'Up to 4 high-z injectors per channel', enabled: true },
      { pinNumber: '2', function: 'Injector Output 2', type: 'injector', specs: '5A max each', notes: 'Up to 4 high-z injectors per channel', enabled: true },
      { pinNumber: '3', function: 'Ignition Output 1', type: 'ignition', specs: 'Logic level', notes: '2-channel standard configuration', enabled: true },
      { pinNumber: '4', function: 'Ignition Output 2', type: 'ignition', specs: 'Logic level', notes: '2-channel standard configuration', enabled: true },
      { pinNumber: '5', function: 'Ignition Output 3', type: 'ignition', specs: 'Logic level', notes: '4-channel for odd-fire or 4-cyl COP', enabled: false },
      { pinNumber: '6', function: 'Ignition Output 4', type: 'ignition', specs: 'Logic level', notes: '4-channel for odd-fire or 4-cyl COP', enabled: false },
      { pinNumber: '7', function: 'Fuel Pump Output', type: 'fuel_pump', specs: 'Relay driver', notes: 'Standard output', enabled: true },
      { pinNumber: '8', function: 'Idle Control Output', type: 'idle_control', specs: 'PWM', notes: 'Idle control', enabled: true },
    ],
    inputs: [
      { pinNumber: '9', function: 'Crank Trigger Input', type: 'crank_trigger', specs: 'VR or Hall', notes: 'Primary trigger', enabled: true },
      { pinNumber: '10', function: 'Cam Trigger Input', type: 'cam_trigger', specs: 'VR or Hall', notes: 'Secondary trigger', enabled: false },
      { pinNumber: '11', function: 'MAP Sensor', type: 'map_sensor', specs: 'Analog', notes: 'Manifold pressure', enabled: true },
      { pinNumber: '12', function: 'TPS', type: 'tps', specs: 'Analog 0-5V', notes: 'Throttle position', enabled: true },
      { pinNumber: '13', function: 'CLT', type: 'clt', specs: 'Analog', notes: 'Coolant temperature', enabled: true },
      { pinNumber: '14', function: 'IAT', type: 'iat', specs: 'Analog', notes: 'Air temperature', enabled: true },
      { pinNumber: '15', function: 'O2/Wideband', type: 'o2_sensor', specs: 'Analog 0-5V', notes: 'Oxygen sensor', enabled: true },
      { pinNumber: '16', function: 'Battery Voltage', type: 'battery_voltage', specs: 'Analog', notes: 'Voltage monitoring', enabled: true },
      { pinNumber: '17', function: 'Flex Fuel', type: 'flex_fuel', specs: 'Frequency', notes: 'Optional', enabled: false },
    ],
    powerGrounds: [
      { pinNumber: 'Multiple', function: 'Grounds', type: 'spare', specs: 'Multiple', notes: 'Distributed grounds', enabled: true },
      { pinNumber: 'Power', function: '+12V', type: 'spare', specs: 'Main power', notes: 'Main power input', enabled: true },
      { pinNumber: 'Vref', function: '+5V Reference', type: 'spare', specs: 'Sensor power', notes: 'For sensors', enabled: true },
    ],
    features: [
      'Compact size',
      'CAN Communications built-in',
      '2.5mm jack for serial connection',
      'Can be used as I/O extender for other MegaSquirt units',
      'Transmission controller firmware available',
    ],
    engineSupport: [
      '1-4 cylinders: Various configurations',
      '8 cylinders: Wasted spark mode',
      'Odd-fire engines: With dual spark firmware',
    ],
    mapSize: '16x16',
    recommendedUse: 'Small engines, motorcycles, ATVs, auxiliary controller applications',
  },

  {
    id: 'ms3pro',
    name: 'MS3Pro (Plug-and-Play Variants)',
    manufacturer: 'MegaSquirt',
    platform: 'MS3 processor + V3.57 board',
    connectorType: 'Vehicle-specific connectors',
    outputs: [
      { pinNumber: 'X1', function: 'Injector 1', type: 'injector', specs: 'High-z, sequential', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X2', function: 'Injector 2', type: 'injector', specs: 'High-z, sequential', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X3', function: 'Injector 3', type: 'injector', specs: 'High-z, sequential', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X4', function: 'Injector 4', type: 'injector', specs: 'High-z, sequential', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X5', function: 'Injector 5', type: 'injector', specs: 'High-z, sequential', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X6', function: 'Injector 6', type: 'injector', specs: 'High-z, sequential', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X7', function: 'Injector 7', type: 'injector', specs: 'High-z, sequential', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X8', function: 'Injector 8', type: 'injector', specs: 'High-z, sequential', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X9', function: 'Ignition 1', type: 'ignition', specs: 'Logic level COP', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X10', function: 'Ignition 2', type: 'ignition', specs: 'Logic level COP', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X11', function: 'Ignition 3', type: 'ignition', specs: 'Logic level COP', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X12', function: 'Ignition 4', type: 'ignition', specs: 'Logic level COP', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X13', function: 'Ignition 5', type: 'ignition', specs: 'Logic level COP', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X14', function: 'Ignition 6', type: 'ignition', specs: 'Logic level COP', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X15', function: 'Ignition 7', type: 'ignition', specs: 'Logic level COP', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X16', function: 'Ignition 8', type: 'ignition', specs: 'Logic level COP', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'Main14', function: 'Fuel Pump', type: 'fuel_pump', specs: 'Relay', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X21', function: 'Fan Control', type: 'fan', specs: 'Relay', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X23', function: 'Tachometer Output', type: 'tachometer', specs: 'Signal', notes: 'Same as MS3 + MS3X', enabled: true },
    ],
    inputs: [
      { pinNumber: 'Main24', function: 'RPM/Crank Input', type: 'crank_trigger', specs: 'VR or Hall', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X24', function: 'Cam Input', type: 'cam_trigger', specs: 'VR or Hall', notes: 'Same as MS3 + MS3X', enabled: false },
      { pinNumber: 'Main28', function: 'TPS Signal', type: 'tps', specs: 'Analog 0-5V', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'Main32', function: 'CLT Signal', type: 'clt', specs: 'Analog', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'Main33', function: 'MAT/IAT Signal', type: 'iat', specs: 'Analog', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'Main34', function: 'O2 Sensor', type: 'o2_sensor', specs: 'Analog 0-5V', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'X25', function: 'MAP Sensor', type: 'map_sensor', specs: 'Analog', notes: 'Same as MS3 + MS3X', enabled: true },
    ],
    powerGrounds: [
      { pinNumber: 'Main1-19', function: 'Grounds', type: 'spare', specs: 'Multiple', notes: 'Same as MS3 + MS3X', enabled: true },
      { pinNumber: 'Main37', function: '+12V Main Power', type: 'spare', specs: 'Main power', notes: 'Same as MS3 + MS3X', enabled: true },
    ],
    features: [
      'Plug-and-play installation (vehicle-specific)',
      'Pre-configured base maps',
      'Professional assembly',
      'Minimal DIY required',
      'Same features as MS3 + MS3X',
    ],
    engineSupport: [
      'Application-specific (Miata, LS1, etc.)',
    ],
    mapSize: '16x16',
    recommendedUse: 'Plug-and-play installations, specific vehicle applications',
  },
];

// Helper functions
export function getECUById(id: string): ECUConfig | undefined {
  return ECU_DATABASE.find(ecu => ecu.id === id);
}

export function getECUsByManufacturer(manufacturer: 'Speeduino' | 'MegaSquirt'): ECUConfig[] {
  return ECU_DATABASE.filter(ecu => ecu.manufacturer === manufacturer);
}

export function getAllECUNames(): { id: string; name: string; manufacturer: string }[] {
  return ECU_DATABASE.map(ecu => ({
    id: ecu.id,
    name: ecu.name,
    manufacturer: ecu.manufacturer,
  }));
}
