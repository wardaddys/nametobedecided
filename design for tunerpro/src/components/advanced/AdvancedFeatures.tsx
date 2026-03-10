interface AdvancedFeaturesProps {
  liveTuning: boolean;
}

export function AdvancedFeatures({ liveTuning }: AdvancedFeaturesProps) {
  return (
    <div className="h-full bg-[#1a1a1a] p-6 overflow-auto">
      <div className="max-w-7xl mx-auto space-y-4">
        {/* Launch Control */}
        <div className="bg-[#1a1a1a] border border-[#333333] p-4">
          <h2 className="text-white mb-4">Launch Control</h2>
          <div className="grid grid-cols-2 gap-4">
            <div className="bg-black border border-[#444444] p-3 space-y-2">
              <div className="flex items-center gap-2">
                <input type="checkbox" className="w-4 h-4" />
                <span className="text-xs text-[#CCCCCC]">Enable Launch Control</span>
              </div>
              <div className="flex items-center justify-between">
                <span className="text-xs text-[#888888]">Launch RPM:</span>
                <input
                  type="number"
                  defaultValue="4500"
                  className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-24"
                />
              </div>
              <div className="flex items-center justify-between">
                <span className="text-xs text-[#888888]">Retard:</span>
                <input
                  type="number"
                  defaultValue="-5"
                  step="0.5"
                  className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-24"
                />
              </div>
            </div>

            <div className="bg-black border border-[#444444] p-3 space-y-2">
              <h3 className="text-sm text-[#CCCCCC] mb-2">Activation</h3>
              <div className="flex items-center gap-2">
                <input type="radio" name="launch-mode" defaultChecked />
                <span className="text-xs text-[#888888]">Clutch + Brake</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="radio" name="launch-mode" />
                <span className="text-xs text-[#888888]">Speed Based</span>
              </div>
            </div>
          </div>
        </div>

        {/* Traction Control */}
        <div className="bg-[#1a1a1a] border border-[#333333] p-4">
          <h2 className="text-white mb-4">Traction Control</h2>
          <div className="grid grid-cols-2 gap-4">
            <div className="bg-black border border-[#444444] p-3 space-y-2">
              <div className="flex items-center gap-2">
                <input type="checkbox" className="w-4 h-4" />
                <span className="text-xs text-[#CCCCCC]">Enable Traction Control</span>
              </div>
              <div className="flex items-center justify-between">
                <span className="text-xs text-[#888888]">Slip Threshold:</span>
                <input
                  type="number"
                  defaultValue="15"
                  step="1"
                  className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-24"
                />
                <span className="text-xs text-[#888888]">%</span>
              </div>
              <div className="flex items-center justify-between">
                <span className="text-xs text-[#888888]">Ignition Cut:</span>
                <input
                  type="number"
                  defaultValue="-3"
                  step="0.5"
                  className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-24"
                />
              </div>
            </div>

            <div className="bg-black border border-[#444444] p-3 space-y-2">
              <div className="flex items-center justify-between">
                <span className="text-xs text-[#888888]">Min Speed:</span>
                <input
                  type="number"
                  defaultValue="10"
                  className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-24"
                />
                <span className="text-xs text-[#888888]">mph</span>
              </div>
              <div className="flex items-center justify-between">
                <span className="text-xs text-[#888888]">Max Speed:</span>
                <input
                  type="number"
                  defaultValue="120"
                  className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-24"
                />
                <span className="text-xs text-[#888888]">mph</span>
              </div>
            </div>
          </div>
        </div>

        {/* Flat Shift / No-Lift Shift */}
        <div className="bg-[#1a1a1a] border border-[#333333] p-4">
          <h2 className="text-white mb-4">Flat Shift (No-Lift Shift)</h2>
          <div className="grid grid-cols-2 gap-4">
            <div className="bg-black border border-[#444444] p-3 space-y-2">
              <div className="flex items-center gap-2">
                <input type="checkbox" className="w-4 h-4" />
                <span className="text-xs text-[#CCCCCC]">Enable Flat Shift</span>
              </div>
              <div className="flex items-center justify-between">
                <span className="text-xs text-[#888888]">Cut Duration:</span>
                <input
                  type="number"
                  defaultValue="100"
                  className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-24"
                />
                <span className="text-xs text-[#888888]">ms</span>
              </div>
            </div>

            <div className="bg-black border border-[#444444] p-3 space-y-2">
              <div className="flex items-center justify-between">
                <span className="text-xs text-[#888888]">Min RPM:</span>
                <input
                  type="number"
                  defaultValue="3000"
                  className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-24"
                />
              </div>
              <div className="flex items-center justify-between">
                <span className="text-xs text-[#888888]">Retard Amount:</span>
                <input
                  type="number"
                  defaultValue="-10"
                  step="0.5"
                  className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-24"
                />
              </div>
            </div>
          </div>
        </div>

        {/* Rev Limiter */}
        <div className="bg-[#1a1a1a] border border-[#333333] p-4">
          <h2 className="text-white mb-4">Rev Limiter</h2>
          <div className="grid grid-cols-3 gap-4">
            <div className="bg-black border border-[#444444] p-3 space-y-2">
              <h3 className="text-sm text-[#CCCCCC] mb-2">Hard Cut</h3>
              <div className="flex items-center justify-between">
                <span className="text-xs text-[#888888]">RPM Limit:</span>
                <input
                  type="number"
                  defaultValue="8500"
                  className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-24"
                />
              </div>
            </div>

            <div className="bg-black border border-[#444444] p-3 space-y-2">
              <h3 className="text-sm text-[#CCCCCC] mb-2">Soft Cut</h3>
              <div className="flex items-center justify-between">
                <span className="text-xs text-[#888888]">Start:</span>
                <input
                  type="number"
                  defaultValue="8300"
                  className="bg-black border border-[#555555] px-2 py-1 text-xs text-white w-24"
                />
              </div>
            </div>

            <div className="bg-black border border-[#444444] p-3 space-y-2">
              <h3 className="text-sm text-[#CCCCCC] mb-2">Method</h3>
              <div className="flex items-center gap-2">
                <input type="radio" name="limiter" defaultChecked />
                <span className="text-xs text-[#888888]">Fuel Cut</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="radio" name="limiter" />
                <span className="text-xs text-[#888888]">Ignition Cut</span>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
