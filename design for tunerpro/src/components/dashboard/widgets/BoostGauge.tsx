import { useEffect, useState } from "react";

interface BoostGaugeProps {
  liveTuning: boolean;
}

export function BoostGauge({ liveTuning }: BoostGaugeProps) {
  const [boost, setBoost] = useState(98.5);

  useEffect(() => {
    if (!liveTuning) {
      setBoost(98.5);
      return;
    }

    const interval = setInterval(() => {
      setBoost((prev) => {
        const change = (Math.random() - 0.5) * 5;
        const newBoost = Math.max(20, Math.min(200, prev + change));
        return newBoost;
      });
    }, 250);

    return () => clearInterval(interval);
  }, [liveTuning]);

  let color = "#00FF00";
  if (boost > 150) color = "#FF0000";
  else if (boost > 120) color = "#FFFF00";

  return (
    <div className="w-full h-full bg-black border border-[#333333] p-3 flex flex-col items-center justify-center">
      <div className="text-xs text-[#888888] mb-2">MAP</div>
      <div
        className="font-data text-center"
        style={{
          fontSize: '40px',
          color: color,
          textShadow: `0 0 8px ${color}`
        }}
      >
        {boost.toFixed(1)}
      </div>
      <div className="text-xs text-[#CCCCCC] mt-1">kPa</div>
      
      {/* Simple bar indicator */}
      <div className="w-full mt-3 h-2 bg-[#1a1a1a] overflow-hidden">
        <div
          className="h-full transition-all duration-300"
          style={{
            width: `${Math.min(100, (boost / 200) * 100)}%`,
            backgroundColor: color
          }}
        />
      </div>
    </div>
  );
}
