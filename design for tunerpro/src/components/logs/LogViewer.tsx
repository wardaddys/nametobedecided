import { useState, useRef } from "react";
import { motion } from "motion/react";
import { Play, Pause, SkipBack, SkipForward } from "lucide-react";
import { Button } from "../ui/button";
import { Slider } from "../ui/slider";
import { Checkbox } from "../ui/checkbox";
import { Label } from "../ui/label";
import { Badge } from "../ui/badge";
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip as RechartsTooltip,
  Legend,
  ResponsiveContainer,
} from "recharts";

interface LogViewerProps {
  liveTuning: boolean;
}

export function LogViewer({ liveTuning }: LogViewerProps) {
  const [isPlaying, setIsPlaying] = useState(false);
  const [currentTime, setCurrentTime] = useState(0);
  const [scrubSpeed, setScrubSpeed] = useState(1);
  const [selectedParams, setSelectedParams] = useState<string[]>([
    "rpm",
    "map",
    "afr",
  ]);

  const logData = generateMockLogData();
  const maxTime = logData.length - 1;

  function generateMockLogData() {
    return Array.from({ length: 500 }, (_, i) => ({
      time: i * 0.1,
      rpm: 1000 + Math.sin(i * 0.05) * 3000 + Math.random() * 200,
      map: 50 + Math.sin(i * 0.03) * 40 + Math.random() * 10,
      afr: 14.7 + Math.sin(i * 0.08) * 1.5 + Math.random() * 0.3,
      ect: 88 + Math.random() * 4,
      iat: 32 + Math.random() * 8,
      tps: Math.max(0, Math.min(100, 20 + Math.sin(i * 0.04) * 40)),
    }));
  }

  const currentFrame = logData[Math.floor(currentTime)] || logData[0];

  // Calculate which map cell is active at current time
  const getActiveCell = () => {
    const rpmIndex = Math.floor((currentFrame.rpm - 1000) / 500);
    const loadIndex = Math.floor(currentFrame.map / 6.25);
    return {
      rpm: Math.max(0, Math.min(15, rpmIndex)),
      load: Math.max(0, Math.min(15, loadIndex)),
    };
  };

  const activeCell = getActiveCell();

  const parameters = [
    { id: "rpm", label: "RPM", color: "#1FB6FF", unit: "" },
    { id: "map", label: "MAP", color: "#00A676", unit: "kPa" },
    { id: "afr", label: "AFR", color: "#FFB62A", unit: "" },
    { id: "ect", label: "ECT", color: "#E24B4B", unit: "°C" },
    { id: "iat", label: "IAT", color: "#9333EA", unit: "°C" },
    { id: "tps", label: "TPS", color: "#EC4899", unit: "%" },
  ];

  const handleScrub = (value: number[]) => {
    setCurrentTime(value[0]);
  };

  const handleMouseWheel = (e: React.WheelEvent) => {
    e.preventDefault();
    const delta = e.deltaY > 0 ? -10 : 10;
    setCurrentTime((prev) => Math.max(0, Math.min(maxTime, prev + delta)));
  };

  return (
    <div className="h-full bg-[#1a1a1a] flex">
      {/* Left: Log List */}
      <div className="w-64 border-r border-[rgba(175,198,210,0.1)] p-4">
        <h3 className="text-[#E6EEF3] mb-4">Log Files</h3>
        <div className="space-y-2">
          {[
            "Run_001_2024-10-17.csv",
            "Run_002_2024-10-17.csv",
            "Dyno_Session_01.csv",
          ].map((log, i) => (
            <button
              key={i}
              className={`w-full text-left px-3 py-2 rounded text-sm ${
                i === 0
                  ? "bg-[#1FB6FF] text-[#0B0F12]"
                  : "text-[#AFC6D2] hover:bg-[rgba(175,198,210,0.05)]"
              }`}
            >
              {log}
            </button>
          ))}
        </div>
      </div>

      {/* Middle & Right: Timeline and Charts */}
      <div className="flex-1 flex flex-col p-6">
        <div className="mb-6">
          <h2 className="text-[#E6EEF3] mb-1">Log Viewer & Analysis</h2>
          <p className="text-sm text-[#AFC6D2]">
            Scrub through recorded data and correlate with map cells
          </p>
        </div>

        {/* Timeline Controls */}
        <div className="glassmorphism rounded-lg p-4 mb-4">
          <div className="flex items-center gap-4 mb-4">
            <Button
              size="sm"
              variant="ghost"
              onClick={() => setCurrentTime(0)}
            >
              <SkipBack className="w-4 h-4" />
            </Button>
            <Button
              size="sm"
              variant="ghost"
              onClick={() => setIsPlaying(!isPlaying)}
            >
              {isPlaying ? (
                <Pause className="w-4 h-4" />
              ) : (
                <Play className="w-4 h-4" />
              )}
            </Button>
            <Button
              size="sm"
              variant="ghost"
              onClick={() => setCurrentTime(maxTime)}
            >
              <SkipForward className="w-4 h-4" />
            </Button>

            <div className="flex-1 px-4">
              <Slider
                value={[currentTime]}
                onValueChange={handleScrub}
                max={maxTime}
                step={1}
              />
            </div>

            <div className="font-data text-sm text-[#E6EEF3]">
              {currentFrame.time.toFixed(1)}s / {logData[logData.length - 1].time.toFixed(1)}s
            </div>
          </div>

          {/* Current Values */}
          <div className="grid grid-cols-6 gap-4">
            {parameters.map((param) => (
              <div key={param.id} className="text-center">
                <div className="text-xs text-[#AFC6D2]">{param.label}</div>
                <div className="font-data text-lg" style={{ color: param.color }}>
                  {currentFrame[param.id as keyof typeof currentFrame].toFixed(
                    param.id === "rpm" ? 0 : 1
                  )}
                  {param.unit && (
                    <span className="text-xs ml-1">{param.unit}</span>
                  )}
                </div>
              </div>
            ))}
          </div>

          {/* Active Cell Indicator */}
          <div className="mt-4 pt-4 border-t border-[rgba(175,198,210,0.15)] flex items-center gap-2">
            <Badge variant="outline" className="border-[#1FB6FF] text-[#1FB6FF]">
              Active Map Cell: [{activeCell.load}, {activeCell.rpm}]
            </Badge>
            <span className="text-xs text-[#AFC6D2]">
              Overlaying map data at current timeline position
            </span>
          </div>
        </div>

        <div className="flex gap-4 flex-1">
          {/* Main Chart */}
          <div className="flex-1 glassmorphism rounded-lg p-4" onWheel={handleMouseWheel}>
            <h3 className="text-sm text-[#E6EEF3] mb-3">Time-Series Data</h3>
            <ResponsiveContainer width="100%" height="100%">
              <LineChart data={logData}>
                <CartesianGrid strokeDasharray="3 3" stroke="rgba(175,198,210,0.1)" />
                <XAxis
                  dataKey="time"
                  stroke="#AFC6D2"
                  tick={{ fill: "#AFC6D2", fontSize: 11 }}
                  label={{ value: "Time (s)", position: "insideBottom", offset: -5, fill: "#AFC6D2" }}
                />
                <YAxis
                  stroke="#AFC6D2"
                  tick={{ fill: "#AFC6D2", fontSize: 11 }}
                />
                <RechartsTooltip
                  contentStyle={{
                    backgroundColor: "#111419",
                    border: "1px solid rgba(175,198,210,0.2)",
                    borderRadius: "6px",
                  }}
                  labelStyle={{ color: "#E6EEF3" }}
                />
                <Legend />
                {parameters
                  .filter((p) => selectedParams.includes(p.id))
                  .map((param) => (
                    <Line
                      key={param.id}
                      type="monotone"
                      dataKey={param.id}
                      stroke={param.color}
                      dot={false}
                      strokeWidth={2}
                      name={param.label}
                    />
                  ))}
                
                {/* Current position indicator */}
                <Line
                  type="monotone"
                  data={[
                    { time: currentFrame.time, value: 0 },
                    { time: currentFrame.time, value: 8000 },
                  ]}
                  dataKey="value"
                  stroke="#1FB6FF"
                  strokeWidth={2}
                  dot={false}
                  strokeDasharray="5 5"
                />
              </LineChart>
            </ResponsiveContainer>
          </div>

          {/* Parameter Selection */}
          <div className="w-64 glassmorphism rounded-lg p-4">
            <h3 className="text-sm text-[#E6EEF3] mb-3">Parameters</h3>
            <div className="space-y-3">
              {parameters.map((param) => (
                <div key={param.id} className="flex items-center gap-2">
                  <Checkbox
                    id={param.id}
                    checked={selectedParams.includes(param.id)}
                    onCheckedChange={(checked) => {
                      if (checked) {
                        setSelectedParams([...selectedParams, param.id]);
                      } else {
                        setSelectedParams(
                          selectedParams.filter((p) => p !== param.id)
                        );
                      }
                    }}
                  />
                  <Label
                    htmlFor={param.id}
                    className="flex items-center gap-2 cursor-pointer"
                  >
                    <div
                      className="w-3 h-3 rounded-full"
                      style={{ backgroundColor: param.color }}
                    />
                    {param.label}
                  </Label>
                </div>
              ))}
            </div>

            <div className="mt-6 pt-4 border-t border-[rgba(175,198,210,0.15)]">
              <h4 className="text-xs text-[#AFC6D2] mb-2">Scrub Speed</h4>
              <Slider
                value={[scrubSpeed]}
                onValueChange={(v) => setScrubSpeed(v[0])}
                min={1}
                max={10}
                step={1}
              />
              <div className="text-xs text-[#E6EEF3] mt-1 text-center">
                {scrubSpeed}x
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
