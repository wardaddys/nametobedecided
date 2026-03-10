import { useState } from "react";
import { Slider } from "../ui/slider";
import { Label } from "../ui/label";
import { Switch } from "../ui/switch";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "../ui/tabs";

interface LimitersProps {
  liveTuning: boolean;
}

export function Limiters({ liveTuning }: LimitersProps) {
  // Rev Limiter
  const [softRevLimit, setSoftRevLimit] = useState(6800);
  const [hardRevLimit, setHardRevLimit] = useState(7200);
  const [softLimitType, setSoftLimitType] = useState<'ignition' | 'fuel'>('ignition');
  
  // Speed Limiter
  const [speedLimitEnabled, setSpeedLimitEnabled] = useState(false);
  const [speedLimit, setSpeedLimit] = useState(180);
  
  // Launch Control
  const [launchEnabled, setLaunchEnabled] = useState(true);
  const [launchRPM, setLaunchRPM] = useState(4500);
  const [launchRetard, setLaunchRetard] = useState(10);

  return (
    <div className="h-full overflow-auto bg-[#1a1a1a] p-6">
      <div className="max-w-6xl mx-auto space-y-6">
        {/* Header */}
        <div className="border-b border-[#333] pb-4">
          <h2 className="text-2xl text-white">Engine Limiters & Protection</h2>
          <p className="text-sm text-[#888] mt-1">
            Configure RPM, speed limiters and launch control settings
          </p>
        </div>

        <Tabs defaultValue="rev" className="w-full">
          <TabsList className="bg-[#252525] border border-[#333]">
            <TabsTrigger value="rev" className="data-[state=active]:bg-[#0066CC]">
              Rev Limiter
            </TabsTrigger>
            <TabsTrigger value="speed" className="data-[state=active]:bg-[#0066CC]">
              Speed Limiter
            </TabsTrigger>
            <TabsTrigger value="launch" className="data-[state=active]:bg-[#0066CC]">
              Launch Control
            </TabsTrigger>
          </TabsList>

          {/* Rev Limiter Tab */}
          <TabsContent value="rev" className="space-y-6 mt-6">
            <div className="grid grid-cols-2 gap-6">
              {/* Soft Rev Limit */}
              <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
                <h3 className="text-white mb-4 border-b border-[#333] pb-2">Soft Rev Limiter</h3>
                
                <div className="space-y-4">
                  <div>
                    <div className="flex justify-between mb-2">
                      <Label className="text-[#aaa]">Soft Limit RPM</Label>
                      <span className="text-[#f80] font-mono">{softRevLimit} rpm</span>
                    </div>
                    <Slider
                      value={[softRevLimit]}
                      onValueChange={(v) => liveTuning && setSoftRevLimit(v[0])}
                      min={5000}
                      max={8500}
                      step={100}
                      disabled={!liveTuning}
                    />
                    <p className="text-xs text-[#666] mt-2">
                      RPM at which soft limiting begins
                    </p>
                  </div>

                  {/* Soft Limit Type */}
                  <div>
                    <Label className="text-[#aaa] mb-3 block">Soft Limit Method</Label>
                    <div className="grid grid-cols-2 gap-3">
                      <button
                        onClick={() => liveTuning && setSoftLimitType('ignition')}
                        disabled={!liveTuning}
                        className={`px-4 py-3 rounded border text-sm transition-all ${
                          softLimitType === 'ignition'
                            ? 'bg-[#0066CC] border-[#0066CC] text-white'
                            : 'bg-[#1a1a1a] border-[#444] text-[#888] hover:border-[#666]'
                        }`}
                      >
                        Ignition Cut
                      </button>
                      <button
                        onClick={() => liveTuning && setSoftLimitType('fuel')}
                        disabled={!liveTuning}
                        className={`px-4 py-3 rounded border text-sm transition-all ${
                          softLimitType === 'fuel'
                            ? 'bg-[#0066CC] border-[#0066CC] text-white'
                            : 'bg-[#1a1a1a] border-[#444] text-[#888] hover:border-[#666]'
                        }`}
                      >
                        Fuel Cut
                      </button>
                    </div>
                    <p className="text-xs text-[#666] mt-2">
                      {softLimitType === 'ignition' 
                        ? 'Cuts ignition - smoother, produces backfire flames' 
                        : 'Cuts fuel - safer for turbo engines'}
                    </p>
                  </div>

                  <div className="p-3 bg-[#1a1a1a] rounded border border-[#444]">
                    <p className="text-xs text-[#888]">
                      <span className="text-[#0af]">TIP:</span> Soft limiter allows brief excursions above the limit during shifts
                    </p>
                  </div>
                </div>
              </div>

              {/* Hard Rev Limit */}
              <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
                <h3 className="text-white mb-4 border-b border-[#333] pb-2">Hard Rev Limiter</h3>
                
                <div className="space-y-4">
                  <div>
                    <div className="flex justify-between mb-2">
                      <Label className="text-[#aaa]">Hard Limit RPM</Label>
                      <span className="text-[#f44] font-mono">{hardRevLimit} rpm</span>
                    </div>
                    <Slider
                      value={[hardRevLimit]}
                      onValueChange={(v) => liveTuning && setHardRevLimit(v[0])}
                      min={6000}
                      max={9000}
                      step={100}
                      disabled={!liveTuning}
                    />
                    <p className="text-xs text-[#666] mt-2">
                      Absolute maximum RPM - aggressive fuel and ignition cut
                    </p>
                  </div>

                  <div className="p-4 bg-[#ff4444]/10 rounded border border-[#ff4444]">
                    <p className="text-xs text-[#ff8888]">
                      <span className="text-[#ff4444]">⚠ WARNING:</span> Hard limit should be 200-500 RPM above soft limit
                    </p>
                  </div>

                  <div className="grid grid-cols-2 gap-4">
                    <div>
                      <Label className="text-[#aaa] text-xs">Cut Type</Label>
                      <div className="mt-1 px-3 py-2 bg-[#1a1a1a] border border-[#444] rounded text-white text-sm">
                        Fuel + Ign
                      </div>
                    </div>
                    <div>
                      <Label className="text-[#aaa] text-xs">Cut Pattern</Label>
                      <div className="mt-1 px-3 py-2 bg-[#1a1a1a] border border-[#444] rounded text-white text-sm">
                        All Cyl
                      </div>
                    </div>
                  </div>
                </div>
              </div>
            </div>

            {/* Rev Limit Visualization */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <h3 className="text-white mb-4 border-b border-[#333] pb-2">Rev Limit Zones</h3>
              
              <div className="space-y-4">
                {/* Visual RPM bar */}
                <div className="relative h-16 border border-[#333] rounded bg-[#0a0a0a] overflow-hidden">
                  {/* Safe zone */}
                  <div
                    className="absolute top-0 bottom-0 left-0 bg-gradient-to-r from-[#0f0]/20 to-[#0f0]/10"
                    style={{ width: `${(softRevLimit / 9000) * 100}%` }}
                  />
                  {/* Soft zone */}
                  <div
                    className="absolute top-0 bottom-0 bg-gradient-to-r from-[#f80]/20 to-[#f80]/10"
                    style={{
                      left: `${(softRevLimit / 9000) * 100}%`,
                      width: `${((hardRevLimit - softRevLimit) / 9000) * 100}%`,
                    }}
                  />
                  {/* Hard zone */}
                  <div
                    className="absolute top-0 bottom-0 right-0 bg-gradient-to-r from-[#f44]/20 to-[#f44]/10"
                    style={{ width: `${((9000 - hardRevLimit) / 9000) * 100}%` }}
                  />
                  
                  {/* Markers */}
                  <div
                    className="absolute top-0 bottom-0 border-r-2 border-[#f80]"
                    style={{ left: `${(softRevLimit / 9000) * 100}%` }}
                  />
                  <div
                    className="absolute top-0 bottom-0 border-r-2 border-[#f44]"
                    style={{ left: `${(hardRevLimit / 9000) * 100}%` }}
                  />
                  
                  {/* Labels */}
                  <div className="absolute inset-0 flex items-center justify-between px-4 text-xs font-mono">
                    <span className="text-[#0f0]">0 rpm</span>
                    <span className="text-[#f80]">{softRevLimit} rpm</span>
                    <span className="text-[#f44]">{hardRevLimit} rpm</span>
                  </div>
                </div>

                {/* Legend */}
                <div className="grid grid-cols-3 gap-4 text-xs">
                  <div className="flex items-center gap-2">
                    <div className="w-4 h-4 bg-[#0f0]/20 border border-[#0f0]" />
                    <span className="text-[#888]">Safe Zone</span>
                  </div>
                  <div className="flex items-center gap-2">
                    <div className="w-4 h-4 bg-[#f80]/20 border border-[#f80]" />
                    <span className="text-[#888]">Soft Limit</span>
                  </div>
                  <div className="flex items-center gap-2">
                    <div className="w-4 h-4 bg-[#f44]/20 border border-[#f44]" />
                    <span className="text-[#888]">Hard Limit</span>
                  </div>
                </div>
              </div>
            </div>
          </TabsContent>

          {/* Speed Limiter Tab */}
          <TabsContent value="speed" className="space-y-6 mt-6">
            <div className="max-w-2xl mx-auto">
              <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
                <div className="flex items-center justify-between mb-4 border-b border-[#333] pb-2">
                  <h3 className="text-white">Speed Limiter</h3>
                  <Switch
                    checked={speedLimitEnabled}
                    onCheckedChange={(checked) => liveTuning && setSpeedLimitEnabled(checked)}
                    disabled={!liveTuning}
                  />
                </div>
                
                {speedLimitEnabled && (
                  <div className="space-y-4">
                    <div>
                      <div className="flex justify-between mb-2">
                        <Label className="text-[#aaa]">Maximum Speed</Label>
                        <span className="text-[#f80] font-mono">{speedLimit} km/h</span>
                      </div>
                      <Slider
                        value={[speedLimit]}
                        onValueChange={(v) => liveTuning && setSpeedLimit(v[0])}
                        min={100}
                        max={300}
                        step={5}
                        disabled={!liveTuning}
                      />
                      <p className="text-xs text-[#666] mt-2">
                        Speed at which fuel/ignition cut activates
                      </p>
                    </div>

                    <div className="grid grid-cols-2 gap-4">
                      <div>
                        <Label className="text-[#aaa] text-xs">Limit Method</Label>
                        <div className="mt-1 px-3 py-2 bg-[#1a1a1a] border border-[#444] rounded text-white text-sm">
                          Ignition Cut
                        </div>
                      </div>
                      <div>
                        <Label className="text-[#aaa] text-xs">Hysteresis</Label>
                        <div className="mt-1 px-3 py-2 bg-[#1a1a1a] border border-[#444] rounded text-white text-sm">
                          -5 km/h
                        </div>
                      </div>
                    </div>

                    <div className="p-3 bg-[#1a1a1a] rounded border border-[#444]">
                      <p className="text-xs text-[#888]">
                        Speed limiter re-enables at {speedLimit - 5} km/h to prevent oscillation
                      </p>
                    </div>
                  </div>
                )}

                {!speedLimitEnabled && (
                  <div className="text-center py-8 text-[#666]">
                    <div className="text-4xl mb-2">∞</div>
                    <div className="text-sm">No speed limit configured</div>
                  </div>
                )}
              </div>
            </div>
          </TabsContent>

          {/* Launch Control Tab */}
          <TabsContent value="launch" className="space-y-6 mt-6">
            <div className="grid grid-cols-2 gap-6">
              <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
                <div className="flex items-center justify-between mb-4 border-b border-[#333] pb-2">
                  <h3 className="text-white">Launch Control</h3>
                  <Switch
                    checked={launchEnabled}
                    onCheckedChange={(checked) => liveTuning && setLaunchEnabled(checked)}
                    disabled={!liveTuning}
                  />
                </div>
                
                {launchEnabled && (
                  <div className="space-y-4">
                    <div>
                      <div className="flex justify-between mb-2">
                        <Label className="text-[#aaa]">Launch RPM</Label>
                        <span className="text-[#0af] font-mono">{launchRPM} rpm</span>
                      </div>
                      <Slider
                        value={[launchRPM]}
                        onValueChange={(v) => liveTuning && setLaunchRPM(v[0])}
                        min={2000}
                        max={7000}
                        step={100}
                        disabled={!liveTuning}
                      />
                      <p className="text-xs text-[#666] mt-2">
                        RPM held during launch (clutch engaged, throttle wide open)
                      </p>
                    </div>

                    <div>
                      <div className="flex justify-between mb-2">
                        <Label className="text-[#aaa]">Timing Retard</Label>
                        <span className="text-[#f80] font-mono">-{launchRetard}°</span>
                      </div>
                      <Slider
                        value={[launchRetard]}
                        onValueChange={(v) => liveTuning && setLaunchRetard(v[0])}
                        min={0}
                        max={25}
                        step={1}
                        disabled={!liveTuning}
                      />
                      <p className="text-xs text-[#666] mt-2">
                        Timing retard for anti-lag effect and traction
                      </p>
                    </div>

                    <div className="grid grid-cols-2 gap-4">
                      <div>
                        <Label className="text-[#aaa] text-xs">Activation</Label>
                        <div className="mt-1 px-3 py-2 bg-[#1a1a1a] border border-[#444] rounded text-white text-sm">
                          Clutch + TPS
                        </div>
                      </div>
                      <div>
                        <Label className="text-[#aaa] text-xs">Max Duration</Label>
                        <div className="mt-1 px-3 py-2 bg-[#1a1a1a] border border-[#444] rounded text-white text-sm">
                          5 sec
                        </div>
                      </div>
                    </div>
                  </div>
                )}
              </div>

              <div className="bg-[#252525] border border-[#0af] rounded-lg p-5">
                <h3 className="text-[#0af] mb-4 border-b border-[#0af]/30 pb-2">Launch Instructions</h3>
                <div className="space-y-3 text-sm text-[#aaa]">
                  <div className="flex gap-3">
                    <div className="text-[#0af] font-bold">1.</div>
                    <div>Come to complete stop with clutch pressed</div>
                  </div>
                  <div className="flex gap-3">
                    <div className="text-[#0af] font-bold">2.</div>
                    <div>Press throttle to 100% (launch activates)</div>
                  </div>
                  <div className="flex gap-3">
                    <div className="text-[#0af] font-bold">3.</div>
                    <div>RPM will hold at {launchRPM} rpm automatically</div>
                  </div>
                  <div className="flex gap-3">
                    <div className="text-[#0af] font-bold">4.</div>
                    <div>Release clutch quickly for maximum launch</div>
                  </div>
                  <div className="flex gap-3">
                    <div className="text-[#0af] font-bold">5.</div>
                    <div>Launch control deactivates when speed &gt; 10 km/h</div>
                  </div>
                </div>
              </div>
            </div>
          </TabsContent>
        </Tabs>
      </div>
    </div>
  );
}
