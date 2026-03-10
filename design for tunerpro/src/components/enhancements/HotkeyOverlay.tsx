import { useState, useEffect } from "react";
import { X, Keyboard, Zap } from "lucide-react";

interface Hotkey {
  keys: string[];
  description: string;
  category: string;
}

const HOTKEYS: Hotkey[] = [
  // File Operations
  { keys: ["Ctrl", "S"], description: "Save calibration", category: "File" },
  { keys: ["Ctrl", "O"], description: "Open calibration", category: "File" },
  { keys: ["Ctrl", "N"], description: "New calibration", category: "File" },
  { keys: ["Ctrl", "Shift", "S"], description: "Save As...", category: "File" },
  
  // ECU Operations
  { keys: ["Ctrl", "U"], description: "Upload to ECU", category: "ECU" },
  { keys: ["Ctrl", "D"], description: "Download from ECU", category: "ECU" },
  { keys: ["Ctrl", "L"], description: "Toggle Live Tuning", category: "ECU" },
  { keys: ["Ctrl", "R"], description: "Read real-time data", category: "ECU" },
  
  // Navigation
  { keys: ["1"], description: "Dashboard", category: "Navigation" },
  { keys: ["2"], description: "Maps Editor", category: "Navigation" },
  { keys: ["3"], description: "VTEC Control", category: "Navigation" },
  { keys: ["4"], description: "Boost Control", category: "Navigation" },
  { keys: ["5"], description: "Advanced Features", category: "Navigation" },
  { keys: ["6"], description: "I/O Configuration", category: "Navigation" },
  { keys: ["7"], description: "O2/VE Tuning", category: "Navigation" },
  { keys: ["8"], description: "Log Viewer", category: "Navigation" },
  { keys: ["9"], description: "Sensors", category: "Navigation" },
  { keys: ["0"], description: "Settings", category: "Navigation" },
  
  // Table Editing
  { keys: ["Arrow Keys"], description: "Navigate table cells", category: "Table" },
  { keys: ["Enter"], description: "Edit selected cell", category: "Table" },
  { keys: ["Tab"], description: "Next cell", category: "Table" },
  { keys: ["Shift", "Tab"], description: "Previous cell", category: "Table" },
  { keys: ["Ctrl", "C"], description: "Copy cell value", category: "Table" },
  { keys: ["Ctrl", "V"], description: "Paste cell value", category: "Table" },
  { keys: ["Ctrl", "Z"], description: "Undo change", category: "Table" },
  { keys: ["Ctrl", "Y"], description: "Redo change", category: "Table" },
  
  // View
  { keys: ["Ctrl", "+"], description: "Zoom in graph", category: "View" },
  { keys: ["Ctrl", "-"], description: "Zoom out graph", category: "View" },
  { keys: ["Ctrl", "0"], description: "Reset zoom", category: "View" },
  { keys: ["F11"], description: "Toggle fullscreen", category: "View" },
  
  // Help
  { keys: ["F1"], description: "Open Help", category: "Help" },
  { keys: ["Ctrl", "Shift", "H"], description: "Show hotkeys", category: "Help" },
  { keys: ["Esc"], description: "Close dialogs", category: "Help" },
];

export function HotkeyOverlay() {
  const [isVisible, setIsVisible] = useState(false);

  useEffect(() => {
    const handleKeyPress = (event: KeyboardEvent) => {
      // F1 to show help
      if (event.key === "F1") {
        event.preventDefault();
        setIsVisible(true);
      }
      
      // Ctrl+Shift+H to show hotkeys
      if ((event.ctrlKey || event.metaKey) && event.shiftKey && event.key === "H") {
        event.preventDefault();
        setIsVisible(true);
      }

      // Escape to close
      if (event.key === "Escape" && isVisible) {
        setIsVisible(false);
      }
    };

    window.addEventListener("keydown", handleKeyPress);
    return () => window.removeEventListener("keydown", handleKeyPress);
  }, [isVisible]);

  if (!isVisible) return null;

  const categories = Array.from(new Set(HOTKEYS.map(h => h.category)));

  return (
    <>
      {/* Backdrop */}
      <div
        className="fixed inset-0 bg-black/70 z-[250] backdrop-blur-sm animate-in fade-in duration-200"
        onClick={() => setIsVisible(false)}
      />

      {/* Overlay Panel */}
      <div className="fixed top-1/2 left-1/2 transform -translate-x-1/2 -translate-y-1/2 z-[251] w-[900px] max-w-[95vw] max-h-[90vh] overflow-hidden">
        <div className="bg-[#1a1a1a] border-2 border-[#0066CC] rounded-lg shadow-2xl overflow-hidden animate-in zoom-in duration-300">
          {/* Header */}
          <div className="bg-gradient-to-r from-[#0066CC] to-[#0099FF] px-6 py-4 flex items-center justify-between">
            <div className="flex items-center gap-3">
              <Keyboard className="w-6 h-6 text-white" />
              <div>
                <h2 className="text-white text-xl">Keyboard Shortcuts</h2>
                <p className="text-white/80 text-sm">Master ECU Tuner Pro with hotkeys</p>
              </div>
            </div>
            <button
              onClick={() => setIsVisible(false)}
              className="text-white/80 hover:text-white transition-colors"
              aria-label="Close"
            >
              <X className="w-6 h-6" />
            </button>
          </div>

          {/* Content */}
          <div className="p-6 overflow-y-auto max-h-[calc(90vh-140px)] custom-scrollbar">
            <div className="grid grid-cols-2 gap-6">
              {categories.map((category) => (
                <div key={category} className="space-y-3">
                  <h3 className="text-[#0066CC] text-sm uppercase tracking-wider flex items-center gap-2">
                    <Zap className="w-4 h-4" />
                    {category}
                  </h3>
                  <div className="space-y-2">
                    {HOTKEYS.filter(h => h.category === category).map((hotkey, index) => (
                      <div
                        key={index}
                        className="flex items-center justify-between py-2 px-3 bg-black/40 rounded border border-[#333333] hover:border-[#0066CC] transition-colors"
                      >
                        <span className="text-[#CCCCCC] text-sm">{hotkey.description}</span>
                        <div className="flex items-center gap-1">
                          {hotkey.keys.map((key, i) => (
                            <div key={i} className="flex items-center gap-1">
                              <kbd className="px-2 py-1 bg-[#2a2a2a] border border-[#444444] rounded text-white text-xs font-mono shadow-md min-w-[32px] text-center">
                                {key}
                              </kbd>
                              {i < hotkey.keys.length - 1 && (
                                <span className="text-[#666666] text-xs">+</span>
                              )}
                            </div>
                          ))}
                        </div>
                      </div>
                    ))}
                  </div>
                </div>
              ))}
            </div>

            {/* Pro Tips */}
            <div className="mt-6 p-4 bg-[#0066CC]/10 border border-[#0066CC] rounded">
              <h4 className="text-[#0066CC] text-sm mb-2">💡 Pro Tips</h4>
              <ul className="text-[#AAAAAA] text-xs space-y-1">
                <li>• Hold <kbd className="px-1 py-0.5 bg-[#2a2a2a] rounded text-white text-xs">Shift</kbd> while dragging to constrain movement</li>
                <li>• Use <kbd className="px-1 py-0.5 bg-[#2a2a2a] rounded text-white text-xs">Ctrl</kbd> + Mouse Wheel to zoom graphs</li>
                <li>• Press <kbd className="px-1 py-0.5 bg-[#2a2a2a] rounded text-white text-xs">Space</kbd> to pause live data updates</li>
                <li>• Double-click table cells for quick edit</li>
                <li>• Right-click for context menus with more options</li>
              </ul>
            </div>
          </div>

          {/* Footer */}
          <div className="bg-[#0f0f0f] px-6 py-3 border-t border-[#333333] flex justify-between items-center">
            <div className="text-xs text-[#888888]">
              Press <kbd className="px-2 py-1 bg-[#2a2a2a] border border-[#444444] rounded text-white text-xs">F1</kbd> or{" "}
              <kbd className="px-1 py-0.5 bg-[#2a2a2a] border border-[#444444] rounded text-white text-xs">Ctrl</kbd>
              +
              <kbd className="px-1 py-0.5 bg-[#2a2a2a] border border-[#444444] rounded text-white text-xs">Shift</kbd>
              +
              <kbd className="px-1 py-0.5 bg-[#2a2a2a] border border-[#444444] rounded text-white text-xs">H</kbd>
              {" "}anytime to show this overlay
            </div>
            <button
              onClick={() => setIsVisible(false)}
              className="px-4 py-2 bg-[#0066CC] hover:bg-[#0080FF] text-white rounded transition-colors text-sm"
            >
              Got it!
            </button>
          </div>
        </div>
      </div>
    </>
  );
}
