import { useState } from "react";
import { Card } from "../../ui/card";
import { Label } from "../../ui/label";
import { Input } from "../../ui/input";
import { Button } from "../../ui/button";
import { X, Calculator } from "lucide-react";

interface RequiredFuelCalculatorProps {
  onClose: () => void;
  onApply: (values: {
    displacement: string;
    cylinders: string;
    injectorFlow: string;
    afr: string;
    requiredFuel: number;
  }) => void;
}

export function RequiredFuelCalculator({ onClose, onApply }: RequiredFuelCalculatorProps) {
  const [displacement, setDisplacement] = useState("122.0");
  const [displacementUnit, setDisplacementUnit] = useState<"cid" | "cc">("cc");
  const [cylinders, setCylinders] = useState("4");
  const [injectorFlow, setInjectorFlow] = useState("180");
  const [flowUnit, setFlowUnit] = useState<"lbhr" | "ccmin">("ccmin");
  const [afr, setAfr] = useState("14.7");
  const [requiredFuel, setRequiredFuel] = useState<number | null>(null);

  const calculate = () => {
    // Convert to consistent units
    let displacementCC = parseFloat(displacement);
    if (displacementUnit === "cid") {
      displacementCC = displacementCC * 16.387; // Convert CID to CC
    }

    let flowCCMin = parseFloat(injectorFlow);
    if (flowUnit === "lbhr") {
      // Convert lb/hr to cc/min (gasoline density ~0.755 g/cc)
      flowCCMin = (flowCCMin * 453.592) / (60 * 0.755);
    }

    const cylCount = parseFloat(cylinders);
    const afrValue = parseFloat(afr);

    // Required Fuel Formula (simplified)
    // RF = (Displacement / Cylinders) / (Injector Flow) * (14.7 / AFR) * constant
    const displacementPerCyl = displacementCC / cylCount;
    const rf = (displacementPerCyl * 14.7) / (flowCCMin * afrValue) * 2.0;

    setRequiredFuel(Math.round(rf * 10) / 10);
  };

  const handleApply = () => {
    if (requiredFuel !== null) {
      onApply({
        displacement: displacementUnit === "cc" ? displacement : (parseFloat(displacement) * 16.387).toFixed(1),
        cylinders,
        injectorFlow: flowUnit === "ccmin" ? injectorFlow : ((parseFloat(injectorFlow) * 453.592) / (60 * 0.755)).toFixed(1),
        afr,
        requiredFuel,
      });
    }
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/80 backdrop-blur-sm">
      <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF] w-full max-w-md p-6 m-4">
        {/* Header */}
        <div className="flex items-center justify-between mb-6">
          <div className="flex items-center gap-3">
            <Calculator className="w-6 h-6 text-[#00CCFF]" />
            <h3 className="text-white text-xl font-semibold">Required Fuel Calculator</h3>
          </div>
          <button
            onClick={onClose}
            className="text-[#888888] hover:text-white transition-colors"
          >
            <X className="w-5 h-5" />
          </button>
        </div>

        {/* Form */}
        <div className="space-y-4">
          {/* Engine Displacement */}
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Engine Displacement</Label>
            <div className="flex gap-2">
              <Input
                type="number"
                value={displacement}
                onChange={(e) => setDisplacement(e.target.value)}
                className="flex-1 bg-black border-[#555555] text-white font-mono"
                placeholder="122.0"
              />
              <div className="flex gap-1">
                <button
                  onClick={() => setDisplacementUnit("cid")}
                  className={`px-3 py-2 rounded border text-sm transition-colors ${
                    displacementUnit === "cid"
                      ? "bg-[#0088FF] border-[#0088FF] text-white"
                      : "bg-black border-[#555555] text-[#888888] hover:border-[#0088FF]"
                  }`}
                >
                  CID
                </button>
                <button
                  onClick={() => setDisplacementUnit("cc")}
                  className={`px-3 py-2 rounded border text-sm transition-colors ${
                    displacementUnit === "cc"
                      ? "bg-[#0088FF] border-[#0088FF] text-white"
                      : "bg-black border-[#555555] text-[#888888] hover:border-[#0088FF]"
                  }`}
                >
                  CC
                </button>
              </div>
            </div>
          </div>

          {/* Number of Cylinders */}
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Number of Cylinders</Label>
            <Input
              type="number"
              value={cylinders}
              onChange={(e) => setCylinders(e.target.value)}
              className="bg-black border-[#555555] text-white font-mono"
              placeholder="4"
            />
          </div>

          {/* Injector Flow */}
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Injector Flow</Label>
            <div className="flex gap-2">
              <Input
                type="number"
                value={injectorFlow}
                onChange={(e) => setInjectorFlow(e.target.value)}
                className="flex-1 bg-black border-[#555555] text-white font-mono"
                placeholder="180"
              />
              <div className="flex gap-1">
                <button
                  onClick={() => setFlowUnit("lbhr")}
                  className={`px-3 py-2 rounded border text-sm transition-colors whitespace-nowrap ${
                    flowUnit === "lbhr"
                      ? "bg-[#0088FF] border-[#0088FF] text-white"
                      : "bg-black border-[#555555] text-[#888888] hover:border-[#0088FF]"
                  }`}
                >
                  lb/hr
                </button>
                <button
                  onClick={() => setFlowUnit("ccmin")}
                  className={`px-3 py-2 rounded border text-sm transition-colors whitespace-nowrap ${
                    flowUnit === "ccmin"
                      ? "bg-[#0088FF] border-[#0088FF] text-white"
                      : "bg-black border-[#555555] text-[#888888] hover:border-[#0088FF]"
                  }`}
                >
                  cc/min
                </button>
              </div>
            </div>
          </div>

          {/* Air:Fuel Ratio */}
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Target Air:Fuel Ratio</Label>
            <Input
              type="number"
              step="0.1"
              value={afr}
              onChange={(e) => setAfr(e.target.value)}
              className="bg-black border-[#555555] text-white font-mono"
              placeholder="14.7"
            />
          </div>

          {/* Calculate Button */}
          <Button
            onClick={calculate}
            className="w-full bg-gradient-to-r from-[#0088FF] to-[#00CCFF] hover:from-[#0066CC] hover:to-[#00AACC]"
          >
            <Calculator className="w-4 h-4 mr-2" />
            Calculate
          </Button>

          {/* Result */}
          {requiredFuel !== null && (
            <div className="p-4 bg-gradient-to-r from-[#00FF88]/20 to-[#00CC66]/20 border-2 border-[#00FF88] rounded-lg">
              <div className="text-center">
                <p className="text-xs text-[#888888] mb-1">Calculated Required Fuel</p>
                <p className="text-3xl font-bold text-[#00FF88] font-mono">{requiredFuel} ms</p>
              </div>
            </div>
          )}
        </div>

        {/* Actions */}
        <div className="flex gap-3 mt-6">
          <Button
            variant="outline"
            onClick={onClose}
            className="flex-1 border-[#555555]"
          >
            Cancel
          </Button>
          <Button
            onClick={handleApply}
            disabled={requiredFuel === null}
            className="flex-1 bg-gradient-to-r from-[#00FF88] to-[#00CC66] hover:from-[#00DD77] hover:to-[#00BB55] text-black font-semibold"
          >
            Apply
          </Button>
        </div>

        {/* Info */}
        <p className="text-xs text-[#666666] mt-4 text-center">
          This calculator provides a starting point. Fine-tuning may be required.
        </p>
      </Card>
    </div>
  );
}
