import { useEffect, useState } from "react";
import { motion, AnimatePresence } from "motion/react";
import { validateSensor, sensorRanges } from "../../validation/SensorValidation";
import { AlertTriangle, Info } from "lucide-react";

interface AFRWidgetProps {
  liveTuning: boolean;
  tps?: number;
}

export function AFRWidget({ liveTuning, tps = 0 }: AFRWidgetProps) {
  const [afr, setAfr] = useState(14.7);
  const [showTooltip, setShowTooltip] = useState(false);
  const targetAFR = 14.7;
  const stoich = 14.7;
  const lambda = afr / stoich;

  useEffect(() => {
    if (!liveTuning) {
      setAfr(14.7);
      return;
    }

    const interval = setInterval(() => {
      setAfr((prev) => {
        const change = (Math.random() - 0.5) * 0.5;
        const newAFR = Math.max(10, Math.min(18, prev + change));
        return newAFR;
      });
    }, 200);

    return () => clearInterval(interval);
  }, [liveTuning]);

  // Validate AFR with Speeduino ranges
  const validation = validateSensor('afr', afr, { tps });
  
  // Color based on validation
  let color = "#4CAF50";
  let barColor = "#4CAF50";
  
  if (validation.level === 'critical') {
    color = "#F44336";
    barColor = "#F44336";
  } else if (validation.level === 'caution') {
    color = "#FF9800";
    barColor = "#FF9800";
  } else {
    // Fine-tune color within normal range
    const diff = Math.abs(afr - targetAFR);
    if (diff > 0.5) {
      color = "#FF9800";
      barColor = "#FF9800";
    } else if (diff > 0.3) {
      color = "#FFB62A";
      barColor = "#FFB62A";
    }
  }

  return (
    <div className="w-full h-full bg-black border border-[#333333] p-3 flex flex-col items-center justify-center relative">
      <div className="flex items-center gap-2 mb-2">
        <div className="text-xs text-[#888888]">Air/Fuel Ratio</div>
        <button
          onMouseEnter={() => setShowTooltip(true)}
          onMouseLeave={() => setShowTooltip(false)}
          className="text-[#888888] hover:text-[#CCCCCC]"
        >
          <Info className="w-3 h-3" />
        </button>
      </div>
      
      {/* Tooltip */}
      <AnimatePresence>
        {showTooltip && (
          <motion.div
            initial={{ opacity: 0, y: -10 }}
            animate={{ opacity: 1, y: 0 }}
            exit={{ opacity: 0, y: -10 }}
            className="absolute top-0 left-0 right-0 bg-[#1a1a1a] border border-[#444444] p-2 rounded text-xs z-50"
          >
            <div className="text-[#CCCCCC]">
              <div><strong>Optimal:</strong> {sensorRanges.afr.optimal.description}</div>
              <div className="mt-1"><strong>Formula:</strong> {sensorRanges.afr.formula}</div>
              <div className="mt-1 text-[#888888]">Lambda: {lambda.toFixed(3)} (Stoich = 1.0)</div>
            </div>
          </motion.div>
        )}
      </AnimatePresence>
      
      <motion.div
        className="font-data text-center"
        style={{
          fontSize: '40px',
          color: color,
          textShadow: `0 0 8px ${color}`
        }}
        animate={validation.level === 'critical' ? {
          color: ['#F44336', '#FF6666', '#F44336'],
        } : {}}
        transition={{ duration: 0.5, repeat: validation.level === 'critical' ? Infinity : 0 }}
      >
        {afr.toFixed(2)}
      </motion.div>
      
      <div className="flex items-center gap-2 mt-1">
        <div className="text-xs text-[#CCCCCC]">
          λ {lambda.toFixed(2)} | Target: {targetAFR.toFixed(1)}
        </div>
        {validation.level !== 'normal' && (
          <motion.div
            initial={{ scale: 0 }}
            animate={{ scale: 1 }}
          >
            <AlertTriangle 
              className="w-3 h-3" 
              style={{ color: validation.color }}
            />
          </motion.div>
        )}
      </div>
      
      {/* Dual-scale bar (AFR and Lambda) */}
      <div className="w-full mt-3 space-y-2">
        {/* AFR Scale */}
        <div className="flex items-center gap-1">
          <div className="text-xs text-[#888888] w-10">Rich</div>
          <div className="flex-1 h-3 bg-[#1a1a1a] relative overflow-hidden border border-[#444444]">
            {/* Stoich zone (14.2-14.9) */}
            <div 
              className="absolute h-full bg-green-500 opacity-20"
              style={{
                left: '42%',
                width: '16%'
              }}
            />
            {/* Current AFR marker */}
            <motion.div
              className="absolute h-full w-1"
              style={{
                left: `${((afr - 10) / 8) * 100}%`,
                backgroundColor: barColor,
                boxShadow: `0 0 4px ${barColor}`
              }}
              initial={{ opacity: 0 }}
              animate={{ opacity: 1 }}
              transition={{ duration: 0.2 }}
            />
          </div>
          <div className="text-xs text-[#888888] w-10 text-right">Lean</div>
        </div>
        
        {/* Validation message */}
        {validation.shouldAlert && (
          <motion.div
            initial={{ opacity: 0, height: 0 }}
            animate={{ opacity: 1, height: 'auto' }}
            className="text-xs p-1 rounded"
            style={{
              backgroundColor: `${validation.color}22`,
              color: validation.color,
              border: `1px solid ${validation.color}66`
            }}
          >
            {validation.message}
          </motion.div>
        )}
      </div>
    </div>
  );
}
