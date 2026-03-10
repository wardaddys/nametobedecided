import { HaltechGauge } from "./widgets/HaltechGauge";
import { BarIndicator } from "./widgets/BarIndicator";
import { useSensors } from "../context/SensorContext";
import { useTheme } from "../context/ThemeContext";
import { useStatusBoxes } from "../context/StatusBoxContext";
import { AlertTriangle, CheckCircle2, Activity, Gauge } from "lucide-react";

interface DashboardProps {
  liveTuning: boolean;
  isMinimized?: boolean;
  gaugeData: {
    boost: number;
    speed: number;
    gear: number;
    rpm: number;
    trip: number;
    manifoldPressure: number;
    fuelPressure: number;
    oilPressure: number;
    oilTemp: number;
  };
}

interface StatusBoxProps {
  label: string;
  status: "ok" | "warning" | "error";
  value?: string;
}

function StatusBox({ label, status, value }: StatusBoxProps) {
  const bgColor = status === "ok" ? "bg-[#00D936]" : status === "warning" ? "bg-[#FFB62A]" : "bg-[#FF3B30]";
  const textColor = status === "ok" ? "text-[#00D936]" : status === "warning" ? "text-[#FFB62A]" : "text-[#FF3B30]";
  const borderColor = status === "ok" ? "border-[#00D936]" : status === "warning" ? "border-[#FFB62A]" : "border-[#FF3B30]";
  const Icon = status === "ok" ? CheckCircle2 : status === "warning" ? Activity : AlertTriangle;
  
  return (
    <div className={`bg-black bg-opacity-80 border-2 ${borderColor} px-3 py-2 flex items-center gap-2 min-w-[140px] backdrop-blur-sm`}>
      <Icon className={`w-4 h-4 ${textColor}`} />
      <div className="flex-1">
        <div className={`text-xs font-bold ${textColor}`}>{label}</div>
        {value && <div className="text-[10px] text-white font-semibold mt-0.5">{value}</div>}
      </div>
    </div>
  );
}

export function Dashboard({ liveTuning, isMinimized = false, gaugeData }: DashboardProps) {
  const { sensors } = useSensors();
  const { theme } = useTheme();
  const { statusBoxes } = useStatusBoxes();
  const { boost, speed, gear, rpm, trip } = gaugeData;

  // Get only sensors that should be shown on dashboard
  const dashboardSensors = sensors.filter((s) => s.showOnDashboard);
  const leftSensors = dashboardSensors.slice(0, Math.ceil(dashboardSensors.length / 2));
  const rightSensors = dashboardSensors.slice(Math.ceil(dashboardSensors.length / 2));

  // Status boxes data - dynamic based on configuration
  const getStatusBoxData = () => {
    const data: { [key: string]: { label: string; status: "ok" | "warning" | "error"; value: string } } = {
      ecu: { label: "ECU COMM", status: "ok", value: "Active" },
      fuel: { label: "FUEL SYSTEM", status: "ok", value: "Normal" },
      ignition: { label: "IGNITION", status: "ok", value: "Normal" },
      boost: { label: "BOOST", status: boost > 15 ? "warning" : "ok", value: `${boost.toFixed(1)} PSI` },
      vtec: { label: "VTEC", status: rpm > 5800 ? "ok" : "error", value: rpm > 5800 ? "Engaged" : "Off" },
      launch: { label: "LAUNCH", status: "error", value: "Inactive" },
      traction: { label: "TRACTION", status: "error", value: "Inactive" },
      revlimit: { label: "REV LIMIT", status: rpm > 8000 ? "warning" : "ok", value: rpm > 8000 ? "Active" : "Normal" },
    };
    return data;
  };

  const statusBoxData = getStatusBoxData();
  const topStatusBoxes = statusBoxes
    .filter((box) => box.enabled && box.position === "top")
    .map((box) => statusBoxData[box.type]);
  
  const bottomStatusBoxes = statusBoxes
    .filter((box) => box.enabled && box.position === "bottom")
    .map((box) => statusBoxData[box.type]);

  // Background gradient - grey and bronze (more bronze)
  const backgroundStyle = theme === "haltech" 
    ? "linear-gradient(135deg, #1a1a1a 0%, #4a3520 30%, #6b4423 50%, #4a3520 70%, #1a1a1a 100%)"
    : "linear-gradient(135deg, #2a2a2a 0%, #5a4530 30%, #7b5433 50%, #5a4530 70%, #2a2a2a 100%)";

  if (isMinimized) {
    // Mini dashboard view - digital displays only (no gauges) - SLIM VERSION
    return (
      <div className="h-full flex items-center justify-center bg-gradient-to-r from-[#0a0a0a] via-[#12121e] to-[#0a0a0a] relative overflow-hidden border-b border-[#333333]">
        {/* Background texture/pattern */}
        <div className="absolute inset-0 opacity-10">
          <div className="absolute inset-0" style={{
            backgroundImage: 'repeating-linear-gradient(0deg, transparent, transparent 2px, rgba(255,255,255,0.03) 2px, rgba(255,255,255,0.03) 4px)',
          }} />
        </div>

        {/* Compact dashboard */}
        <div className="relative w-full max-w-[1400px] h-full flex items-center justify-center px-3 py-1">
          <div className="flex items-center gap-4 w-full justify-center">
            {/* Left sensors - compact */}
            {leftSensors.length > 0 && (
              <div className="flex gap-3">
                {leftSensors.map((sensor) => (
                  <BarIndicator
                    key={sensor.id}
                    label={sensor.name}
                    value={Math.round(sensor.value)}
                    unit={sensor.unit}
                    maxBars={6}
                    compact={true}
                  />
                ))}
              </div>
            )}

            {/* RPM Digital Display - SMALLER */}
            <div className="bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border border-[#00FF00] rounded px-3 py-1.5 min-w-[140px] shadow-md shadow-[#00FF00]/10">
              <div className="text-center">
                <div className="text-[#00FF00] text-[9px] font-semibold mb-0.5 tracking-wider">RPM</div>
                <div 
                  className="text-[#00FF00] font-mono font-bold leading-none"
                  style={{
                    fontSize: '1.75rem',
                    textShadow: '0 0 10px rgba(0, 255, 0, 0.6)',
                  }}
                >
                  {Math.round(rpm)}
                </div>
              </div>
            </div>

            {/* Speed Digital Display - SMALLER */}
            <div className="bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border border-[#00CCFF] rounded px-3 py-1.5 min-w-[140px] shadow-md shadow-[#00CCFF]/10">
              <div className="text-center">
                <div className="text-[#00CCFF] text-[9px] font-semibold mb-0.5 tracking-wider">SPEED</div>
                <div 
                  className="text-[#00CCFF] font-mono font-bold leading-none"
                  style={{
                    fontSize: '1.75rem',
                    textShadow: '0 0 10px rgba(0, 204, 255, 0.6)',
                  }}
                >
                  {Math.round(speed)}
                </div>
                <div className="text-[#00CCFF]/60 text-[9px] font-semibold mt-0.5 tracking-wider">KM/H</div>
              </div>
            </div>

            {/* Right sensors - compact */}
            {rightSensors.length > 0 && (
              <div className="flex gap-3">
                {rightSensors.map((sensor) => (
                  <BarIndicator
                    key={sensor.id}
                    label={sensor.name}
                    value={Math.round(sensor.value)}
                    unit={sensor.unit}
                    maxBars={6}
                    align="right"
                    compact={true}
                  />
                ))}
              </div>
            )}
          </div>
        </div>
      </div>
    );
  }

  // Full dashboard view
  return (
    <div 
      className="h-full flex items-center justify-center relative overflow-hidden"
      style={{
        background: backgroundStyle
      }}
    >
      {/* Background texture/pattern */}
      <div className="absolute inset-0 opacity-10">
        <div className="absolute inset-0" style={{
          backgroundImage: 'repeating-linear-gradient(0deg, transparent, transparent 2px, rgba(255,255,255,0.03) 2px, rgba(255,255,255,0.03) 4px)',
        }} />
      </div>

      {/* Main dashboard container */}
      <div className="relative w-full h-full flex flex-col items-center justify-center px-8 py-6">
        {/* Top status boxes */}
        <div className="flex gap-3 mb-6">
          {topStatusBoxes.map((box, i) => (
            <StatusBox key={i} label={box.label} status={box.status} value={box.value} />
          ))}
        </div>

        {/* Main gauge area */}
        <div className="grid grid-cols-[1fr_auto_auto_auto_1fr] gap-8 items-center w-full max-w-[1600px]">
          {/* Left vertical panel - filled with sensors */}
          <div className="flex flex-col justify-center h-[500px] gap-6 items-end">
            {leftSensors.length > 0 ? (
              leftSensors.map((sensor) => (
                <BarIndicator
                  key={sensor.id}
                  label={sensor.name.toUpperCase()}
                  value={Math.round(sensor.value)}
                  unit={sensor.unit}
                  maxBars={8}
                />
              ))
            ) : (
              <div className="text-[#CCCCCC] text-xs text-center" style={{ textShadow: '0 0 4px rgba(255, 255, 255, 0.2)' }}>
                Configure sensors<br />in I/O tab
              </div>
            )}
          </div>

          {/* RPM Gauge */}
          <div className="flex items-center justify-center">
            <HaltechGauge
              value={rpm}
              min={0}
              max={9000}
              majorTicks={[0, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000]}
              greenZone={[0, 6000]}
              orangeZone={[6000, 7500]}
              redZone={[7500, 9000]}
              centerLabel="RPM"
              bottomLabel=""
              size={420}
              showDigitalReadout={true}
              digitalValue={rpm}
              digitalLabel="ENGINE RPM"
            />
          </div>

          {/* Center Gear Indicator */}
          <div className="flex flex-col items-center justify-center px-6">
            <div className="bg-black border-4 border-[#00d4ff] rounded-lg p-6 min-w-[140px]">
              <div className="text-center">
                <div className="text-[#888888] text-xs mb-2">GEAR</div>
                <div className="text-[#00d4ff] text-7xl font-bold font-mono leading-none" style={{
                  textShadow: '0 0 20px rgba(0, 212, 255, 0.8)',
                }}>
                  {gear}
                </div>
              </div>
            </div>
            <div className="mt-4 text-center">
              <div className="text-[#888888] text-[10px]">TRIP</div>
              <div className="text-white text-sm font-mono">{trip} km</div>
            </div>
          </div>

          {/* Speed Gauge */}
          <div className="flex items-center justify-center">
            <HaltechGauge
              value={speed}
              min={0}
              max={200}
              majorTicks={[0, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200]}
              greenZone={[0, 140]}
              orangeZone={[140, 160]}
              redZone={[160, 200]}
              centerLabel="SPEED"
              bottomLabel=""
              size={420}
              showDigitalReadout={true}
              digitalValue={Math.round(speed)}
              digitalLabel="KM/H"
            />
          </div>

          {/* Right vertical panel - filled with sensors */}
          <div className="flex flex-col justify-center h-[500px] gap-6 items-start">
            {rightSensors.length > 0 ? (
              rightSensors.map((sensor) => (
                <BarIndicator
                  key={sensor.id}
                  label={sensor.name.toUpperCase()}
                  value={Math.round(sensor.value)}
                  unit={sensor.unit}
                  maxBars={8}
                  align="right"
                />
              ))
            ) : (
              <div className="text-[#CCCCCC] text-xs text-center" style={{ textShadow: '0 0 4px rgba(255, 255, 255, 0.2)' }}>
                Configure sensors<br />in I/O tab
              </div>
            )}
          </div>
        </div>

        {/* Bottom status boxes */}
        <div className="flex gap-3 mt-6">
          {bottomStatusBoxes.map((box, i) => (
            <StatusBox key={i} label={box.label} status={box.status} value={box.value} />
          ))}
        </div>
      </div>
    </div>
  );
}
