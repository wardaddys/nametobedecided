interface StatusBarProps {
  rpm: number;
  map: number;
  afr: number;
  ect: number;
  boost: number;
  speed: number;
  gear: number;
  oilTemp: number;
  oilPressure: number;
  fuelPressure: number;
  lastBackup: string;
  commHealth: "good" | "warning" | "error";
  liveTuning: boolean;
  ioStatus?: string; // For displaying I/O configuration changes
}

export function StatusBar({
  rpm,
  map,
  afr,
  ect,
  boost,
  speed,
  gear,
  oilTemp,
  oilPressure,
  fuelPressure,
  lastBackup,
  commHealth,
  liveTuning,
  ioStatus,
}: StatusBarProps) {
  const healthColors = {
    good: "#00FF00",
    warning: "#FFFF00",
    error: "#FF0000",
  };

  return (
    <div className="h-6 bg-[#1a1a1a] border-t border-[#333333] flex items-center justify-between px-3 text-xs font-data">
      {/* Left: Primary Sensor Readouts */}
      <div className="flex items-center gap-3">
        <div className="flex items-center gap-1">
          <span className="text-[#888888]">RPM:</span>
          <span className="text-[#00FF00]">{rpm.toLocaleString()}</span>
        </div>
        <div className="w-px h-3 bg-[#333333]" />
        <div className="flex items-center gap-1">
          <span className="text-[#888888]">MAP:</span>
          <span className="text-white">{Math.round(map)}</span>
          <span className="text-[#666666]">kPa</span>
        </div>
        <div className="flex items-center gap-1">
          <span className="text-[#888888]">AFR:</span>
          <span className="text-white">{afr.toFixed(1)}</span>
        </div>
        <div className="flex items-center gap-1">
          <span className="text-[#888888]">ECT:</span>
          <span className="text-white">{ect}°C</span>
        </div>
        <div className="w-px h-3 bg-[#333333]" />
        <div className="flex items-center gap-1">
          <span className="text-[#888888]">Boost:</span>
          <span className="text-[#00CCFF]">{boost.toFixed(1)}</span>
          <span className="text-[#666666]">psi</span>
        </div>
        <div className="flex items-center gap-1">
          <span className="text-[#888888]">Speed:</span>
          <span className="text-[#00CCFF]">{Math.round(speed)}</span>
          <span className="text-[#666666]">km/h</span>
        </div>
        <div className="flex items-center gap-1">
          <span className="text-[#888888]">Gear:</span>
          <span className="text-white">{gear}</span>
        </div>
        <div className="w-px h-3 bg-[#333333]" />
        <div className="flex items-center gap-1">
          <span className="text-[#888888]">Oil T:</span>
          <span className="text-white">{Math.round(oilTemp)}°C</span>
        </div>
        <div className="flex items-center gap-1">
          <span className="text-[#888888]">Oil P:</span>
          <span className="text-white">{Math.round(oilPressure)}</span>
          <span className="text-[#666666]">psi</span>
        </div>
        <div className="flex items-center gap-1">
          <span className="text-[#888888]">Fuel P:</span>
          <span className="text-white">{Math.round(fuelPressure)}</span>
          <span className="text-[#666666]">psi</span>
        </div>
        {ioStatus && (
          <>
            <div className="w-px h-3 bg-[#333333]" />
            <div className="flex items-center gap-1">
              <span className="text-[#FFB62A]">I/O:</span>
              <span className="text-white">{ioStatus}</span>
            </div>
          </>
        )}
      </div>

      {/* Right: Status Indicators */}
      <div className="flex items-center gap-3">
        <div className="flex items-center gap-1.5">
          <div
            className="w-2 h-2 rounded-full"
            style={{ backgroundColor: healthColors[commHealth] }}
          />
          <span className="text-[#888888]">ECU</span>
        </div>
        {liveTuning && (
          <div className="flex items-center gap-1">
            <div className="w-2 h-2 rounded-full bg-[#FF0000] animate-pulse" />
            <span className="text-[#FF0000]">LIVE</span>
          </div>
        )}
        <span className="text-[#888888]">Saved: {lastBackup}</span>
      </div>
    </div>
  );
}
