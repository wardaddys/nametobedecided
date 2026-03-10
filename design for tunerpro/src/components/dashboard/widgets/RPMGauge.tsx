import { useEffect, useState } from "react";
import { motion, AnimatePresence } from "motion/react";
import { validateSensor, sensorRanges } from "../../validation/SensorValidation";
import { AlertTriangle } from "lucide-react";

interface RPMGaugeProps {
  liveTuning: boolean;
}

export function RPMGauge({ liveTuning }: RPMGaugeProps) {
  const [rpm, setRpm] = useState(1200);
  const [showAlert, setShowAlert] = useState(false);
  const maxRpm = 10000;
  const warningRpm = 7500;
  const redlineRpm = 8000;
  const criticalRpm = 8200;

  useEffect(() => {
    if (!liveTuning) {
      setRpm(1200);
      setShowAlert(false);
      return;
    }

    const interval = setInterval(() => {
      setRpm((prev) => {
        // Simulate realistic RPM changes
        const change = (Math.random() - 0.5) * 500;
        const newRpm = Math.max(800, Math.min(maxRpm, prev + change));
        
        // Check for critical RPM
        if (newRpm >= criticalRpm) {
          setShowAlert(true);
        } else if (newRpm < redlineRpm) {
          setShowAlert(false);
        }
        
        return newRpm;
      });
    }, 200);

    return () => clearInterval(interval);
  }, [liveTuning]);

  const percentage = (rpm / maxRpm) * 100;
  const angle = (percentage / 100) * 270 - 135; // -135 to 135 degrees

  // Validate RPM with Speeduino ranges
  const validation = validateSensor('rpm', rpm, { rpm });
  
  // Determine color based on validation
  let needleColor = "#1FB6FF";
  let digitalColor = "#00FF00";
  let glowColor = "rgba(0, 255, 0, 0.5)";
  
  if (validation.level === 'critical') {
    needleColor = "#FF0000";
    digitalColor = "#FF0000";
    glowColor = "rgba(255, 0, 0, 0.8)";
  } else if (validation.level === 'caution') {
    needleColor = "#FF9800";
    digitalColor = "#FFB62A";
    glowColor = "rgba(255, 152, 0, 0.6)";
  } else if (rpm >= redlineRpm) {
    needleColor = "#FF3B30";
    digitalColor = "#FF0000";
    glowColor = "rgba(255, 0, 0, 0.5)";
  } else if (rpm >= warningRpm) {
    needleColor = "#FFB62A";
    digitalColor = "#FFB62A";
    glowColor = "rgba(255, 182, 42, 0.5)";
  }

  return (
    <div className="w-full h-full bg-black rounded border border-[#333333] p-4 flex flex-col items-center justify-center relative">
      <div className="relative w-full max-w-[380px] aspect-square">
        <svg className="w-full h-full" viewBox="0 0 240 240">
          {/* Outer bezel */}
          <circle
            cx="120"
            cy="120"
            r="118"
            fill="none"
            stroke="#1a1a1a"
            strokeWidth="2"
          />
          
          {/* Gauge face background */}
          <circle
            cx="120"
            cy="120"
            r="115"
            fill="#000000"
          />

          {/* Major tick marks and zones */}
          {[0, 1, 2, 3, 4, 5, 6, 7, 8, 9].map((i) => {
            const tickAngle = -225 + (i * 270) / 9;
            const rad = (tickAngle * Math.PI) / 180;
            
            // Determine color based on zone
            let tickColor = "#00FF00"; // Green zone (0-6)
            if (i >= 6 && i < 8) tickColor = "#FFFF00"; // Yellow zone (6-8)
            if (i >= 8) tickColor = "#FF0000"; // Red zone (8-9)
            
            // Major tick
            const x1 = 120 + Math.cos(rad) * 95;
            const y1 = 120 + Math.sin(rad) * 95;
            const x2 = 120 + Math.cos(rad) * 105;
            const y2 = 120 + Math.sin(rad) * 105;
            
            // Number position
            const xNum = 120 + Math.cos(rad) * 80;
            const yNum = 120 + Math.sin(rad) * 80;

            return (
              <g key={i}>
                {/* Major tick */}
                <line
                  x1={x1}
                  y1={y1}
                  x2={x2}
                  y2={y2}
                  stroke={tickColor}
                  strokeWidth="3"
                  strokeLinecap="square"
                />
                
                {/* Number label */}
                <text
                  x={xNum}
                  y={yNum}
                  textAnchor="middle"
                  dominantBaseline="middle"
                  className="font-data"
                  style={{ 
                    fontSize: '16px',
                    fill: '#CCCCCC',
                    fontWeight: 700
                  }}
                >
                  {i}
                </text>
              </g>
            );
          })}

          {/* Minor tick marks between major ticks */}
          {Array.from({ length: 45 }).map((_, i) => {
            // Skip major tick positions
            if (i % 5 === 0) return null;
            
            const tickAngle = -225 + (i * 270) / 45;
            const rad = (tickAngle * Math.PI) / 180;
            const x1 = 120 + Math.cos(rad) * 100;
            const y1 = 120 + Math.sin(rad) * 100;
            const x2 = 120 + Math.cos(rad) * 105;
            const y2 = 120 + Math.sin(rad) * 105;

            return (
              <line
                key={`minor-${i}`}
                x1={x1}
                y1={y1}
                x2={x2}
                y2={y2}
                stroke="#444444"
                strokeWidth="1"
                strokeLinecap="square"
              />
            );
          })}

          {/* Color zone arcs (behind the needle) */}
          {/* Green zone arc */}
          <path
            d="M 27,183 A 105 105 0 0 1 171,38"
            fill="none"
            stroke="rgba(0, 255, 0, 0.15)"
            strokeWidth="12"
          />
          
          {/* Yellow zone arc */}
          <path
            d="M 171,38 A 105 105 0 0 1 207,120"
            fill="none"
            stroke="rgba(255, 255, 0, 0.15)"
            strokeWidth="12"
          />
          
          {/* Red zone arc */}
          <path
            d="M 207,120 A 105 105 0 0 1 213,183"
            fill="none"
            stroke="rgba(255, 0, 0, 0.25)"
            strokeWidth="12"
          />

          {/* Needle assembly */}
          <motion.g
            initial={{ rotate: -225 }}
            animate={{ rotate: angle }}
            transition={{ type: "spring", stiffness: 80, damping: 12 }}
            style={{ transformOrigin: "120px 120px" }}
          >
            {/* Needle - simple red pointer */}
            <path
              d="M 120 120 L 117 115 L 118.5 30 L 120 25 L 121.5 30 L 123 115 Z"
              fill="#FF0000"
              stroke="#FFFFFF"
              strokeWidth="0.5"
            />
            
            {/* Needle shine */}
            <path
              d="M 120 120 L 118.5 115 L 119.5 30 L 120 25 L 120 30 L 120 115 Z"
              fill="rgba(255, 255, 255, 0.4)"
            />
          </motion.g>

          {/* Center cap */}
          <circle cx="120" cy="120" r="8" fill="#1a1a1a" stroke="#666666" strokeWidth="1" />
          <circle cx="120" cy="120" r="5" fill="#000000" />
          <circle cx="120" cy="120" r="2" fill="#FF0000" />

          {/* RPM x1000 label at bottom */}
          <text
            x="120"
            y="200"
            textAnchor="middle"
            className="font-data"
            style={{ 
              fontSize: '11px',
              fill: '#888888',
              fontWeight: 400,
              letterSpacing: '1px'
            }}
          >
            RPM x1000
          </text>
        </svg>

        {/* Digital display box in center */}
        <div className="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 mt-8">
          <motion.div 
            className="bg-[#0a0a0a] border border-[#333333] px-4 py-2 rounded-sm"
            animate={validation.level === 'critical' ? {
              borderColor: ['#FF0000', '#FF6666', '#FF0000'],
              boxShadow: [
                '0 0 10px rgba(255, 0, 0, 0.5)',
                '0 0 20px rgba(255, 0, 0, 0.8)',
                '0 0 10px rgba(255, 0, 0, 0.5)'
              ]
            } : {}}
            transition={{ duration: 0.5, repeat: validation.level === 'critical' ? Infinity : 0 }}
          >
            <div className="font-data text-3xl tracking-wider text-center" style={{ 
              fontWeight: 700,
              color: digitalColor,
              textShadow: `0 0 8px ${glowColor}`,
              fontVariantNumeric: 'tabular-nums'
            }}>
              {Math.round(rpm).toLocaleString()}
            </div>
          </motion.div>
        </div>

        {/* Title at top */}
        <div className="absolute top-6 left-1/2 -translate-x-1/2">
          <div className="font-data text-xs text-[#CCCCCC] tracking-wide">
            ENGINE SPEED
          </div>
        </div>
      </div>
      
      {/* Critical RPM Alert Modal */}
      <AnimatePresence>
        {showAlert && validation.level === 'critical' && (
          <motion.div
            initial={{ opacity: 0, scale: 0.8 }}
            animate={{ opacity: 1, scale: 1 }}
            exit={{ opacity: 0, scale: 0.8 }}
            className="absolute top-2 left-1/2 -translate-x-1/2 bg-red-600 text-white px-4 py-2 rounded flex items-center gap-2 shadow-lg z-50"
          >
            <AlertTriangle className="w-5 h-5" />
            <div>
              <div className="font-data">REV LIMIT EXCEEDED</div>
              <div className="text-xs">Risk of engine damage</div>
            </div>
          </motion.div>
        )}
      </AnimatePresence>
    </div>
  );
}
