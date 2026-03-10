import { useState, useEffect, useRef } from "react";
import { motion, AnimatePresence } from "motion/react";
import { Play, Pause, Square, Circle, Download, Settings } from "lucide-react";
import { Progress } from "../ui/progress";
import { Badge } from "../ui/badge";
import { LogDataPoint, STANDARD_CHANNELS } from "./LoggingTypes";

interface LiveCaptureProps {
  selectedChannels: string[];
  onCaptureStopped: (data: LogDataPoint[]) => void;
}

export function LiveCapture({ selectedChannels, onCaptureStopped }: LiveCaptureProps) {
  const [isCapturing, setIsCapturing] = useState(false);
  const [isPaused, setIsPaused] = useState(false);
  const [captureData, setCaptureData] = useState<LogDataPoint[]>([]);
  const [duration, setDuration] = useState(0);
  const [sampleRate, setSampleRate] = useState(10); // Hz
  const maxDuration = 300; // 5 minutes max

  const startTimeRef = useRef<number>(0);
  const intervalRef = useRef<NodeJS.Timeout>();

  useEffect(() => {
    if (isCapturing && !isPaused) {
      intervalRef.current = setInterval(() => {
        const now = Date.now();
        const elapsed = (now - startTimeRef.current) / 1000;
        setDuration(elapsed);

        // Simulate sensor data
        const dataPoint: LogDataPoint = {
          timestamp: now,
          values: {}
        };

        selectedChannels.forEach(channelId => {
          const channel = STANDARD_CHANNELS.find(c => c.id === channelId);
          if (channel) {
            // Simulate realistic values
            let value: number;
            switch (channelId) {
              case 'rpm':
                value = 2000 + Math.random() * 3000;
                break;
              case 'afr':
                value = 14.2 + (Math.random() - 0.5) * 1;
                break;
              case 'map':
                value = 80 + Math.random() * 40;
                break;
              case 'tps':
                value = 20 + Math.random() * 60;
                break;
              case 'timing':
                value = 15 + Math.random() * 10;
                break;
              case 'clt':
                value = 85 + (Math.random() - 0.5) * 5;
                break;
              case 'iat':
                value = 30 + (Math.random() - 0.5) * 10;
                break;
              case 'battery':
                value = 13.5 + (Math.random() - 0.5) * 0.5;
                break;
              case 'boost':
                value = Math.random() * 10;
                break;
              default:
                value = channel.min + Math.random() * (channel.max - channel.min);
            }
            dataPoint.values[channelId] = value;
          }
        });

        setCaptureData(prev => [...prev, dataPoint]);

        // Auto-stop at max duration
        if (elapsed >= maxDuration) {
          handleStop();
        }
      }, 1000 / sampleRate);

      return () => {
        if (intervalRef.current) clearInterval(intervalRef.current);
      };
    }
  }, [isCapturing, isPaused, selectedChannels, sampleRate]);

  const handleStart = () => {
    startTimeRef.current = Date.now();
    setCaptureData([]);
    setDuration(0);
    setIsCapturing(true);
    setIsPaused(false);
  };

  const handlePause = () => {
    setIsPaused(!isPaused);
  };

  const handleStop = () => {
    setIsCapturing(false);
    setIsPaused(false);
    if (intervalRef.current) clearInterval(intervalRef.current);
    onCaptureStopped(captureData);
  };

  const progress = (duration / maxDuration) * 100;
  const dataPoints = captureData.length;
  const estimatedSize = (dataPoints * selectedChannels.length * 8) / 1024; // KB

  return (
    <div className="h-full flex flex-col bg-[#1a1a1a] border-r border-[#333333]">
      {/* Header */}
      <div className="p-4 border-b border-[#333333]">
        <div className="flex items-center justify-between mb-3">
          <div>
            <h3 className="text-[#E6EEF3] flex items-center gap-2">
              Live Capture
              {isCapturing && (
                <motion.div
                  animate={{ opacity: [1, 0.3, 1] }}
                  transition={{ duration: 1, repeat: Infinity }}
                >
                  <Circle className="w-3 h-3 fill-red-500 text-red-500" />
                </motion.div>
              )}
            </h3>
            <p className="text-xs text-[#888888] mt-1">
              Real-time data streaming at {sampleRate}Hz
            </p>
          </div>
        </div>

        {/* Controls */}
        <div className="flex items-center gap-2">
          {!isCapturing ? (
            <motion.button
              onClick={handleStart}
              disabled={selectedChannels.length === 0}
              className="flex items-center gap-2 px-4 py-2 bg-[#00C4B4] hover:bg-[#00D4C4] text-black rounded transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
              whileHover={{ scale: 1.05 }}
              whileTap={{ scale: 0.95 }}
            >
              <Play className="w-4 h-4" />
              Start Recording
            </motion.button>
          ) : (
            <>
              <motion.button
                onClick={handlePause}
                className="flex items-center gap-2 px-4 py-2 bg-[#FFB62A] hover:bg-[#FFC64A] text-black rounded transition-colors"
                whileHover={{ scale: 1.05 }}
                whileTap={{ scale: 0.95 }}
              >
                {isPaused ? <Play className="w-4 h-4" /> : <Pause className="w-4 h-4" />}
                {isPaused ? 'Resume' : 'Pause'}
              </motion.button>
              <motion.button
                onClick={handleStop}
                className="flex items-center gap-2 px-4 py-2 bg-[#FF3B30] hover:bg-[#FF4B40] text-white rounded transition-colors"
                whileHover={{ scale: 1.05 }}
                whileTap={{ scale: 0.95 }}
              >
                <Square className="w-4 h-4" />
                Stop
              </motion.button>
            </>
          )}
        </div>

        {selectedChannels.length === 0 && (
          <div className="mt-3 p-2 bg-yellow-950/30 border border-yellow-600 rounded text-xs text-yellow-200">
            ⚠️ Select at least one channel to start recording
          </div>
        )}
      </div>

      {/* Stats */}
      <div className="flex-1 p-4 space-y-4">
        {/* Duration progress */}
        <div>
          <div className="flex items-center justify-between mb-2">
            <span className="text-sm text-[#888888]">Duration</span>
            <span className="font-mono text-[#E6EEF3]">
              {Math.floor(duration / 60)}:{String(Math.floor(duration % 60)).padStart(2, '0')} / {Math.floor(maxDuration / 60)}:00
            </span>
          </div>
          <Progress value={progress} className="h-2" />
        </div>

        {/* Live stats */}
        <div className="grid grid-cols-2 gap-3">
          <div className="p-3 bg-[#0a0a0a] border border-[#333333] rounded">
            <div className="text-xs text-[#666666] mb-1">Data Points</div>
            <div className="font-mono text-xl text-[#00C4B4]">
              {dataPoints.toLocaleString()}
            </div>
          </div>
          <div className="p-3 bg-[#0a0a0a] border border-[#333333] rounded">
            <div className="text-xs text-[#666666] mb-1">Est. Size</div>
            <div className="font-mono text-xl text-[#E6EEF3]">
              {estimatedSize.toFixed(1)} KB
            </div>
          </div>
          <div className="p-3 bg-[#0a0a0a] border border-[#333333] rounded">
            <div className="text-xs text-[#666666] mb-1">Channels</div>
            <div className="font-mono text-xl text-[#E6EEF3]">
              {selectedChannels.length}
            </div>
          </div>
          <div className="p-3 bg-[#0a0a0a] border border-[#333333] rounded">
            <div className="text-xs text-[#666666] mb-1">Sample Rate</div>
            <div className="font-mono text-xl text-[#00FF00]">
              {sampleRate} Hz
            </div>
          </div>
        </div>

        {/* Status badges */}
        <div className="flex flex-wrap gap-2">
          <Badge className={`${
            isCapturing && !isPaused
              ? 'bg-green-600 text-white'
              : isPaused
              ? 'bg-yellow-600 text-black'
              : 'bg-gray-600 text-white'
          }`}>
            {isCapturing && !isPaused ? 'Recording' : isPaused ? 'Paused' : 'Ready'}
          </Badge>
          {isCapturing && (
            <Badge variant="outline" className="border-[#00C4B4] text-[#00C4B4]">
              {(dataPoints / duration || 0).toFixed(1)} pts/s
            </Badge>
          )}
        </div>

        {/* Recent values (live preview) */}
        {isCapturing && captureData.length > 0 && (
          <motion.div
            initial={{ opacity: 0, y: 10 }}
            animate={{ opacity: 1, y: 0 }}
            className="p-3 bg-[#0a0a0a] border border-[#333333] rounded"
          >
            <div className="text-xs text-[#666666] mb-2">Live Values</div>
            <div className="space-y-1 max-h-[200px] overflow-y-auto">
              {selectedChannels.slice(0, 6).map(channelId => {
                const channel = STANDARD_CHANNELS.find(c => c.id === channelId);
                const lastValue = captureData[captureData.length - 1]?.values[channelId];
                if (!channel || lastValue === undefined) return null;

                return (
                  <div key={channelId} className="flex items-center justify-between text-xs">
                    <span className="text-[#888888]">{channel.name}</span>
                    <span className="font-mono text-[#E6EEF3]">
                      {lastValue.toFixed(channel.decimals)} {channel.unit}
                    </span>
                  </div>
                );
              })}
              {selectedChannels.length > 6 && (
                <div className="text-xs text-[#666666] text-center pt-1">
                  +{selectedChannels.length - 6} more...
                </div>
              )}
            </div>
          </motion.div>
        )}
      </div>

      {/* Settings footer */}
      <div className="p-3 border-t border-[#333333] bg-[#0a0a0a]">
        <button className="w-full flex items-center justify-center gap-2 px-3 py-2 bg-[#2a2a2a] hover:bg-[#333333] text-[#E6EEF3] rounded transition-colors text-sm">
          <Settings className="w-4 h-4" />
          Capture Settings
        </button>
      </div>
    </div>
  );
}
