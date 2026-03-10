import { useState } from "react";
import { Lightbulb, ChevronRight, X } from "lucide-react";

interface InlineSuggestionChipProps {
  suggestion: string;
  onAccept: () => void;
  onDismiss: () => void;
  details?: string;
  actionLabel?: string;
}

/**
 * Inline suggestion chip - Smart assistant for tuning workflows
 * Appears contextually to suggest optimizations
 */
export function InlineSuggestionChip({
  suggestion,
  onAccept,
  onDismiss,
  details,
  actionLabel = "Apply",
}: InlineSuggestionChipProps) {
  const [isExpanded, setIsExpanded] = useState(false);

  return (
    <div className="inline-flex items-center gap-2 animate-in slide-in-from-right duration-300">
      <div
        className={`bg-gradient-to-r from-[#0066CC]/20 to-[#0099FF]/20 border-2 border-[#0066CC] rounded-lg overflow-hidden transition-all duration-300 ${
          isExpanded ? "w-[400px]" : "w-auto"
        }`}
      >
        {/* Collapsed state */}
        {!isExpanded ? (
          <button
            onClick={() => setIsExpanded(true)}
            className="flex items-center gap-2 px-3 py-2 hover:bg-[#0066CC]/10 transition-colors group"
          >
            <Lightbulb className="w-4 h-4 text-[#FFD700] animate-pulse" />
            <span className="text-sm text-[#00CCFF]">{suggestion}</span>
            <ChevronRight className="w-4 h-4 text-[#00CCFF] group-hover:translate-x-1 transition-transform" />
          </button>
        ) : (
          /* Expanded state */
          <div className="p-4">
            <div className="flex items-start justify-between mb-3">
              <div className="flex items-start gap-2">
                <Lightbulb className="w-5 h-5 text-[#FFD700] mt-0.5" />
                <div>
                  <div className="text-white text-sm mb-1">{suggestion}</div>
                  {details && (
                    <div className="text-[#AAAAAA] text-xs leading-relaxed">{details}</div>
                  )}
                </div>
              </div>
              <button
                onClick={onDismiss}
                className="text-[#666666] hover:text-white transition-colors"
                aria-label="Dismiss"
              >
                <X className="w-4 h-4" />
              </button>
            </div>

            <div className="flex items-center gap-2">
              <button
                onClick={() => {
                  onAccept();
                  setIsExpanded(false);
                }}
                className="flex-1 px-3 py-1.5 bg-[#0066CC] hover:bg-[#0080FF] text-white rounded text-sm transition-colors"
              >
                {actionLabel}
              </button>
              <button
                onClick={() => setIsExpanded(false)}
                className="px-3 py-1.5 bg-transparent border border-[#333333] hover:bg-[#333333] text-[#CCCCCC] rounded text-sm transition-colors"
              >
                Later
              </button>
            </div>
          </div>
        )}
      </div>
    </div>
  );
}

/**
 * Pre-configured suggestion scenarios
 */
export const SuggestionScenarios = {
  VETableAutoTune: {
    suggestion: "Auto-tune from log data?",
    details: "Detected recent dyno log with 150+ data points. AI can optimize VE table cells automatically based on actual AFR measurements.",
    actionLabel: "Auto-Tune VE Table",
  },

  IgnitionKnockDetected: {
    suggestion: "⚠ Knock detected - retard timing?",
    details: "Knock sensor triggered 12 times in last run. Recommend reducing ignition advance by 2-4° BTDC in affected cells.",
    actionLabel: "Apply Safe Timing",
  },

  FuelTrimOutOfRange: {
    suggestion: "Fuel trim correction needed",
    details: "Long-term fuel trim is ±15% from target. Base VE table may need adjustment to reduce ECU compensation.",
    actionLabel: "Adjust VE Table",
  },

  BoostOvershooting: {
    suggestion: "Boost control tuning suggested",
    details: "Target boost not met - overshooting by 2.3 PSI. PID tuning wizard can optimize wastegate control.",
    actionLabel: "Tune Boost PID",
  },

  IdleUnstable: {
    suggestion: "Idle stabilization available",
    details: "RPM variance detected (±150 RPM). Idle control parameters can be auto-adjusted for smoother operation.",
    actionLabel: "Stabilize Idle",
  },

  ColdStartRich: {
    suggestion: "Cold start enrichment optimization",
    details: "AFR shows excessive richness during cranking (10.2:1 vs target 12.5:1). Reduce cranking fuel by 15%.",
    actionLabel: "Optimize Cranking",
  },

  InterpolationRecommended: {
    suggestion: "Smooth table with interpolation?",
    details: "Detected abrupt transitions between cells. Cubic interpolation can create smoother fuel/ignition curves.",
    actionLabel: "Apply Interpolation",
  },

  BackupRecommended: {
    suggestion: "💾 Save backup before major change?",
    details: "You're about to modify rev limit and boost settings. It's recommended to save a backup calibration first.",
    actionLabel: "Save Backup",
  },
};
