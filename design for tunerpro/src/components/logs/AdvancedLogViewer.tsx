import { useState, useEffect } from "react";
import { motion } from "motion/react";
import {
  FileText,
  Download,
  Share2,
  Trash2,
  Play,
  BarChart3,
  Zap,
  Plus,
  TrendingUp
} from "lucide-react";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "../ui/tabs";
import { Badge } from "../ui/badge";
import { ScrollArea } from "../ui/scroll-area";
import { ChannelSelector } from "./ChannelSelector";
import { LiveCapture } from "./LiveCapture";
import { LogGraph } from "./LogGraph";
import { LogFile, LogDataPoint, STANDARD_CHANNELS } from "./LoggingTypes";

export function AdvancedLogViewer() {
  const [activeTab, setActiveTab] = useState<'browser' | 'capture' | 'analyze'>('browser');
  const [selectedLog, setSelectedLog] = useState<string | null>(null);
  const [logFiles, setLogFiles] = useState<LogFile[]>([]);
  const [selectedChannels, setSelectedChannels] = useState<string[]>([
    'rpm', 'afr', 'map', 'tps', 'timing'
  ]);
  const [currentLogData, setCurrentLogData] = useState<LogDataPoint[]>([]);

  // Load sample log files
  useEffect(() => {
    const sampleLogs: LogFile[] = [
      {
        id: 'log1',
        name: 'Run_2025-10-26_BMW_E36_Baseline',
        timestamp: Date.now() - 86400000,
        duration: 323,
        sampleRate: 10,
        size: 2457600,
        channels: ['rpm', 'afr', 'map', 'tps', 'timing', 'clt', 'iat'],
        data: [],
        metadata: {
          vehicle: 'BMW E36 M3',
          ecu: 'Speeduino v0.4',
          notes: 'Baseline run before tuning',
          avgAFR: 14.3,
          maxBoost: 0,
          maxRPM: 6800
        }
      },
      {
        id: 'log2',
        name: 'Run_2025-10-26_BMW_E36_Stage1',
        timestamp: Date.now() - 43200000,
        duration: 298,
        sampleRate: 20,
        size: 3891200,
        channels: ['rpm', 'afr', 'map', 'tps', 'timing', 'boost', 'knock'],
        data: [],
        metadata: {
          vehicle: 'BMW E36 M3',
          ecu: 'Speeduino v0.4',
          notes: 'After Stage 1 tune - boost control active',
          avgAFR: 12.8,
          maxBoost: 8.5,
          maxRPM: 7200
        }
      },
      {
        id: 'log3',
        name: 'Dyno_Session_2025-10-25',
        timestamp: Date.now() - 172800000,
        duration: 456,
        sampleRate: 50,
        size: 12582912,
        channels: ['rpm', 'afr', 'map', 'tps', 'timing', 'boost', 'knock', 'idc'],
        data: [],
        metadata: {
          vehicle: 'BMW E36 M3',
          ecu: 'Speeduino v0.4',
          notes: 'Full dyno session - power pulls',
          avgAFR: 12.5,
          maxBoost: 12.3,
          maxRPM: 7500
        }
      }
    ];
    setLogFiles(sampleLogs);
  }, []);

  const handleChannelToggle = (channelId: string) => {
    setSelectedChannels(prev => 
      prev.includes(channelId)
        ? prev.filter(id => id !== channelId)
        : [...prev, channelId]
    );
  };

  const handleSelectAll = () => {
    setSelectedChannels(STANDARD_CHANNELS.map(c => c.id));
  };

  const handleDeselectAll = () => {
    setSelectedChannels([]);
  };

  const handleCaptureStopped = (data: LogDataPoint[]) => {
    if (data.length === 0) return;

    const newLog: LogFile = {
      id: `log-${Date.now()}`,
      name: `Capture_${new Date().toISOString().split('T')[0]}_${Date.now()}`,
      timestamp: Date.now(),
      duration: (data[data.length - 1].timestamp - data[0].timestamp) / 1000,
      sampleRate: 10,
      size: data.length * selectedChannels.length * 8,
      channels: selectedChannels,
      data: data,
      metadata: {
        vehicle: 'Unknown',
        ecu: 'Speeduino',
        notes: 'Live capture session'
      }
    };

    setLogFiles(prev => [newLog, ...prev]);
    setSelectedLog(newLog.id);
    setCurrentLogData(data);
    setActiveTab('analyze');
  };

  const handleDeleteLog = (logId: string) => {
    setLogFiles(prev => prev.filter(log => log.id !== logId));
    if (selectedLog === logId) {
      setSelectedLog(null);
      setCurrentLogData([]);
    }
  };

  const selectedLogFile = logFiles.find(log => log.id === selectedLog);

  return (
    <div className="h-full bg-[#1a1a1a] flex flex-col">
      {/* Header with tabs */}
      <div className="border-b border-[#333333] bg-[#0a0a0a]">
        <div className="p-4 pb-0">
          <div className="flex items-center justify-between mb-4">
            <div>
              <h2 className="text-xl text-[#E6EEF3]">Data Logging & Analysis</h2>
              <p className="text-sm text-[#888888] mt-1">
                MegaLogViewer-inspired logging suite for Speeduino
              </p>
            </div>
            <div className="flex items-center gap-2">
              <Badge variant="outline" className="border-[#00C4B4] text-[#00C4B4]">
                {logFiles.length} Logs
              </Badge>
              <Badge variant="outline" className="border-[#666666] text-[#888888]">
                {selectedChannels.length} Channels
              </Badge>
            </div>
          </div>

          <Tabs value={activeTab} onValueChange={(v) => setActiveTab(v as any)} className="w-full">
            <TabsList className="bg-[#1a1a1a] border-b border-[#333333]">
              <TabsTrigger value="browser" className="data-[state=active]:bg-[#2a2a2a] data-[state=active]:text-[#00C4B4]">
                <FileText className="w-4 h-4 mr-2" />
                Log Browser
              </TabsTrigger>
              <TabsTrigger value="capture" className="data-[state=active]:bg-[#2a2a2a] data-[state=active]:text-[#00C4B4]">
                <Play className="w-4 h-4 mr-2" />
                Live Capture
              </TabsTrigger>
              <TabsTrigger value="analyze" className="data-[state=active]:bg-[#2a2a2a] data-[state=active]:text-[#00C4B4]">
                <BarChart3 className="w-4 h-4 mr-2" />
                Analysis
              </TabsTrigger>
            </TabsList>
          </Tabs>
        </div>
      </div>

      {/* Content */}
      <div className="flex-1 overflow-hidden">
        <Tabs value={activeTab} className="h-full">
          {/* Log Browser */}
          <TabsContent value="browser" className="h-full m-0">
            <div className="h-full flex">
              {/* Left: File list */}
              <div className="w-80 border-r border-[#333333] flex flex-col">
                <div className="p-4 border-b border-[#333333]">
                  <h3 className="text-[#E6EEF3] mb-2">Saved Logs</h3>
                  <button className="w-full flex items-center justify-center gap-2 px-3 py-2 bg-[#00C4B4] hover:bg-[#00D4C4] text-black rounded transition-colors">
                    <Plus className="w-4 h-4" />
                    Import Log File
                  </button>
                </div>

                <ScrollArea className="flex-1">
                  <div className="p-3 space-y-2">
                    {logFiles.map((log) => (
                      <motion.button
                        key={log.id}
                        onClick={() => {
                          setSelectedLog(log.id);
                          setCurrentLogData(log.data);
                        }}
                        className={`w-full text-left p-3 rounded border transition-all ${
                          selectedLog === log.id
                            ? "bg-[#2a2a2a] border-[#00C4B4]"
                            : "bg-[#1a1a1a] border-[#333333] hover:border-[#444444]"
                        }`}
                        whileHover={{ scale: 1.02 }}
                      >
                        <div className="flex items-start justify-between gap-2 mb-2">
                          <div className="flex-1 min-w-0">
                            <div className="text-sm text-[#E6EEF3] truncate mb-1">
                              {log.name}
                            </div>
                            <div className="text-xs text-[#666666]">
                              {new Date(log.timestamp).toLocaleDateString()}
                            </div>
                          </div>
                          <FileText className="w-4 h-4 text-[#00C4B4] flex-shrink-0" />
                        </div>

                        <div className="flex items-center gap-2 text-xs text-[#888888]">
                          <span>{(log.size / 1024 / 1024).toFixed(1)} MB</span>
                          <span>•</span>
                          <span>{Math.floor(log.duration / 60)}:{String(Math.floor(log.duration % 60)).padStart(2, '0')}</span>
                          <span>•</span>
                          <span>{log.sampleRate}Hz</span>
                        </div>

                        {/* KPIs */}
                        {log.metadata && (
                          <div className="mt-2 pt-2 border-t border-[#333333] grid grid-cols-3 gap-2 text-xs">
                            {log.metadata.avgAFR && (
                              <div>
                                <div className="text-[#666666]">AFR</div>
                                <div className="text-[#E6EEF3] font-mono">
                                  {log.metadata.avgAFR.toFixed(1)}
                                </div>
                              </div>
                            )}
                            {log.metadata.maxBoost !== undefined && (
                              <div>
                                <div className="text-[#666666]">Boost</div>
                                <div className="text-[#E6EEF3] font-mono">
                                  {log.metadata.maxBoost.toFixed(1)}
                                </div>
                              </div>
                            )}
                            {log.metadata.maxRPM && (
                              <div>
                                <div className="text-[#666666]">RPM</div>
                                <div className="text-[#E6EEF3] font-mono">
                                  {log.metadata.maxRPM}
                                </div>
                              </div>
                            )}
                          </div>
                        )}
                      </motion.button>
                    ))}

                    {logFiles.length === 0 && (
                      <div className="text-center py-8 text-[#666666]">
                        <FileText className="w-12 h-12 mx-auto mb-3 opacity-30" />
                        <p className="text-sm">No log files</p>
                        <p className="text-xs mt-1">Import or capture a new log</p>
                      </div>
                    )}
                  </div>
                </ScrollArea>
              </div>

              {/* Right: Log details */}
              <div className="flex-1 p-6">
                {selectedLogFile ? (
                  <div className="space-y-4">
                    {/* Header */}
                    <div className="flex items-start justify-between">
                      <div>
                        <h3 className="text-xl text-[#E6EEF3] mb-2">{selectedLogFile.name}</h3>
                        <div className="flex items-center gap-3 text-sm text-[#888888]">
                          <span>{selectedLogFile.metadata.vehicle}</span>
                          <span>•</span>
                          <span>{selectedLogFile.metadata.ecu}</span>
                          <span>•</span>
                          <span>{new Date(selectedLogFile.timestamp).toLocaleString()}</span>
                        </div>
                      </div>
                      <div className="flex items-center gap-2">
                        <button className="p-2 bg-[#2a2a2a] hover:bg-[#333333] text-[#E6EEF3] rounded transition-colors">
                          <Download className="w-4 h-4" />
                        </button>
                        <button className="p-2 bg-[#2a2a2a] hover:bg-[#333333] text-[#E6EEF3] rounded transition-colors">
                          <Share2 className="w-4 h-4" />
                        </button>
                        <button
                          onClick={() => handleDeleteLog(selectedLogFile.id)}
                          className="p-2 bg-red-950/30 hover:bg-red-950/50 text-red-500 rounded transition-colors"
                        >
                          <Trash2 className="w-4 h-4" />
                        </button>
                      </div>
                    </div>

                    {/* Metadata cards */}
                    <div className="grid grid-cols-4 gap-4">
                      <div className="p-4 bg-[#0a0a0a] border border-[#333333] rounded">
                        <div className="text-xs text-[#666666] mb-1">Duration</div>
                        <div className="font-mono text-xl text-[#E6EEF3]">
                          {Math.floor(selectedLogFile.duration / 60)}:{String(Math.floor(selectedLogFile.duration % 60)).padStart(2, '0')}
                        </div>
                      </div>
                      <div className="p-4 bg-[#0a0a0a] border border-[#333333] rounded">
                        <div className="text-xs text-[#666666] mb-1">Sample Rate</div>
                        <div className="font-mono text-xl text-[#00C4B4]">
                          {selectedLogFile.sampleRate} Hz
                        </div>
                      </div>
                      <div className="p-4 bg-[#0a0a0a] border border-[#333333] rounded">
                        <div className="text-xs text-[#666666] mb-1">Channels</div>
                        <div className="font-mono text-xl text-[#E6EEF3]">
                          {selectedLogFile.channels.length}
                        </div>
                      </div>
                      <div className="p-4 bg-[#0a0a0a] border border-[#333333] rounded">
                        <div className="text-xs text-[#666666] mb-1">File Size</div>
                        <div className="font-mono text-xl text-[#E6EEF3]">
                          {(selectedLogFile.size / 1024 / 1024).toFixed(1)} MB
                        </div>
                      </div>
                    </div>

                    {/* Notes */}
                    {selectedLogFile.metadata.notes && (
                      <div className="p-4 bg-[#0a0a0a] border border-[#333333] rounded">
                        <div className="text-sm text-[#666666] mb-2">Notes</div>
                        <p className="text-[#E6EEF3]">{selectedLogFile.metadata.notes}</p>
                      </div>
                    )}

                    {/* Quick actions */}
                    <div className="flex items-center gap-3">
                      <button
                        onClick={() => setActiveTab('analyze')}
                        className="flex items-center gap-2 px-4 py-2 bg-[#00C4B4] hover:bg-[#00D4C4] text-black rounded transition-colors"
                      >
                        <BarChart3 className="w-4 h-4" />
                        Analyze
                      </button>
                      <button className="flex items-center gap-2 px-4 py-2 bg-[#2a2a2a] hover:bg-[#333333] text-[#E6EEF3] rounded transition-colors">
                        <TrendingUp className="w-4 h-4" />
                        VE Analyze
                      </button>
                      <button className="flex items-center gap-2 px-4 py-2 bg-[#2a2a2a] hover:bg-[#333333] text-[#E6EEF3] rounded transition-colors">
                        <Zap className="w-4 h-4" />
                        Find Anomalies
                      </button>
                    </div>
                  </div>
                ) : (
                  <div className="h-full flex items-center justify-center text-[#666666]">
                    <div className="text-center">
                      <FileText className="w-16 h-16 mx-auto mb-4 opacity-20" />
                      <p>Select a log file to view details</p>
                      <p className="text-sm mt-2">or import a new log from the left panel</p>
                    </div>
                  </div>
                )}
              </div>
            </div>
          </TabsContent>

          {/* Live Capture */}
          <TabsContent value="capture" className="h-full m-0">
            <div className="h-full flex">
              <div className="w-80">
                <ChannelSelector
                  selectedChannels={selectedChannels}
                  onChannelToggle={handleChannelToggle}
                  onSelectAll={handleSelectAll}
                  onDeselectAll={handleDeselectAll}
                />
              </div>
              <div className="w-80">
                <LiveCapture
                  selectedChannels={selectedChannels}
                  onCaptureStopped={handleCaptureStopped}
                />
              </div>
              <div className="flex-1 p-4">
                <LogGraph
                  data={currentLogData}
                  channels={selectedChannels}
                  title="Live Capture Preview"
                  height={600}
                />
              </div>
            </div>
          </TabsContent>

          {/* Analysis */}
          <TabsContent value="analyze" className="h-full m-0 p-4">
            <div className="h-full">
              <LogGraph
                data={currentLogData}
                channels={selectedChannels}
                title={selectedLogFile?.name || "Analysis"}
                height={700}
              />
            </div>
          </TabsContent>
        </Tabs>
      </div>
    </div>
  );
}
