import { ReactNode } from "react";
import {
  Tooltip,
  TooltipContent,
  TooltipProvider,
  TooltipTrigger,
} from "../ui/tooltip";

interface TooltipWrapperProps {
  children: ReactNode;
  content: string | ReactNode;
  glossaryLink?: string;
  side?: "top" | "bottom" | "left" | "right";
  delayDuration?: number;
}

/**
 * Enhanced tooltip wrapper with glossary integration
 * Provides contextual help for ECU tuning parameters
 */
export function TooltipWrapper({
  children,
  content,
  glossaryLink,
  side = "top",
  delayDuration = 200,
}: TooltipWrapperProps) {
  return (
    <TooltipProvider delayDuration={delayDuration}>
      <Tooltip>
        <TooltipTrigger asChild>{children}</TooltipTrigger>
        <TooltipContent
          side={side}
          className="max-w-[300px] bg-[#1a1a1a] border-2 border-[#0066CC] text-white p-3 shadow-xl"
        >
          <div className="space-y-2">
            {typeof content === "string" ? (
              <p className="text-sm leading-relaxed">{content}</p>
            ) : (
              content
            )}
            
            {glossaryLink && (
              <div className="pt-2 border-t border-[#333333]">
                <a
                  href={glossaryLink}
                  target="_blank"
                  rel="noopener noreferrer"
                  className="text-xs text-[#0066CC] hover:text-[#0080FF] transition-colors flex items-center gap-1"
                  onClick={(e) => {
                    e.stopPropagation();
                    // Open glossary panel instead of external link
                  }}
                >
                  → View in Glossary
                </a>
              </div>
            )}
          </div>
        </TooltipContent>
      </Tooltip>
    </TooltipProvider>
  );
}

/**
 * Pre-configured tooltips for common ECU parameters
 */
export const ECUTooltips = {
  VETable: (
    <div className="space-y-1">
      <div className="text-sm">
        <strong>Volumetric Efficiency Table</strong>
      </div>
      <div className="text-xs text-[#CCCCCC]">
        Maps engine breathing efficiency across RPM and load. Higher values = more fuel delivery.
      </div>
      <div className="text-xs text-[#888888] mt-2">
        Interpolation: Linear or Cubic selectable
      </div>
    </div>
  ),

  IgnitionTiming: (
    <div className="space-y-1">
      <div className="text-sm">
        <strong>Ignition Timing Map</strong>
      </div>
      <div className="text-xs text-[#CCCCCC]">
        Spark advance in degrees BTDC (Before Top Dead Center). More advance = earlier ignition.
      </div>
      <div className="text-xs text-[#FF0000] mt-2">
        ⚠ Too much advance can cause knock/detonation
      </div>
    </div>
  ),

  AFRTarget: (
    <div className="space-y-1">
      <div className="text-sm">
        <strong>Air-Fuel Ratio Target</strong>
      </div>
      <div className="text-xs text-[#CCCCCC]">
        Stoichiometric: 14.7:1 (gasoline)
      </div>
      <div className="text-xs text-[#CCCCCC]">
        Rich (power): 12.5:1 - 13.5:1
      </div>
      <div className="text-xs text-[#CCCCCC]">
        Lean (economy): 15.0:1 - 16.0:1
      </div>
    </div>
  ),

  BoostControl: (
    <div className="space-y-1">
      <div className="text-sm">
        <strong>Boost Pressure Control</strong>
      </div>
      <div className="text-xs text-[#CCCCCC]">
        Electronic wastegate control for turbo applications. Measured in kPa or PSI.
      </div>
      <div className="text-xs text-[#FFFF00] mt-2">
        ⚠ Start conservative - too much boost without proper tuning is dangerous
      </div>
    </div>
  ),

  VTECPoint: (
    <div className="space-y-1">
      <div className="text-sm">
        <strong>VTEC Engagement Point</strong>
      </div>
      <div className="text-xs text-[#CCCCCC]">
        RPM threshold where VTEC cam profile switches (Honda engines).
      </div>
      <div className="text-xs text-[#888888] mt-2">
        Typical: 5,000-6,000 RPM (stock), 4,000-5,500 RPM (performance)
      </div>
    </div>
  ),

  DeadTime: (
    <div className="space-y-1">
      <div className="text-sm">
        <strong>Injector Dead Time</strong>
      </div>
      <div className="text-xs text-[#CCCCCC]">
        Opening/closing delay in milliseconds. Varies with battery voltage.
      </div>
      <div className="text-xs text-[#888888] mt-2">
        Range: 0.5ms - 2.0ms (depends on injector type)
      </div>
    </div>
  ),

  LaunchRPM: (
    <div className="space-y-1">
      <div className="text-sm">
        <strong>Launch Control RPM Limit</strong>
      </div>
      <div className="text-xs text-[#CCCCCC]">
        Two-step rev limiter for drag racing launches. Builds boost at standstill.
      </div>
      <div className="text-xs text-[#FF0000] mt-2">
        ⚠ Hard on engine - use sparingly
      </div>
    </div>
  ),

  CrankingFuel: (
    <div className="space-y-1">
      <div className="text-sm">
        <strong>Cranking Fuel Enrichment</strong>
      </div>
      <div className="text-xs text-[#CCCCCC]">
        Extra fuel added during engine start. Temperature compensated.
      </div>
      <div className="text-xs text-[#888888] mt-2">
        Cold starts need more enrichment (150-200%)
      </div>
    </div>
  ),
};
