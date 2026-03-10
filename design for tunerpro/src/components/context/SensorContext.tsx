import { createContext, useContext, useState, ReactNode, useEffect } from "react";
import { validateSensor, ValidationResult, AnomalyDetector } from "../validation/SensorValidation";

export interface SensorConfig {
  id: string;
  name: string;
  channel: string;
  value: number;
  unit: string;
  min: number;
  max: number;
  showOnDashboard: boolean;
  validation?: ValidationResult;
}

interface SensorContextType {
  sensors: SensorConfig[];
  addSensor: (sensor: SensorConfig) => void;
  updateSensor: (id: string, updates: Partial<SensorConfig>) => void;
  removeSensor: (id: string) => void;
  getSensor: (id: string) => SensorConfig | undefined;
  anomalyDetector: AnomalyDetector;
}

const SensorContext = createContext<SensorContextType | undefined>(undefined);

export function SensorProvider({ children }: { children: ReactNode }) {
  const [sensors, setSensors] = useState<SensorConfig[]>([]);
  const [anomalyDetector] = useState(() => new AnomalyDetector());

  const addSensor = (sensor: SensorConfig) => {
    setSensors((prev) => [...prev, sensor]);
  };

  const updateSensor = (id: string, updates: Partial<SensorConfig>) => {
    setSensors((prev) =>
      prev.map((sensor) => {
        if (sensor.id === id) {
          const updated = { ...sensor, ...updates };
          
          // Auto-validate on value update
          if (updates.value !== undefined) {
            // Get context from other sensors
            const rpm = prev.find(s => s.name.toLowerCase().includes('rpm'))?.value;
            const map = prev.find(s => s.name.toLowerCase().includes('map'))?.value;
            const tps = prev.find(s => s.name.toLowerCase().includes('tps'))?.value;
            
            // Map sensor names to validation types
            const sensorTypeMap: Record<string, string> = {
              'rpm': 'rpm',
              'map': 'map',
              'tps': 'tps',
              'iat': 'iat',
              'clt': 'clt',
              'coolant': 'clt',
              'afr': 'afr',
              'o2': 'afr',
              'battery': 'batteryVoltage',
              'voltage': 'batteryVoltage',
              'boost': 'boost',
              'oil pressure': 'oilPressure',
              'oil temp': 'oilTemp',
              'fuel pressure': 'fuelPressure'
            };
            
            const sensorType = Object.keys(sensorTypeMap).find(key => 
              sensor.name.toLowerCase().includes(key)
            );
            
            if (sensorType) {
              const validation = validateSensor(
                sensorTypeMap[sensorType],
                updates.value,
                { rpm, map, tps }
              );
              
              // Check for anomalies
              const anomaly = anomalyDetector.detect(sensorTypeMap[sensorType], updates.value);
              if (anomaly.detected) {
                validation.message = anomaly.message;
                validation.shouldAlert = true;
              }
              
              updated.validation = validation;
            }
          }
          
          return updated;
        }
        return sensor;
      })
    );
  };

  const removeSensor = (id: string) => {
    setSensors((prev) => prev.filter((sensor) => sensor.id !== id));
  };

  const getSensor = (id: string) => {
    return sensors.find((sensor) => sensor.id === id);
  };

  return (
    <SensorContext.Provider
      value={{ sensors, addSensor, updateSensor, removeSensor, getSensor, anomalyDetector }}
    >
      {children}
    </SensorContext.Provider>
  );
}

export function useSensors() {
  const context = useContext(SensorContext);
  if (!context) {
    throw new Error("useSensors must be used within a SensorProvider");
  }
  return context;
}
