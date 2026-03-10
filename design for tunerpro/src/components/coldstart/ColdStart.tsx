import { useState } from "react";
import { Slider } from "../ui/slider";
import { Label } from "../ui/label";

interface ColdStartProps {
  liveTuning: boolean;
}

export function ColdStart({ liveTuning }: ColdStartProps) {
  const [crankingEnrich, setCrankingEnrich] = useState(150);
  const [afterstartEnrich, setAfterstartEnrich] = useState(120);
  const [afterstartDecay, setAfterstartDecay] = useState(3);
  const [warmupEnrich, setWarmupEnrich] = useState([100, 80, 60, 40, 20, 10, 0]);

  return (
    <div className="h-full overflow-auto bg-[#1a1a1a] p-6">
      <div className="max-w-6xl mx-auto space-y-6">
        {/* Header */}
        <div className="border-b border-[#333] pb-4">
          <h2 className="text-2xl text-white">Cold Start & Warmup</h2>
          <p className="text-sm text-[#888] mt-1">
            Configure fuel enrichment for cold starting and warmup conditions
          </p>
        </div>

        <div className="grid grid-cols-2 gap-6">
          {/* Left Column - Cranking & Afterstart */}
          <div className="space-y-6">
            {/* Cranking Enrichment */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <h3 className="text-white mb-4 border-b border-[#333] pb-2">Cranking Enrichment</h3>
              
              <div className="space-y-4">
                <div>
                  <div className="flex justify-between mb-2">
                    <Label className="text-[#aaa]">Cranking Fuel Multiplier</Label>
                    <span className="text-[#0af] font-mono">{crankingEnrich}%</span>
                  </div>
                  <Slider
                    value={[crankingEnrich]}
                    onValueChange={(v) => liveTuning && setCrankingEnrich(v[0])}
                    min={50}
                    max={300}
                    step={5}
                    disabled={!liveTuning}
                  />
                  <p className="text-xs text-[#666] mt-2">
                    Fuel multiplier during engine cranking (RPM &lt; 400)
                  </p>
                </div>

                {/* Temperature Compensation Table */}
                <div className="mt-4 p-4 bg-[#1a1a1a] rounded border border-[#444]">
                  <div className="text-xs text-[#888] mb-3">Cranking by Temperature</div>
                  <div className="space-y-1 text-xs font-mono">
                    <div className="flex justify-between border-b border-[#333] pb-1">
                      <span className="text-[#666]">Temp</span>
                      <span className="text-[#666]">Multiplier</span>
                    </div>
                    <div className="flex justify-between">
                      <span className="text-[#888]">-20°C</span>
                      <span className="text-[#0af]">{Math.round(crankingEnrich * 1.3)}%</span>
                    </div>
                    <div className="flex justify-between">
                      <span className="text-[#888]">0°C</span>
                      <span className="text-[#0af]">{Math.round(crankingEnrich * 1.15)}%</span>
                    </div>
                    <div className="flex justify-between">
                      <span className="text-[#888]">20°C</span>
                      <span className="text-[#0af]">{crankingEnrich}%</span>
                    </div>
                    <div className="flex justify-between">
                      <span className="text-[#888]">60°C</span>
                      <span className="text-[#0af]">{Math.round(crankingEnrich * 0.85)}%</span>
                    </div>
                    <div className="flex justify-between">
                      <span className="text-[#888]">80°C+</span>
                      <span className="text-[#0af]">{Math.round(crankingEnrich * 0.7)}%</span>
                    </div>
                  </div>
                </div>
              </div>
            </div>

            {/* Afterstart Enrichment */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <h3 className="text-white mb-4 border-b border-[#333] pb-2">Afterstart Enrichment</h3>
              
              <div className="space-y-4">
                <div>
                  <div className="flex justify-between mb-2">
                    <Label className="text-[#aaa]">Initial Enrichment</Label>
                    <span className="text-[#0af] font-mono">{afterstartEnrich}%</span>
                  </div>
                  <Slider
                    value={[afterstartEnrich]}
                    onValueChange={(v) => liveTuning && setAfterstartEnrich(v[0])}
                    min={100}
                    max={200}
                    step={5}
                    disabled={!liveTuning}
                  />
                  <p className="text-xs text-[#666] mt-2">
                    Extra fuel immediately after engine starts
                  </p>
                </div>

                <div>
                  <div className="flex justify-between mb-2">
                    <Label className="text-[#aaa]">Decay Time</Label>
                    <span className="text-[#0af] font-mono">{afterstartDecay.toFixed(1)}s</span>
                  </div>
                  <Slider
                    value={[afterstartDecay * 10]}
                    onValueChange={(v) => liveTuning && setAfterstartDecay(v[0] / 10)}
                    min={10}
                    max={100}
                    step={5}
                    disabled={!liveTuning}
                  />
                  <p className="text-xs text-[#666] mt-2">
                    Time to taper afterstart enrichment to zero
                  </p>
                </div>

                {/* Decay Preview */}
                <div className="mt-4 p-4 bg-[#1a1a1a] rounded border border-[#444]">
                  <div className="text-xs text-[#888] mb-3">Decay Profile</div>
                  <div className="h-24 border border-[#333] rounded bg-[#0a0a0a] p-2 relative">
                    <svg className="w-full h-full">
                      <polyline
                        points={`0,10 20,15 40,25 60,45 80,70 100,85`}
                        fill="none"
                        stroke="#0af"
                        strokeWidth="2"
                      />
                      <line x1="0" y1="85" x2="100" y2="85" stroke="#333" strokeWidth="1" strokeDasharray="2,2" />
                    </svg>
                    <div className="absolute bottom-1 left-2 text-[10px] text-[#666]">0s</div>
                    <div className="absolute bottom-1 right-2 text-[10px] text-[#666]">{afterstartDecay}s</div>
                  </div>
                </div>
              </div>
            </div>

            {/* Priming */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <h3 className="text-white mb-4 border-b border-[#333] pb-2">Fuel Priming</h3>
              
              <div className="space-y-4">
                <div className="grid grid-cols-2 gap-4">
                  <div>
                    <Label className="text-[#aaa] text-xs">Prime Pulses</Label>
                    <div className="mt-1 px-3 py-2 bg-[#1a1a1a] border border-[#444] rounded text-[#0af] font-mono">
                      3
                    </div>
                  </div>
                  <div>
                    <Label className="text-[#aaa] text-xs">Pulse Width</Label>
                    <div className="mt-1 px-3 py-2 bg-[#1a1a1a] border border-[#444] rounded text-[#0af] font-mono">
                      15 ms
                    </div>
                  </div>
                </div>
                <p className="text-xs text-[#666]">
                  Fuel injected on key-on before cranking to prime fuel rails
                </p>
              </div>
            </div>
          </div>

          {/* Right Column - Warmup Enrichment */}
          <div className="space-y-6">
            {/* Warmup Enrichment Curve */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <h3 className="text-white mb-4 border-b border-[#333] pb-2">Warmup Enrichment vs Temperature</h3>
              
              <div className="space-y-4">
                <p className="text-xs text-[#666]">
                  Fuel enrichment during engine warmup, tapered by coolant temperature
                </p>

                {/* Temperature Points */}
                {[
                  { temp: -10, index: 0 },
                  { temp: 0, index: 1 },
                  { temp: 20, index: 2 },
                  { temp: 40, index: 3 },
                  { temp: 60, index: 4 },
                  { temp: 70, index: 5 },
                  { temp: 80, index: 6 },
                ].map(({ temp, index }) => (
                  <div key={temp}>
                    <div className="flex justify-between mb-2">
                      <Label className="text-[#aaa] text-sm">{temp}°C</Label>
                      <span className="text-[#0af] font-mono">+{warmupEnrich[index]}%</span>
                    </div>
                    <Slider
                      value={[warmupEnrich[index]]}
                      onValueChange={(v) => {
                        if (liveTuning) {
                          const newEnrich = [...warmupEnrich];
                          newEnrich[index] = v[0];
                          setWarmupEnrich(newEnrich);
                        }
                      }}
                      min={0}
                      max={150}
                      step={5}
                      disabled={!liveTuning}
                    />
                  </div>
                ))}

                {/* Visualization */}
                <div className="mt-6 p-4 bg-[#1a1a1a] rounded border border-[#444]">
                  <div className="text-xs text-[#888] mb-2">Enrichment Curve</div>
                  <div className="h-32 border border-[#333] rounded bg-[#0a0a0a] p-3 relative">
                    <svg className="w-full h-full" viewBox="0 0 100 100" preserveAspectRatio="none">
                      <polyline
                        points={warmupEnrich
                          .map((val, i) => `${(i / 6) * 100},${100 - val * 0.6}`)
                          .join(" ")}
                        fill="none"
                        stroke="#0af"
                        strokeWidth="2"
                      />
                      {/* Grid lines */}
                      <line x1="0" y1="100" x2="100" y2="100" stroke="#333" strokeWidth="1" />
                      <line x1="0" y1="70" x2="100" y2="70" stroke="#222" strokeWidth="1" strokeDasharray="2,2" />
                      <line x1="0" y1="40" x2="100" y2="40" stroke="#222" strokeWidth="1" strokeDasharray="2,2" />
                    </svg>
                    <div className="absolute bottom-0 left-0 text-[9px] text-[#666]">-10°C</div>
                    <div className="absolute bottom-0 right-0 text-[9px] text-[#666]">80°C</div>
                    <div className="absolute top-0 left-0 text-[9px] text-[#666]">+150%</div>
                  </div>
                </div>
              </div>
            </div>

            {/* Tips */}
            <div className="bg-[#252525] border border-[#0af] rounded-lg p-5">
              <h3 className="text-[#0af] mb-3 border-b border-[#0af]/30 pb-2">Tuning Tips</h3>
              <div className="space-y-2 text-xs text-[#aaa]">
                <div className="flex gap-2">
                  <span className="text-[#0af]">•</span>
                  <span>Start with higher values and reduce if engine runs rich during warmup</span>
                </div>
                <div className="flex gap-2">
                  <span className="text-[#0af]">•</span>
                  <span>Monitor AFR during cold starts - target 13.0-13.5:1 initially</span>
                </div>
                <div className="flex gap-2">
                  <span className="text-[#0af]">•</span>
                  <span>Afterstart enrichment prevents lean stumble immediately after start</span>
                </div>
                <div className="flex gap-2">
                  <span className="text-[#0af]">•</span>
                  <span>Priming ensures fuel rail pressure before first crank</span>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
