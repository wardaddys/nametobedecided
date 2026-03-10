interface BoostControlProps {
  liveTuning: boolean;
}

export function BoostControl({ liveTuning }: BoostControlProps) {
  return (
    <div className="h-full bg-[#1a1a1a] p-6 overflow-auto">
      <div className="max-w-7xl mx-auto space-y-4">
        {/* Boost Control Settings */}
        <div className="bg-[#1a1a1a] border border-[#333333] p-4">
          <h2 className="text-white mb-4">Boost Control Settings</h2>
          
          <div className="grid grid-cols-3 gap-4">
            <div className="bg-black border border-[#444444] p-3">
              <h3 className="text-sm text-[#CCCCCC] mb-3">General</h3>
              <div className="space-y-2">
                <div className="flex items-center gap-2">
                  <input type="checkbox" className="w-4 h-4" />
                  <span className="text-xs text-[#888888]">Enable Boost Control</span>
                </div>
                <div className="flex items-center justify-between">
                  <span className="text-xs text-[#888888]">Max Boost:</span>
                  <input
                    type="number"
                    defaultValue="18"
                    step="0.1"
                    className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-20"
                  />
                  <span className="text-xs text-[#888888]">psi</span>
                </div>
              </div>
            </div>

            <div className="bg-black border border-[#444444] p-3">
              <h3 className="text-sm text-[#CCCCCC] mb-3">PID Tuning</h3>
              <div className="space-y-2">
                <div className="flex items-center justify-between">
                  <span className="text-xs text-[#888888]">P Gain:</span>
                  <input
                    type="number"
                    defaultValue="2.5"
                    step="0.1"
                    className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-20"
                  />
                </div>
                <div className="flex items-center justify-between">
                  <span className="text-xs text-[#888888]">I Gain:</span>
                  <input
                    type="number"
                    defaultValue="0.5"
                    step="0.1"
                    className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-20"
                  />
                </div>
                <div className="flex items-center justify-between">
                  <span className="text-xs text-[#888888]">D Gain:</span>
                  <input
                    type="number"
                    defaultValue="0.1"
                    step="0.1"
                    className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-20"
                  />
                </div>
              </div>
            </div>

            <div className="bg-black border border-[#444444] p-3">
              <h3 className="text-sm text-[#CCCCCC] mb-3">Safety</h3>
              <div className="space-y-2">
                <div className="flex items-center justify-between">
                  <span className="text-xs text-[#888888]">Overboost Cut:</span>
                  <input
                    type="number"
                    defaultValue="20"
                    step="0.1"
                    className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-20"
                  />
                </div>
                <div className="flex items-center gap-2">
                  <input type="checkbox" className="w-4 h-4" defaultChecked />
                  <span className="text-xs text-[#888888]">Enable Fuel Cut</span>
                </div>
              </div>
            </div>
          </div>
        </div>

        {/* Boost Target Map */}
        <div className="bg-[#1a1a1a] border border-[#333333] p-4">
          <h3 className="text-white mb-3">Boost Target Map (psi)</h3>
          <div className="overflow-auto">
            <table className="w-full border-collapse font-data text-xs">
              <thead>
                <tr>
                  <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC]">
                    RPM\Throttle
                  </th>
                  {[20, 40, 60, 80, 100].map((throttle) => (
                    <th
                      key={throttle}
                      className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC]"
                    >
                      {throttle}%
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
                    {[20, 40, 60, 80, 100].map((throttle) => (
                      <td
                        key={`${rpm}-${throttle}`}
                        className="border border-[#444444] p-0"
                      >
                        <input
                          type="number"
                          defaultValue={(throttle / 100 * 18).toFixed(1)}
                          step="0.1"
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
