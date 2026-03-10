import { useState } from "react";
import { LineChart, Line, XAxis, YAxis, CartesianGrid, ResponsiveContainer } from "recharts";
import { Switch } from "../ui/switch";
import { Label } from "../ui/label";
import { Input } from "../ui/input";
import { Button } from "../ui/button";

interface O2VETuningProps {
  liveTuning: boolean;
}

export function O2VETuning({ liveTuning }: O2VETuningProps) {
  const [o2FBControl, setO2FBControl] = useState(false);
  const [o2Target, setO2Target] = useState(14.7);
  const [o2FB1, setO2FB1] = useState(14.7);

  // Mock chart data for O2 FB time vs temp
  const chartData = Array.from({ length: 30 }, (_, i) => ({
    temp: i * 10,
    time: i < 10 ? 25 : i < 20 ? 15 : 10,
  }));

  // Mock chart data for O2 FB time vs load
  const loadChartData = Array.from({ length: 30 }, (_, i) => ({
    speed: i * 400,
    time: i < 10 ? 50 : i < 20 ? 0 : -10,
  }));

  return (
    <div className="h-full bg-[#1a1a1a] overflow-auto">
      <div className="grid grid-cols-[300px_1fr] h-full">
        {/* Left Sidebar - O2 Parameters */}
        <div className="bg-[#1a1a1a] border-r border-[#333333] overflow-auto">
          {/* O2 Target */}
          <div className="border-b border-[#333333] p-3">
            <h3 className="text-white mb-3 text-sm">O2 Target</h3>
            <table className="w-full text-xs border-collapse">
              <tbody>
                <tr className="border-b border-[#2a2a2a]">
                  <td className="py-2 text-[#CCCCCC]">O2 Target</td>
                  <td className="py-2 text-right">
                    <input
                      type="number"
                      value={o2Target}
                      onChange={(e) => setO2Target(parseFloat(e.target.value))}
                      step="0.1"
                      className="w-20 bg-black text-white border border-[#444444] px-2 py-1 text-right"
                    />
                  </td>
                  <td className="py-2 text-[#888888] pl-2">AFR</td>
                </tr>
                <tr className="border-b border-[#2a2a2a]">
                  <td className="py-2 text-[#CCCCCC]">O2 #1</td>
                  <td className="py-2 text-right">
                    <input
                      type="number"
                      value={o2FB1}
                      onChange={(e) => setO2FB1(parseFloat(e.target.value))}
                      step="0.1"
                      className="w-20 bg-black text-white border border-[#444444] px-2 py-1 text-right"
                    />
                  </td>
                  <td className="py-2 text-[#888888] pl-2">AFR</td>
                </tr>
                <tr>
                  <td className="py-2 text-[#CCCCCC]">O2 #1 FB Value</td>
                  <td className="py-2 text-right text-[#00FF00]">
                    {(Math.random() * 20 - 10).toFixed(1)}
                  </td>
                  <td className="py-2 text-[#888888] pl-2">%</td>
                </tr>
              </tbody>
            </table>
          </div>

          {/* O2 FB Cleared */}
          <div className="border-b border-[#333333] p-3">
            <h3 className="text-white mb-3 text-sm">O2 FB Cleared</h3>
            <table className="w-full text-xs border-collapse">
              <tbody>
                <tr>
                  <td className="py-2 text-[#CCCCCC]">O2 FB Cleared</td>
                  <td className="py-2 text-right text-[#00FF00]">ON</td>
                </tr>
              </tbody>
            </table>
          </div>

          {/* O2 FB Delay */}
          <div className="border-b border-[#333333] p-3">
            <h3 className="text-white mb-3 text-sm">O2 FB Delay</h3>
            <table className="w-full text-xs border-collapse">
              <tbody>
                <tr>
                  <td className="py-2 text-[#CCCCCC]">O2 FB Delay</td>
                  <td className="py-2 text-right">
                    <input
                      type="number"
                      defaultValue="5"
                      step="1"
                      className="w-20 bg-black text-white border border-[#444444] px-2 py-1 text-right"
                    />
                  </td>
                  <td className="py-2 text-[#888888] pl-2">S</td>
                </tr>
              </tbody>
            </table>
          </div>

          {/* O2 FB Frozen */}
          <div className="border-b border-[#333333] p-3">
            <h3 className="text-white mb-3 text-sm">O2 FB Frozen</h3>
            <table className="w-full text-xs border-collapse">
              <tbody>
                <tr>
                  <td className="py-2 text-[#CCCCCC]">O2 FB Frozen</td>
                  <td className="py-2 text-right text-[#00FF00]">ON</td>
                </tr>
              </tbody>
            </table>
          </div>

          {/* O2 FB Status */}
          <div className="border-b border-[#333333] p-3">
            <h3 className="text-white mb-3 text-sm">O2 FB Status</h3>
            <table className="w-full text-xs border-collapse">
              <tbody>
                <tr className="border-b border-[#2a2a2a]">
                  <td className="py-2 text-[#CCCCCC]">Accel O2 FB Hold</td>
                  <td className="py-2 text-right">
                    <input
                      type="number"
                      defaultValue="1000"
                      step="100"
                      className="w-20 bg-black text-white border border-[#444444] px-2 py-1 text-right"
                    />
                  </td>
                  <td className="py-2 text-[#888888] pl-2">mS</td>
                </tr>
                <tr className="border-b border-[#2a2a2a]">
                  <td className="py-2 text-[#CCCCCC]">O2 K# Limit</td>
                  <td className="py-2 text-right">
                    <input
                      type="number"
                      defaultValue="1.0"
                      step="0.1"
                      className="w-20 bg-black text-white border border-[#444444] px-2 py-1 text-right"
                      disabled
                    />
                  </td>
                  <td className="py-2 text-[#888888] pl-2">AFR</td>
                </tr>
                <tr className="border-b border-[#2a2a2a]">
                  <td className="py-2 text-[#CCCCCC]">O2 K# Engine</td>
                  <td className="py-2 text-right text-[#00FF00]">ON</td>
                  <td className="py-2 text-[#888888] pl-2"></td>
                </tr>
                <tr className="border-b border-[#2a2a2a]">
                  <td className="py-2 text-[#CCCCCC]">DFCO</td>
                  <td className="py-2 text-right">
                    <input
                      type="number"
                      defaultValue="1800"
                      step="100"
                      className="w-20 bg-black text-white border border-[#444444] px-2 py-1 text-right"
                    />
                  </td>
                  <td className="py-2 text-[#888888] pl-2">mS</td>
                </tr>
                <tr>
                  <td className="py-2 text-[#CCCCCC]">Accel Fuel</td>
                  <td className="py-2 text-right">
                    <input
                      type="number"
                      defaultValue="0"
                      step="1"
                      className="w-20 bg-black text-white border border-[#444444] px-2 py-1 text-right"
                    />
                  </td>
                  <td className="py-2 text-[#888888] pl-2">mS</td>
                </tr>
              </tbody>
            </table>
          </div>

          {/* Options - O2 Feedback */}
          <div className="border-b border-[#333333] p-3">
            <h3 className="text-white mb-3 text-sm">Options - O2 Feedback</h3>
            <table className="w-full text-xs border-collapse">
              <tbody>
                <tr className="border-b border-[#2a2a2a]">
                  <td className="py-2 text-[#CCCCCC]">O2 FB Control</td>
                  <td className="py-2 text-right">
                    <div className="flex items-center justify-end gap-2">
                      <span className="text-white">{o2FBControl ? "ON" : "OFF"}</span>
                      <Switch
                        checked={o2FBControl}
                        onCheckedChange={setO2FBControl}
                        disabled={!liveTuning}
                      />
                    </div>
                  </td>
                </tr>
                <tr className="border-b border-[#2a2a2a]">
                  <td className="py-2 text-[#CCCCCC]">O2 FB Accel Max</td>
                  <td className="py-2 text-right">
                    <input
                      type="number"
                      defaultValue="1000"
                      step="100"
                      className="w-20 bg-black text-white border border-[#444444] px-2 py-1 text-right"
                    />
                  </td>
                  <td className="py-2 text-[#888888] pl-2">mS</td>
                </tr>
                <tr className="border-b border-[#2a2a2a]">
                  <td className="py-2 text-[#CCCCCC]">O2 FB Accel Inhibit</td>
                  <td className="py-2 text-right">
                    <input
                      type="number"
                      defaultValue="459.4"
                      step="0.1"
                      className="w-20 bg-black text-white border border-[#444444] px-2 py-1 text-right"
                    />
                  </td>
                  <td className="py-2 text-[#888888] pl-2">mS</td>
                </tr>
                <tr className="border-b border-[#2a2a2a]">
                  <td className="py-2 text-[#CCCCCC]">O2 FB Cut Inhibit</td>
                  <td className="py-2 text-right">
                    <input
                      type="number"
                      defaultValue="426.6"
                      step="0.1"
                      className="w-20 bg-black text-white border border-[#444444] px-2 py-1 text-right"
                    />
                  </td>
                  <td className="py-2 text-[#888888] pl-2">mS</td>
                </tr>
                <tr className="border-b border-[#2a2a2a]">
                  <td className="py-2 text-[#CCCCCC]">O2 FB Update Rate</td>
                  <td className="py-2 text-right">
                    <input
                      type="number"
                      defaultValue="58.8"
                      step="0.1"
                      className="w-20 bg-black text-white border border-[#444444] px-2 py-1 text-right"
                    />
                  </td>
                  <td className="py-2 text-[#888888] pl-2">mS/Sec</td>
                </tr>
                <tr className="border-b border-[#2a2a2a]">
                  <td className="py-2 text-[#CCCCCC]">O2 FB Accel Clear</td>
                  <td className="py-2 text-right text-[#00FF00]">ON</td>
                </tr>
                <tr className="border-b border-[#2a2a2a]">
                  <td className="py-2 text-[#CCCCCC]">O2 FB DFCO Clear</td>
                  <td className="py-2 text-right text-[#00FF00]">ON</td>
                </tr>
                <tr>
                  <td className="py-2 text-[#CCCCCC]">O2 FB Over Clear</td>
                  <td className="py-2 text-right text-[#00FF00]">ON</td>
                </tr>
              </tbody>
            </table>
          </div>

          {/* Options - O2 Feedback (continued) */}
          <div className="border-b border-[#333333] p-3">
            <table className="w-full text-xs border-collapse">
              <tbody>
                <tr className="border-b border-[#2a2a2a]">
                  <td className="py-2 text-[#CCCCCC]">O2 FB Decel Inhibit</td>
                  <td className="py-2 text-right">
                    <input
                      type="number"
                      defaultValue="426.6"
                      step="0.1"
                      className="w-20 bg-black text-white border border-[#444444] px-2 py-1 text-right"
                    />
                  </td>
                  <td className="py-2 text-[#888888] pl-2">mS</td>
                </tr>
              </tbody>
            </table>
          </div>
        </div>

        {/* Right Side - O2 Target Map */}
        <div className="bg-black p-4 overflow-auto">
          <div className="bg-[#1a1a1a] border border-[#333333] p-4">
            <div className="mb-4">
              <h3 className="text-white mb-2">O2 FB Target</h3>
              <p className="text-xs text-[#888888]">AFR target values based on load and RPM</p>
            </div>
            
            <div className="overflow-auto">
              <table className="border-collapse text-xs font-data">
                <thead>
                  <tr>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] sticky left-0 z-10">
                      O2 FB
                    </th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">500</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">750</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">1000</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">1500</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">2000</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">2500</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">3000</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">3500</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">4000</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">4500</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">4750</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">5000</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">6250</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">5500</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">6000</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">6500</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">7000</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">7500</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">8000</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">8500</th>
                    <th className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] min-w-[60px]">9000</th>
                  </tr>
                </thead>
                <tbody>
                  {[
                    { load: "10.8", values: Array(21).fill(11.0) },
                    { load: "9.3", values: Array(21).fill(11.0) },
                    { load: "6.2", values: Array(21).fill(11.0) },
                    { load: "4.6", values: Array(21).fill(11.3) },
                    { load: "3.1", values: Array(21).fill(11.7) },
                    { load: "1.5", values: Array(21).fill(12.0) },
                    { load: "0.0", values: Array(21).fill(12.0) },
                    { load: "-1.6", values: Array(21).fill(12.0) },
                    { load: "-3.0", values: Array(21).fill(12.0) },
                    { load: "-4.5", values: Array(21).fill(12.5) },
                    { load: "-5.9", values: Array(21).fill(13.4) },
                    { load: "-7.4", values: Array(21).fill(13.9) },
                    { load: "-8.9", values: [14.5, 14.5, 14.5, 14.5, 14.5, 14.5, 14.5, 14.5, 14.5, 13.8, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5] },
                    { load: "-10.3", values: [14.5, 14.5, 14.5, 14.5, 14.5, 14.5, 14.5, 14.5, 14.5, 13.8, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5] },
                    { load: "-11.8", values: [14.5, 14.5, 14.5, 14.5, 14.5, 14.5, 14.5, 14.5, 14.5, 13.8, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5] },
                    { load: "-13.2", values: [14.5, 14.5, 14.5, 14.5, 14.5, 14.5, 14.5, 14.5, 14.5, 13.8, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5, 13.5] },
                  ].map((row, rowIndex) => (
                    <tr key={rowIndex}>
                      <td className="border border-[#444444] bg-[#2a2a2a] p-2 text-[#CCCCCC] sticky left-0 z-10">
                        {row.load}
                      </td>
                      {row.values.map((value, colIndex) => {
                        // Color coding based on AFR value
                        let bgColor = "#2d5c2d"; // Default green
                        if (value >= 14.0) {
                          bgColor = "#5c5c2d"; // Yellow
                        } else if (value >= 13.0) {
                          bgColor = "#4d5c2d"; // Yellow-green
                        } else if (value >= 12.5) {
                          bgColor = "#3d5c2d"; // Light green
                        } else if (value >= 12.0) {
                          bgColor = "#2d5c35"; // Green
                        } else if (value >= 11.5) {
                          bgColor = "#2d5c3d"; // Darker green
                        }
                        
                        return (
                          <td
                            key={colIndex}
                            className="border border-[#444444] p-2 text-white text-center cursor-pointer hover:brightness-125 transition-all"
                            style={{ backgroundColor: bgColor }}
                          >
                            {value.toFixed(1)}
                          </td>
                        );
                      })}
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
            
            <div className="mt-4 text-xs text-[#888888]">
              <p>Click any cell to edit AFR target value. Values in green indicate rich mixtures (11.0-12.0), yellow indicates lean mixtures (13.5-14.5).</p>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
