import { useState } from "react";
import { Card } from "../ui/card";
import { Input } from "../ui/input";
import { Button } from "../ui/button";
import { 
  Search, 
  ChevronRight, 
  ChevronDown, 
  Settings as SettingsIcon,
  RotateCcw,
  Save,
  Upload,
  Download,
  Info
} from "lucide-react";
import { EngineBasicSettings } from "./panels/EngineBasicSettings";
import { GeneralSettings } from "./panels/GeneralSettings";
import { RevLimiterSettings } from "./panels/RevLimiterSettings";
import { ShiftLightSettings } from "./panels/ShiftLightSettings";
import { EngineStateSettings } from "./panels/EngineStateSettings";
import {
  TachoOutputSettings,
  FanControlSettings,
  TorqueConvertorSettings,
  AlternatorControlSettings,
  BarometricCorrectionSettings,
  IATMAPSampleSettings,
  ITBLoadSettings,
  MAFSettings,
  MATAirDensitySettings,
  MATCLTCorrectionSettings,
  RealtimeDisplaySettings,
  IOPinsFeatureList,
  IOPinsUsageSettings,
  CANLoopSettings,
  ExpansionIOSettings,
  GaugeSettingsLimits,
} from "./panels/stub-panels";

export interface SettingsMenuItem {
  id: string;
  label: string;
  icon: string;
  children?: SettingsMenuItem[];
}

const SETTINGS_MENU: SettingsMenuItem[] = [
  {
    id: "engine-basic",
    label: "Engine & Basic Load Settings",
    icon: "🔧",
    children: [
      { id: "engine-sequential", label: "Engine and Sequential Settings", icon: "⚙️" },
      { id: "sequential-fuel", label: "Sequential Fuel Injection", icon: "💉" },
    ]
  },
  { id: "general", label: "General Settings", icon: "⚙️" },
  { id: "rev-limiter", label: "Rev Limiter", icon: "🔄" },
  { id: "shift-light", label: "Shift Light", icon: "💡" },
  { id: "engine-state", label: "Engine State Settings", icon: "🌡️" },
  { id: "tacho-output", label: "Tacho Output", icon: "📊" },
  { id: "fan-control", label: "Fan Control", icon: "🌀" },
  { id: "torque-convertor", label: "Torque Convertor Lockup", icon: "🔒" },
  { id: "alternator", label: "Alternator Control", icon: "⚡" },
  { id: "barometric", label: "Barometric Correction", icon: "🌤️" },
  { id: "iat-map-sample", label: "IAT/MAP Sample Settings", icon: "📈" },
  { id: "itb-load", label: "ITB Load Settings", icon: "🎛️" },
  {
    id: "maf-settings",
    label: "MAF Settings",
    icon: "📊",
    children: [
      { id: "maf-flow-curve", label: "MAF Flow Curve", icon: "📈" },
      { id: "maf-mat-correction", label: "MAF/MAT Correction Table", icon: "📉" },
    ]
  },
  { id: "mat-air-density", label: "MAT Air Density Table", icon: "🌡️" },
  { id: "mat-clt-correction", label: "MAT/CLT Correction", icon: "📉" },
  { id: "realtime-display", label: "Realtime Display", icon: "📺" },
  { id: "io-pins-feature", label: "Feature List Showing IO pins", icon: "📍" },
  {
    id: "io-pins-usage",
    label: "IO pins Usage & Status",
    icon: "🔌",
    children: [
      { id: "io-pins-list-1", label: "IO pins List/1 Usage and Status", icon: "📋" },
      { id: "io-pins-list-2", label: "IO pins List/2 Usage and Status", icon: "📋" },
    ]
  },
  { id: "canloop", label: "CANloop Usage & Status", icon: "🔗" },
  { id: "expansion-io", label: "Expansion IO pins", icon: "➕" },
  { id: "gauge-limits", label: "Gauge and Settings Limits", icon: "⚠️" },
];

interface SettingsTabProps {
  liveTuning: boolean;
}

export function SettingsTab({ liveTuning }: SettingsTabProps) {
  const [activePanel, setActivePanel] = useState("engine-sequential");
  const [expandedItems, setExpandedItems] = useState<Set<string>>(new Set(["engine-basic", "maf-settings", "io-pins-usage"]));
  const [searchQuery, setSearchQuery] = useState("");
  const [hasUnsavedChanges, setHasUnsavedChanges] = useState(false);
  const [sidebarCollapsed, setSidebarCollapsed] = useState(false);

  const toggleExpanded = (id: string) => {
    const newExpanded = new Set(expandedItems);
    if (newExpanded.has(id)) {
      newExpanded.delete(id);
    } else {
      newExpanded.add(id);
    }
    setExpandedItems(newExpanded);
  };

  const handleSave = () => {
    console.log("Saving settings...");
    setHasUnsavedChanges(false);
  };

  const handleRevert = () => {
    if (confirm("Revert all unsaved changes?")) {
      setHasUnsavedChanges(false);
    }
  };

  const renderPanel = () => {
    switch (activePanel) {
      case "engine-sequential":
      case "sequential-fuel":
        return <EngineBasicSettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "general":
        return <GeneralSettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "rev-limiter":
        return <RevLimiterSettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "shift-light":
        return <ShiftLightSettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "engine-state":
        return <EngineStateSettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "tacho-output":
        return <TachoOutputSettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "fan-control":
        return <FanControlSettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "torque-convertor":
        return <TorqueConvertorSettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "alternator":
        return <AlternatorControlSettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "barometric":
        return <BarometricCorrectionSettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "iat-map-sample":
        return <IATMAPSampleSettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "itb-load":
        return <ITBLoadSettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "maf-flow-curve":
      case "maf-mat-correction":
        return <MAFSettings onSettingsChange={() => setHasUnsavedChanges(true)} activeSubPanel={activePanel} />;
      case "mat-air-density":
        return <MATAirDensitySettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "mat-clt-correction":
        return <MATCLTCorrectionSettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "realtime-display":
        return <RealtimeDisplaySettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "io-pins-feature":
        return <IOPinsFeatureList onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "io-pins-list-1":
      case "io-pins-list-2":
        return <IOPinsUsageSettings onSettingsChange={() => setHasUnsavedChanges(true)} listNumber={activePanel === "io-pins-list-1" ? 1 : 2} />;
      case "canloop":
        return <CANLoopSettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "expansion-io":
        return <ExpansionIOSettings onSettingsChange={() => setHasUnsavedChanges(true)} />;
      case "gauge-limits":
        return <GaugeSettingsLimits onSettingsChange={() => setHasUnsavedChanges(true)} />;
      default:
        return (
          <div className="flex items-center justify-center h-full">
            <p className="text-[#888888]">Select a settings category from the menu</p>
          </div>
        );
    }
  };

  const renderMenuItem = (item: SettingsMenuItem, level: number = 0) => {
    const isExpanded = expandedItems.has(item.id);
    const isActive = activePanel === item.id;
    const hasChildren = item.children && item.children.length > 0;

    return (
      <div key={item.id}>
        <div
          className={`flex items-center gap-2 px-3 py-2 cursor-pointer smooth-transition ${
            isActive
              ? "bg-gradient-to-r from-[#0088FF]/20 to-[#00CCFF]/20 border-l-2 border-[#00CCFF]"
              : "hover:bg-[#2a2a2a]"
          }`}
          style={{ paddingLeft: `${12 + level * 16}px` }}
          onClick={() => {
            if (hasChildren) {
              toggleExpanded(item.id);
            } else {
              setActivePanel(item.id);
            }
          }}
        >
          {hasChildren && (
            <div className="w-4 h-4 flex items-center justify-center">
              {isExpanded ? (
                <ChevronDown className="w-3 h-3 text-[#00CCFF]" />
              ) : (
                <ChevronRight className="w-3 h-3 text-[#666666]" />
              )}
            </div>
          )}
          {!hasChildren && <div className="w-4" />}
          <span className="text-sm">{item.icon}</span>
          <span className={`text-sm flex-1 ${isActive ? "text-[#00CCFF] font-semibold" : "text-[#CCCCCC]"}`}>
            {item.label}
          </span>
        </div>
        {hasChildren && isExpanded && (
          <div>
            {item.children?.map(child => renderMenuItem(child, level + 1))}
          </div>
        )}
      </div>
    );
  };

  return (
    <div className="h-full bg-[#1a1a1a] flex flex-col overflow-hidden">
      {/* Header */}
      <div className="flex-shrink-0 border-b-2 border-[#0088FF] bg-gradient-to-r from-[#0a0a14] via-[#12121e] to-[#0a0a14] px-4 py-3">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-3">
            <SettingsIcon className="w-6 h-6 text-[#00CCFF]" />
            <div>
              <h2 className="text-white">ECU Settings & Configuration</h2>
              <p className="text-xs text-[#888888] mt-0.5">
                Complete engine and system configuration
              </p>
            </div>
          </div>
          <div className="flex items-center gap-2">
            {hasUnsavedChanges && (
              <div className="flex items-center gap-2 px-3 py-1.5 bg-[#FFAA00]/10 border border-[#FFAA00] rounded">
                <div className="w-2 h-2 bg-[#FFAA00] rounded-full animate-pulse" />
                <span className="text-xs text-[#FFAA00]">Unsaved Changes</span>
              </div>
            )}
            <Button
              variant="outline"
              size="sm"
              onClick={handleRevert}
              disabled={!hasUnsavedChanges}
              className="border-[#555555] hover:border-[#FFAA00]"
            >
              <RotateCcw className="w-4 h-4 mr-2" />
              Revert
            </Button>
            <Button
              size="sm"
              onClick={handleSave}
              disabled={!hasUnsavedChanges}
              className="bg-gradient-to-r from-[#0088FF] to-[#00CCFF] hover:from-[#0066CC] hover:to-[#00AACC]"
            >
              <Save className="w-4 h-4 mr-2" />
              Save Settings
            </Button>
          </div>
        </div>
      </div>

      {/* Search Bar */}
      <div className="flex-shrink-0 border-b border-[#333333] bg-black p-3">
        <div className="relative">
          <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 w-4 h-4 text-[#666666]" />
          <Input
            type="text"
            placeholder="Search settings..."
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
            className="pl-10 bg-[#1a1a1a] border-[#333333] text-white placeholder:text-[#666666]"
          />
        </div>
      </div>

      {/* Main Content Area */}
      <div className="flex-1 flex min-h-0">
        {/* Left Sidebar - Tree Navigation */}
        <div
          className={`flex-shrink-0 border-r border-[#333333] bg-black overflow-y-auto smooth-transition ${
            sidebarCollapsed ? "w-12" : "w-64"
          }`}
        >
          {!sidebarCollapsed && (
            <div className="py-2">
              {SETTINGS_MENU.map(item => renderMenuItem(item))}
            </div>
          )}
        </div>

        {/* Main Content Panel */}
        <div className="flex-1 overflow-y-auto p-6">
          <div className="max-w-6xl mx-auto">
            {renderPanel()}
          </div>
        </div>
      </div>

      {/* Bottom Action Bar */}
      <div className="flex-shrink-0 border-t-2 border-[#333333] bg-black px-4 py-3">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-4 text-xs text-[#888888]">
            <span>Last saved: 2024-11-30 14:23:45</span>
            <span className="text-[#333333]">|</span>
            <span>Configuration: Default.ini</span>
          </div>
          <div className="flex items-center gap-2">
            <Button variant="outline" size="sm" className="border-[#555555]">
              <Upload className="w-4 h-4 mr-2" />
              Import
            </Button>
            <Button variant="outline" size="sm" className="border-[#555555]">
              <Download className="w-4 h-4 mr-2" />
              Export
            </Button>
          </div>
        </div>
      </div>
    </div>
  );
}