import { useState } from "react";
import { Info } from "lucide-react";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "../ui/tabs";
import { Label } from "../ui/label";
import { Input } from "../ui/input";
import { Switch } from "../ui/switch";
import { Slider } from "../ui/slider";
import { RadioGroup, RadioGroupItem } from "../ui/radio-group";
import {
  Tooltip,
  TooltipContent,
  TooltipProvider,
  TooltipTrigger,
} from "../ui/tooltip";
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "../ui/select";
import { useTheme } from "../context/ThemeContext";
import { useStatusBoxes } from "../context/StatusBoxContext";

interface EngineParametersProps {
  liveTuning: boolean;
}

export function EngineParameters({ liveTuning }: EngineParametersProps) {
  const [revLimit, setRevLimit] = useState([8000]);
  const [cutType, setCutType] = useState("fuel");
  const [launchControl, setLaunchControl] = useState(false);
  const [boostControl, setBoostControl] = useState(true);
  const { theme, setTheme } = useTheme();
  const { statusBoxes, updateStatusBox, toggleStatusBox } = useStatusBoxes();

  return (
    <div className="h-full bg-[#1a1a1a] p-6 overflow-auto">
      <div className="mb-6">
        <h2 className="text-[#E6EEF3] mb-1">Engine Parameters & Safeties</h2>
        <p className="text-sm text-[#AFC6D2]">
          Configure engine constants, features, and safety limits
        </p>
      </div>

      <Tabs defaultValue="constants" className="w-full">
        <TabsList className="grid w-full max-w-2xl grid-cols-5">
          <TabsTrigger value="constants">Engine Constants</TabsTrigger>
          <TabsTrigger value="features">Features & Safeties</TabsTrigger>
          <TabsTrigger value="outputs">Outputs</TabsTrigger>
          <TabsTrigger value="statusboxes">Status Boxes</TabsTrigger>
          <TabsTrigger value="appearance">Appearance</TabsTrigger>
        </TabsList>

        {/* Engine Constants Tab */}
        <TabsContent value="constants" className="space-y-6 mt-6">
          <div className="glassmorphism rounded-lg p-6 space-y-6">
            <div className="grid grid-cols-2 gap-6">
              <div className="space-y-2">
                <div className="flex items-center gap-2">
                  <Label>Displacement</Label>
                  <TooltipProvider>
                    <Tooltip>
                      <TooltipTrigger>
                        <Info className="w-3 h-3 text-[#AFC6D2]" />
                      </TooltipTrigger>
                      <TooltipContent>
                        <p>Engine displacement in liters</p>
                      </TooltipContent>
                    </Tooltip>
                  </TooltipProvider>
                </div>
                <Input type="number" defaultValue="2.0" step="0.1" />
              </div>

              <div className="space-y-2">
                <div className="flex items-center gap-2">
                  <Label>Number of Cylinders</Label>
                </div>
                <Select defaultValue="4">
                  <SelectTrigger>
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="3">3</SelectItem>
                    <SelectItem value="4">4</SelectItem>
                    <SelectItem value="5">5</SelectItem>
                    <SelectItem value="6">6</SelectItem>
                    <SelectItem value="8">8</SelectItem>
                    <SelectItem value="10">10</SelectItem>
                    <SelectItem value="12">12</SelectItem>
                  </SelectContent>
                </Select>
              </div>

              <div className="space-y-2">
                <div className="flex items-center gap-2">
                  <Label>Firing Order</Label>
                  <TooltipProvider>
                    <Tooltip>
                      <TooltipTrigger>
                        <Info className="w-3 h-3 text-[#AFC6D2]" />
                      </TooltipTrigger>
                      <TooltipContent>
                        <p>Cylinder firing sequence</p>
                      </TooltipContent>
                    </Tooltip>
                  </TooltipProvider>
                </div>
                <Input defaultValue="1-3-4-2" />
              </div>

              <div className="space-y-2">
                <div className="flex items-center gap-2">
                  <Label>Stroke Type</Label>
                </div>
                <Select defaultValue="4-stroke">
                  <SelectTrigger>
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="2-stroke">2-Stroke</SelectItem>
                    <SelectItem value="4-stroke">4-Stroke</SelectItem>
                  </SelectContent>
                </Select>
              </div>

              <div className="space-y-2">
                <div className="flex items-center gap-2">
                  <Label>Injector Size (cc/min)</Label>
                  <TooltipProvider>
                    <Tooltip>
                      <TooltipTrigger>
                        <Info className="w-3 h-3 text-[#AFC6D2]" />
                      </TooltipTrigger>
                      <TooltipContent>
                        <p>Fuel injector flow rate at reference pressure</p>
                      </TooltipContent>
                    </Tooltip>
                  </TooltipProvider>
                </div>
                <Input type="number" defaultValue="550" />
              </div>

              <div className="space-y-2">
                <div className="flex items-center gap-2">
                  <Label>Fuel Pressure (kPa)</Label>
                </div>
                <Input type="number" defaultValue="300" />
              </div>
            </div>
          </div>

          <div className="glassmorphism rounded-lg p-6 space-y-4">
            <h3 className="text-[#E6EEF3]">Engine Type & Configuration</h3>

            <div className="space-y-2">
              <Label>Aspiration</Label>
              <RadioGroup defaultValue="turbo">
                <div className="flex items-center space-x-2">
                  <RadioGroupItem value="na" id="na" />
                  <Label htmlFor="na" className="cursor-pointer">
                    Naturally Aspirated
                  </Label>
                </div>
                <div className="flex items-center space-x-2">
                  <RadioGroupItem value="turbo" id="turbo" />
                  <Label htmlFor="turbo" className="cursor-pointer">
                    Turbocharged
                  </Label>
                </div>
                <div className="flex items-center space-x-2">
                  <RadioGroupItem value="super" id="super" />
                  <Label htmlFor="super" className="cursor-pointer">
                    Supercharged
                  </Label>
                </div>
              </RadioGroup>
            </div>
          </div>
        </TabsContent>

        {/* Features & Safeties Tab */}
        <TabsContent value="features" className="space-y-6 mt-6">
          <div className="glassmorphism rounded-lg p-6">
            <h3 className="text-[#E6EEF3] mb-6">Rev Limiter</h3>

            <div className="space-y-6">
              <div>
                <div className="flex items-center justify-between mb-4">
                  <Label>
                    Rev Limit: {revLimit[0].toLocaleString()} RPM
                  </Label>
                  <Input
                    type="number"
                    value={revLimit[0]}
                    onChange={(e) => setRevLimit([parseInt(e.target.value) || 8000])}
                    className="w-32 font-data"
                  />
                </div>
                <Slider
                  value={revLimit}
                  onValueChange={setRevLimit}
                  min={5000}
                  max={9500}
                  step={100}
                />
              </div>

              <div>
                <Label className="mb-3 block">Cut Type</Label>
                <RadioGroup value={cutType} onValueChange={setCutType}>
                  <div className="flex items-center space-x-2">
                    <RadioGroupItem value="fuel" id="fuel-cut" />
                    <Label htmlFor="fuel-cut" className="cursor-pointer">
                      Fuel Cut
                    </Label>
                  </div>
                  <div className="flex items-center space-x-2">
                    <RadioGroupItem value="ignition" id="ign-cut" />
                    <Label htmlFor="ign-cut" className="cursor-pointer">
                      Ignition Cut
                    </Label>
                  </div>
                  <div className="flex items-center space-x-2">
                    <RadioGroupItem value="both" id="both-cut" />
                    <Label htmlFor="both-cut" className="cursor-pointer">
                      Both
                    </Label>
                  </div>
                </RadioGroup>
              </div>
            </div>
          </div>

          <div className="glassmorphism rounded-lg p-6 space-y-6">
            <h3 className="text-[#E6EEF3] mb-4">Advanced Features</h3>

            <div className="space-y-4">
              <div className="flex items-center justify-between p-4 bg-[rgba(17,20,25,0.5)] rounded-lg">
                <div className="flex-1">
                  <div className="flex items-center gap-2">
                    <Label>Launch Control</Label>
                    <TooltipProvider>
                      <Tooltip>
                        <TooltipTrigger>
                          <Info className="w-3 h-3 text-[#AFC6D2]" />
                        </TooltipTrigger>
                        <TooltipContent>
                          <p>Two-step rev limiter for launches</p>
                        </TooltipContent>
                      </Tooltip>
                    </TooltipProvider>
                  </div>
                  <p className="text-xs text-[#AFC6D2] mt-1">
                    Two-step rev limiter for optimal launches
                  </p>
                </div>
                <Switch
                  checked={launchControl}
                  onCheckedChange={setLaunchControl}
                />
              </div>

              {launchControl && (
                <div className="ml-6 p-4 border-l-2 border-[#1FB6FF] space-y-3">
                  <div className="flex items-center gap-4">
                    <Label className="w-32">Launch RPM</Label>
                    <Input
                      type="number"
                      defaultValue="4000"
                      className="font-data"
                    />
                  </div>
                  <div className="flex items-center gap-4">
                    <Label className="w-32">Launch Retard</Label>
                    <Input
                      type="number"
                      defaultValue="-5"
                      className="font-data"
                    />
                    <span className="text-sm text-[#AFC6D2]">degrees</span>
                  </div>
                </div>
              )}

              <div className="flex items-center justify-between p-4 bg-[rgba(17,20,25,0.5)] rounded-lg">
                <div className="flex-1">
                  <div className="flex items-center gap-2">
                    <Label>Full Throttle Shift</Label>
                    <TooltipProvider>
                      <Tooltip>
                        <TooltipTrigger>
                          <Info className="w-3 h-3 text-[#AFC6D2]" />
                        </TooltipTrigger>
                        <TooltipContent>
                          <p>Ignition cut during gear changes</p>
                        </TooltipContent>
                      </Tooltip>
                    </TooltipProvider>
                  </div>
                  <p className="text-xs text-[#AFC6D2] mt-1">
                    Momentary cut for no-lift shifts
                  </p>
                </div>
                <Switch defaultChecked />
              </div>

              <div className="flex items-center justify-between p-4 bg-[rgba(17,20,25,0.5)] rounded-lg">
                <div className="flex-1">
                  <div className="flex items-center gap-2">
                    <Label>Boost Control</Label>
                    <TooltipProvider>
                      <Tooltip>
                        <TooltipTrigger>
                          <Info className="w-3 h-3 text-[#AFC6D2]" />
                        </TooltipTrigger>
                        <TooltipContent>
                          <p>Electronic boost control valve management</p>
                        </TooltipContent>
                      </Tooltip>
                    </TooltipProvider>
                  </div>
                  <p className="text-xs text-[#AFC6D2] mt-1">
                    Electronic wastegate control
                  </p>
                </div>
                <Switch
                  checked={boostControl}
                  onCheckedChange={setBoostControl}
                />
              </div>

              <div className="flex items-center justify-between p-4 bg-[rgba(17,20,25,0.5)] rounded-lg">
                <div className="flex-1">
                  <div className="flex items-center gap-2">
                    <Label>VTEC Control</Label>
                    <TooltipProvider>
                      <Tooltip>
                        <TooltipTrigger>
                          <Info className="w-3 h-3 text-[#AFC6D2]" />
                        </TooltipTrigger>
                        <TooltipContent>
                          <p>Variable valve timing control</p>
                        </TooltipContent>
                      </Tooltip>
                    </TooltipProvider>
                  </div>
                  <p className="text-xs text-[#AFC6D2] mt-1">
                    Honda VTEC engagement mapping
                  </p>
                </div>
                <Switch defaultChecked />
              </div>
            </div>
          </div>
        </TabsContent>

        {/* Outputs Tab */}
        <TabsContent value="outputs" className="space-y-6 mt-6">
          <div className="glassmorphism rounded-lg p-6">
            <h3 className="text-[#E6EEF3] mb-6">Output Configuration</h3>

            <div className="space-y-4">
              {[
                { name: "Fuel Pump", pin: "D12", status: "Active" },
                { name: "Cooling Fan", pin: "D13", status: "Active" },
                { name: "Boost Solenoid", pin: "D14", status: "Active" },
                { name: "VTEC Solenoid", pin: "D15", status: "Inactive" },
                { name: "Nitrous Stage 1", pin: "D16", status: "Inactive" },
              ].map((output, i) => (
                <div
                  key={i}
                  className="flex items-center justify-between p-4 border border-[rgba(175,198,210,0.15)] rounded-lg"
                >
                  <div className="flex-1">
                    <div className="text-[#E6EEF3]">{output.name}</div>
                    <div className="text-xs text-[#AFC6D2] mt-1">
                      Pin: {output.pin}
                    </div>
                  </div>
                  <div className="flex items-center gap-4">
                    <div
                      className={`w-2 h-2 rounded-full ${
                        output.status === "Active"
                          ? "bg-[#00A676]"
                          : "bg-[#6B7280]"
                      }`}
                    />
                    <span className="text-sm text-[#AFC6D2] w-16">
                      {output.status}
                    </span>
                    <Switch defaultChecked={output.status === "Active"} />
                  </div>
                </div>
              ))}
            </div>
          </div>
        </TabsContent>

        {/* Status Boxes Tab */}
        <TabsContent value="statusboxes" className="space-y-6 mt-6">
          <div className="glassmorphism rounded-lg p-6">
            <h3 className="text-[#E6EEF3] mb-6">Dashboard Status Box Configuration</h3>
            <p className="text-sm text-[#AFC6D2] mb-6">
              Configure which status indicators appear on the dashboard
            </p>
            
            <div className="space-y-6">
              <div>
                <h4 className="text-[#E6EEF3] mb-4">Top Status Boxes</h4>
                <div className="space-y-3">
                  {statusBoxes.filter(box => box.position === "top").map((box) => (
                    <div
                      key={box.id}
                      className="flex items-center justify-between p-4 border border-[rgba(175,198,210,0.15)] rounded-lg"
                    >
                      <div className="flex-1">
                        <div className="text-[#E6EEF3]">{box.label}</div>
                        <div className="text-xs text-[#AFC6D2] mt-1">
                          Display status on top of dashboard
                        </div>
                      </div>
                      <div className="flex items-center gap-4">
                        <div
                          className={`w-2 h-2 rounded-full ${
                            box.enabled ? "bg-[#00A676]" : "bg-[#6B7280]"
                          }`}
                        />
                        <span className="text-sm text-[#AFC6D2] w-16">
                          {box.enabled ? "Enabled" : "Disabled"}
                        </span>
                        <Switch
                          checked={box.enabled}
                          onCheckedChange={() => toggleStatusBox(box.id)}
                        />
                      </div>
                    </div>
                  ))}
                </div>
              </div>

              <div className="border-t border-[rgba(175,198,210,0.15)] pt-6">
                <h4 className="text-[#E6EEF3] mb-4">Bottom Status Boxes</h4>
                <div className="space-y-3">
                  {statusBoxes.filter(box => box.position === "bottom").map((box) => (
                    <div
                      key={box.id}
                      className="flex items-center justify-between p-4 border border-[rgba(175,198,210,0.15)] rounded-lg"
                    >
                      <div className="flex-1">
                        <div className="text-[#E6EEF3]">{box.label}</div>
                        <div className="text-xs text-[#AFC6D2] mt-1">
                          Display status on bottom of dashboard
                        </div>
                      </div>
                      <div className="flex items-center gap-4">
                        <div
                          className={`w-2 h-2 rounded-full ${
                            box.enabled ? "bg-[#00A676]" : "bg-[#6B7280]"
                          }`}
                        />
                        <span className="text-sm text-[#AFC6D2] w-16">
                          {box.enabled ? "Enabled" : "Disabled"}
                        </span>
                        <Switch
                          checked={box.enabled}
                          onCheckedChange={() => toggleStatusBox(box.id)}
                        />
                      </div>
                    </div>
                  ))}
                </div>
              </div>
            </div>
          </div>
        </TabsContent>

        {/* Appearance Tab */}
        <TabsContent value="appearance" className="space-y-6 mt-6">
          <div className="glassmorphism rounded-lg p-6">
            <h3 style={{ color: "var(--text-primary)" }} className="mb-6">Application Theme</h3>
            
            <div className="space-y-6">
              <div className="space-y-3">
                <Label>Select Theme Style</Label>
                <Select value={theme} onValueChange={(value: "light" | "midnight" | "dark") => setTheme(value)}>
                  <SelectTrigger className="w-full max-w-md">
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="light">Light - Minimalistic Grey</SelectItem>
                    <SelectItem value="midnight">Midnight - Teal Blue</SelectItem>
                    <SelectItem value="dark">Dark - Pure Black</SelectItem>
                  </SelectContent>
                </Select>
                <p className="text-xs" style={{ color: "var(--text-secondary)" }}>
                  Choose a theme optimized for readability. Changes apply immediately across the entire application.
                </p>
              </div>

              <div className="pt-6" style={{ borderTop: "1px solid var(--border)" }}>
                <h4 style={{ color: "var(--text-primary)" }} className="mb-4">Theme Preview</h4>
                <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
                  <div 
                    className={`p-4 border-2 rounded-lg cursor-pointer transition-all ${
                      theme === "light" 
                        ? "border-[#007aff]" 
                        : ""
                    }`}
                    style={{
                      background: theme === "light" ? "rgba(0, 122, 255, 0.1)" : "var(--surface)",
                      borderColor: theme === "light" ? "#007aff" : "var(--border)"
                    }}
                    onClick={() => setTheme("light")}
                  >
                    <div className="mb-2" style={{ color: "var(--text-primary)" }}>Light Theme</div>
                    <div className="text-xs mb-3" style={{ color: "var(--text-secondary)" }}>
                      Clean whitish-grey with dark text for maximum readability in bright environments
                    </div>
                    <div className="h-16 rounded flex items-center justify-center gap-2" style={{
                      background: "linear-gradient(135deg, #f5f5f7 0%, #ffffff 50%, #f5f5f7 100%)"
                    }}>
                      <div className="w-8 h-8 rounded" style={{ background: "#007aff" }}></div>
                      <div className="w-8 h-8 rounded" style={{ background: "#1d1d1f" }}></div>
                      <div className="w-8 h-8 rounded" style={{ background: "#6e6e73" }}></div>
                    </div>
                  </div>

                  <div 
                    className={`p-4 border-2 rounded-lg cursor-pointer transition-all ${
                      theme === "midnight" 
                        ? "border-[#00d4ff]" 
                        : ""
                    }`}
                    style={{
                      background: theme === "midnight" ? "rgba(0, 212, 255, 0.1)" : "var(--surface)",
                      borderColor: theme === "midnight" ? "#00d4ff" : "var(--border)"
                    }}
                    onClick={() => setTheme("midnight")}
                  >
                    <div className="mb-2" style={{ color: "var(--text-primary)" }}>Midnight Theme</div>
                    <div className="text-xs mb-3" style={{ color: "var(--text-secondary)" }}>
                      Dark blue-teal background with bright cyan accents for professional tuning
                    </div>
                    <div className="h-16 rounded flex items-center justify-center gap-2" style={{
                      background: "linear-gradient(135deg, #0a1628 0%, #0f1f35 50%, #0a1628 100%)"
                    }}>
                      <div className="w-8 h-8 rounded" style={{ background: "#00d4ff" }}></div>
                      <div className="w-8 h-8 rounded" style={{ background: "#e8f4f8" }}></div>
                      <div className="w-8 h-8 rounded" style={{ background: "#8fb8d0" }}></div>
                    </div>
                  </div>

                  <div 
                    className={`p-4 border-2 rounded-lg cursor-pointer transition-all ${
                      theme === "dark" 
                        ? "border-[#1FB6FF]" 
                        : ""
                    }`}
                    style={{
                      background: theme === "dark" ? "rgba(31, 182, 255, 0.1)" : "var(--surface)",
                      borderColor: theme === "dark" ? "#1FB6FF" : "var(--border)"
                    }}
                    onClick={() => setTheme("dark")}
                  >
                    <div className="mb-2" style={{ color: "var(--text-primary)" }}>Dark Theme</div>
                    <div className="text-xs mb-3" style={{ color: "var(--text-secondary)" }}>
                      Pure black background with white text - classic ECU software aesthetic
                    </div>
                    <div className="h-16 rounded flex items-center justify-center gap-2" style={{
                      background: "linear-gradient(135deg, #000000 0%, #1a1a1a 50%, #000000 100%)"
                    }}>
                      <div className="w-8 h-8 rounded" style={{ background: "#1FB6FF" }}></div>
                      <div className="w-8 h-8 rounded" style={{ background: "#FFFFFF" }}></div>
                      <div className="w-8 h-8 rounded" style={{ background: "#CCCCCC" }}></div>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </TabsContent>
      </Tabs>
    </div>
  );
}
