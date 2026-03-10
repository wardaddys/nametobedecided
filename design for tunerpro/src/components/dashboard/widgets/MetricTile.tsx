import { useEffect, useState } from "react";
import { motion } from "motion/react";

interface MetricTileProps {
  label: string;
  value?: number;
  unit: string;
  liveTuning: boolean;
  min: number;
  max: number;
  warningThreshold?: number;
  criticalThreshold?: number;
  inverted?: boolean; // For values where lower is worse (like battery voltage)
}

export function MetricTile({
  label,
  value: staticValue,
  unit,
  liveTuning,
  min,
  max,
  warningThreshold,
  criticalThreshold,
  inverted = false,
}: MetricTileProps) {
  const [value, setValue] = useState(staticValue || min);

  useEffect(() => {
    if (!liveTuning) {
      setValue(staticValue || min);
      return;
    }

    const interval = setInterval(() => {
      setValue((prev) => {
        const range = max - min;
        const change = (Math.random() - 0.5) * (range * 0.05);
        const newValue = Math.max(min, Math.min(max, prev + change));
        return newValue;
      });
    }, 300);

    return () => clearInterval(interval);
  }, [liveTuning, staticValue, min, max]);

  // Determine status color
  let statusColor = "#00FF00"; // Good
  let statusText = "Normal";

  if (warningThreshold !== undefined && criticalThreshold !== undefined) {
    if (inverted) {
      if (value <= criticalThreshold) {
        statusColor = "#FF0000";
        statusText = "Critical";
      } else if (value <= warningThreshold) {
        statusColor = "#FFFF00";
        statusText = "Warning";
      }
    } else {
      if (value >= criticalThreshold) {
        statusColor = "#FF0000";
        statusText = "Critical";
      } else if (value >= warningThreshold) {
        statusColor = "#FFFF00";
        statusText = "Warning";
      }
    }
  }

  const percentage = ((value - min) / (max - min)) * 100;

  return (
    <div className="w-full h-full bg-black border border-[#333333] p-3 flex flex-col justify-between">
      <div className="text-xs text-[#888888]">{label}</div>

      <div className="text-center">
        <motion.div
          className="font-data"
          style={{ 
            color: statusColor,
            fontSize: '28px',
            textShadow: statusText !== "Normal" ? `0 0 8px ${statusColor}` : 'none'
          }}
          animate={{ scale: statusText !== "Normal" ? [1, 1.05, 1] : 1 }}
          transition={{ duration: 0.5, repeat: Infinity }}
        >
          {typeof value === "number" ? value.toFixed(1) : "--"}
          <span style={{ fontSize: '16px' }} className="ml-1">{unit}</span>
        </motion.div>
      </div>

      {/* Progress bar */}
      <div className="w-full h-1 bg-[#1a1a1a] overflow-hidden">
        <motion.div
          className="h-full"
          style={{ backgroundColor: statusColor }}
          initial={{ width: 0 }}
          animate={{ width: `${percentage}%` }}
          transition={{ duration: 0.3 }}
        />
      </div>
    </div>
  );
}
