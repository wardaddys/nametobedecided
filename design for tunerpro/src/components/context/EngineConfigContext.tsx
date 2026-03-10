import { createContext, useContext, useState, ReactNode } from 'react';
import { InjectorSpec } from '../database/InjectorDatabase';
import { DeadTimeData, DEFAULT_DEAD_TIME_DATA } from '../settings/DeadTimeTable';

export type EngineType = 'na' | 'turbocharged' | 'supercharged' | 'twin-turbo' | 'sequential-turbo';
export type ForcedInductionType = 'single-turbo' | 'parallel-twin' | 'sequential-twin' | 'supercharger' | 'twin-charger' | 'staged-boost';
export type ValveTrainType = 'standard' | 'vtec' | 'vvt' | 'vvti' | 'vanos' | 'variocam';
export type FuelSystemType = 'port-single' | 'port-staged' | 'direct-injection' | 'dual-injection';
export type ControlMode = 'closed-loop' | 'open-loop';

export interface InjectorConfig {
  injector: InjectorSpec | null;
  customFlowRate?: number;
  customPressure: number;
  deadTimeData: DeadTimeData;
}

export interface EngineConfig {
  engineType: EngineType;
  forcedInductionType: ForcedInductionType | null;
  valveTrainType: ValveTrainType;
  fuelSystemType: FuelSystemType;
  controlMode: ControlMode;
  
  // Injector configuration
  primaryInjector: InjectorConfig;
  secondaryInjector: InjectorConfig | null;
  
  // Feature flags based on configuration
  features: {
    hasBoostControl: boolean;
    hasVTECControl: boolean;
    hasStagedInjection: boolean;
    hasVVTControl: boolean;
    requiresDualMaps: boolean; // For VTEC low/high speed profiles
  };
}

interface EngineConfigContextType {
  config: EngineConfig;
  setEngineType: (type: EngineType) => void;
  setForcedInductionType: (type: ForcedInductionType | null) => void;
  setValveTrainType: (type: ValveTrainType) => void;
  setFuelSystemType: (type: FuelSystemType) => void;
  setControlMode: (mode: ControlMode) => void;
  setPrimaryInjector: (injector: InjectorSpec | null) => void;
  setSecondaryInjector: (injector: InjectorSpec | null) => void;
  updatePrimaryInjectorConfig: (updates: Partial<InjectorConfig>) => void;
  updateSecondaryInjectorConfig: (updates: Partial<InjectorConfig>) => void;
  updateConfig: (updates: Partial<EngineConfig>) => void;
}

const EngineConfigContext = createContext<EngineConfigContextType | undefined>(undefined);

// Calculate features based on configuration
function calculateFeatures(
  engineType: EngineType,
  valveTrainType: ValveTrainType,
  fuelSystemType: FuelSystemType
) {
  return {
    hasBoostControl: engineType !== 'na',
    hasVTECControl: valveTrainType === 'vtec',
    hasStagedInjection: fuelSystemType === 'port-staged',
    hasVVTControl: valveTrainType === 'vvt' || valveTrainType === 'vvti' || valveTrainType === 'vanos' || valveTrainType === 'variocam',
    requiresDualMaps: valveTrainType === 'vtec',
  };
}

const defaultInjectorConfig: InjectorConfig = {
  injector: null,
  customPressure: 3.0,
  deadTimeData: DEFAULT_DEAD_TIME_DATA,
};

const defaultConfig: EngineConfig = {
  engineType: 'turbocharged',
  forcedInductionType: 'single-turbo',
  valveTrainType: 'vtec',
  fuelSystemType: 'port-single',
  controlMode: 'closed-loop',
  primaryInjector: { ...defaultInjectorConfig },
  secondaryInjector: null,
  features: {
    hasBoostControl: true,
    hasVTECControl: true,
    hasStagedInjection: false,
    hasVVTControl: false,
    requiresDualMaps: true,
  },
};

export function EngineConfigProvider({ children }: { children: ReactNode }) {
  const [config, setConfig] = useState<EngineConfig>(defaultConfig);

  const setEngineType = (type: EngineType) => {
    setConfig(prev => {
      const features = calculateFeatures(type, prev.valveTrainType, prev.fuelSystemType);
      // Auto-set forced induction type based on engine type
      let forcedInductionType = prev.forcedInductionType;
      if (type === 'na') {
        forcedInductionType = null;
      } else if (type === 'turbocharged' && !forcedInductionType) {
        forcedInductionType = 'single-turbo';
      } else if (type === 'supercharged') {
        forcedInductionType = 'supercharger';
      }
      return { ...prev, engineType: type, forcedInductionType, features };
    });
  };

  const setForcedInductionType = (type: ForcedInductionType | null) => {
    setConfig(prev => ({ ...prev, forcedInductionType: type }));
  };

  const setValveTrainType = (type: ValveTrainType) => {
    setConfig(prev => {
      const features = calculateFeatures(prev.engineType, type, prev.fuelSystemType);
      return { ...prev, valveTrainType: type, features };
    });
  };

  const setFuelSystemType = (type: FuelSystemType) => {
    setConfig(prev => {
      const features = calculateFeatures(prev.engineType, prev.valveTrainType, type);
      const secondaryInjector = features.hasStagedInjection && !prev.secondaryInjector 
        ? { ...defaultInjectorConfig }
        : prev.secondaryInjector;
      return { ...prev, fuelSystemType: type, features, secondaryInjector };
    });
  };

  const setControlMode = (mode: ControlMode) => {
    setConfig(prev => ({ ...prev, controlMode: mode }));
  };

  const setPrimaryInjector = (injector: InjectorSpec | null) => {
    setConfig(prev => ({
      ...prev,
      primaryInjector: {
        ...prev.primaryInjector,
        injector,
      },
    }));
  };

  const setSecondaryInjector = (injector: InjectorSpec | null) => {
    setConfig(prev => ({
      ...prev,
      secondaryInjector: prev.secondaryInjector ? {
        ...prev.secondaryInjector,
        injector,
      } : null,
    }));
  };

  const updatePrimaryInjectorConfig = (updates: Partial<InjectorConfig>) => {
    setConfig(prev => ({
      ...prev,
      primaryInjector: {
        ...prev.primaryInjector,
        ...updates,
      },
    }));
  };

  const updateSecondaryInjectorConfig = (updates: Partial<InjectorConfig>) => {
    setConfig(prev => ({
      ...prev,
      secondaryInjector: prev.secondaryInjector ? {
        ...prev.secondaryInjector,
        ...updates,
      } : null,
    }));
  };

  const updateConfig = (updates: Partial<EngineConfig>) => {
    setConfig(prev => ({ ...prev, ...updates }));
  };

  return (
    <EngineConfigContext.Provider
      value={{
        config,
        setEngineType,
        setForcedInductionType,
        setValveTrainType,
        setFuelSystemType,
        setControlMode,
        setPrimaryInjector,
        setSecondaryInjector,
        updatePrimaryInjectorConfig,
        updateSecondaryInjectorConfig,
        updateConfig,
      }}
    >
      {children}
    </EngineConfigContext.Provider>
  );
}

export function useEngineConfig() {
  const context = useContext(EngineConfigContext);
  if (context === undefined) {
    throw new Error('useEngineConfig must be used within an EngineConfigProvider');
  }
  return context;
}
