import { useEngineConfig } from "../context/EngineConfigContext";

interface TabBarProps {
  activeScreen: string;
  onScreenChange: (screen: string) => void;
}

const allTabs = [
  { id: "dashboard", label: "Dashboard", alwaysShow: true },
  { id: "tables", label: "All Tables", alwaysShow: true },
  { id: "idle", label: "Idle", alwaysShow: true },
  { id: "coldstart", label: "Cold Start", alwaysShow: true },
  { id: "accel", label: "Accel", alwaysShow: true },
  { id: "trims", label: "Trims", alwaysShow: true },
  { id: "vtec", label: "VVT/VTEC", alwaysShow: true },
  { id: "boost", label: "Boost", requiresBoost: true }, // Only show for boosted engines
  { id: "knock", label: "Knock", alwaysShow: true },
  { id: "limiters", label: "Limiters", alwaysShow: true },
  { id: "o2ve", label: "O2/VE", alwaysShow: true },
  { id: "io", label: "I/O Config", alwaysShow: true },
  { id: "sensors", label: "Engine Setup", alwaysShow: true }, // Renamed from "Sensors" to "Engine Setup"
  { id: "settings", label: "ECU Settings", alwaysShow: true }, // Comprehensive settings tab
  { id: "logs", label: "Logging", alwaysShow: true },
];

export function TabBar({ activeScreen, onScreenChange }: TabBarProps) {
  const { config } = useEngineConfig();
  
  // Filter tabs based on engine configuration
  const tabs = allTabs.filter(tab => {
    if (tab.alwaysShow) return true;
    if (tab.requiresBoost) return config.features.hasBoostControl;
    return true;
  });
  return (
    <div className="bg-[#2a2a2a] border-b border-[#444444] flex items-center px-1 py-0">
      {tabs.map((tab) => (
        <button
          key={tab.id}
          onClick={() => onScreenChange(tab.id)}
          className={`
            px-4 py-2 text-sm transition-colors relative
            ${
              activeScreen === tab.id
                ? "bg-[#1a1a1a] text-white border-t-2 border-t-[#0066CC]"
                : "bg-[#3a3a3a] text-[#CCCCCC] hover:bg-[#333333] border-t-2 border-t-transparent"
            }
            border-l border-r border-[#555555]
            font-medium
          `}
          style={{
            marginRight: '1px',
            borderBottom: activeScreen === tab.id ? '2px solid #1a1a1a' : '1px solid #444444'
          }}
        >
          {tab.label}
        </button>
      ))}
    </div>
  );
}