import { useVTEC } from "../context/VTECContext";
import { Switch } from "../ui/switch";
import { Label } from "../ui/label";

interface VTECControlProps {
  liveTuning: boolean;
}

export function VTECControl({ liveTuning }: VTECControlProps) {
  const { vtecEnabled, setVtecEnabled } = useVTEC();

  return (
    <div className="h-full bg-[#1a1a1a] p-6 overflow-auto">
      <div className="max-w-7xl mx-auto">
        <div className="bg-[#1a1a1a] border border-[#333333] p-4 mb-4">
          <div className="flex items-center justify-between mb-4">
            <h2 className="text-white">VVT/VTEC Control</h2>
            <div className="flex items-center gap-3">
              <Label htmlFor="vtec-enable" className="text-[#CCCCCC]">
                Enable VTEC/Dual Maps
              </Label>
              <Switch
                id="vtec-enable"
                checked={vtecEnabled}
                onCheckedChange={setVtecEnabled}
              />
            </div>
          </div>
          
          <div className="grid grid-cols-2 gap-4">
            {/* VTEC Engagement */}
            <div className="bg-black border border-[#444444] p-3">
              <h3 className="text-sm text-[#CCCCCC] mb-3">VTEC Engagement</h3>
              <div className="space-y-2">
                <div className="flex items-center justify-between">
                  <span className="text-xs text-[#888888]">Engagement RPM:</span>
                  <input
                    type="number"
                    defaultValue="5800"
                    className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-24"
                  />
                </div>
                <div className="flex items-center justify-between">
                  <span className="text-xs text-[#888888]">Hysteresis:</span>
                  <input
                    type="number"
                    defaultValue="200"
                    className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-24"
                  />
                </div>
              </div>
            </div>

            {/* VVT Control */}
            <div className="bg-black border border-[#444444] p-3">
              <h3 className="text-sm text-[#CCCCCC] mb-3">VVT Control</h3>
              <div className="space-y-2">
                <div className="flex items-center justify-between">
                  <span className="text-xs text-[#888888]">Max Advance:</span>
                  <input
                    type="number"
                    defaultValue="50"
                    className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-24"
                  />
                </div>
                <div className="flex items-center justify-between">
                  <span className="text-xs text-[#888888]">Min Advance:</span>
                  <input
                    type="number"
                    defaultValue="0"
                    className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-24"
                  />
                </div>
              </div>
            </div>
          </div>
        </div>

        {/* VVT Map Table */}
        <div className="bg-[#1a1a1a] border border-[#333333] p-4">
          <h3 className="text-white mb-3">VVT Advance Map (Degrees)</h3>
          <div className="overflow-auto">
            <table className="w-full border-collapse font-data text-xs">
              <thead>
                <tr>
                  <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC]">
                    RPM\MAP
                  </th>
                  {[20, 40, 60, 80, 100].map((kpa) => (
                    <th
                      key={kpa}
                      className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC]"
                    >
                      {kpa}
                    </th>
                  ))}
                </tr>
              </thead>
              <tbody>
                {[2000, 3000, 4000, 5000, 6000, 7000].map((rpm) => (
                  <tr key={rpm}>
                    <td className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC]">
                      {rpm}
                    </td>
                    {[20, 40, 60, 80, 100].map((kpa) => (
                      <td
                        key={`${rpm}-${kpa}`}
                        className="border border-[#444444] p-0"
                      >
                        <input
                          type="number"
                          defaultValue={Math.round(Math.random() * 30 + 10)}
                          className="w-full bg-black text-white text-center p-2 border-0 outline-none focus:bg-[#0066CC] focus:text-white"
                        />
                      </td>
                    ))}
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </div>
      </div>
    </div>
  );
}
