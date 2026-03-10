import { useEffect } from "react";
import { toast } from "sonner@2.0.3";

interface KeyboardShortcutsProps {
  onScreenChange: (screen: string) => void;
  onSave?: () => void;
  onUpload?: () => void;
  onDownload?: () => void;
  liveTuning?: boolean;
  onToggleLiveTuning?: () => void;
}

export function KeyboardShortcuts({
  onScreenChange,
  onSave,
  onUpload,
  onDownload,
  liveTuning,
  onToggleLiveTuning,
}: KeyboardShortcutsProps) {
  useEffect(() => {
    const handleKeyPress = (event: KeyboardEvent) => {
      // Check for modifier keys (Ctrl/Cmd)
      const isCtrl = event.ctrlKey || event.metaKey;
      const isShift = event.shiftKey;

      // Ctrl/Cmd + S - Save
      if (isCtrl && event.key === "s") {
        event.preventDefault();
        onSave?.();
        return;
      }

      // Ctrl/Cmd + U - Upload to ECU
      if (isCtrl && event.key === "u") {
        event.preventDefault();
        onUpload?.();
        return;
      }

      // Ctrl/Cmd + D - Download from ECU
      if (isCtrl && event.key === "d") {
        event.preventDefault();
        onDownload?.();
        return;
      }

      // Ctrl/Cmd + L - Toggle Live Tuning
      if (isCtrl && event.key === "l") {
        event.preventDefault();
        onToggleLiveTuning?.();
        return;
      }

      // Ctrl/Cmd + Shift + H - Show keyboard shortcuts help
      if (isCtrl && isShift && event.key === "H") {
        event.preventDefault();
        showShortcutsHelp();
        return;
      }

      // Number keys 1-9 for quick screen navigation (when no input focused)
      if (
        !event.ctrlKey &&
        !event.metaKey &&
        !event.altKey &&
        event.target instanceof HTMLElement &&
        event.target.tagName !== "INPUT" &&
        event.target.tagName !== "TEXTAREA"
      ) {
        const screens = [
          "dashboard",
          "maps",
          "vtec",
          "boost",
          "advanced",
          "io",
          "o2ve",
          "logs",
          "sensors",
          "settings",
        ];
        const keyNum = parseInt(event.key);
        if (keyNum >= 1 && keyNum <= screens.length) {
          event.preventDefault();
          onScreenChange(screens[keyNum - 1]);
          return;
        }
      }

      // Escape key - Close modals/dropdowns (handled by individual components)
    };

    const showShortcutsHelp = () => {
      toast.info("Keyboard Shortcuts", {
        description: (
          <div className="text-xs space-y-1 mt-2">
            <div><strong>Ctrl+S</strong>: Save calibration</div>
            <div><strong>Ctrl+U</strong>: Upload to ECU</div>
            <div><strong>Ctrl+D</strong>: Download from ECU</div>
            <div><strong>Ctrl+L</strong>: Toggle Live Tuning</div>
            <div><strong>1-9</strong>: Quick screen navigation</div>
            <div><strong>Ctrl+Shift+H</strong>: Show this help</div>
            <div className="mt-2 pt-2 border-t border-[#333333]">
              <div><strong>In Tables:</strong></div>
              <div>Arrow keys: Navigate cells</div>
              <div>Enter: Edit cell</div>
              <div>Tab: Next field</div>
            </div>
          </div>
        ),
        duration: 8000,
      });
    };

    window.addEventListener("keydown", handleKeyPress);
    return () => window.removeEventListener("keydown", handleKeyPress);
  }, [onScreenChange, onSave, onUpload, onDownload, onToggleLiveTuning]);

  return null; // This is a logic-only component
}
