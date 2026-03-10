import { useState } from "react";
import { Card } from "../../ui/card";
import { Label } from "../../ui/label";
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "../../ui/select";
import { Info } from "lucide-react";

interface GeneralSettingsProps {
  onSettingsChange: () => void;
}

export function GeneralSettings({ onSettingsChange }: GeneralSettingsProps) {
  const [tempUnit, setTempUnit] = useState("celsius");
  const [pressureUnit, setPressureUnit] = useState("kpa");
  const [speedUnit, setSpeedUnit] = useState("kph");
  const [afrDisplay, setAfrDisplay] = useState("afr");
  const [comPort, setComPort] = useState("com3");
  const [baudRate, setBaudRate] = useState("115200");

  const handleChange = (setter: (value: string) => void) => (value: string) => {
    setter(value);
    onSettingsChange();
  };

  return (
    <div className="space-y-6">
      {/* Units & Display Preferences */}
      <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
        <h3 className="text-white mb-6 flex items-center gap-2 text-xl">
          <span className="text-2xl">📏</span>
          Units & Display Preferences
        </h3>

        <div className="grid grid-cols-2 gap-6">
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Temperature Units</Label>
            <Select value={tempUnit} onValueChange={handleChange(setTempUnit)}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue />
              </SelectTrigger>
              <SelectContent className="bg-[#1a1a1a] border-[#555555]">
                <SelectItem value="celsius" className="text-white">Celsius (°C)</SelectItem>
                <SelectItem value="fahrenheit" className="text-white">Fahrenheit (°F)</SelectItem>
              </SelectContent>
            </Select>
          </div>

          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Pressure Units</Label>
            <Select value={pressureUnit} onValueChange={handleChange(setPressureUnit)}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue />
              </SelectTrigger>
              <SelectContent className="bg-[#1a1a1a] border-[#555555]">
                <SelectItem value="kpa" className="text-white">kPa (Kilopascals)</SelectItem>
                <SelectItem value="psi" className="text-white">PSI (Pounds per Square Inch)</SelectItem>
                <SelectItem value="bar" className="text-white">Bar</SelectItem>
              </SelectContent>
            </Select>
          </div>

          <div>
            <Label className="text-[#CCCCCC] mb-2 block">Speed Units</Label>
            <Select value={speedUnit} onValueChange={handleChange(setSpeedUnit)}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue />
              </SelectTrigger>
              <SelectContent className="bg-[#1a1a1a] border-[#555555]">
                <SelectItem value="kph" className="text-white">KPH (Kilometers per Hour)</SelectItem>
                <SelectItem value="mph" className="text-white">MPH (Miles per Hour)</SelectItem>
              </SelectContent>
            </Select>
          </div>

          <div>
            <Label className="text-[#CCCCCC] mb-2 block">AFR Display</Label>
            <Select value={afrDisplay} onValueChange={handleChange(setAfrDisplay)}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue />
              </SelectTrigger>
              <SelectContent className="bg-[#1a1a1a] border-[#555555]">
                <SelectItem value="afr" className="text-white">AFR (Air:Fuel Ratio)</SelectItem>
                <SelectItem value="lambda" className="text-white">Lambda (λ)</SelectItem>
              </SelectContent>
            </Select>
          </div>
        </div>
      </Card>

      {/* Communication Settings */}
      <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#00CCFF]/40 p-6">
        <h3 className="text-white mb-6 flex items-center gap-2 text-xl">
          <span className="text-2xl">🔌</span>
          Communication Settings
        </h3>

        <div className="grid grid-cols-2 gap-6">
          <div>
            <div className="flex items-center gap-2 mb-2">
              <Label className="text-[#CCCCCC]">COM Port</Label>
              <Info className="w-3 h-3 text-[#00CCFF] cursor-help" title="Serial port for ECU communication" />
            </div>
            <Select value={comPort} onValueChange={handleChange(setComPort)}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue />
              </SelectTrigger>
              <SelectContent className="bg-[#1a1a1a] border-[#555555]">
                <SelectItem value="com1" className="text-white">COM1</SelectItem>
                <SelectItem value="com3" className="text-white">COM3</SelectItem>
                <SelectItem value="com4" className="text-white">COM4</SelectItem>
                <SelectItem value="usb" className="text-white">USB Virtual COM</SelectItem>
              </SelectContent>
            </Select>
          </div>

          <div>
            <div className="flex items-center gap-2 mb-2">
              <Label className="text-[#CCCCCC]">Baud Rate</Label>
              <Info className="w-3 h-3 text-[#00CCFF] cursor-help" title="Communication speed" />
            </div>
            <Select value={baudRate} onValueChange={handleChange(setBaudRate)}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue />
              </SelectTrigger>
              <SelectContent className="bg-[#1a1a1a] border-[#555555]">
                <SelectItem value="9600" className="text-white">9600</SelectItem>
                <SelectItem value="19200" className="text-white">19200</SelectItem>
                <SelectItem value="38400" className="text-white">38400</SelectItem>
                <SelectItem value="57600" className="text-white">57600</SelectItem>
                <SelectItem value="115200" className="text-white">115200</SelectItem>
              </SelectContent>
            </Select>
          </div>
        </div>
      </Card>
    </div>
  );
}
