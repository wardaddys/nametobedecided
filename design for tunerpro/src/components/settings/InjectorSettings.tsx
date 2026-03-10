import { useState } from 'react';
import { Info, AlertCircle, Calculator } from 'lucide-react';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '../ui/select';
import { Input } from '../ui/input';
import { Badge } from '../ui/badge';
import { Separator } from '../ui/separator';
import { 
  InjectorSpec, 
  INJECTOR_DATABASE,
  getAllManufacturers,
  getInjectorsByManufacturer,
  calculateRequiredFlow,
  suggestInjectorsForHP,
  calculateFlowAtPressure,
} from '../database/InjectorDatabase';

interface InjectorSettingsProps {
  isPrimary: boolean;
  selectedInjector: InjectorSpec | null;
  onInjectorChange: (injector: InjectorSpec | null) => void;
  customFlowRate?: number;
  customPressure?: number;
  onCustomFlowChange?: (flow: number) => void;
  onCustomPressureChange?: (pressure: number) => void;
}

export function InjectorSettings({
  isPrimary,
  selectedInjector,
  onInjectorChange,
  customFlowRate,
  customPressure = 3.0,
  onCustomFlowChange,
  onCustomPressureChange,
}: InjectorSettingsProps) {
  const [selectedManufacturer, setSelectedManufacturer] = useState<string>('');
  const [showCalculator, setShowCalculator] = useState(false);
  const [targetHP, setTargetHP] = useState(400);
  const [numInjectors, setNumInjectors] = useState(4);
  const [isBoosted, setIsBoosted] = useState(true);

  const manufacturers = getAllManufacturers();
  const availableInjectors = selectedManufacturer 
    ? getInjectorsByManufacturer(selectedManufacturer)
    : INJECTOR_DATABASE;

  const handleManufacturerChange = (manufacturer: string) => {
    // Use 'all' as a special value instead of empty string
    const mfr = manufacturer === 'all' ? '' : manufacturer;
    setSelectedManufacturer(mfr);
    onInjectorChange(null);
  };

  const handleInjectorChange = (injectorId: string) => {
    if (injectorId === 'none') {
      onInjectorChange(null);
      return;
    }
    const injector = INJECTOR_DATABASE.find(inj => inj.id === injectorId);
    onInjectorChange(injector || null);
  };

  const calculateSuggestions = () => {
    return suggestInjectorsForHP(targetHP, numInjectors, isBoosted);
  };

  // Ensure we have valid numbers, not NaN
  const safeCustomPressure = isNaN(customPressure) ? 3.0 : customPressure;
  const currentFlow = customFlowRate || selectedInjector?.flowRateCCMin || 0;
  const flowAtPressure = selectedInjector 
    ? calculateFlowAtPressure(selectedInjector.flowRateCCMin, selectedInjector.pressure, safeCustomPressure)
    : currentFlow;

  const maxHP = selectedInjector
    ? (selectedInjector.flowRateCCMin * numInjectors * 0.85) / (isBoosted ? 0.6 : 0.5) / 0.5 / 10.5
    : 0;

  return (
    <div className="space-y-4">
      {/* Header */}
      <div className="flex items-center justify-between">
        <h3 className="text-white flex items-center gap-2">
          {isPrimary ? '🔵 Primary' : '🟣 Secondary'} Injectors
          <button className="text-[#00CCFF] hover:text-white">
            <Info className="w-4 h-4" />
          </button>
        </h3>
        <button
          onClick={() => setShowCalculator(!showCalculator)}
          className="bg-[#3a3a3a] hover:bg-[#4a4a4a] text-white px-3 py-1 rounded text-xs flex items-center gap-1"
        >
          <Calculator className="w-3 h-3" />
          {showCalculator ? 'Hide' : 'Show'} Calculator
        </button>
      </div>

      {/* Calculator Panel */}
      {showCalculator && (
        <div className="bg-[#2a2a2a] border-2 border-[#0066CC] rounded p-4 space-y-3">
          <h4 className="text-[#00CCFF] text-sm">Injector Size Calculator</h4>
          <div className="grid grid-cols-3 gap-3">
            <div>
              <label className="text-xs text-[#888888]">Target HP</label>
              <Input
                type="number"
                value={targetHP}
                onChange={(e) => {
                  const val = parseInt(e.target.value);
                  setTargetHP(isNaN(val) ? 0 : val);
                }}
                className="bg-[#1a1a1a] border-[#444444] text-white"
              />
            </div>
            <div>
              <label className="text-xs text-[#888888]">Num Injectors</label>
              <Input
                type="number"
                value={numInjectors}
                onChange={(e) => {
                  const val = parseInt(e.target.value);
                  setNumInjectors(isNaN(val) ? 1 : val);
                }}
                className="bg-[#1a1a1a] border-[#444444] text-white"
              />
            </div>
            <div>
              <label className="text-xs text-[#888888]">Engine Type</label>
              <Select value={isBoosted ? 'boosted' : 'na'} onValueChange={(v) => setIsBoosted(v === 'boosted')}>
                <SelectTrigger className="bg-[#1a1a1a] border-[#444444]">
                  <SelectValue />
                </SelectTrigger>
                <SelectContent>
                  <SelectItem value="na">NA (BSFC 0.5)</SelectItem>
                  <SelectItem value="boosted">Boosted (BSFC 0.6)</SelectItem>
                </SelectContent>
              </Select>
            </div>
          </div>

          <div className="bg-[#1a1a1a] p-3 rounded">
            <div className="text-xs text-[#888888] mb-2">Required Flow Rate:</div>
            <div className="text-xl text-[#00CCFF]">
              {calculateRequiredFlow(targetHP, isBoosted ? 0.6 : 0.5, numInjectors)} cc/min
            </div>
            <div className="text-xs text-[#00FF00] mt-1">
              ({(calculateRequiredFlow(targetHP, isBoosted ? 0.6 : 0.5, numInjectors) / 10.5).toFixed(1)} lb/hr)
            </div>
          </div>

          <div className="space-y-2">
            <div className="text-xs text-[#CCCCCC]">Suggested Injectors:</div>
            {calculateSuggestions().slice(0, 3).map(inj => (
              <button
                key={inj.id}
                onClick={() => handleInjectorChange(inj.id)}
                className="w-full bg-[#1a1a1a] hover:bg-[#2a2a2a] border border-[#444444] rounded p-2 text-left text-xs"
              >
                <div className="flex items-center justify-between">
                  <span className="text-white">{inj.brand} {inj.model}</span>
                  <Badge className="bg-[#0066CC]">{inj.category}</Badge>
                </div>
                <div className="text-[#888888] mt-1">
                  {inj.flowRateCCMin} cc/min ({inj.flowRateLbHr} lb/hr) @ {inj.pressure} bar
                </div>
              </button>
            ))}
          </div>
        </div>
      )}

      {/* Manufacturer Selection */}
      <div>
        <label className="text-xs text-[#888888] mb-2 block">Manufacturer</label>
        <Select value={selectedManufacturer || 'all'} onValueChange={handleManufacturerChange}>
          <SelectTrigger className="bg-[#1a1a1a] border-[#444444]">
            <SelectValue placeholder="All Manufacturers" />
          </SelectTrigger>
          <SelectContent>
            <SelectItem value="all">All Manufacturers</SelectItem>
            {manufacturers.map(mfr => (
              <SelectItem key={mfr} value={mfr}>{mfr}</SelectItem>
            ))}
          </SelectContent>
        </Select>
      </div>

      {/* Injector Selection */}
      <div>
        <label className="text-xs text-[#888888] mb-2 block">Injector Model</label>
        <Select 
          value={selectedInjector?.id || 'none'} 
          onValueChange={handleInjectorChange}
        >
          <SelectTrigger className="bg-[#1a1a1a] border-[#444444]">
            <SelectValue placeholder="Select injector..." />
          </SelectTrigger>
          <SelectContent className="max-h-[300px]">
            <SelectItem value="none">-- Select Injector --</SelectItem>
            {availableInjectors.map(inj => (
              <SelectItem key={inj.id} value={inj.id}>
                {inj.brand} {inj.model} - {inj.flowRateCCMin}cc ({inj.flowRateLbHr}lb/hr)
              </SelectItem>
            ))}
          </SelectContent>
        </Select>
      </div>

      {/* Selected Injector Details */}
      {selectedInjector && (
        <div className="bg-[#2a2a2a] border border-[#444444] rounded p-4 space-y-3">
          <div className="flex items-start justify-between">
            <div>
              <h4 className="text-white">{selectedInjector.brand} {selectedInjector.model}</h4>
              <p className="text-xs text-[#888888] mt-1">{selectedInjector.notes}</p>
            </div>
            <Badge className={
              selectedInjector.category === 'street' ? 'bg-[#00FF00] text-black' :
              selectedInjector.category === 'performance' ? 'bg-[#FFD700] text-black' :
              selectedInjector.category === 'race' ? 'bg-[#FF8800] text-black' :
              'bg-[#FF0000] text-white'
            }>
              {selectedInjector.category.toUpperCase()}
            </Badge>
          </div>

          <Separator className="bg-[#444444]" />

          <div className="grid grid-cols-2 gap-4 text-xs">
            <div>
              <div className="text-[#888888] mb-1">Flow Rate</div>
              <div className="text-white">
                {selectedInjector.flowRateCCMin} cc/min
                <br />
                {selectedInjector.flowRateLbHr} lb/hr
              </div>
            </div>
            <div>
              <div className="text-[#888888] mb-1">Impedance</div>
              <div className="text-white">{selectedInjector.impedance} Ω</div>
            </div>
            <div>
              <div className="text-[#888888] mb-1">Dead Time @ 14V</div>
              <div className="text-white">{selectedInjector.deadTimeOffset14V} ms</div>
            </div>
            <div>
              <div className="text-[#888888] mb-1">Base Pressure</div>
              <div className="text-white">{selectedInjector.pressure} bar (43.5 PSI)</div>
            </div>
          </div>

          <Separator className="bg-[#444444]" />

          {/* Pressure Adjustment */}
          <div>
            <label className="text-xs text-[#888888] mb-2 block">
              Actual Fuel Pressure (bar)
            </label>
            <Input
              type="number"
              value={safeCustomPressure}
              onChange={(e) => {
                const val = parseFloat(e.target.value);
                onCustomPressureChange?.(isNaN(val) ? 3.0 : val);
              }}
              step="0.1"
              min="2"
              max="5"
              className="bg-[#1a1a1a] border-[#444444] text-white"
            />
            {Math.abs(safeCustomPressure - selectedInjector.pressure) > 0.1 && (
              <div className="mt-2 bg-[#1a1a1a] p-2 rounded text-xs">
                <div className="text-[#FFD700] mb-1">Adjusted Flow Rate:</div>
                <div className="text-white">
                  {flowAtPressure.toFixed(0)} cc/min ({(flowAtPressure / 10.5).toFixed(1)} lb/hr)
                </div>
                <div className="text-[#888888] mt-1">
                  Flow ∝ √Pressure: {((flowAtPressure / selectedInjector.flowRateCCMin - 1) * 100).toFixed(1)}% 
                  {flowAtPressure > selectedInjector.flowRateCCMin ? ' increase' : ' decrease'}
                </div>
              </div>
            )}
          </div>

          <Separator className="bg-[#444444]" />

          {/* Performance Metrics */}
          <div className="bg-[#1a1a1a] p-3 rounded">
            <div className="text-xs text-[#888888] mb-2">Estimated Max HP @ 85% Duty:</div>
            <div className="text-xl text-[#00CCFF]">{maxHP.toFixed(0)} HP</div>
            <div className="text-xs text-[#888888] mt-2">
              {numInjectors} injectors × {flowAtPressure.toFixed(0)} cc/min × 85% duty
            </div>
          </div>

          {/* Warnings */}
          {selectedInjector.impedance < 12 && (
            <div className="bg-[#440000] border border-[#FF3B30] rounded p-2 flex items-start gap-2">
              <AlertCircle className="w-4 h-4 text-[#FF3B30] flex-shrink-0 mt-0.5" />
              <div className="text-xs text-[#FF3B30]">
                Low impedance injector ({selectedInjector.impedance}Ω). Verify ECU supports peak & hold or low-Z injectors.
                May require resistor box.
              </div>
            </div>
          )}
        </div>
      )}
    </div>
  );
}
