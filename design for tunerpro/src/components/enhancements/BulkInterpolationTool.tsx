import { useState } from "react";
import { TrendingUp, Zap, Grid3x3 } from "lucide-react";
import { Button } from "../ui/button";
import { RadioGroup, RadioGroupItem } from "../ui/radio-group";
import { Label } from "../ui/label";

interface BulkInterpolationToolProps {
  isOpen: boolean;
  onClose: () => void;
  onApply: (method: "linear" | "cubic" | "spline", fillGaps: boolean) => void;
  selectedCells: { row: number; col: number }[];
}

/**
 * Bulk Interpolation Tool for table editing
 * Allows smoothing and filling table data with mathematical curves
 */
export function BulkInterpolationTool({
  isOpen,
  onClose,
  onApply,
  selectedCells,
}: BulkInterpolationToolProps) {
  const [method, setMethod] = useState<"linear" | "cubic" | "spline">("linear");
  const [fillGaps, setFillGaps] = useState(true);
  const [showPreview, setShowPreview] = useState(false);

  if (!isOpen) return null;

  const handleApply = () => {
    onApply(method, fillGaps);
    onClose();
  };

  return (
    <>
      {/* Backdrop */}
      <div
        className="fixed inset-0 bg-black/70 z-[200] backdrop-blur-sm"
        onClick={onClose}
      />

      {/* Tool Panel */}
      <div className="fixed top-1/2 left-1/2 transform -translate-x-1/2 -translate-y-1/2 z-[201] w-[600px] max-w-[90vw]">
        <div className="bg-[#1a1a1a] border-2 border-[#0066CC] rounded-lg shadow-2xl overflow-hidden">
          {/* Header */}
          <div className="bg-[#0f0f0f] px-6 py-4 border-b border-[#333333]">
            <div className="flex items-center gap-3">
              <Grid3x3 className="w-5 h-5 text-[#0066CC]" />
              <div>
                <h3 className="text-white">Bulk Interpolation</h3>
                <p className="text-[#888888] text-sm">
                  {selectedCells.length} cells selected
                </p>
              </div>
            </div>
          </div>

          {/* Content */}
          <div className="p-6 space-y-6">
            {/* Method Selection */}
            <div>
              <Label className="text-[#CCCCCC] mb-3 block">
                Interpolation Method
              </Label>
              <RadioGroup value={method} onValueChange={(val) => setMethod(val as any)}>
                <div className="space-y-3">
                  {/* Linear */}
                  <div className="flex items-start gap-3 p-3 bg-black/40 rounded border border-[#333333] hover:border-[#0066CC] transition-colors">
                    <RadioGroupItem value="linear" id="linear" className="mt-1" />
                    <div className="flex-1">
                      <Label htmlFor="linear" className="text-white cursor-pointer">
                        Linear Interpolation
                      </Label>
                      <p className="text-[#888888] text-xs mt-1">
                        Straight line between points. Fast and predictable.
                      </p>
                      <div className="mt-2 h-12 bg-[#0a0a0a] rounded flex items-center justify-center">
                        <svg width="120" height="40" viewBox="0 0 120 40">
                          <path
                            d="M 10,30 L 110,10"
                            stroke="#00CCFF"
                            strokeWidth="2"
                            fill="none"
                          />
                          <circle cx="10" cy="30" r="3" fill="#00FF00" />
                          <circle cx="110" cy="10" r="3" fill="#00FF00" />
                        </svg>
                      </div>
                    </div>
                  </div>

                  {/* Cubic */}
                  <div className="flex items-start gap-3 p-3 bg-black/40 rounded border border-[#333333] hover:border-[#0066CC] transition-colors">
                    <RadioGroupItem value="cubic" id="cubic" className="mt-1" />
                    <div className="flex-1">
                      <Label htmlFor="cubic" className="text-white cursor-pointer">
                        Cubic Interpolation
                      </Label>
                      <p className="text-[#888888] text-xs mt-1">
                        Smooth curves through points. Best for natural transitions.
                      </p>
                      <div className="mt-2 h-12 bg-[#0a0a0a] rounded flex items-center justify-center">
                        <svg width="120" height="40" viewBox="0 0 120 40">
                          <path
                            d="M 10,30 Q 40,5 70,15 T 110,10"
                            stroke="#00CCFF"
                            strokeWidth="2"
                            fill="none"
                          />
                          <circle cx="10" cy="30" r="3" fill="#00FF00" />
                          <circle cx="110" cy="10" r="3" fill="#00FF00" />
                        </svg>
                      </div>
                    </div>
                  </div>

                  {/* Spline */}
                  <div className="flex items-start gap-3 p-3 bg-black/40 rounded border border-[#333333] hover:border-[#0066CC] transition-colors">
                    <RadioGroupItem value="spline" id="spline" className="mt-1" />
                    <div className="flex-1">
                      <Label htmlFor="spline" className="text-white cursor-pointer">
                        Spline Interpolation
                      </Label>
                      <p className="text-[#888888] text-xs mt-1">
                        Multiple curves with continuous derivatives. Smoothest result.
                      </p>
                      <div className="mt-2 h-12 bg-[#0a0a0a] rounded flex items-center justify-center">
                        <svg width="120" height="40" viewBox="0 0 120 40">
                          <path
                            d="M 10,30 C 30,20 50,10 70,15 S 90,20 110,10"
                            stroke="#00CCFF"
                            strokeWidth="2"
                            fill="none"
                          />
                          <circle cx="10" cy="30" r="3" fill="#00FF00" />
                          <circle cx="110" cy="10" r="3" fill="#00FF00" />
                        </svg>
                      </div>
                    </div>
                  </div>
                </div>
              </RadioGroup>
            </div>

            {/* Options */}
            <div className="space-y-3">
              <Label className="text-[#CCCCCC]">Options</Label>
              
              <div className="flex items-center gap-3 p-3 bg-black/40 rounded border border-[#333333]">
                <input
                  type="checkbox"
                  id="fillGaps"
                  checked={fillGaps}
                  onChange={(e) => setFillGaps(e.target.checked)}
                  className="w-4 h-4"
                />
                <div className="flex-1">
                  <Label htmlFor="fillGaps" className="text-white cursor-pointer">
                    Fill gaps in selection
                  </Label>
                  <p className="text-[#888888] text-xs mt-0.5">
                    Interpolate empty cells between selected points
                  </p>
                </div>
              </div>

              <div className="flex items-center gap-3 p-3 bg-black/40 rounded border border-[#333333]">
                <input
                  type="checkbox"
                  id="preview"
                  checked={showPreview}
                  onChange={(e) => setShowPreview(e.target.checked)}
                  className="w-4 h-4"
                />
                <div className="flex-1">
                  <Label htmlFor="preview" className="text-white cursor-pointer">
                    Show preview before applying
                  </Label>
                  <p className="text-[#888888] text-xs mt-0.5">
                    Display interpolated values in split-view mode
                  </p>
                </div>
              </div>
            </div>

            {/* Info */}
            <div className="bg-[#0066CC]/10 border border-[#0066CC] rounded p-3">
              <div className="flex items-start gap-2">
                <TrendingUp className="w-4 h-4 text-[#0066CC] mt-0.5" />
                <div className="text-xs text-[#CCCCCC]">
                  <strong className="text-[#0066CC]">Tip:</strong> Cubic interpolation works
                  best for VE and ignition tables. Use linear for simple fills.
                </div>
              </div>
            </div>
          </div>

          {/* Footer */}
          <div className="bg-[#0f0f0f] px-6 py-4 border-t border-[#333333] flex justify-end gap-3">
            <Button
              onClick={onClose}
              variant="outline"
              className="bg-transparent border-[#333333] text-white hover:bg-[#333333]"
            >
              Cancel
            </Button>
            <Button
              onClick={handleApply}
              className="bg-[#0066CC] hover:bg-[#0080FF] text-white"
            >
              <Zap className="w-4 h-4 mr-2" />
              Apply Interpolation
            </Button>
          </div>
        </div>
      </div>
    </>
  );
}
