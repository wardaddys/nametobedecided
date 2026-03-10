import { useState } from "react";
import { Slider } from "../ui/slider";
import { Label } from "../ui/label";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "../ui/tabs";

interface TrimsEditorProps {
  liveTuning: boolean;
}

export function TrimsEditor({ liveTuning }: TrimsEditorProps) {
  const [globalFuelTrim, setGlobalFuelTrim] = useState(0);
  const [globalIgnTrim, setGlobalIgnTrim] = useState(0);
  
  // Per-cylinder fuel trims
  const [cylFuelTrims, setCylFuelTrims] = useState([0, 0, 0, 0]);
  
  // Per-cylinder ignition trims
  const [cylIgnTrims, setCylIgnTrims] = useState([0, 0, 0, 0]);

  const updateCylFuelTrim = (index: number, value: number) => {
    if (liveTuning) {
      const newTrims = [...cylFuelTrims];
      newTrims[index] = value;
      setCylFuelTrims(newTrims);
    }
  };

  const updateCylIgnTrim = (index: number, value: number) => {
    if (liveTuning) {
      const newTrims = [...cylIgnTrims];
      newTrims[index] = value;
      setCylIgnTrims(newTrims);
    }
  };

  return (
    <div className="h-full overflow-auto bg-[#1a1a1a] p-6">
      <div className="max-w-6xl mx-auto space-y-6">
        {/* Header */}
        <div className="border-b border-[#333] pb-4">
          <h2 className="text-2xl text-white">Fuel & Ignition Trims</h2>
          <p className="text-sm text-[#888] mt-1">
            Apply global and per-cylinder corrections to fuel and ignition timing
          </p>
        </div>

        <Tabs defaultValue="fuel" className="w-full">
          <TabsList className="bg-[#252525] border border-[#333]">
            <TabsTrigger value="fuel" className="data-[state=active]:bg-[#0066CC]">
              Fuel Trims
            </TabsTrigger>
            <TabsTrigger value="ignition" className="data-[state=active]:bg-[#0066CC]">
              Ignition Trims
            </TabsTrigger>
          </TabsList>

          {/* Fuel Trims Tab */}
          <TabsContent value="fuel" className="space-y-6 mt-6">
            <div className="grid grid-cols-2 gap-6">
              {/* Global Fuel Trim */}
              <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
                <h3 className="text-white mb-4 border-b border-[#333] pb-2">Global Fuel Trim</h3>
                
                <div className="space-y-4">
                  <div>
                    <div className="flex justify-between mb-2">
                      <Label className="text-[#aaa]">All Cylinders Adjustment</Label>
                      <span className={`font-mono ${globalFuelTrim >= 0 ? 'text-[#0af]' : 'text-[#f80]'}`}>
                        {globalFuelTrim >= 0 ? '+' : ''}{globalFuelTrim}%
                      </span>
                    </div>
                    <Slider
                      value={[globalFuelTrim + 50]}
                      onValueChange={(v) => liveTuning && setGlobalFuelTrim(v[0] - 50)}
                      min={0}
                      max={100}
                      step={1}
                      disabled={!liveTuning}
                    />
                    <p className="text-xs text-[#666] mt-2">
                      Applies to all cylinders across entire fuel map
                    </p>
                  </div>

                  <div className="p-4 bg-[#1a1a1a] rounded border border-[#444]">
                    <div className="text-xs text-[#888] mb-2">Effect on Base Fueling</div>
                    <div className="space-y-1 text-xs font-mono">
                      <div className="flex justify-between">
                        <span className="text-[#666]">Base Pulse Width:</span>
                        <span className="text-white">10.5 ms</span>
                      </div>
                      <div className="flex justify-between">
                        <span className="text-[#666]">With Trim:</span>
                        <span className="text-[#0af]">{(10.5 * (1 + globalFuelTrim / 100)).toFixed(2)} ms</span>
                      </div>
                    </div>
                  </div>

                  <div className="p-3 bg-[#1a1a1a] rounded border border-[#444]">
                    <p className="text-xs text-[#888]">
                      <span className="text-[#0af]">TIP:</span> Use global trim for quick AFR adjustments across all conditions
                    </p>
                  </div>
                </div>
              </div>

              {/* Per-Cylinder Fuel Trims */}
              <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
                <h3 className="text-white mb-4 border-b border-[#333] pb-2">Per-Cylinder Fuel Trims</h3>
                
                <div className="space-y-4">
                  {[1, 2, 3, 4].map((cyl, index) => (
                    <div key={cyl}>
                      <div className="flex justify-between mb-2">
                        <Label className="text-[#aaa]">Cylinder {cyl}</Label>
                        <span className={`font-mono ${cylFuelTrims[index] >= 0 ? 'text-[#0af]' : 'text-[#f80]'}`}>
                          {cylFuelTrims[index] >= 0 ? '+' : ''}{cylFuelTrims[index]}%
                        </span>
                      </div>
                      <Slider
                        value={[cylFuelTrims[index] + 25]}
                        onValueChange={(v) => updateCylFuelTrim(index, v[0] - 25)}
                        min={0}
                        max={50}
                        step={1}
                        disabled={!liveTuning}
                      />
                    </div>
                  ))}

                  <div className="mt-4 p-3 bg-[#1a1a1a] rounded border border-[#444]">
                    <p className="text-xs text-[#888]">
                      <span className="text-[#0af]">TIP:</span> Use cylinder trims to balance AFR between cylinders. Check individual O2 sensors.
                    </p>
                  </div>
                </div>
              </div>
            </div>

            {/* Fuel Trim Visualization */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <h3 className="text-white mb-4 border-b border-[#333] pb-2">Cylinder Fuel Balance</h3>
              
              <div className="grid grid-cols-4 gap-4">
                {[1, 2, 3, 4].map((cyl, index) => {
                  const totalTrim = globalFuelTrim + cylFuelTrims[index];
                  return (
                    <div key={cyl} className="text-center">
                      <div className="text-sm text-[#888] mb-2">Cyl {cyl}</div>
                      <div className="h-32 border border-[#333] rounded bg-[#0a0a0a] relative overflow-hidden">
                        <div
                          className={`absolute bottom-0 left-0 right-0 transition-all ${
                            totalTrim >= 0 ? 'bg-gradient-to-t from-[#0af] to-[#0af]/50' : 'bg-gradient-to-t from-[#f80] to-[#f80]/50'
                          }`}
                          style={{
                            height: `${Math.min(100, 50 + totalTrim * 2)}%`,
                          }}
                        />
                        <div className="absolute inset-0 flex items-center justify-center">
                          <span className={`text-xl font-mono ${totalTrim >= 0 ? 'text-[#0af]' : 'text-[#f80]'}`}>
                            {totalTrim >= 0 ? '+' : ''}{totalTrim}%
                          </span>
                        </div>
                        {/* Zero line */}
                        <div className="absolute left-0 right-0 border-t border-[#666] border-dashed" style={{ top: '50%' }} />
                      </div>
                    </div>
                  );
                })}
              </div>
            </div>
          </TabsContent>

          {/* Ignition Trims Tab */}
          <TabsContent value="ignition" className="space-y-6 mt-6">
            <div className="grid grid-cols-2 gap-6">
              {/* Global Ignition Trim */}
              <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
                <h3 className="text-white mb-4 border-b border-[#333] pb-2">Global Ignition Trim</h3>
                
                <div className="space-y-4">
                  <div>
                    <div className="flex justify-between mb-2">
                      <Label className="text-[#aaa]">All Cylinders Adjustment</Label>
                      <span className={`font-mono ${globalIgnTrim >= 0 ? 'text-[#0af]' : 'text-[#f80]'}`}>
                        {globalIgnTrim >= 0 ? '+' : ''}{globalIgnTrim}°
                      </span>
                    </div>
                    <Slider
                      value={[globalIgnTrim + 20]}
                      onValueChange={(v) => liveTuning && setGlobalIgnTrim(v[0] - 20)}
                      min={0}
                      max={40}
                      step={0.5}
                      disabled={!liveTuning}
                    />
                    <p className="text-xs text-[#666] mt-2">
                      Applies to all cylinders across entire ignition map
                    </p>
                  </div>

                  <div className="p-4 bg-[#1a1a1a] rounded border border-[#444]">
                    <div className="text-xs text-[#888] mb-2">Effect on Base Timing</div>
                    <div className="space-y-1 text-xs font-mono">
                      <div className="flex justify-between">
                        <span className="text-[#666]">Base Timing:</span>
                        <span className="text-white">28° BTDC</span>
                      </div>
                      <div className="flex justify-between">
                        <span className="text-[#666]">With Trim:</span>
                        <span className="text-[#0af]">{(28 + globalIgnTrim).toFixed(1)}° BTDC</span>
                      </div>
                    </div>
                  </div>

                  <div className="p-3 bg-[#ff4444]/10 rounded border border-[#ff4444]">
                    <p className="text-xs text-[#ff8888]">
                      <span className="text-[#ff4444]">⚠ WARNING:</span> Advanced timing can cause detonation. Monitor knock sensors!
                    </p>
                  </div>
                </div>
              </div>

              {/* Per-Cylinder Ignition Trims */}
              <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
                <h3 className="text-white mb-4 border-b border-[#333] pb-2">Per-Cylinder Ignition Trims</h3>
                
                <div className="space-y-4">
                  {[1, 2, 3, 4].map((cyl, index) => (
                    <div key={cyl}>
                      <div className="flex justify-between mb-2">
                        <Label className="text-[#aaa]">Cylinder {cyl}</Label>
                        <span className={`font-mono ${cylIgnTrims[index] >= 0 ? 'text-[#0af]' : 'text-[#f80]'}`}>
                          {cylIgnTrims[index] >= 0 ? '+' : ''}{cylIgnTrims[index]}°
                        </span>
                      </div>
                      <Slider
                        value={[cylIgnTrims[index] + 10]}
                        onValueChange={(v) => updateCylIgnTrim(index, v[0] - 10)}
                        min={0}
                        max={20}
                        step={0.5}
                        disabled={!liveTuning}
                      />
                    </div>
                  ))}

                  <div className="mt-4 p-3 bg-[#1a1a1a] rounded border border-[#444]">
                    <p className="text-xs text-[#888]">
                      <span className="text-[#0af]">TIP:</span> Retard timing on cylinders showing knock activity
                    </p>
                  </div>
                </div>
              </div>
            </div>

            {/* Ignition Trim Visualization */}
            <div className="bg-[#252525] border border-[#333] rounded-lg p-5">
              <h3 className="text-white mb-4 border-b border-[#333] pb-2">Cylinder Timing Balance</h3>
              
              <div className="grid grid-cols-4 gap-4">
                {[1, 2, 3, 4].map((cyl, index) => {
                  const totalTrim = globalIgnTrim + cylIgnTrims[index];
                  return (
                    <div key={cyl} className="text-center">
                      <div className="text-sm text-[#888] mb-2">Cyl {cyl}</div>
                      <div className="h-32 border border-[#333] rounded bg-[#0a0a0a] relative overflow-hidden">
                        <div
                          className={`absolute bottom-0 left-0 right-0 transition-all ${
                            totalTrim >= 0 ? 'bg-gradient-to-t from-[#0af] to-[#0af]/50' : 'bg-gradient-to-t from-[#f80] to-[#f80]/50'
                          }`}
                          style={{
                            height: `${Math.min(100, 50 + totalTrim * 2.5)}%`,
                          }}
                        />
                        <div className="absolute inset-0 flex items-center justify-center">
                          <span className={`text-xl font-mono ${totalTrim >= 0 ? 'text-[#0af]' : 'text-[#f80]'}`}>
                            {totalTrim >= 0 ? '+' : ''}{totalTrim.toFixed(1)}°
                          </span>
                        </div>
                        {/* Zero line */}
                        <div className="absolute left-0 right-0 border-t border-[#666] border-dashed" style={{ top: '50%' }} />
                      </div>
                    </div>
                  );
                })}
              </div>
            </div>
          </TabsContent>
        </Tabs>
      </div>
    </div>
  );
}
