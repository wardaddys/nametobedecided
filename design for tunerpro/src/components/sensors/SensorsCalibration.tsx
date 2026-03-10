import { useState } from "react";
import { Card } from "../ui/card";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "../ui/tabs";
import { Label } from "../ui/label";
import { Input } from "../ui/input";
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "../ui/select";
import { Button } from "../ui/button";
import { Checkbox } from "../ui/checkbox";
import { Badge } from "../ui/badge";
import { Alert, AlertDescription } from "../ui/alert";
import {
  Info,
  Zap,
  Settings,
  CheckCircle2,
  AlertCircle,
  AlertTriangle,
  Thermometer,
  Gauge,
  Activity,
} from "lucide-react";
import {
  triggerPatterns,
  cltCurves,
  mapSensors,
  tpsSensors,
  o2Sensors,
  recommendedConfigs,
} from "./SensorDatabase";
import {
  FIRING_ORDER_DATABASE,
  getFiringOrdersByCylinders,
  getFiringOrdersByConfig,
  getAvailableLayouts,
  SUPPORTED_CYLINDER_COUNTS,
  LAYOUT_DISPLAY_NAMES,
  FiringOrderDefinition,
} from "./FiringOrderDatabase";
import { FiringOrderSelector } from "./FiringOrderSelector";

interface SensorsCalibrationProps {
  liveTuning: boolean;
}

export function SensorsCalibration({ liveTuning }: SensorsCalibrationProps) {
  const [activeTab, setActiveTab] = useState<"wizard" | "trigger" | "engine" | "advanced">("wizard");

  // Trigger Configuration
  const [triggerPattern, setTriggerPattern] = useState("");
  const [primarySensorType, setPrimarySensorType] = useState<"hall" | "vr" | "">("");
  const [secondarySensorType, setSecondarySensorType] = useState<"hall" | "vr" | "none">("none");
  const [triggerAngle, setTriggerAngle] = useState("0");
  const [useVRConditioner, setUseVRConditioner] = useState(false);
  const [vrConditioner, setVRConditioner] = useState("max9926");

  // Engine Sensors
  const [cltCalibration, setCltCalibration] = useState("gm-clt");
  const [iatCalibration, setIatCalibration] = useState("gm-clt");
  const [mapSensor, setMapSensor] = useState("");
  const [tpsSensor, setTpsSensor] = useState("");
  const [o2Sensor, setO2Sensor] = useState("");

  // Validation States
  const [triggerValidated, setTriggerValidated] = useState(false);
  const [sensorsValidated, setSensorsValidated] = useState(false);

  // Get selected pattern info
  const selectedPattern = triggerPatterns.find(p => p.id === triggerPattern);
  const selectedMapSensor = mapSensors.find(s => s.id === mapSensor);

  const validateTriggerSetup = () => {
    const errors: string[] = [];
    
    if (!triggerPattern) errors.push("Select a trigger pattern");
    if (!primarySensorType) errors.push("Select primary sensor type");
    if (primarySensorType === "vr" && !useVRConditioner) {
      errors.push("VR sensors require signal conditioning (MAX9926 recommended)");
    }
    if (selectedPattern?.sequential && secondarySensorType === "none") {
      errors.push("Sequential operation requires cam sensor");
    }

    return errors;
  };

  const validateEngineSensors = () => {
    const errors: string[] = [];
    
    if (!cltCalibration) errors.push("Select CLT calibration");
    if (!iatCalibration) errors.push("Select IAT calibration");
    if (!mapSensor) errors.push("Select MAP sensor");
    if (!tpsSensor) errors.push("Select TPS sensor");
    if (!o2Sensor) errors.push("Select O2/Wideband sensor");

    return errors;
  };

  const triggerErrors = validateTriggerSetup();
  const sensorErrors = validateEngineSensors();

  return (
    <div className="h-full bg-[#1a1a1a] flex flex-col overflow-hidden">
      {/* Header */}
      <div className="flex-shrink-0 border-b-2 border-[#0088FF] bg-gradient-to-r from-[#0a0a14] via-[#12121e] to-[#0a0a14] px-4 py-3">
        <h2 className="text-white">Sensors & Trigger Configuration</h2>
        <p className="text-xs text-[#888888] mt-1">
          Complete sensor database for Speeduino & MegaSquirt ECUs - Configure all inputs properly to avoid issues
        </p>
      </div>

      {/* Main Tabs */}
      <Tabs value={activeTab} onValueChange={(v) => setActiveTab(v as any)} className="flex-1 flex flex-col min-h-0">
        <div className="flex-shrink-0 border-b border-[#333333] bg-black px-4">
          <TabsList className="bg-transparent border-0">
            <TabsTrigger value="wizard" className="data-[state=active]:bg-[#0088FF] data-[state=active]:text-white">
              🧙 Setup Wizard
            </TabsTrigger>
            <TabsTrigger value="trigger" className="data-[state=active]:bg-[#0088FF] data-[state=active]:text-white">
              ⚡ Trigger System
            </TabsTrigger>
            <TabsTrigger value="engine" className="data-[state=active]:bg-[#0088FF] data-[state=active]:text-white">
              🌡️ Engine Sensors
            </TabsTrigger>
            <TabsTrigger value="advanced" className="data-[state=active]:bg-[#0088FF] data-[state=active]:text-white">
              ⚙️ Advanced Config
            </TabsTrigger>
          </TabsList>
        </div>

        <div className="flex-1 overflow-y-auto overflow-x-hidden p-4 min-h-0">
          {/* WIZARD TAB */}
          <TabsContent value="wizard" className="m-0">
            <div className="max-w-5xl mx-auto space-y-4">
              <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6 slide-in-bottom smooth-transition hover:neon-glow-cyan">
                <h3 className="text-white mb-4 flex items-center gap-2">
                  <Info className="w-5 h-5 text-[#00CCFF] pulse-glow-cyan" />
                  Recommended Sensor Configurations
                </h3>
                <p className="text-xs text-[#888888] mb-4">
                  Select a preset configuration based on your engine type. You can customize individual sensors later.
                </p>

                <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                  {recommendedConfigs.map((config) => (
                    <Card
                      key={config.name}
                      className="glass-panel bg-black/60 border border-[#333333] p-4 hover:border-[#0088FF] hover:neon-glow-cyan cursor-pointer smooth-transition premium-button"
                      onClick={() => {
                        // Auto-populate based on config
                        if (config.name.includes("EDIS")) {
                          setTriggerPattern("ford-edis");
                          setPrimarySensorType("vr");
                          setUseVRConditioner(true);
                        } else if (config.name.includes("Turbo")) {
                          setTriggerPattern("missing-60-2");
                          setPrimarySensorType(config.name.includes("High") ? "hall" : "vr");
                          if (config.name.includes("High")) {
                            setMapSensor("gm-3bar");
                          } else {
                            setMapSensor("speeduino-2.5bar");
                          }
                        } else {
                          setTriggerPattern("missing-36-1");
                          setPrimarySensorType("hall");
                          setMapSensor("gm-1bar");
                        }
                        setSecondarySensorType("hall");
                        setCltCalibration("gm-clt");
                        setIatCalibration("gm-clt");
                        setTpsSensor("gm-3wire-tps");
                        setO2Sensor("bosch-lsu-4.9");
                      }}
                    >
                      <div className="flex items-start justify-between mb-2">
                        <h4 className="text-white font-semibold">{config.name}</h4>
                        <Badge
                          variant="outline"
                          className={
                            config.difficulty === "Beginner"
                              ? "border-[#00FF00] text-[#00FF00]"
                              : config.difficulty === "Intermediate"
                              ? "border-[#FFAA00] text-[#FFAA00]"
                              : "border-[#FF0000] text-[#FF0000]"
                          }
                        >
                          {config.difficulty}
                        </Badge>
                      </div>
                      <p className="text-xs text-[#888888] mb-3">{config.description}</p>
                      <div className="space-y-1 text-xs">
                        <div className="flex items-center gap-2">
                          <Zap className="w-3 h-3 text-[#00CCFF]" />
                          <span className="text-[#CCCCCC]">{config.primaryTrigger}</span>
                        </div>
                        <div className="flex items-center gap-2">
                          <Gauge className="w-3 h-3 text-[#00FF00]" />
                          <span className="text-[#CCCCCC]">{config.map}</span>
                        </div>
                        <div className="flex items-center gap-2">
                          <Activity className="w-3 h-3 text-[#FF9900]" />
                          <span className="text-[#CCCCCC]">{config.o2}</span>
                        </div>
                      </div>
                      <p className="text-xs text-[#666666] mt-3 italic">{config.notes}</p>
                    </Card>
                  ))}
                </div>
              </Card>

              {/* Firing Order Configuration - NEW SECTION */}
              <FiringOrderSelector onSelect={(fo) => console.log('Firing order selected:', fo)} />

              {/* Status Cards */}
              <div className="grid grid-cols-2 gap-4">
                <Card
                  className={`glass-panel p-4 smooth-transition scale-in-center ${
                    triggerErrors.length === 0
                      ? "bg-gradient-to-br from-[#00FF00]/10 to-[#00CC00]/10 border-[#00FF00] neon-glow-green"
                      : "bg-gradient-to-br from-[#FF0000]/10 to-[#CC0000]/10 border-[#FF0000] neon-glow-pink"
                  }`}
                >
                  <div className="flex items-center gap-3 mb-2">
                    {triggerErrors.length === 0 ? (
                      <CheckCircle2 className="w-6 h-6 text-[#00FF00]" />
                    ) : (
                      <AlertTriangle className="w-6 h-6 text-[#FF0000]" />
                    )}
                    <h4 className="text-white font-semibold">Trigger System</h4>
                  </div>
                  {triggerErrors.length === 0 ? (
                    <p className="text-xs text-[#00FF00]">✓ Configuration valid</p>
                  ) : (
                    <ul className="text-xs text-[#FF6666] space-y-1">
                      {triggerErrors.map((error, i) => (
                        <li key={i}>• {error}</li>
                      ))}
                    </ul>
                  )}
                </Card>

                <Card
                  className={`glass-panel p-4 smooth-transition scale-in-center ${
                    sensorErrors.length === 0
                      ? "bg-gradient-to-br from-[#00FF00]/10 to-[#00CC00]/10 border-[#00FF00] neon-glow-green"
                      : "bg-gradient-to-br from-[#FF0000]/10 to-[#CC0000]/10 border-[#FF0000] neon-glow-pink"
                  }`}
                >
                  <div className="flex items-center gap-3 mb-2">
                    {sensorErrors.length === 0 ? (
                      <CheckCircle2 className="w-6 h-6 text-[#00FF00]" />
                    ) : (
                      <AlertTriangle className="w-6 h-6 text-[#FF0000]" />
                    )}
                    <h4 className="text-white font-semibold">Engine Sensors</h4>
                  </div>
                  {sensorErrors.length === 0 ? (
                    <p className="text-xs text-[#00FF00]">✓ All sensors configured</p>
                  ) : (
                    <ul className="text-xs text-[#FF6666] space-y-1">
                      {sensorErrors.map((error, i) => (
                        <li key={i}>• {error}</li>
                      ))}
                    </ul>
                  )}
                </Card>
              </div>
            </div>
          </TabsContent>

          {/* TRIGGER SYSTEM TAB */}
          <TabsContent value="trigger" className="m-0">
            <div className="max-w-6xl mx-auto space-y-4">
              {/* Trigger Pattern Selection */}
              <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6 slide-in-bottom smooth-transition hover:neon-glow-cyan">
                <h3 className="text-white mb-4 flex items-center gap-2">
                  <Zap className="w-5 h-5 text-[#00CCFF] pulse-glow-cyan" />
                  Step 1: Select Trigger Pattern
                </h3>

                <div className="mb-4">
                  <Label className="text-[#CCCCCC]">Trigger Pattern</Label>
                  <Select value={triggerPattern} onValueChange={setTriggerPattern}>
                    <SelectTrigger className="bg-black border-[#555555] text-white">
                      <SelectValue placeholder="Choose your trigger wheel pattern..." />
                    </SelectTrigger>
                    <SelectContent className="bg-[#1a1a1a] border-[#555555]">
                      <div className="px-2 py-1.5 text-xs font-semibold text-[#00CCFF] uppercase">
                        Universal Patterns
                      </div>
                      {triggerPatterns
                        .filter((p) => p.id.startsWith("missing-") || p.id.startsWith("dual-"))
                        .map((pattern) => (
                          <SelectItem key={pattern.id} value={pattern.id} className="text-white">
                            <div className="flex items-center gap-2">
                              <span>{pattern.name}</span>
                              {pattern.difficulty === "Easy" && (
                                <Badge className="bg-[#00FF00] text-black text-xs">Easy</Badge>
                              )}
                            </div>
                          </SelectItem>
                        ))}

                      <div className="px-2 py-1.5 text-xs font-semibold text-[#00CCFF] uppercase mt-2">
                        OEM Patterns
                      </div>
                      {triggerPatterns
                        .filter((p) => !p.id.startsWith("missing-") && !p.id.startsWith("dual-"))
                        .map((pattern) => (
                          <SelectItem key={pattern.id} value={pattern.id} className="text-white">
                            {pattern.name}
                          </SelectItem>
                        ))}
                    </SelectContent>
                  </Select>
                </div>

                {selectedPattern && (
                  <Card className="bg-black/60 border border-[#333333] p-4">
                    <div className="grid grid-cols-2 gap-4">
                      <div>
                        <h4 className="text-sm text-[#00CCFF] mb-2">Pattern Details</h4>
                        <div className="space-y-1 text-xs">
                          <div className="flex justify-between">
                            <span className="text-[#888888]">Crank Teeth:</span>
                            <span className="text-white font-mono">{selectedPattern.crankTeeth}</span>
                          </div>
                          <div className="flex justify-between">
                            <span className="text-[#888888]">Cam Teeth:</span>
                            <span className="text-white font-mono">{selectedPattern.camTeeth}</span>
                          </div>
                          <div className="flex justify-between">
                            <span className="text-[#888888]">Resolution:</span>
                            <span className="text-white font-mono">{selectedPattern.resolution}</span>
                          </div>
                          <div className="flex justify-between">
                            <span className="text-[#888888]">Max RPM:</span>
                            <span className="text-white font-mono">{selectedPattern.maxRPM}</span>
                          </div>
                        </div>
                      </div>
                      <div>
                        <h4 className="text-sm text-[#00CCFF] mb-2">Capabilities</h4>
                        <div className="space-y-1 text-xs">
                          <div className="flex items-center gap-2">
                            {selectedPattern.sequential ? (
                              <CheckCircle2 className="w-3 h-3 text-[#00FF00]" />
                            ) : (
                              <AlertCircle className="w-3 h-3 text-[#888888]" />
                            )}
                            <span className="text-[#CCCCCC]">Sequential Capable</span>
                          </div>
                          <div className="flex items-center gap-2">
                            {selectedPattern.speeduino ? (
                              <CheckCircle2 className="w-3 h-3 text-[#00FF00]" />
                            ) : (
                              <AlertCircle className="w-3 h-3 text-[#FF0000]" />
                            )}
                            <span className="text-[#CCCCCC]">Speeduino Support</span>
                          </div>
                          <div className="flex items-center gap-2">
                            {selectedPattern.megasquirt ? (
                              <CheckCircle2 className="w-3 h-3 text-[#00FF00]" />
                            ) : (
                              <AlertCircle className="w-3 h-3 text-[#FF0000]" />
                            )}
                            <span className="text-[#CCCCCC]">MegaSquirt Support</span>
                          </div>
                        </div>
                        <Alert className="mt-3 bg-[#0088FF]/10 border-[#0088FF]">
                          <Info className="w-4 h-4" />
                          <AlertDescription className="text-xs">{selectedPattern.notes}</AlertDescription>
                        </Alert>
                      </div>
                    </div>
                  </Card>
                )}
              </Card>

              {/* Sensor Type Selection */}
              <Card className="bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
                <h3 className="text-white mb-4 flex items-center gap-2">
                  <Settings className="w-5 h-5 text-[#00CCFF]" />
                  Step 2: Sensor Types
                </h3>

                <div className="grid grid-cols-2 gap-6">
                  {/* Primary Sensor */}
                  <div>
                    <Label className="text-[#CCCCCC]">Primary Sensor (Crank)</Label>
                    <div className="grid grid-cols-2 gap-2 mt-2">
                      <Button
                        variant={primarySensorType === "hall" ? "default" : "outline"}
                        onClick={() => {
                          setPrimarySensorType("hall");
                          setUseVRConditioner(false);
                        }}
                        className={
                          primarySensorType === "hall"
                            ? "bg-[#0088FF] text-white"
                            : "border-[#555555] text-[#CCCCCC]"
                        }
                      >
                        Hall Effect
                      </Button>
                      <Button
                        variant={primarySensorType === "vr" ? "default" : "outline"}
                        onClick={() => {
                          setPrimarySensorType("vr");
                          setUseVRConditioner(true);
                        }}
                        className={
                          primarySensorType === "vr" ? "bg-[#0088FF] text-white" : "border-[#555555] text-[#CCCCCC]"
                        }
                      >
                        VR (Variable Reluctance)
                      </Button>
                    </div>

                    {primarySensorType === "hall" && (
                      <Card className="mt-3 bg-black/40 border border-[#00FF00]/30 p-3">
                        <h5 className="text-xs text-[#00FF00] font-semibold mb-2">✓ Hall Effect Advantages:</h5>
                        <ul className="text-xs text-[#CCCCCC] space-y-1">
                          <li>• Clean digital signal</li>
                          <li>• Works at 0 RPM (hand cranking)</li>
                          <li>• No conditioning required</li>
                          <li>• Forgiving air gap (1mm typical)</li>
                          <li>• Easy to configure</li>
                        </ul>
                        <h5 className="text-xs text-[#FF9900] font-semibold mb-2 mt-3">! Considerations:</h5>
                        <ul className="text-xs text-[#CCCCCC] space-y-1">
                          <li>• Temperature limited (125°C max)</li>
                          <li>• Requires power supply</li>
                          <li>• Thicker trigger teeth needed</li>
                        </ul>
                      </Card>
                    )}

                    {primarySensorType === "vr" && (
                      <Card className="mt-3 bg-black/40 border border-[#FF9900]/30 p-3">
                        <h5 className="text-xs text-[#00FF00] font-semibold mb-2">✓ VR Advantages:</h5>
                        <ul className="text-xs text-[#CCCCCC] space-y-1">
                          <li>• Extremely rugged & reliable</li>
                          <li>• High temperature (200°C+)</li>
                          <li>• No power required (passive)</li>
                          <li>• Works with thin wheels</li>
                        </ul>
                        <h5 className="text-xs text-[#FF0000] font-semibold mb-2 mt-3">⚠ CRITICAL Requirements:</h5>
                        <ul className="text-xs text-[#CCCCCC] space-y-1">
                          <li>• MUST use VR conditioner (MAX9926)</li>
                          <li>• Precise air gap (0.25-0.50mm)</li>
                          <li>• Polarity must be correct</li>
                          <li>• Won't work at 0 RPM</li>
                        </ul>

                        <div className="mt-3 flex items-center gap-2">
                          <Checkbox
                            id="vr-conditioner"
                            checked={useVRConditioner}
                            onCheckedChange={(checked) => setUseVRConditioner(checked as boolean)}
                          />
                          <Label htmlFor="vr-conditioner" className="text-xs text-white">
                            I am using a VR signal conditioner (MAX9926 recommended)
                          </Label>
                        </div>

                        {useVRConditioner && (
                          <div className="mt-2">
                            <Label className="text-xs text-[#888888]">VR Conditioner Type</Label>
                            <Select value={vrConditioner} onValueChange={setVRConditioner}>
                              <SelectTrigger className="h-8 bg-black border-[#555555] text-white text-xs">
                                <SelectValue />
                              </SelectTrigger>
                              <SelectContent>
                                <SelectItem value="max9926">
                                  MAX9926 (Recommended - Adaptive threshold, excellent noise rejection)
                                </SelectItem>
                                <SelectItem value="lm1815">
                                  LM1815 (Older - Manual threshold, less reliable)
                                </SelectItem>
                                <SelectItem value="oem">OEM Module (EDIS, HEI, DIS)</SelectItem>
                              </SelectContent>
                            </Select>
                          </div>
                        )}
                      </Card>
                    )}
                  </div>

                  {/* Secondary Sensor */}
                  <div>
                    <Label className="text-[#CCCCCC]">Secondary Sensor (Cam) - Optional</Label>
                    <div className="grid grid-cols-3 gap-2 mt-2">
                      <Button
                        variant={secondarySensorType === "hall" ? "default" : "outline"}
                        onClick={() => setSecondarySensorType("hall")}
                        className={
                          secondarySensorType === "hall"
                            ? "bg-[#0088FF] text-white"
                            : "border-[#555555] text-[#CCCCCC]"
                        }
                      >
                        Hall
                      </Button>
                      <Button
                        variant={secondarySensorType === "vr" ? "default" : "outline"}
                        onClick={() => setSecondarySensorType("vr")}
                        className={
                          secondarySensorType === "vr" ? "bg-[#0088FF] text-white" : "border-[#555555] text-[#CCCCCC]"
                        }
                      >
                        VR
                      </Button>
                      <Button
                        variant={secondarySensorType === "none" ? "default" : "outline"}
                        onClick={() => setSecondarySensorType("none")}
                        className={
                          secondarySensorType === "none"
                            ? "bg-[#666666] text-white"
                            : "border-[#555555] text-[#CCCCCC]"
                        }
                      >
                        None
                      </Button>
                    </div>

                    <Card className="mt-3 bg-black/40 border border-[#0088FF]/30 p-3">
                      <h5 className="text-xs text-[#00CCFF] font-semibold mb-2">Cam Sensor Purpose:</h5>
                      <ul className="text-xs text-[#CCCCCC] space-y-1">
                        <li>• Provides cylinder identification</li>
                        <li>• Required for sequential fuel injection</li>
                        <li>• Required for coil-on-plug ignition</li>
                        <li>• Optional for wasted spark/batch fire</li>
                      </ul>

                      {secondarySensorType === "none" && (
                        <Alert className="mt-3 bg-[#FF9900]/10 border-[#FF9900]">
                          <AlertTriangle className="w-4 h-4" />
                          <AlertDescription className="text-xs">
                            Without cam sensor: Batch fire injection and wasted spark ignition only
                          </AlertDescription>
                        </Alert>
                      )}
                    </Card>
                  </div>
                </div>
              </Card>

              {/* Trigger Angle */}
              <Card className="bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
                <h3 className="text-white mb-4">Step 3: Trigger Angle</h3>
                <div className="grid grid-cols-2 gap-4">
                  <div>
                    <Label className="text-[#CCCCCC]">Trigger Angle (degrees ATDC)</Label>
                    <Input
                      type="number"
                      value={triggerAngle}
                      onChange={(e) => setTriggerAngle(e.target.value)}
                      className="bg-black border-[#555555] text-white font-mono"
                    />
                    <p className="text-xs text-[#888888] mt-1">
                      Crank degrees AFTER TDC #1 when first tooth passes sensor
                    </p>
                  </div>
                  <Card className="bg-black/40 border border-[#0088FF]/30 p-3">
                    <h5 className="text-xs text-[#00CCFF] font-semibold mb-2">How to Measure:</h5>
                    <ol className="text-xs text-[#CCCCCC] space-y-1 list-decimal list-inside">
                      <li>Rotate engine to TDC #1 (both valves closed)</li>
                      <li>Mark trigger wheel position at sensor</li>
                      <li>Rotate to first tooth after missing teeth</li>
                      <li>Measure degrees between marks</li>
                      <li>Enter value (can be negative)</li>
                    </ol>
                  </Card>
                </div>
              </Card>

              {/* Firing Order Configuration - NEW SECTION */}
              <FiringOrderSelector onSelect={(fo) => console.log('Firing order selected:', fo)} />

              {/* Status Cards */}
              <div className="grid grid-cols-2 gap-4">
                <Card
                  className={`glass-panel p-4 smooth-transition scale-in-center ${
                    triggerErrors.length === 0
                      ? "bg-gradient-to-br from-[#00FF00]/10 to-[#00CC00]/10 border-[#00FF00] neon-glow-green"
                      : "bg-gradient-to-br from-[#FF0000]/10 to-[#CC0000]/10 border-[#FF0000] neon-glow-pink"
                  }`}
                >
                  <div className="flex items-center gap-3 mb-2">
                    {triggerErrors.length === 0 ? (
                      <CheckCircle2 className="w-6 h-6 text-[#00FF00]" />
                    ) : (
                      <AlertTriangle className="w-6 h-6 text-[#FF0000]" />
                    )}
                    <h4 className="text-white font-semibold">Trigger System</h4>
                  </div>
                  {triggerErrors.length === 0 ? (
                    <p className="text-xs text-[#00FF00]">✓ Configuration valid</p>
                  ) : (
                    <ul className="text-xs text-[#FF6666] space-y-1">
                      {triggerErrors.map((error, i) => (
                        <li key={i}>• {error}</li>
                      ))}
                    </ul>
                  )}
                </Card>

                <Card
                  className={`glass-panel p-4 smooth-transition scale-in-center ${
                    sensorErrors.length === 0
                      ? "bg-gradient-to-br from-[#00FF00]/10 to-[#00CC00]/10 border-[#00FF00] neon-glow-green"
                      : "bg-gradient-to-br from-[#FF0000]/10 to-[#CC0000]/10 border-[#FF0000] neon-glow-pink"
                  }`}
                >
                  <div className="flex items-center gap-3 mb-2">
                    {sensorErrors.length === 0 ? (
                      <CheckCircle2 className="w-6 h-6 text-[#00FF00]" />
                    ) : (
                      <AlertTriangle className="w-6 h-6 text-[#FF0000]" />
                    )}
                    <h4 className="text-white font-semibold">Engine Sensors</h4>
                  </div>
                  {sensorErrors.length === 0 ? (
                    <p className="text-xs text-[#00FF00]">✓ All sensors configured</p>
                  ) : (
                    <ul className="text-xs text-[#FF6666] space-y-1">
                      {sensorErrors.map((error, i) => (
                        <li key={i}>• {error}</li>
                      ))}
                    </ul>
                  )}
                </Card>
              </div>
            </div>
          </TabsContent>

          {/* ENGINE SENSORS TAB */}
          <TabsContent value="engine" className="m-0">
            <div className="max-w-6xl mx-auto space-y-4">
              {/* Temperature Sensors */}
              <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6 smooth-transition hover:neon-glow-cyan">
                <h3 className="neon-text-gold mb-4 flex items-center gap-2">
                  <Thermometer className="w-5 h-5 text-[#FF6600] floating" />
                  Temperature Sensors (CLT / IAT)
                </h3>

                <div className="grid grid-cols-2 gap-6">
                  <div>
                    <Label className="text-[#CCCCCC]">Coolant Temperature (CLT) Calibration</Label>
                    <Select value={cltCalibration} onValueChange={setCltCalibration}>
                      <SelectTrigger className="bg-black border-[#555555] text-white">
                        <SelectValue placeholder="Select CLT calibration..." />
                      </SelectTrigger>
                      <SelectContent>
                        {cltCurves.map((curve) => (
                          <SelectItem key={curve.id} value={curve.id}>
                            {curve.name}
                          </SelectItem>
                        ))}
                      </SelectContent>
                    </Select>
                    <p className="text-xs text-[#888888] mt-1">
                      NTC thermistor - Resistance decreases as temperature increases
                    </p>
                  </div>

                  <div>
                    <Label className="text-[#CCCCCC]">Intake Air Temperature (IAT) Calibration</Label>
                    <Select value={iatCalibration} onValueChange={setIatCalibration}>
                      <SelectTrigger className="bg-black border-[#555555] text-white">
                        <SelectValue placeholder="Select IAT calibration..." />
                      </SelectTrigger>
                      <SelectContent>
                        {cltCurves.map((curve) => (
                          <SelectItem key={curve.id} value={curve.id}>
                            {curve.name}
                          </SelectItem>
                        ))}
                      </SelectContent>
                    </Select>
                    <p className="text-xs text-[#888888] mt-1">
                      Same sensor type as CLT - Install post-intercooler, pre-throttle
                    </p>
                  </div>
                </div>

                <Alert className="mt-4 bg-[#00CCFF]/10 border-[#00CCFF]">
                  <Info className="w-4 h-4" />
                  <AlertDescription className="text-xs">
                    <strong>Recommended:</strong> GM sensors (PN: 12146312 CLT, same for IAT). Most popular,
                    well-documented, ~$15-25 each. 2-wire NTC thermistor, works with all ECUs.
                  </AlertDescription>
                </Alert>
              </Card>

              {/* MAP Sensor */}
              <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#00FF88]/40 p-6 smooth-transition hover:neon-glow-green">
                <h3 className="neon-text-green mb-4 flex items-center gap-2">
                  <Gauge className="w-5 h-5 text-[#00FF00] pulse-glow-green" />
                  Manifold Absolute Pressure (MAP) Sensor
                </h3>

                <div className="mb-4">
                  <Label className="text-[#CCCCCC]">MAP Sensor Range</Label>
                  <Select value={mapSensor} onValueChange={setMapSensor}>
                    <SelectTrigger className="bg-black border-[#555555] text-white">
                      <SelectValue placeholder="Select MAP sensor..." />
                    </SelectTrigger>
                    <SelectContent>
                      <div className="px-2 py-1.5 text-xs font-semibold text-[#00CCFF] uppercase">
                        Naturally Aspirated
                      </div>
                      <SelectItem value="gm-1bar">
                        GM 1-Bar (NA Only) - 10-100 kPa - Highest NA resolution
                      </SelectItem>

                      <div className="px-2 py-1.5 text-xs font-semibold text-[#00CCFF] uppercase mt-2">
                        Mild Boost (Street)
                      </div>
                      <SelectItem value="gm-2bar">GM 2-Bar - Up to 14.7 psi boost - Good for street turbo</SelectItem>
                      <SelectItem value="speeduino-2.5bar">
                        Speeduino 2.5-Bar (Onboard) - Up to 22 psi - Most versatile
                      </SelectItem>

                      <div className="px-2 py-1.5 text-xs font-semibold text-[#00CCFF] uppercase mt-2">
                        High Boost (Performance/Race)
                      </div>
                      <SelectItem value="gm-3bar">
                        GM 3-Bar - Up to 29 psi - Gold standard aftermarket
                      </SelectItem>
                      <SelectItem value="bosch-3.5bar">Bosch 3.5-Bar - Up to 36 psi - European diesels</SelectItem>
                      <SelectItem value="speeduino-4bar">
                        Speeduino 4-Bar (Onboard) - Up to 43 psi - Drag racing
                      </SelectItem>
                    </SelectContent>
                  </Select>
                </div>

                {selectedMapSensor && (
                  <Card className="glass-panel bg-black/60 border border-[#00FF00]/30 p-4 fade-in neon-glow-green">
                    <div className="grid grid-cols-2 gap-4">
                      <div>
                        <h5 className="text-sm text-[#00FF00] mb-2">Sensor Specifications</h5>
                        <div className="space-y-1 text-xs">
                          <div className="flex justify-between">
                            <span className="text-[#888888]">Manufacturer:</span>
                            <span className="text-white">{selectedMapSensor.manufacturer}</span>
                          </div>
                          <div className="flex justify-between">
                            <span className="text-[#888888]">Part Number:</span>
                            <span className="text-white font-mono">{selectedMapSensor.partNumber}</span>
                          </div>
                          <div className="flex justify-between">
                            <span className="text-[#888888]">Output Range:</span>
                            <span className="text-white font-mono">{selectedMapSensor.voltage}</span>
                          </div>
                          <div className="flex justify-between">
                            <span className="text-[#888888]">Pressure Range:</span>
                            <span className="text-white font-mono">{selectedMapSensor.range}</span>
                          </div>
                        </div>
                      </div>
                      <div>
                        <h5 className="text-sm text-[#00CCFF] mb-2">Installation Notes</h5>
                        <p className="text-xs text-[#CCCCCC]">{selectedMapSensor.notes}</p>
                      </div>
                    </div>
                  </Card>
                )}

                <Alert className="mt-4 bg-[#FF9900]/10 border-[#FF9900]">
                  <AlertTriangle className="w-4 h-4" />
                  <AlertDescription className="text-xs">
                    <strong>CRITICAL:</strong> Use 3-4mm vacuum hose. Keep line short (&lt;2 feet). No leaks! Route
                    away from heat. Vacuum source must be post-throttle in intake manifold plenum.
                  </AlertDescription>
                </Alert>
              </Card>

              {/* TPS */}
              <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#00D4FF]/40 p-6 smooth-transition hover:neon-glow-cyan">
                <h3 className="neon-text-cyan mb-4 flex items-center gap-2">
                  <Activity className="w-5 h-5 text-[#0088FF] pulse-glow-cyan" />
                  Throttle Position Sensor (TPS)
                </h3>

                <div className="grid grid-cols-2 gap-4">
                  <div>
                    <Label className="text-[#CCCCCC]">TPS Type</Label>
                    <Select value={tpsSensor} onValueChange={setTpsSensor}>
                      <SelectTrigger className="bg-black border-[#555555] text-white">
                        <SelectValue placeholder="Select TPS..." />
                      </SelectTrigger>
                      <SelectContent>
                        {tpsSensors.map((sensor) => (
                          <SelectItem key={sensor.id} value={sensor.id}>
                            {sensor.name} - {sensor.notes}
                          </SelectItem>
                        ))}
                      </SelectContent>
                    </Select>
                    <p className="text-xs text-[#888888] mt-1">
                      3-wire potentiometer: +5V, Signal, Ground. Calibration required after installation.
                    </p>
                  </div>

                  <Card className="bg-black/60 border border-[#0088FF]/30 p-3">
                    <h5 className="text-xs text-[#0088FF] font-semibold mb-2">Calibration Procedure:</h5>
                    <ol className="text-xs text-[#CCCCCC] space-y-1 list-decimal list-inside">
                      <li>Key ON, engine OFF</li>
                      <li>Close throttle fully → Set 0% (0.5-1.0V)</li>
                      <li>Open throttle fully → Set 100% (4.0-4.8V)</li>
                      <li>Verify smooth 0-100% sweep</li>
                      <li>No jumps or dead spots</li>
                    </ol>
                  </Card>
                </div>
              </Card>

              {/* O2/Wideband */}
              <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#FF9900]/40 p-6 smooth-transition hover:shadow-[0_0_30px_rgba(255,153,0,0.3)]">
                <h3 className="text-white mb-4 flex items-center gap-2">
                  <Activity className="w-5 h-5 text-[#FF6600]" />
                  Oxygen / Wideband Sensor
                </h3>

                <div className="mb-4">
                  <Label className="text-[#CCCCCC]">Wideband O2 Sensor</Label>
                  <Select value={o2Sensor} onValueChange={setO2Sensor}>
                    <SelectTrigger className="bg-black border-[#555555] text-white">
                      <SelectValue placeholder="Select wideband sensor..." />
                    </SelectTrigger>
                    <SelectContent>
                      {o2Sensors.map((sensor) => (
                        <SelectItem key={sensor.id} value={sensor.id}>
                          {sensor.name} - {sensor.range} - {sensor.notes}
                        </SelectItem>
                      ))}
                    </SelectContent>
                  </Select>
                </div>

                <Alert className="bg-[#FF0000]/10 border-[#FF0000]">
                  <AlertTriangle className="w-4 h-4" />
                  <AlertDescription className="text-xs">
                    <strong>REQUIRES CONTROLLER:</strong> Wideband sensors need dedicated controller (AEM UEGO, Innovate
                    LC-2, etc.). Controller heats sensor and outputs 0-5V analog signal to ECU. Sensor alone will not
                    work!
                  </AlertDescription>
                </Alert>
              </Card>
            </div>
          </TabsContent>

          {/* ADVANCED CONFIG TAB */}
          <TabsContent value="advanced" className="m-0">
            <div className="max-w-6xl mx-auto">
              <Card className="bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
                <h3 className="text-white mb-4">Advanced Trigger Settings</h3>
                <div className="grid grid-cols-2 gap-4">
                  <div>
                    <Label className="text-[#CCCCCC] text-sm">Trigger Edge</Label>
                    <Select defaultValue="rising">
                      <SelectTrigger className="bg-black border-[#555555] text-white">
                        <SelectValue />
                      </SelectTrigger>
                      <SelectContent>
                        <SelectItem value="rising">Rising Edge (VR with MAX9926)</SelectItem>
                        <SelectItem value="falling">Falling Edge</SelectItem>
                      </SelectContent>
                    </Select>
                  </div>
                  <div>
                    <Label className="text-[#CCCCCC] text-sm">Trigger Filter</Label>
                    <Select defaultValue="medium">
                      <SelectTrigger className="bg-black border-[#555555] text-white">
                        <SelectValue />
                      </SelectTrigger>
                      <SelectContent>
                        <SelectItem value="low">Low (Clean signal)</SelectItem>
                        <SelectItem value="medium">Medium (Recommended)</SelectItem>
                        <SelectItem value="aggressive">Aggressive (Noisy environment)</SelectItem>
                      </SelectContent>
                    </Select>
                  </div>
                  <div>
                    <Label className="text-[#CCCCCC] text-sm">Skip Revolutions</Label>
                    <Select defaultValue="0">
                      <SelectTrigger className="bg-black border-[#555555] text-white">
                        <SelectValue />
                      </SelectTrigger>
                      <SelectContent>
                        <SelectItem value="0">0 (Default)</SelectItem>
                        <SelectItem value="1">1</SelectItem>
                        <SelectItem value="2">2</SelectItem>
                      </SelectContent>
                    </Select>
                  </div>
                  <div className="flex items-center gap-2">
                    <Checkbox id="resync" defaultChecked />
                    <Label htmlFor="resync" className="text-sm text-white">
                      Re-sync Every Cycle
                    </Label>
                  </div>
                </div>

                <Alert className="mt-6 bg-[#FF9900]/10 border-[#FF9900]">
                  <AlertTriangle className="w-4 h-4" />
                  <AlertDescription className="text-xs">
                    <strong>WARNING:</strong> Advanced settings should only be changed if you understand their purpose.
                    Incorrect values can prevent engine from starting or cause sync loss at high RPM.
                  </AlertDescription>
                </Alert>
              </Card>
            </div>
          </TabsContent>
        </div>
      </Tabs>

      {/* Bottom Action Bar */}
      <div className="border-t-2 border-[#333333] bg-black p-4">
        <div className="max-w-6xl mx-auto flex items-center justify-between">
          <div className="flex items-center gap-4">
            {triggerErrors.length === 0 && sensorErrors.length === 0 ? (
              <div className="flex items-center gap-2 text-[#00FF00]">
                <CheckCircle2 className="w-5 h-5" />
                <span className="text-sm font-semibold">Configuration Complete - Ready to Save</span>
              </div>
            ) : (
              <div className="flex items-center gap-2 text-[#FF9900]">
                <AlertTriangle className="w-5 h-5" />
                <span className="text-sm font-semibold">
                  {triggerErrors.length + sensorErrors.length} issue(s) need attention
                </span>
              </div>
            )}
          </div>
          <div className="flex gap-2">
            <Button variant="outline" className="border-[#555555]">
              Reset to Defaults
            </Button>
            <Button
              className="bg-gradient-to-r from-[#0088FF] to-[#00CCFF] hover:from-[#0066CC] hover:to-[#00AACC]"
              disabled={triggerErrors.length > 0 || sensorErrors.length > 0}
            >
              Save Configuration
            </Button>
          </div>
        </div>
      </div>
    </div>
  );
}