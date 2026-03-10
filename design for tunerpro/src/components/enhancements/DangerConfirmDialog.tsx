import { useState } from "react";
import { AlertTriangle, ShieldAlert, X } from "lucide-react";
import { Button } from "../ui/button";
import { Checkbox } from "../ui/checkbox";

interface DangerConfirmDialogProps {
  isOpen: boolean;
  onClose: () => void;
  onConfirm: () => void;
  title: string;
  description: string;
  dangerLevel: "warning" | "critical";
  risks: string[];
  requireCheckbox?: boolean;
  checkboxText?: string;
}

/**
 * Mandatory confirmation dialog for dangerous ECU operations
 * Implements safety-first UX for high-stakes tuning decisions
 */
export function DangerConfirmDialog({
  isOpen,
  onClose,
  onConfirm,
  title,
  description,
  dangerLevel,
  risks,
  requireCheckbox = true,
  checkboxText = "I understand the risks and have verified this change",
}: DangerConfirmDialogProps) {
  const [isChecked, setIsChecked] = useState(false);
  const [countdown, setCountdown] = useState(3);
  const [canConfirm, setCanConfirm] = useState(false);

  // 3-second countdown for critical operations
  useState(() => {
    if (isOpen && dangerLevel === "critical") {
      setCountdown(3);
      setCanConfirm(false);
      const timer = setInterval(() => {
        setCountdown((prev) => {
          if (prev <= 1) {
            clearInterval(timer);
            setCanConfirm(true);
            return 0;
          }
          return prev - 1;
        });
      }, 1000);
      return () => clearInterval(timer);
    } else if (isOpen) {
      setCanConfirm(true);
    }
  });

  const handleConfirm = () => {
    if (!requireCheckbox || isChecked) {
      onConfirm();
      setIsChecked(false);
    }
  };

  const handleClose = () => {
    onClose();
    setIsChecked(false);
  };

  if (!isOpen) return null;

  const borderColor = dangerLevel === "critical" ? "border-[#FF0000]" : "border-[#FFFF00]";
  const bgGradient = dangerLevel === "critical" 
    ? "from-[#330000] to-[#1a0000]"
    : "from-[#332200] to-[#1a1100]";
  const iconColor = dangerLevel === "critical" ? "text-[#FF0000]" : "text-[#FFFF00]";

  return (
    <>
      {/* High-priority backdrop */}
      <div className="fixed inset-0 bg-black/90 z-[300] backdrop-blur-md animate-in fade-in duration-200" />

      {/* Dialog */}
      <div className="fixed top-1/2 left-1/2 transform -translate-x-1/2 -translate-y-1/2 z-[301] w-[600px] max-w-[90vw]">
        <div className={`bg-gradient-to-br ${bgGradient} border-4 ${borderColor} rounded-lg shadow-2xl overflow-hidden animate-in zoom-in duration-300`}>
          {/* Header with pulsing icon */}
          <div className={`bg-black/50 px-6 py-4 flex items-center justify-between border-b-2 ${borderColor}`}>
            <div className="flex items-center gap-3">
              <div className={`${iconColor} animate-pulse`}>
                {dangerLevel === "critical" ? (
                  <ShieldAlert className="w-8 h-8" />
                ) : (
                  <AlertTriangle className="w-8 h-8" />
                )}
              </div>
              <div>
                <h2 className="text-white text-xl">
                  {dangerLevel === "critical" ? "⚠️ CRITICAL WARNING" : "⚠ Warning"}
                </h2>
                <p className={`text-sm ${iconColor}`}>{title}</p>
              </div>
            </div>
            <button
              onClick={handleClose}
              className="text-[#888888] hover:text-white transition-colors"
              aria-label="Cancel"
            >
              <X className="w-6 h-6" />
            </button>
          </div>

          {/* Content */}
          <div className="px-6 py-6 space-y-4">
            <p className="text-[#CCCCCC] leading-relaxed">{description}</p>

            {/* Risk list */}
            <div className="bg-black/40 border-2 border-[#333333] rounded p-4">
              <div className={`${iconColor} text-sm mb-2`}>
                Potential Risks:
              </div>
              <ul className="space-y-2">
                {risks.map((risk, index) => (
                  <li key={index} className="flex items-start gap-2 text-[#AAAAAA] text-sm">
                    <span className={iconColor}>•</span>
                    <span>{risk}</span>
                  </li>
                ))}
              </ul>
            </div>

            {/* Safety recommendations */}
            <div className="bg-[#0066CC]/10 border border-[#0066CC] rounded p-3">
              <div className="text-[#0066CC] text-xs mb-1">💡 Safety Recommendations:</div>
              <ul className="text-[#AAAAAA] text-xs space-y-1">
                <li>• Ensure ECU is in bench mode (not connected to running engine)</li>
                <li>• Have backup calibration file saved</li>
                <li>• Monitor sensor readings after change</li>
                {dangerLevel === "critical" && (
                  <li className="text-[#FF0000]">• Consider consulting experienced tuner first</li>
                )}
              </ul>
            </div>

            {/* Confirmation checkbox */}
            {requireCheckbox && (
              <div className="flex items-center space-x-3 pt-2">
                <Checkbox
                  id="risk-acknowledge"
                  checked={isChecked}
                  onCheckedChange={(checked) => setIsChecked(checked as boolean)}
                  className="border-[#666666]"
                />
                <label
                  htmlFor="risk-acknowledge"
                  className="text-sm text-white cursor-pointer select-none"
                >
                  {checkboxText}
                </label>
              </div>
            )}
          </div>

          {/* Footer */}
          <div className={`bg-black/60 px-6 py-4 border-t-2 ${borderColor} flex justify-between items-center`}>
            <Button
              onClick={handleClose}
              variant="outline"
              className="bg-transparent border-[#666666] text-white hover:bg-[#333333]"
            >
              Cancel (Safe)
            </Button>

            <div className="flex items-center gap-3">
              {dangerLevel === "critical" && !canConfirm && (
                <div className="text-[#FFFF00] text-sm animate-pulse">
                  Please wait {countdown}s...
                </div>
              )}
              <Button
                onClick={handleConfirm}
                disabled={!canConfirm || (requireCheckbox && !isChecked)}
                className={`${
                  dangerLevel === "critical"
                    ? "bg-[#FF0000] hover:bg-[#CC0000]"
                    : "bg-[#FFAA00] hover:bg-[#FF8800]"
                } text-white disabled:opacity-30 disabled:cursor-not-allowed transition-all duration-200`}
              >
                {dangerLevel === "critical" ? "⚠ Apply Critical Change" : "Apply Change"}
              </Button>
            </div>
          </div>
        </div>
      </div>
    </>
  );
}

/**
 * Pre-configured danger dialogs for common operations
 */
export const DangerDialogs = {
  RevLimit: (rpm: number) => ({
    title: `Rev Limit: ${rpm} RPM`,
    description: `You are setting the rev limiter to ${rpm} RPM. This is ${
      rpm > 8500 ? "EXTREMELY HIGH" : rpm > 7500 ? "HIGH" : "within normal range"
    } for most engines.`,
    dangerLevel: (rpm > 8500 ? "critical" : rpm > 7500 ? "warning" : "warning") as "warning" | "critical",
    risks: [
      ...(rpm > 8500 ? ["Severe engine damage from over-revving", "Valve float and potential piston contact"] : []),
      ...(rpm > 7500 ? ["Increased wear on valvetrain components", "Risk of rod bearing failure"] : []),
      "Potential warranty void",
      "May require upgraded valve springs and retainers",
    ],
  }),

  BoostLimit: (psi: number) => ({
    title: `Boost Limit: ${psi} PSI`,
    description: `You are setting maximum boost to ${psi} PSI. Stock engines typically handle 8-12 PSI safely.`,
    dangerLevel: (psi > 20 ? "critical" : "warning") as "warning" | "critical",
    risks: [
      ...(psi > 20 ? ["Engine detonation and catastrophic failure", "Turbo overspeeding"] : []),
      "Increased cylinder pressure and stress",
      "May require forged pistons and upgraded rods",
      "Fueling must be adjusted accordingly",
    ],
  }),

  AFRLean: (afr: number) => ({
    title: `Lean AFR: ${afr.toFixed(1)}:1`,
    description: `You are setting AFR to ${afr.toFixed(1)}:1 which is LEAN. Lean mixtures can cause detonation.`,
    dangerLevel: (afr > 15.5 ? "critical" : "warning") as "warning" | "critical",
    risks: [
      ...(afr > 15.5 ? ["Severe detonation risk", "Piston melting and engine seizure"] : []),
      "Increased exhaust gas temperatures",
      "Potential catalyst damage",
      "Loss of power under load",
    ],
  }),

  IgnitionAdvance: (degrees: number) => ({
    title: `Ignition Advance: ${degrees}° BTDC`,
    description: `You are setting ignition timing to ${degrees}° before TDC. Too much advance causes knock.`,
    dangerLevel: (degrees > 35 ? "critical" : "warning") as "warning" | "critical",
    risks: [
      ...(degrees > 35 ? ["Severe engine knock and detonation", "Piston damage"] : []),
      "Pre-ignition under boost",
      "Requires high-octane fuel",
      "Monitor knock sensor activity",
    ],
  }),
};
