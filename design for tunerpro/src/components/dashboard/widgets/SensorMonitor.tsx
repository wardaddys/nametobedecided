import { useEffect, useState } from "react";
import { motion, AnimatePresence } from "motion/react";
import { validateSensor, sensorRanges, formatSensorValue } from "../../validation/SensorValidation";
import { AlertTriangle, CheckCircle2, AlertCircle, Info, Thermometer, Gauge, Zap, Activity } from "lucide-react";

interface SensorMonitorProps {
  liveTuning: boolean;
}

interface SensorData {
  name: string;
  type: string;
  value: number;
  icon: React.ReactNode;
}

export function SensorMonitor({ liveTuning }: SensorMonitorProps) {
  const [sensors, setSensors] = useState<SensorData[]>([
    { name: 'MAP', type: 'map', value: 100, icon: <Gauge className="w-4 h-4" /> },
    { name: 'TPS', type: 'tps', value: 5, icon: <Activity className="w-4 h-4" /> },
    { name: 'IAT', type: 'iat', value: 25, icon: <Thermometer className="w-4 h-4" /> },
    { name: 'CLT', type: 'clt', value: 85, icon: <Thermometer className="w-4 h-4" /> },
    { name: 'Battery', type: 'batteryVoltage', value: 13.8, icon: <Zap className="w-4 h-4" /> },
    { name: 'Timing', type: 'ignitionTiming', value: 15, icon: <Activity className="w-4 h-4" /> }
  ]);

  const [selectedSensor, setSelectedSensor] = useState<string | null>(null);

  useEffect(() => {
    if (!liveTuning) {
      setSensors([
        { name: 'MAP', type: 'map', value: 100, icon: <Gauge className="w-4 h-4" /> },
        { name: 'TPS', type: 'tps', value: 5, icon: <Activity className="w-4 h-4" /> },
        { name: 'IAT', type: 'iat', value: 25, icon: <Thermometer className="w-4 h-4" /> },
        { name: 'CLT', type: 'clt', value: 85, icon: <Thermometer className="w-4 h-4" /> },
        { name: 'Battery', type: 'batteryVoltage', value: 13.8, icon: <Zap className="w-4 h-4" /> },
        { name: 'Timing', type: 'ignitionTiming', value: 15, icon: <Activity className="w-4 h-4" /> }
      ]);
      return;
    }

    const interval = setInterval(() => {
      setSensors(prev => prev.map(sensor => {
        let change = 0;
        let newValue = sensor.value;
        
        switch (sensor.type) {
          case 'map':
            change = (Math.random() - 0.5) * 5;
            newValue = Math.max(30, Math.min(200, sensor.value + change));
            break;
          case 'tps':
            change = (Math.random() - 0.5) * 10;
            newValue = Math.max(0, Math.min(100, sensor.value + change));
            break;
          case 'iat':
            change = (Math.random() - 0.5) * 2;
            newValue = Math.max(-20, Math.min(80, sensor.value + change));
            break;
          case 'clt':
            change = (Math.random() - 0.5) * 1;
            newValue = Math.max(40, Math.min(110, sensor.value + change));
            break;
          case 'batteryVoltage':
            change = (Math.random() - 0.5) * 0.2;
            newValue = Math.max(11, Math.min(15, sensor.value + change));
            break;
          case 'ignitionTiming':
            change = (Math.random() - 0.5) * 3;
            newValue = Math.max(5, Math.min(45, sensor.value + change));
            break;
        }
        
        return { ...sensor, value: newValue };
      }));
    }, 500);

    return () => clearInterval(interval);
  }, [liveTuning]);

  // Get context for validation
  const rpm = 3000; // Could be passed as prop
  const map = sensors.find(s => s.type === 'map')?.value || 100;
  const tps = sensors.find(s => s.type === 'tps')?.value || 5;

  const renderSensorRow = (sensor: SensorData) => {
    const validation = validateSensor(sensor.type, sensor.value, { rpm, map, tps });
    const range = sensorRanges[sensor.type];
    
    if (!range) return null;

    const percentage = ((sensor.value - range.critical.min) / (range.critical.max - range.critical.min)) * 100;

    return (
      <motion.div
        key={sensor.name}
        className="relative group cursor-pointer"
        onClick={() => setSelectedSensor(selectedSensor === sensor.name ? null : sensor.name)}
        whileHover={{ scale: 1.02 }}
      >
        <div className={`p-3 rounded border transition-all ${
          validation.level === 'critical' 
            ? 'bg-red-950/30 border-red-600' 
            : validation.level === 'caution'
            ? 'bg-yellow-950/30 border-yellow-600'
            : 'bg-[#1a1a1a] border-[#333333] hover:border-[#444444]'
        }`}>
          <div className="flex items-center justify-between mb-2">
            <div className="flex items-center gap-2">
              <div style={{ color: validation.color }}>
                {sensor.icon}
              </div>
              <span className="text-sm text-[#CCCCCC]">{sensor.name}</span>
              {validation.level === 'critical' && (
                <motion.div
                  animate={{ rotate: [0, 15, -15, 0] }}
                  transition={{ duration: 0.5, repeat: Infinity }}
                >
                  <AlertTriangle className="w-4 h-4 text-red-500" />
                </motion.div>
              )}
              {validation.level === 'caution' && (
                <AlertCircle className="w-4 h-4 text-yellow-500" />
              )}
              {validation.level === 'normal' && (
                <CheckCircle2 className="w-3 h-3 text-green-500" />
              )}
            </div>
            <div 
              className="font-data"
              style={{ 
                color: validation.color,
                textShadow: validation.level !== 'normal' ? `0 0 4px ${validation.color}` : 'none'
              }}
            >
              {formatSensorValue(sensor.type, sensor.value)} {range.unit}
            </div>
          </div>

          {/* Progress bar showing position in range */}
          <div className="h-1.5 bg-[#0a0a0a] rounded-full overflow-hidden relative">
            {/* Optimal zone indicator */}
            <div 
              className="absolute h-full bg-green-500/20"
              style={{
                left: `${((range.optimal.min - range.critical.min) / (range.critical.max - range.critical.min)) * 100}%`,
                width: `${((range.optimal.max - range.optimal.min) / (range.critical.max - range.critical.min)) * 100}%`
              }}
            />
            {/* Current value */}
            <motion.div
              className="h-full rounded-full"
              style={{ 
                width: `${Math.min(100, Math.max(0, percentage))}%`,
                backgroundColor: validation.color
              }}
              initial={{ width: 0 }}
              animate={{ width: `${Math.min(100, Math.max(0, percentage))}%` }}
              transition={{ duration: 0.3 }}
            />
          </div>

          {/* Expanded info */}
          <AnimatePresence>
            {selectedSensor === sensor.name && (
              <motion.div
                initial={{ opacity: 0, height: 0 }}
                animate={{ opacity: 1, height: 'auto' }}
                exit={{ opacity: 0, height: 0 }}
                className="mt-2 pt-2 border-t border-[#333333]"
              >
                <div className="text-xs space-y-1 text-[#888888]">
                  <div><strong className="text-[#CCCCCC]">Optimal:</strong> {range.optimal.description}</div>
                  <div><strong className="text-[#CCCCCC]">Calibration:</strong> {range.calibration}</div>
                  {range.formula && (
                    <div className="font-mono text-[10px] bg-black/50 p-1 rounded mt-1">
                      {range.formula}
                    </div>
                  )}
                  {validation.shouldAlert && (
                    <div className="mt-2 p-2 rounded" style={{
                      backgroundColor: `${validation.color}22`,
                      border: `1px solid ${validation.color}66`,
                      color: validation.color
                    }}>
                      {validation.message}
                    </div>
                  )}
                </div>
              </motion.div>
            )}
          </AnimatePresence>
        </div>
      </motion.div>
    );
  };

  // Count alerts
  const criticalCount = sensors.filter(s => 
    validateSensor(s.type, s.value, { rpm, map, tps }).level === 'critical'
  ).length;
  const cautionCount = sensors.filter(s => 
    validateSensor(s.type, s.value, { rpm, map, tps }).level === 'caution'
  ).length;

  return (
    <div className="w-full h-full bg-black border border-[#333333] p-4 flex flex-col overflow-auto">
      {/* Header */}
      <div className="flex items-center justify-between mb-4">
        <div>
          <h3 className="text-[#CCCCCC]">Sensor Monitor</h3>
          <p className="text-xs text-[#888888]">Speeduino Validated Ranges</p>
        </div>
        <div className="flex items-center gap-2">
          {criticalCount > 0 && (
            <motion.div 
              className="flex items-center gap-1 bg-red-950/30 border border-red-600 px-2 py-1 rounded text-xs text-red-500"
              animate={{ scale: [1, 1.05, 1] }}
              transition={{ duration: 1, repeat: Infinity }}
            >
              <AlertTriangle className="w-3 h-3" />
              {criticalCount}
            </motion.div>
          )}
          {cautionCount > 0 && (
            <div className="flex items-center gap-1 bg-yellow-950/30 border border-yellow-600 px-2 py-1 rounded text-xs text-yellow-500">
              <AlertCircle className="w-3 h-3" />
              {cautionCount}
            </div>
          )}
          {criticalCount === 0 && cautionCount === 0 && (
            <div className="flex items-center gap-1 text-xs text-green-500">
              <CheckCircle2 className="w-3 h-3" />
              All Normal
            </div>
          )}
        </div>
      </div>

      {/* Sensor list */}
      <div className="space-y-2 flex-1">
        {sensors.map(renderSensorRow)}
      </div>

      {/* Legend */}
      <div className="mt-4 pt-3 border-t border-[#333333] text-xs text-[#888888]">
        <div className="flex items-center gap-4">
          <div className="flex items-center gap-1">
            <div className="w-2 h-2 rounded-full bg-green-500"></div>
            <span>Normal</span>
          </div>
          <div className="flex items-center gap-1">
            <div className="w-2 h-2 rounded-full bg-yellow-500"></div>
            <span>Caution</span>
          </div>
          <div className="flex items-center gap-1">
            <div className="w-2 h-2 rounded-full bg-red-500"></div>
            <span>Critical</span>
          </div>
          <div className="ml-auto flex items-center gap-1 text-[#666666]">
            <Info className="w-3 h-3" />
            <span>Click for details</span>
          </div>
        </div>
      </div>
    </div>
  );
}
