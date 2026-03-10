import { useState } from "react";
import { Card } from "../../ui/card";
import { Label } from "../../ui/label";
import { Input } from "../../ui/input";
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "../../ui/select";
import { Button } from "../../ui/button";
import { Alert, AlertDescription } from "../../ui/alert";
import { Info, Calculator, AlertTriangle } from "lucide-react";
import { RequiredFuelCalculator } from "../modals/RequiredFuelCalculator";
import { FiringOrderEditor } from "../modals/FiringOrderEditor";

interface EngineBasicSettingsProps {
  onSettingsChange: () => void;
}

export function EngineBasicSettings({ onSettingsChange }: EngineBasicSettingsProps) {
  const [showCalculator, setShowCalculator] = useState(false);
  const [showFiringOrderEditor, setShowFiringOrderEditor] = useState(false);

  // Engine Settings State
  const [controlAlgorithm, setControlAlgorithm] = useState("speed-density");
  const [squirtsPerCycle, setSquirtsPerCycle] = useState("2");
  const [injectorStaging, setInjectorStaging] = useState("alternating");
  const [engineStroke, setEngineStroke] = useState("four-stroke");
  const [numCylinders, setNumCylinders] = useState("4");
  const [numInjectors, setNumInjectors] = useState("4");
  const [engineType, setEngineType] = useState("even-fire");
  const [engineSize, setEngineSize] = useState("2000");
  const [injectorSize, setInjectorSize] = useState("180");

  // Sequential Settings
  const [sequentialEnabled, setSequentialEnabled] = useState(true);
  const [sequentialOutput, setSequentialOutput] = useState("main-fuel");
  const [angleBTDC, setAngleBTDC] = useState("355");

  const handleChange = (setter: (value: string) => void) => (value: string) => {
    setter(value);
    onSettingsChange();
  };

  return (
    <div className="space-y-6">
      {/* Engine and Sequential Settings */}
      <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
        <h3 className="text-white mb-6 flex items-center gap-2 text-xl">
          <span className="text-2xl">⚙️</span>
          Engine and Sequential Settings
        </h3>

        <div className="grid grid-cols-2 gap-6">
          {/* Control Algorithm */}
          <div>
            <div className="flex items-center gap-2 mb-2">
              <Label className="text-[#CCCCCC]">Control Algorithm</Label>
              <Info className="w-3 h-3 text-[#00CCFF] cursor-help" title="Method used to calculate engine load" />
            </div>
            <Select value={controlAlgorithm} onValueChange={handleChange(setControlAlgorithm)}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue />
              </SelectTrigger>
              <SelectContent className="bg-[#1a1a1a] border-[#555555]">
                <SelectItem value="speed-density" className="text-white">Speed Density (MAP based)</SelectItem>
                <SelectItem value="alpha-n" className="text-white">Alpha-N (TPS based)</SelectItem>
                <SelectItem value="maf" className="text-white">MAF (Mass Air Flow)</SelectItem>
                <SelectItem value="itb" className="text-white">ITB (Individual Throttle Bodies)</SelectItem>
                <SelectItem value="sd-an-blend" className="text-white">SD/Alpha-N Blend</SelectItem>
              </SelectContent>
            </Select>
          </div>

          {/* Squirts Per Engine Cycle */}
          <div>
            <div className="flex items-center gap-2 mb-2">
              <Label className="text-[#CCCCCC]">Squirts Per Engine Cycle</Label>
              <Info className="w-3 h-3 text-[#00CCFF] cursor-help" title="Number of injection events per engine cycle" />
            </div>
            <Select value={squirtsPerCycle} onValueChange={handleChange(setSquirtsPerCycle)}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue />
              </SelectTrigger>
              <SelectContent className="bg-[#1a1a1a] border-[#555555]">
                {[1, 2, 3, 4, 6, 8].map(n => (
                  <SelectItem key={n} value={n.toString()} className="text-white">{n}</SelectItem>
                ))}
              </SelectContent>
            </Select>
          </div>

          {/* Injector Staging */}
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Injector Staging</Label>
            <Select value={injectorStaging} onValueChange={handleChange(setInjectorStaging)}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue />
              </SelectTrigger>
              <SelectContent className="bg-[#1a1a1a] border-[#555555]">
                <SelectItem value="alternating" className="text-white">Alternating (Bank-to-Bank)</SelectItem>
                <SelectItem value="simultaneous" className="text-white">Simultaneous (All at once)</SelectItem>
                <SelectItem value="sequential" className="text-white">Sequential (Individual)</SelectItem>
              </SelectContent>
            </Select>
          </div>

          {/* Engine Stroke */}
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Engine Stroke/Rotary</Label>
            <Select value={engineStroke} onValueChange={handleChange(setEngineStroke)}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue />
              </SelectTrigger>
              <SelectContent className="bg-[#1a1a1a] border-[#555555]">
                <SelectItem value="two-stroke" className="text-white">Two-stroke</SelectItem>
                <SelectItem value="four-stroke" className="text-white">Four-stroke</SelectItem>
                <SelectItem value="rotary" className="text-white">Rotary (Wankel)</SelectItem>
              </SelectContent>
            </Select>
          </div>

          {/* Number of Cylinders */}
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">No. Cylinders/Rotors</Label>
            <Select value={numCylinders} onValueChange={handleChange(setNumCylinders)}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue />
              </SelectTrigger>
              <SelectContent className="bg-[#1a1a1a] border-[#555555]">
                {[1, 2, 3, 4, 5, 6, 8, 10, 12].map(n => (
                  <SelectItem key={n} value={n.toString()} className="text-white">{n}</SelectItem>
                ))}
              </SelectContent>
            </Select>
          </div>

          {/* Number of Injectors */}
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Number of Injectors</Label>
            <Select value={numInjectors} onValueChange={handleChange(setNumInjectors)}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue />
              </SelectTrigger>
              <SelectContent className="bg-[#1a1a1a] border-[#555555]">
                {[1, 2, 4, 6, 8, 10, 12, 16].map(n => (
                  <SelectItem key={n} value={n.toString()} className="text-white">{n}</SelectItem>
                ))}
              </SelectContent>
            </Select>
          </div>

          {/* Engine Type */}
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Engine Type</Label>
            <Select value={engineType} onValueChange={handleChange(setEngineType)}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue />
              </SelectTrigger>
              <SelectContent className="bg-[#1a1a1a] border-[#555555]">
                <SelectItem value="even-fire" className="text-white">Even Fire</SelectItem>
                <SelectItem value="odd-fire" className="text-white">Odd Fire</SelectItem>
                <SelectItem value="rotary" className="text-white">Rotary</SelectItem>
              </SelectContent>
            </Select>
          </div>

          {/* Engine Size */}
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Engine Size (cc)</Label>
            <Input
              type="number"
              value={engineSize}
              onChange={(e) => {
                setEngineSize(e.target.value);
                onSettingsChange();
              }}
              className="bg-black border-[#555555] text-white font-mono"
              placeholder="2000"
            />
          </div>

          {/* Injector Size */}
          <div className="col-span-2">
            <Label className="text-[#CCCCCC] mb-2 block">Injector Size Each (cc/min)</Label>
            <div className="flex gap-2">
              <Input
                type="number"
                value={injectorSize}
                onChange={(e) => {
                  setInjectorSize(e.target.value);
                  onSettingsChange();
                }}
                className="bg-black border-[#555555] text-white font-mono"
                placeholder="180"
              />
              <span className="flex items-center text-[#888888] text-sm">cc/min</span>
            </div>
          </div>
        </div>

        {/* Required Fuel Calculator Button */}
        <div className="mt-6">
          <Button
            onClick={() => setShowCalculator(true)}
            className="w-full bg-gradient-to-r from-[#0088FF]/20 to-[#00CCFF]/20 border-2 border-[#0088FF] hover:from-[#0088FF]/30 hover:to-[#00CCFF]/30"
          >
            <Calculator className="w-5 h-5 mr-2" />
            Required Fuel Calculator
          </Button>
        </div>

        <Alert className="mt-4 bg-[#00CCFF]/10 border-[#00CCFF]">
          <Info className="w-4 h-4" />
          <AlertDescription className="text-xs">
            <strong>Tip:</strong> Use the Required Fuel Calculator to automatically determine optimal base fuel pulse width based on your engine specifications.
          </AlertDescription>
        </Alert>
      </Card>

      {/* Sequential Fuel Injection */}
      <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#00FF88]/40 p-6">
        <h3 className="text-white mb-6 flex items-center gap-2 text-xl">
          <span className="text-2xl">💉</span>
          Sequential Fuel Injection
        </h3>

        <div className="grid grid-cols-2 gap-6">
          {/* Main Fuel Outputs */}
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Main Fuel Outputs</Label>
            <Select value={sequentialOutput} onValueChange={handleChange(setSequentialOutput)}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue />
              </SelectTrigger>
              <SelectContent className="bg-[#1a1a1a] border-[#555555]">
                <SelectItem value="main-fuel" className="text-white">Main Fuel Outputs (FuelA-FuelD)</SelectItem>
                <SelectItem value="ign-outputs" className="text-white">Ignition Outputs (Ign1-Ign4)</SelectItem>
                <SelectItem value="aux-outputs" className="text-white">Auxiliary Outputs (Aux1-Aux4)</SelectItem>
              </SelectContent>
            </Select>
          </div>

          {/* Sequential Enable */}
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Sequential Injection</Label>
            <div className="flex items-center gap-3 h-10">
              <button
                onClick={() => {
                  setSequentialEnabled(!sequentialEnabled);
                  onSettingsChange();
                }}
                className={`relative inline-flex h-6 w-11 items-center rounded-full transition-colors ${
                  sequentialEnabled ? "bg-[#00FF88]" : "bg-[#555555]"
                }`}
              >
                <span
                  className={`inline-block h-4 w-4 transform rounded-full bg-white transition-transform ${
                    sequentialEnabled ? "translate-x-6" : "translate-x-1"
                  }`}
                />
              </button>
              <span className={sequentialEnabled ? "text-[#00FF88]" : "text-[#888888]"}>
                {sequentialEnabled ? "Enabled" : "Disabled"}
              </span>
            </div>
          </div>

          {/* Angle BTDC */}
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Angle BTDC</Label>
            <div className="flex gap-2">
              <Input
                type="number"
                value={angleBTDC}
                onChange={(e) => {
                  setAngleBTDC(e.target.value);
                  onSettingsChange();
                }}
                className="bg-black border-[#555555] text-white font-mono"
                placeholder="355"
              />
              <span className="flex items-center text-[#888888] text-sm">degrees</span>
            </div>
          </div>
        </div>

        {/* Firing Order Editor Button */}
        <div className="mt-6">
          <Button
            onClick={() => setShowFiringOrderEditor(true)}
            className="w-full bg-gradient-to-r from-[#00FF88]/20 to-[#00CC66]/20 border-2 border-[#00FF88] hover:from-[#00FF88]/30 hover:to-[#00CC66]/30"
          >
            <span className="mr-2">🔢</span>
            Configure Firing Order & Injector Trim
          </Button>
        </div>

        {!sequentialEnabled && (
          <Alert className="mt-4 bg-[#FFAA00]/10 border-[#FFAA00]">
            <AlertTriangle className="w-4 h-4" />
            <AlertDescription className="text-xs">
              <strong>Notice:</strong> Sequential injection is disabled. Engine will run in batch fire mode.
            </AlertDescription>
          </Alert>
        )}
      </Card>

      {/* Modals */}
      {showCalculator && (
        <RequiredFuelCalculator
          onClose={() => setShowCalculator(false)}
          onApply={(values) => {
            setEngineSize(values.displacement);
            setNumCylinders(values.cylinders);
            setInjectorSize(values.injectorFlow);
            onSettingsChange();
            setShowCalculator(false);
          }}
        />
      )}

      {showFiringOrderEditor && (
        <FiringOrderEditor
          numCylinders={parseInt(numCylinders)}
          onClose={() => setShowFiringOrderEditor(false)}
          onApply={() => {
            onSettingsChange();
            setShowFiringOrderEditor(false);
          }}
        />
      )}
    </div>
  );
}
