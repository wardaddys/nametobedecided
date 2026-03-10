import { useState } from "react";
import { Card } from "../../ui/card";
import { Label } from "../../ui/label";
import { Input } from "../../ui/input";
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "../../ui/select";
import { Checkbox } from "../../ui/checkbox";

interface ShiftLightSettingsProps {
  onSettingsChange: () => void;
}

export function ShiftLightSettings({ onSettingsChange }: ShiftLightSettingsProps) {
  const [enabled, setEnabled] = useState(true);
  const [activationRPM, setActivationRPM] = useState("6500");
  const [outputPin, setOutputPin] = useState("aux1");
  const [flashPattern, setFlashPattern] = useState("solid");

  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#FFAA00]/40 p-6">
      <h3 className="text-white mb-6 flex items-center gap-2 text-xl">
        <span className="text-2xl">💡</span>
        Shift Light Configuration
      </h3>

      <div className="space-y-4">
        <div className="flex items-center gap-3">
          <Checkbox checked={enabled} onCheckedChange={(c) => { setEnabled(c as boolean); onSettingsChange(); }} />
          <Label className="text-white">Enable Shift Light</Label>
        </div>

        {enabled && (
          <div className="grid grid-cols-2 gap-4">
            <div>
              <Label className="text-[#CCCCCC] mb-2 block">Activation RPM</Label>
              <Input type="number" value={activationRPM} onChange={(e) => { setActivationRPM(e.target.value); onSettingsChange(); }} className="bg-black border-[#555555] text-white font-mono" />
            </div>
            <div>
              <Label className="text-[#CCCCCC] mb-2 block">Output Pin</Label>
              <Select value={outputPin} onValueChange={(v) => { setOutputPin(v); onSettingsChange(); }}>
                <SelectTrigger className="bg-black border-[#555555] text-white">
                  <SelectValue />
                </SelectTrigger>
                <SelectContent>
                  <SelectItem value="aux1">AUX1</SelectItem>
                  <SelectItem value="aux2">AUX2</SelectItem>
                  <SelectItem value="spare1">SPARE1</SelectItem>
                </SelectContent>
              </Select>
            </div>
            <div className="col-span-2">
              <Label className="text-[#CCCCCC] mb-2 block">Flash Pattern</Label>
              <Select value={flashPattern} onValueChange={(v) => { setFlashPattern(v); onSettingsChange(); }}>
                <SelectTrigger className="bg-black border-[#555555] text-white">
                  <SelectValue />
                </SelectTrigger>
                <SelectContent>
                  <SelectItem value="solid">Solid On</SelectItem>
                  <SelectItem value="slow-flash">Slow Flash (1 Hz)</SelectItem>
                  <SelectItem value="fast-flash">Fast Flash (5 Hz)</SelectItem>
                  <SelectItem value="progressive">Progressive (faster as RPM increases)</SelectItem>
                </SelectContent>
              </Select>
            </div>
          </div>
        )}
      </div>
    </Card>
  );
}
