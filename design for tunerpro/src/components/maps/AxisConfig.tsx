import { useState } from 'react';
import { Button } from '../ui/button';
import { Input } from '../ui/input';
import { Label } from '../ui/label';
import { X, Settings2 } from 'lucide-react';

interface AxisConfigProps {
  rpmAxis: number[];
  mapAxis: number[];
  onRpmAxisChange: (axis: number[]) => void;
  onMapAxisChange: (axis: number[]) => void;
  onClose: () => void;
}

export function AxisConfig({ rpmAxis, mapAxis, onRpmAxisChange, onMapAxisChange, onClose }: AxisConfigProps) {
  const [rpmMin, setRpmMin] = useState(rpmAxis[0]);
  const [rpmMax, setRpmMax] = useState(rpmAxis[rpmAxis.length - 1]);
  const [rpmStep, setRpmStep] = useState(rpmAxis[1] - rpmAxis[0]);

  const [mapMin, setMapMin] = useState(mapAxis[0]);
  const [mapMax, setMapMax] = useState(mapAxis[mapAxis.length - 1]);
  const [mapStep, setMapStep] = useState(mapAxis[1] - mapAxis[0]);

  const handleApply = () => {
    // Generate new RPM axis
    const newRpmAxis: number[] = [];
    for (let i = rpmMin; i <= rpmMax; i += rpmStep) {
      newRpmAxis.push(i);
    }
    if (newRpmAxis[newRpmAxis.length - 1] !== rpmMax) {
      newRpmAxis.push(rpmMax);
    }
    onRpmAxisChange(newRpmAxis.slice(0, 10)); // Limit to 10 columns

    // Generate new MAP axis
    const newMapAxis: number[] = [];
    for (let i = mapMin; i <= mapMax; i += mapStep) {
      newMapAxis.push(i);
    }
    if (newMapAxis[newMapAxis.length - 1] !== mapMax) {
      newMapAxis.push(mapMax);
    }
    onMapAxisChange(newMapAxis.slice(0, 16)); // Limit to 16 rows
  };

  return (
    <div className="absolute top-16 left-4 bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF] rounded-lg p-4 shadow-2xl z-50 w-80">
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center gap-2">
          <Settings2 className="w-5 h-5 text-[#00CCFF]" />
          <h3 className="text-white font-semibold">Axis Configuration</h3>
        </div>
        <Button variant="ghost" size="sm" onClick={onClose} className="h-6 w-6 p-0">
          <X className="w-4 h-4" />
        </Button>
      </div>

      <div className="space-y-4">
        {/* RPM Axis */}
        <div className="bg-black/40 rounded-lg p-3 border border-[#333333]">
          <h4 className="text-[#00FF00] text-sm font-semibold mb-3">RPM Axis</h4>
          <div className="grid grid-cols-3 gap-2">
            <div>
              <Label className="text-xs text-[#888888]">Min</Label>
              <Input
                type="number"
                value={rpmMin}
                onChange={(e) => setRpmMin(Number(e.target.value))}
                className="h-8 bg-black border-[#555555] text-white text-xs"
              />
            </div>
            <div>
              <Label className="text-xs text-[#888888]">Max</Label>
              <Input
                type="number"
                value={rpmMax}
                onChange={(e) => setRpmMax(Number(e.target.value))}
                className="h-8 bg-black border-[#555555] text-white text-xs"
              />
            </div>
            <div>
              <Label className="text-xs text-[#888888]">Step</Label>
              <Input
                type="number"
                value={rpmStep}
                onChange={(e) => setRpmStep(Number(e.target.value))}
                className="h-8 bg-black border-[#555555] text-white text-xs"
              />
            </div>
          </div>
        </div>

        {/* MAP Axis */}
        <div className="bg-black/40 rounded-lg p-3 border border-[#333333]">
          <h4 className="text-[#00CCFF] text-sm font-semibold mb-3">MAP Axis (mbar)</h4>
          <div className="grid grid-cols-3 gap-2">
            <div>
              <Label className="text-xs text-[#888888]">Min</Label>
              <Input
                type="number"
                value={mapMin}
                onChange={(e) => setMapMin(Number(e.target.value))}
                className="h-8 bg-black border-[#555555] text-white text-xs"
              />
            </div>
            <div>
              <Label className="text-xs text-[#888888]">Max</Label>
              <Input
                type="number"
                value={mapMax}
                onChange={(e) => setMapMax(Number(e.target.value))}
                className="h-8 bg-black border-[#555555] text-white text-xs"
              />
            </div>
            <div>
              <Label className="text-xs text-[#888888]">Step</Label>
              <Input
                type="number"
                value={mapStep}
                onChange={(e) => setMapStep(Number(e.target.value))}
                className="h-8 bg-black border-[#555555] text-white text-xs"
              />
            </div>
          </div>
        </div>

        <Button onClick={handleApply} className="w-full bg-[#0088FF] hover:bg-[#0066CC]">
          Apply Changes
        </Button>
      </div>
    </div>
  );
}
