import { useState } from "react";
import { Slider } from "../ui/slider";
import { Label } from "../ui/label";
import { Switch } from "../ui/switch";

interface KnockControlProps {
  liveTuning: boolean;
}

export function KnockControl({ liveTuning }: KnockControlProps) {
  const [knockEnabled, setKnockEnabled] = useState(true);
  const [threshold, setThreshold] = useState(45);
  const [maxRetard, setMaxRetard] = useState(8);
  const [retardStep, setRetardStep] = useState(2);
  const [recoveryRate, setRecoveryRate] = useState(0.5);
  const [windowStart, setWindowStart] = useState(15);
  const [windowEnd, setWindowEnd] = useState(45);

  // Mock knock sensor data
  const knockLevels = [23, 42, 18, 31];
  const knockRetards = [0, 0, 0, 0];

  return (
    <div className="h-full overflow-auto bg-[#1a1a1a] p-6">
      <div className="max-w-6xl mx-auto space-y-6">
        {/* Header */}
        <div className="border-b border-[#333] pb-4">
          <h2 className="text-2xl text-white">Knock Control & Detection</h2>
          <p className="text-sm text-[#888] mt-1">
            Configure knock sensor detection and automatic timing retard protection
          </p>
        </div>

        <div className="grid grid-cols-2 gap-6">
          {/* Left Column - Detection Settings */}
          <div className="space-y-6">
            {/* Knock Detection */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <div className="flex items-center justify-between mb-4 border-b border-[#333] pb-2">
                <h3 className="text-white">Knock Detection</h3>
                <Switch
                  checked={knockEnabled}
                  onCheckedChange={(checked) => liveTuning && setKnockEnabled(checked)}
                  disabled={!liveTuning}
                />
              </div>
              
              {knockEnabled && (
                <div className="space-y-4">
                  <div>
                    <div className="flex justify-between mb-2">
                      <Label className="text-[#aaa]">Detection Threshold</Label>
                      <span className="text-[#0af] font-mono">{threshold} counts</span>
                    </div>
                    <Slider
                      value={[threshold]}
                      onValueChange={(v) => liveTuning && setThreshold(v[0])}
                      min={20}
                      max={100}
                      step={1}
                      disabled={!liveTuning}
                    />
                    <p className="text-xs text-[#666] mt-2">
                      Sensor value above which knock is detected
                    </p>
                  </div>

                  <div className="grid grid-cols-2 gap-4">
                    <div>
                      <Label className="text-[#aaa] text-xs">Window Start</Label>
                      <div className="flex items-center gap-2 mt-1">
                        <Slider
                          value={[windowStart]}
                          onValueChange={(v) => liveTuning && setWindowStart(v[0])}
                          min={0}
                          max={30}
                          step={1}
                          disabled={!liveTuning}
                          className="flex-1"
                        />
                        <span className="text-[#0af] font-mono text-sm w-12">{windowStart}°</span>
                      </div>
                    </div>
                    <div>
                      <Label className="text-[#aaa] text-xs">Window End</Label>
                      <div className="flex items-center gap-2 mt-1">
                        <Slider
                          value={[windowEnd]}
                          onValueChange={(v) => liveTuning && setWindowEnd(v[0])}
                          min={30}
                          max={90}
                          step={1}
                          disabled={!liveTuning}
                          className="flex-1"
                        />
                        <span className="text-[#0af] font-mono text-sm w-12">{windowEnd}°</span>
                      </div>
                    </div>
                  </div>
                  <p className="text-xs text-[#666]">
                    Crank angle window (ATDC) for knock detection sampling
                  </p>

                  {/* Detection Window Diagram */}
                  <div className="mt-4 p-4 bg-[#1a1a1a] rounded border border-[#444]">
                    <div className="text-xs text-[#888] mb-3">Detection Window</div>
                    <div className="h-24 border border-[#333] rounded bg-[#0a0a0a] p-2 relative">
                      <svg className="w-full h-full">
                        {/* TDC line */}
                        <line x1="20" y1="0" x2="20" y2="100" stroke="#666" strokeWidth="1" strokeDasharray="2,2" />
                        {/* Window region */}
                        <rect x="30" y="10" width="50" height="80" fill="#0af" fillOpacity="0.2" stroke="#0af" />
                        {/* Labels */}
                        <text x="20" y="8" fill="#666" fontSize="8">TDC</text>
                        <text x="32" y="105" fill="#0af" fontSize="8">{windowStart}°</text>
                        <text x="72" y="105" fill="#0af" fontSize="8">{windowEnd}°</text>
                      </svg>
                    </div>
                  </div>
                </div>
              )}
            </div>

            {/* Retard Response */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <h3 className="text-white mb-4 border-b border-[#333] pb-2">Timing Retard Response</h3>
              
              <div className="space-y-4">
                <div>
                  <div className="flex justify-between mb-2">
                    <Label className="text-[#aaa]">Maximum Retard</Label>
                    <span className="text-[#f80] font-mono">{maxRetard}°</span>
                  </div>
                  <Slider
                    value={[maxRetard]}
                    onValueChange={(v) => liveTuning && setMaxRetard(v[0])}
                    min={0}
                    max={20}
                    step={0.5}
                    disabled={!liveTuning}
                  />
                  <p className="text-xs text-[#666] mt-2">
                    Maximum timing retard allowed per cylinder
                  </p>
                </div>

                <div>
                  <div className="flex justify-between mb-2">
                    <Label className="text-[#aaa]">Retard Step Size</Label>
                    <span className="text-[#f80] font-mono">{retardStep}°</span>
                  </div>
                  <Slider
                    value={[retardStep * 10]}
                    onValueChange={(v) => liveTuning && setRetardStep(v[0] / 10)}
                    min={5}
                    max={50}
                    step={5}
                    disabled={!liveTuning}
                  />
                  <p className="text-xs text-[#666] mt-2">
                    Amount to retard timing per knock event
                  </p>
                </div>

                <div>
                  <div className="flex justify-between mb-2">
                    <Label className="text-[#aaa]">Recovery Rate</Label>
                    <span className="text-[#0af] font-mono">{recoveryRate}°/s</span>
                  </div>
                  <Slider
                    value={[recoveryRate * 10]}
                    onValueChange={(v) => liveTuning && setRecoveryRate(v[0] / 10)}
                    min={1}
                    max={30}
                    step={1}
                    disabled={!liveTuning}
                  />
                  <p className="text-xs text-[#666] mt-2">
                    Rate timing advances back after knock clears
                  </p>
                </div>

                {/* Response Diagram */}
                <div className="mt-4 p-4 bg-[#1a1a1a] rounded border border-[#444]">
                  <div className="text-xs text-[#888] mb-3">Retard Response Profile</div>
                  <div className="h-24 border border-[#333] rounded bg-[#0a0a0a] p-3 relative">
                    <svg className="w-full h-full" viewBox="0 0 100 100" preserveAspectRatio="none">
                      {/* Knock events */}
                      <rect x="20" y="0" width="2" height="10" fill="#f44" />
                      <rect x="40" y="0" width="2" height="10" fill="#f44" />
                      {/* Timing response */}
                      <polyline
                        points="0,80 20,80 22,50 40,55 42,30 70,60 100,75"
                        fill="none"
                        stroke="#0af"
                        strokeWidth="2"
                      />
                      <line x1="0" y1="80" x2="100" y2="80" stroke="#666" strokeDasharray="2,2" strokeWidth="1" />
                    </svg>
                    <div className="absolute top-0 left-2 text-[9px] text-[#f44]">Knock</div>
                    <div className="absolute bottom-0 left-2 text-[9px] text-[#666]">Base Timing</div>
                  </div>
                </div>
              </div>
            </div>
          </div>

          {/* Right Column - Live Monitoring */}
          <div className="space-y-6">
            {/* Knock Sensor Status */}
            <div className="bg-[#252525] border border-[#0af] rounded-lg p-5">
              <h3 className="text-[#0af] mb-4 border-b border-[#0af]/30 pb-2">Live Knock Sensor Levels</h3>
              
              <div className="space-y-4">
                {[1, 2, 3, 4].map((sensor, index) => {
                  const level = knockLevels[index];
                  const isKnocking = level > threshold;
                  
                  return (
                    <div key={sensor}>
                      <div className="flex justify-between mb-2">
                        <Label className="text-[#aaa]">Sensor {sensor}</Label>
                        <span className={`font-mono ${isKnocking ? 'text-[#f44]' : 'text-[#0af]'}`}>
                          {level} counts {isKnocking && '⚠ KNOCK'}
                        </span>
                      </div>
                      <div className="h-6 border border-[#333] rounded bg-[#0a0a0a] relative overflow-hidden">
                        {/* Level bar */}
                        <div
                          className={`absolute left-0 top-0 bottom-0 transition-all ${
                            isKnocking
                              ? 'bg-gradient-to-r from-[#f44] to-[#f88]'
                              : 'bg-gradient-to-r from-[#0af] to-[#0cf]'
                          }`}
                          style={{ width: `${(level / 100) * 100}%` }}
                        />
                        {/* Threshold line */}
                        <div
                          className="absolute top-0 bottom-0 border-r-2 border-[#f80] border-dashed"
                          style={{ left: `${(threshold / 100) * 100}%` }}
                        />
                        {/* Value text */}
                        <div className="absolute inset-0 flex items-center justify-center text-xs font-mono text-white">
                          {level}
                        </div>
                      </div>
                    </div>
                  );
                })}

                <div className="mt-4 p-3 bg-[#1a1a1a] rounded border border-[#444]">
                  <div className="flex items-center gap-2 text-xs">
                    <div className="w-3 h-3 border-2 border-[#f80] border-dashed" />
                    <span className="text-[#888]">Threshold: {threshold} counts</span>
                  </div>
                </div>
              </div>
            </div>

            {/* Active Retard Status */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <h3 className="text-white mb-4 border-b border-[#333] pb-2">Active Timing Retard</h3>
              
              <div className="grid grid-cols-2 gap-4">
                {[1, 2, 3, 4].map((cyl, index) => (
                  <div key={cyl} className="text-center">
                    <div className="text-sm text-[#888] mb-2">Cylinder {cyl}</div>
                    <div className="h-24 border border-[#333] rounded bg-[#0a0a0a] relative overflow-hidden">
                      {knockRetards[index] > 0 && (
                        <div
                          className="absolute bottom-0 left-0 right-0 bg-gradient-to-t from-[#f80] to-[#f80]/50"
                          style={{ height: `${(knockRetards[index] / maxRetard) * 100}%` }}
                        />
                      )}
                      <div className="absolute inset-0 flex items-center justify-center">
                        <span className={`text-xl font-mono ${knockRetards[index] > 0 ? 'text-[#f80]' : 'text-[#0f0]'}`}>
                          {knockRetards[index] > 0 ? `-${knockRetards[index]}°` : '0°'}
                        </span>
                      </div>
                    </div>
                  </div>
                ))}
              </div>

              <div className="mt-4 text-center">
                <div className="text-xs text-[#888]">Total Knock Events (Session)</div>
                <div className="text-2xl text-white font-mono mt-1">0</div>
              </div>
            </div>

            {/* Knock History */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <h3 className="text-white mb-4 border-b border-[#333] pb-2">Recent Knock Events</h3>
              
              <div className="text-center text-[#666] py-8">
                <div className="text-4xl mb-2">✓</div>
                <div className="text-sm">No knock events detected</div>
              </div>
            </div>

            {/* Safety Warning */}
            <div className="bg-[#ff4444]/10 border border-[#ff4444] rounded-lg p-5">
              <h3 className="text-[#ff4444] mb-3 border-b border-[#ff4444]/30 pb-2">⚠ Safety Notice</h3>
              <div className="space-y-2 text-xs text-[#ff8888]">
                <div className="flex gap-2">
                  <span>•</span>
                  <span>Persistent knock can cause engine damage</span>
                </div>
                <div className="flex gap-2">
                  <span>•</span>
                  <span>If knock is detected, reduce boost or timing</span>
                </div>
                <div className="flex gap-2">
                  <span>•</span>
                  <span>Always use premium fuel when tuning for high output</span>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
