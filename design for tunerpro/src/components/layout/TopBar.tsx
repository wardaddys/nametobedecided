import { useState } from "react";
import { Save, Upload, Download, Settings } from "lucide-react";
import { EnhancedSettingsDropdown } from "../settings/EnhancedSettingsDropdown";

interface TopBarProps {
  liveTuning: boolean;
  onLiveTuningChange: (value: boolean) => void;
  connectionStatus: "connected" | "disconnected" | "error";
  onSave: () => void;
  onUpload: () => void;
  onDownload: () => void;
}

export function TopBar({
  liveTuning,
  onLiveTuningChange,
  connectionStatus,
  onSave,
  onUpload,
  onDownload,
}: TopBarProps) {
  const [settingsOpen, setSettingsOpen] = useState(false);
  const statusColors = {
    connected: "#00FF00",
    disconnected: "#666666",
    error: "#FF0000",
  };

  const statusLabels = {
    connected: "Connected",
    disconnected: "Disconnected",
    error: "Error",
  };

  return (
    <div className="h-12 bg-[#2a2a2a] border-b border-[#444444] flex items-center justify-between px-3">
      {/* Left: Application Title */}
      <div className="flex items-center gap-4">
        <div className="text-sm text-white">
          <span className="text-[#CCCCCC]">K20A2_Stage2.cal</span>
        </div>
      </div>

      {/* Center: Connection Status and Live Tuning */}
      <div className="flex items-center gap-4">
        <div className="flex items-center gap-2 bg-black border border-[#444444] px-3 py-1">
          <div
            className="w-3 h-3 rounded-sm"
            style={{ backgroundColor: statusColors[connectionStatus] }}
          />
          <span className="text-xs text-[#CCCCCC]">
            ECU: {statusLabels[connectionStatus]}
          </span>
        </div>

        <div className="flex items-center gap-2 bg-black border border-[#444444] px-3 py-1">
          <input
            type="checkbox"
            checked={liveTuning}
            onChange={(e) => onLiveTuningChange(e.target.checked)}
            className="w-4 h-4"
            style={{ accentColor: liveTuning ? "#FF0000" : "#666666" }}
          />
          <span
            className="text-xs"
            style={{ color: liveTuning ? "#FF0000" : "#CCCCCC" }}
          >
            Live Tuning {liveTuning ? "ON" : "OFF"}
          </span>
        </div>
      </div>

      {/* Right: Action Buttons */}
      <div className="flex items-center gap-1">
        <button
          onClick={onSave}
          className="bg-[#3a3a3a] hover:bg-[#4a4a4a] border border-[#555555] px-3 py-1 text-xs text-white flex items-center gap-2"
        >
          <Save className="w-3 h-3" />
          Save
        </button>
        <button
          onClick={onDownload}
          className="bg-[#3a3a3a] hover:bg-[#4a4a4a] border border-[#555555] px-3 py-1 text-xs text-white flex items-center gap-2"
        >
          <Download className="w-3 h-3" />
          Read ECU
        </button>
        <button
          onClick={onUpload}
          disabled={!liveTuning}
          className={`border border-[#555555] px-3 py-1 text-xs flex items-center gap-2 ${
            liveTuning
              ? "bg-[#3a3a3a] hover:bg-[#4a4a4a] text-white"
              : "bg-[#1a1a1a] text-[#666666] cursor-not-allowed"
          }`}
        >
          <Upload className="w-3 h-3" />
          Write ECU
        </button>
        <button 
          onClick={() => setSettingsOpen(!settingsOpen)}
          className="bg-[#3a3a3a] hover:bg-[#4a4a4a] border border-[#555555] px-2 py-1 text-xs text-white"
        >
          <Settings className="w-3 h-3" />
        </button>
      </div>

      {/* Settings Dropdown */}
      <EnhancedSettingsDropdown 
        isOpen={settingsOpen}
        onClose={() => setSettingsOpen(false)}
      />
    </div>
  );
}
