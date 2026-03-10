import { useState } from "react";
import { Slider } from "../ui/slider";
import { Label } from "../ui/label";
import { Switch } from "../ui/switch";

interface AccelEnrichmentProps {
  liveTuning: boolean;
}

export function AccelEnrichment({ liveTuning }: AccelEnrichmentProps) {
  const [tpsBasedEnabled, setTpsBasedEnabled] = useState(true);
  const [tpsThreshold, setTpsThreshold] = useState(5);
  const [tpsAmount, setTpsAmount] = useState(40);
  const [tpsDecay, setTpsDecay] = useState(0.5);
  
  const [mapBasedEnabled, setMapBasedEnabled] = useState(true);
  const [mapThreshold, setMapThreshold] = useState(10);
  const [mapAmount, setMapAmount] = useState(30);
  const [mapDecay, setMapDecay] = useState(0.4);

  const [decelCutEnabled, setDecelCutEnabled] = useState(true);
  const [decelThreshold, setDecelThreshold] = useState(-10);

  return (
    <div className="h-full overflow-auto bg-[#1a1a1a] p-6">
      <div className="max-w-6xl mx-auto space-y-6">
        {/* Header */}
        <div className="border-b border-[#333] pb-4">
          <h2 className="text-2xl text-white">Acceleration Enrichment</h2>
          <p className="text-sm text-[#888] mt-1">
            Configure transient fuel compensation for throttle and load changes
          </p>
        </div>

        <div className="grid grid-cols-2 gap-6">
          {/* Left Column - TPS Based */}
          <div className="space-y-6">
            {/* TPS-Based Enrichment */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <div className="flex items-center justify-between mb-4 border-b border-[#333] pb-2">
                <h3 className="text-white">TPS-Based Enrichment</h3>
                <Switch
                  checked={tpsBasedEnabled}
                  onCheckedChange={(checked) => liveTuning && setTpsBasedEnabled(checked)}
                  disabled={!liveTuning}
                />
              </div>
              
              {tpsBasedEnabled && (
                <div className="space-y-4">
                  <div>
                    <div className="flex justify-between mb-2">
                      <Label className="text-[#aaa]">TPS Rate Threshold</Label>
                      <span className="text-[#0af] font-mono">{tpsThreshold}%/s</span>
                    </div>
                    <Slider
                      value={[tpsThreshold]}
                      onValueChange={(v) => liveTuning && setTpsThreshold(v[0])}
                      min={1}
                      max={20}
                      step={1}
                      disabled={!liveTuning}
                    />
                    <p className="text-xs text-[#666] mt-2">
                      Minimum TPS change rate to trigger enrichment
                    </p>
                  </div>

                  <div>
                    <div className="flex justify-between mb-2">
                      <Label className="text-[#aaa]">Enrichment Amount</Label>
                      <span className="text-[#0af] font-mono">{tpsAmount}%</span>
                    </div>
                    <Slider
                      value={[tpsAmount]}
                      onValueChange={(v) => liveTuning && setTpsAmount(v[0])}
                      min={0}
                      max={100}
                      step={5}
                      disabled={!liveTuning}
                    />
                    <p className="text-xs text-[#666] mt-2">
                      Fuel added during rapid throttle opening
                    </p>
                  </div>

                  <div>
                    <div className="flex justify-between mb-2">
                      <Label className="text-[#aaa]">Decay Time Constant</Label>
                      <span className="text-[#0af] font-mono">{tpsDecay.toFixed(1)}s</span>
                    </div>
                    <Slider
                      value={[tpsDecay * 10]}
                      onValueChange={(v) => liveTuning && setTpsDecay(v[0] / 10)}
                      min={1}
                      max={20}
                      step={1}
                      disabled={!liveTuning}
                    />
                    <p className="text-xs text-[#666] mt-2">
                      How quickly enrichment tapers off
                    </p>
                  </div>

                  {/* Response Curve */}
                  <div className="mt-4 p-4 bg-[#1a1a1a] rounded border border-[#444]">
                    <div className="text-xs text-[#888] mb-3">TPS Response Curve</div>
                    <div className="h-32 border border-[#333] rounded bg-[#0a0a0a] p-3 relative">
                      <svg className="w-full h-full" viewBox="0 0 100 100" preserveAspectRatio="none">
                        {/* Enrichment curve */}
                        <polyline
                          points="0,100 10,20 20,25 35,35 50,50 70,70 100,95"
                          fill="none"
                          stroke="#0af"
                          strokeWidth="2"
                        />
                        {/* TPS curve */}
                        <polyline
                          points="0,100 10,30 100,30"
                          fill="none"
                          stroke="#f80"
                          strokeWidth="1.5"
                          strokeDasharray="3,2"
                        />
                        {/* Grid */}
                        <line x1="0" y1="100" x2="100" y2="100" stroke="#333" strokeWidth="1" />
                      </svg>
                      <div className="absolute bottom-0 left-0 text-[9px] text-[#666]">Time</div>
                      <div className="absolute top-2 right-2 text-[9px] text-[#f80]">TPS</div>
                      <div className="absolute top-10 right-2 text-[9px] text-[#0af]">Fuel</div>
                    </div>
                  </div>

                  {/* TPS Table */}
                  <div className="p-4 bg-[#1a1a1a] rounded border border-[#444]">
                    <div className="text-xs text-[#888] mb-3">Enrichment vs TPS Rate</div>
                    <div className="space-y-1 text-xs font-mono">
                      <div className="flex justify-between border-b border-[#333] pb-1">
                        <span className="text-[#666]">TPS Rate</span>
                        <span className="text-[#666]">Fuel Add</span>
                      </div>
                      <div className="flex justify-between">
                        <span className="text-[#888]">5%/s</span>
                        <span className="text-[#0af]">{Math.round(tpsAmount * 0.3)}%</span>
                      </div>
                      <div className="flex justify-between">
                        <span className="text-[#888]">20%/s</span>
                        <span className="text-[#0af]">{Math.round(tpsAmount * 0.6)}%</span>
                      </div>
                      <div className="flex justify-between">
                        <span className="text-[#888]">50%/s</span>
                        <span className="text-[#0af]">{tpsAmount}%</span>
                      </div>
                      <div className="flex justify-between">
                        <span className="text-[#888]">100%/s</span>
                        <span className="text-[#0af]">{Math.round(tpsAmount * 1.2)}%</span>
                      </div>
                    </div>
                  </div>
                </div>
              )}
            </div>
          </div>

          {/* Right Column - MAP Based & Decel */}
          <div className="space-y-6">
            {/* MAP-Based Enrichment */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <div className="flex items-center justify-between mb-4 border-b border-[#333] pb-2">
                <h3 className="text-white">MAP-Based Enrichment</h3>
                <Switch
                  checked={mapBasedEnabled}
                  onCheckedChange={(checked) => liveTuning && setMapBasedEnabled(checked)}
                  disabled={!liveTuning}
                />
              </div>
              
              {mapBasedEnabled && (
                <div className="space-y-4">
                  <div>
                    <div className="flex justify-between mb-2">
                      <Label className="text-[#aaa]">MAP Rate Threshold</Label>
                      <span className="text-[#0af] font-mono">{mapThreshold} kPa/s</span>
                    </div>
                    <Slider
                      value={[mapThreshold]}
                      onValueChange={(v) => liveTuning && setMapThreshold(v[0])}
                      min={5}
                      max={50}
                      step={5}
                      disabled={!liveTuning}
                    />
                    <p className="text-xs text-[#666] mt-2">
                      Minimum MAP change rate to trigger enrichment
                    </p>
                  </div>

                  <div>
                    <div className="flex justify-between mb-2">
                      <Label className="text-[#aaa]">Enrichment Amount</Label>
                      <span className="text-[#0af] font-mono">{mapAmount}%</span>
                    </div>
                    <Slider
                      value={[mapAmount]}
                      onValueChange={(v) => liveTuning && setMapAmount(v[0])}
                      min={0}
                      max={100}
                      step={5}
                      disabled={!liveTuning}
                    />
                    <p className="text-xs text-[#666] mt-2">
                      Fuel added during rapid load increase
                    </p>
                  </div>

                  <div>
                    <div className="flex justify-between mb-2">
                      <Label className="text-[#aaa]">Decay Time Constant</Label>
                      <span className="text-[#0af] font-mono">{mapDecay.toFixed(1)}s</span>
                    </div>
                    <Slider
                      value={[mapDecay * 10]}
                      onValueChange={(v) => liveTuning && setMapDecay(v[0] / 10)}
                      min={1}
                      max={20}
                      step={1}
                      disabled={!liveTuning}
                    />
                  </div>

                  <div className="p-3 bg-[#1a1a1a] rounded border border-[#444]">
                    <p className="text-xs text-[#888]">
                      <span className="text-[#0af]">TIP:</span> MAP-based is better for turbo engines with slow TPS response
                    </p>
                  </div>
                </div>
              )}
            </div>

            {/* Deceleration Fuel Cut */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <div className="flex items-center justify-between mb-4 border-b border-[#333] pb-2">
                <h3 className="text-white">Deceleration Fuel Cut</h3>
                <Switch
                  checked={decelCutEnabled}
                  onCheckedChange={(checked) => liveTuning && setDecelCutEnabled(checked)}
                  disabled={!liveTuning}
                />
              </div>
              
              {decelCutEnabled && (
                <div className="space-y-4">
                  <div>
                    <div className="flex justify-between mb-2">
                      <Label className="text-[#aaa]">TPS Close Threshold</Label>
                      <span className="text-[#0af] font-mono">{decelThreshold}%/s</span>
                    </div>
                    <Slider
                      value={[Math.abs(decelThreshold)]}
                      onValueChange={(v) => liveTuning && setDecelThreshold(-v[0])}
                      min={5}
                      max={50}
                      step={5}
                      disabled={!liveTuning}
                    />
                    <p className="text-xs text-[#666] mt-2">
                      Rate of throttle closure to trigger fuel reduction
                    </p>
                  </div>

                  <div className="grid grid-cols-2 gap-4">
                    <div>
                      <Label className="text-[#aaa] text-xs">Min RPM</Label>
                      <div className="mt-1 px-3 py-2 bg-[#1a1a1a] border border-[#444] rounded text-[#0af] font-mono text-sm">
                        1500 rpm
                      </div>
                    </div>
                    <div>
                      <Label className="text-[#aaa] text-xs">Min TPS</Label>
                      <div className="mt-1 px-3 py-2 bg-[#1a1a1a] border border-[#444] rounded text-[#0af] font-mono text-sm">
                        2%
                      </div>
                    </div>
                  </div>

                  <p className="text-xs text-[#666]">
                    Cuts fuel on deceleration to improve fuel economy and reduce emissions
                  </p>
                </div>
              )}
            </div>

            {/* Live Status */}
            <div className="bg-[#252525] border border-[#0af] rounded-lg p-5">
              <h3 className="text-[#0af] mb-4 border-b border-[#0af]/30 pb-2">Live Enrichment Status</h3>
              
              <div className="space-y-3">
                <div className="grid grid-cols-2 gap-3 text-sm">
                  <div>
                    <div className="text-[#888] text-xs">TPS Rate</div>
                    <div className="text-white font-mono mt-1">2.3%/s</div>
                  </div>
                  <div>
                    <div className="text-[#888] text-xs">MAP Rate</div>
                    <div className="text-white font-mono mt-1">5.1 kPa/s</div>
                  </div>
                  <div>
                    <div className="text-[#888] text-xs">TPS Enrich</div>
                    <div className="text-[#0af] font-mono mt-1">+8%</div>
                  </div>
                  <div>
                    <div className="text-[#888] text-xs">MAP Enrich</div>
                    <div className="text-[#0af] font-mono mt-1">+0%</div>
                  </div>
                </div>

                <div className="pt-3 border-t border-[#333]">
                  <div className="text-[#888] text-xs">Total Accel Enrichment</div>
                  <div className="text-[#0f0] font-mono mt-1 text-lg">+8%</div>
                </div>
              </div>
            </div>

            {/* Tuning Tips */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <h3 className="text-white mb-3 border-b border-[#333] pb-2 text-sm">Tuning Guide</h3>
              <div className="space-y-2 text-xs text-[#aaa]">
                <div className="flex gap-2">
                  <span className="text-[#0af]">•</span>
                  <span>Start conservative - too much causes rich stumble</span>
                </div>
                <div className="flex gap-2">
                  <span className="text-[#0af]">•</span>
                  <span>Use TPS-based for NA engines, MAP-based for turbo</span>
                </div>
                <div className="flex gap-2">
                  <span className="text-[#0af]">•</span>
                  <span>Increase if lean stumble occurs on throttle tip-in</span>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
