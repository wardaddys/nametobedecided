import { useState } from "react";
import { Slider } from "../ui/slider";
import { Switch } from "../ui/switch";
import { Label } from "../ui/label";

interface IdleControlProps {
  liveTuning: boolean;
}

export function IdleControl({ liveTuning }: IdleControlProps) {
  const [targetRPM, setTargetRPM] = useState(850);
  const [idleKp, setIdleKp] = useState(0.15);
  const [idleKi, setIdleKi] = useState(0.08);
  const [idleKd, setIdleKd] = useState(0.05);
  const [dashpotEnabled, setDashpotEnabled] = useState(true);
  const [dashpotRate, setDashpotRate] = useState(5);
  const [acIdleUp, setAcIdleUp] = useState(100);
  const [warmupAdder, setWarmupAdder] = useState(200);

  return (
    <div className="h-full overflow-auto bg-[#1a1a1a] p-6">
      <div className="max-w-6xl mx-auto space-y-6">
        {/* Header */}
        <div className="border-b border-[#333] pb-4">
          <h2 className="text-2xl text-white">Idle Control</h2>
          <p className="text-sm text-[#888] mt-1">
            Configure closed-loop idle speed control and idle-up conditions
          </p>
        </div>

        <div className="grid grid-cols-2 gap-6">
          {/* Left Column - Target & PID */}
          <div className="space-y-6">
            {/* Target RPM */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <h3 className="text-white mb-4 border-b border-[#333] pb-2">Target Idle Speed</h3>
              
              <div className="space-y-4">
                <div>
                  <div className="flex justify-between mb-2">
                    <Label className="text-[#aaa]">Target RPM</Label>
                    <span className="text-[#0af] font-mono">{targetRPM} rpm</span>
                  </div>
                  <Slider
                    value={[targetRPM]}
                    onValueChange={(v) => liveTuning && setTargetRPM(v[0])}
                    min={600}
                    max={1200}
                    step={10}
                    disabled={!liveTuning}
                    className="w-full"
                  />
                </div>
              </div>
            </div>

            {/* PID Control */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <h3 className="text-white mb-4 border-b border-[#333] pb-2">PID Parameters</h3>
              
              <div className="space-y-4">
                {/* Proportional */}
                <div>
                  <div className="flex justify-between mb-2">
                    <Label className="text-[#aaa]">P Gain (Kp)</Label>
                    <span className="text-[#0af] font-mono">{idleKp.toFixed(3)}</span>
                  </div>
                  <Slider
                    value={[idleKp * 100]}
                    onValueChange={(v) => liveTuning && setIdleKp(v[0] / 100)}
                    min={0}
                    max={50}
                    step={1}
                    disabled={!liveTuning}
                  />
                </div>

                {/* Integral */}
                <div>
                  <div className="flex justify-between mb-2">
                    <Label className="text-[#aaa]">I Gain (Ki)</Label>
                    <span className="text-[#0af] font-mono">{idleKi.toFixed(3)}</span>
                  </div>
                  <Slider
                    value={[idleKi * 100]}
                    onValueChange={(v) => liveTuning && setIdleKi(v[0] / 100)}
                    min={0}
                    max={30}
                    step={1}
                    disabled={!liveTuning}
                  />
                </div>

                {/* Derivative */}
                <div>
                  <div className="flex justify-between mb-2">
                    <Label className="text-[#aaa]">D Gain (Kd)</Label>
                    <span className="text-[#0af] font-mono">{idleKd.toFixed(3)}</span>
                  </div>
                  <Slider
                    value={[idleKd * 100]}
                    onValueChange={(v) => liveTuning && setIdleKd(v[0] / 100)}
                    min={0}
                    max={20}
                    step={1}
                    disabled={!liveTuning}
                  />
                </div>
              </div>

              <div className="mt-4 p-3 bg-[#1a1a1a] rounded border border-[#444]">
                <p className="text-xs text-[#888]">
                  <span className="text-[#0af]">TIP:</span> Start with P=0.15, I=0.08, D=0.05 and adjust based on idle stability
                </p>
              </div>
            </div>

            {/* Dashpot */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <div className="flex items-center justify-between mb-4 border-b border-[#333] pb-2">
                <h3 className="text-white">Dashpot (Throttle Close Delay)</h3>
                <Switch
                  checked={dashpotEnabled}
                  onCheckedChange={(checked) => liveTuning && setDashpotEnabled(checked)}
                  disabled={!liveTuning}
                />
              </div>
              
              {dashpotEnabled && (
                <div>
                  <div className="flex justify-between mb-2">
                    <Label className="text-[#aaa]">Decay Rate</Label>
                    <span className="text-[#0af] font-mono">{dashpotRate}%/s</span>
                  </div>
                  <Slider
                    value={[dashpotRate]}
                    onValueChange={(v) => liveTuning && setDashpotRate(v[0])}
                    min={1}
                    max={20}
                    step={1}
                    disabled={!liveTuning}
                  />
                  <p className="text-xs text-[#666] mt-2">
                    Controls how quickly idle valve closes when throttle is released
                  </p>
                </div>
              )}
            </div>
          </div>

          {/* Right Column - Idle Up Conditions */}
          <div className="space-y-6">
            {/* Warmup Idle Up */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <h3 className="text-white mb-4 border-b border-[#333] pb-2">Warmup Idle-Up</h3>
              
              <div className="space-y-4">
                <div>
                  <div className="flex justify-between mb-2">
                    <Label className="text-[#aaa]">Cold Start Adder</Label>
                    <span className="text-[#0af] font-mono">+{warmupAdder} rpm</span>
                  </div>
                  <Slider
                    value={[warmupAdder]}
                    onValueChange={(v) => liveTuning && setWarmupAdder(v[0])}
                    min={0}
                    max={500}
                    step={10}
                    disabled={!liveTuning}
                  />
                  <p className="text-xs text-[#666] mt-2">
                    Additional RPM when coolant temp is below 60°C
                  </p>
                </div>

                {/* Warmup Curve Preview */}
                <div className="mt-4 p-4 bg-[#1a1a1a] rounded border border-[#444]">
                  <div className="text-xs text-[#888] mb-2">Warmup Curve Preview</div>
                  <div className="space-y-1 text-xs font-mono">
                    <div className="flex justify-between">
                      <span className="text-[#666]">-10°C:</span>
                      <span className="text-[#0af]">{targetRPM + warmupAdder} rpm</span>
                    </div>
                    <div className="flex justify-between">
                      <span className="text-[#666]">20°C:</span>
                      <span className="text-[#0af]">{targetRPM + Math.round(warmupAdder * 0.6)} rpm</span>
                    </div>
                    <div className="flex justify-between">
                      <span className="text-[#666]">60°C:</span>
                      <span className="text-[#0af]">{targetRPM} rpm</span>
                    </div>
                  </div>
                </div>
              </div>
            </div>

            {/* AC Idle Up */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <h3 className="text-white mb-4 border-b border-[#333] pb-2">A/C Idle-Up</h3>
              
              <div>
                <div className="flex justify-between mb-2">
                  <Label className="text-[#aaa]">A/C Compressor Adder</Label>
                  <span className="text-[#0af] font-mono">+{acIdleUp} rpm</span>
                </div>
                <Slider
                  value={[acIdleUp]}
                  onValueChange={(v) => liveTuning && setAcIdleUp(v[0])}
                  min={0}
                  max={300}
                  step={10}
                  disabled={!liveTuning}
                />
                <p className="text-xs text-[#666] mt-2">
                  Additional RPM when A/C compressor clutch is engaged
                </p>
              </div>
            </div>

            {/* Output Limits */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <h3 className="text-white mb-4 border-b border-[#333] pb-2">Output Limits</h3>
              
              <div className="space-y-4">
                <div className="grid grid-cols-2 gap-4">
                  <div>
                    <Label className="text-[#aaa] text-xs">Min Duty</Label>
                    <div className="mt-1 px-3 py-2 bg-[#1a1a1a] border border-[#444] rounded text-[#0af] font-mono text-sm">
                      5%
                    </div>
                  </div>
                  <div>
                    <Label className="text-[#aaa] text-xs">Max Duty</Label>
                    <div className="mt-1 px-3 py-2 bg-[#1a1a1a] border border-[#444] rounded text-[#0af] font-mono text-sm">
                      95%
                    </div>
                  </div>
                </div>
                <p className="text-xs text-[#666]">
                  Limits for idle air control valve duty cycle
                </p>
              </div>
            </div>

            {/* Current Status */}
            <div className="bg-[#252525] border border-[#0af] rounded-lg p-5">
              <h3 className="text-[#0af] mb-4 border-b border-[#0af]/30 pb-2">Live Status</h3>
              
              <div className="grid grid-cols-2 gap-3 text-sm">
                <div>
                  <div className="text-[#888] text-xs">Current RPM</div>
                  <div className="text-white font-mono mt-1">847 rpm</div>
                </div>
                <div>
                  <div className="text-[#888] text-xs">Target RPM</div>
                  <div className="text-[#0af] font-mono mt-1">{targetRPM} rpm</div>
                </div>
                <div>
                  <div className="text-[#888] text-xs">IAC Duty</div>
                  <div className="text-white font-mono mt-1">42.5%</div>
                </div>
                <div>
                  <div className="text-[#888] text-xs">Error</div>
                  <div className="text-[#0f0] font-mono mt-1">-3 rpm</div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
