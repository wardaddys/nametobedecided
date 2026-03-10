import { useState } from "react";
import { Card } from "../../ui/card";
import { Label } from "../../ui/label";
import { Input } from "../../ui/input";
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "../../ui/select";
import { Checkbox } from "../../ui/checkbox";
import { Alert, AlertDescription } from "../../ui/alert";
import { AlertTriangle } from "lucide-react";

interface RevLimiterSettingsProps {
  onSettingsChange: () => void;
}

export function RevLimiterSettings({ onSettingsChange }: RevLimiterSettingsProps) {
  const [hardLimit, setHardLimit] = useState("7200");
  const [softLimit, setSoftLimit] = useState("6800");
  const [limiterType, setLimiterType] = useState("spark-cut");
  const [overrunEnabled, setOverrunEnabled] = useState(true);
  const [overrunRPM, setOverrunRPM] = useState("1500");
  const [overrunTPS, setOverrunTPS] = useState("2");
  const [overrunDelay, setOverrunDelay] = useState("1.0");

  return (
    <div className="space-y-6">
      <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#FF3366]/40 p-6">
        <h3 className="text-white mb-6 flex items-center gap-2 text-xl">
          <span className="text-2xl">🔄</span>
          Rev Limiter Configuration
        </h3>

        <div className="grid grid-cols-2 gap-6">
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Hard Rev Limit (RPM)</Label>
            <Input
              type="number"
              value={hardLimit}
              onChange={(e) => { setHardLimit(e.target.value); onSettingsChange(); }}
              className="bg-black border-[#555555] text-white font-mono"
            />
          </div>

          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Soft Rev Limit (RPM)</Label>
            <Input
              type="number"
              value={softLimit}
              onChange={(e) => { setSoftLimit(e.target.value); onSettingsChange(); }}
              className="bg-black border-[#555555] text-white font-mono"
            />
          </div>

          <div className="col-span-2">
            <Label className="text-[#CCCCCC] mb-2 block">Rev Limiter Type</Label>
            <Select value={limiterType} onValueChange={(v) => { setLimiterType(v); onSettingsChange(); }}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue />
              </SelectTrigger>
              <SelectContent>
                <SelectItem value="spark-cut">Spark Cut Only</SelectItem>
                <SelectItem value="fuel-cut">Fuel Cut Only</SelectItem>
                <SelectItem value="both">Spark & Fuel Cut</SelectItem>
              </SelectContent>
            </Select>
          </div>
        </div>

        <Alert className="mt-4 bg-[#FF3366]/10 border-[#FF3366]">
          <AlertTriangle className="w-4 h-4" />
          <AlertDescription className="text-xs">
            <strong>Warning:</strong> Set limits below engine's maximum safe RPM. Over-revving can cause catastrophic engine damage.
          </AlertDescription>
        </Alert>
      </Card>

      <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#FFAA00]/40 p-6">
        <h3 className="text-white mb-4 flex items-center gap-2">
          <Checkbox checked={overrunEnabled} onCheckedChange={(c) => { setOverrunEnabled(c as boolean); onSettingsChange(); }} />
          <span>Overrun Fuel Cut</span>
        </h3>

        {overrunEnabled && (
          <div className="grid grid-cols-3 gap-4 ml-8">
            <div>
              <Label className="text-[#CCCCCC] text-sm mb-1 block">RPM Threshold</Label>
              <Input type="number" value={overrunRPM} onChange={(e) => { setOverrunRPM(e.target.value); onSettingsChange(); }} className="bg-black border-[#555555] text-white" />
            </div>
            <div>
              <Label className="text-[#CCCCCC] text-sm mb-1 block">TPS Threshold (%)</Label>
              <Input type="number" value={overrunTPS} onChange={(e) => { setOverrunTPS(e.target.value); onSettingsChange(); }} className="bg-black border-[#555555] text-white" />
            </div>
            <div>
              <Label className="text-[#CCCCCC] text-sm mb-1 block">Delay (sec)</Label>
              <Input type="number" step="0.1" value={overrunDelay} onChange={(e) => { setOverrunDelay(e.target.value); onSettingsChange(); }} className="bg-black border-[#555555] text-white" />
            </div>
          </div>
        )}
      </Card>
    </div>
  );
}
