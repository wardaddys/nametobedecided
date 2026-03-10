import { useState, useEffect } from "react";
import { useSensors } from "../context/SensorContext";
import { useECU } from "../context/ECUContext";
import { Button } from "../ui/button";
import { Checkbox } from "../ui/checkbox";
import { Badge } from "../ui/badge";
import { ECUSelector } from "./ECUSelector";
import { Pin, PinType } from "../context/ECUDatabase";

interface IOConfigurationProps {
  liveTuning: boolean;
  onStatusChange?: (status: string) => void;
}

export function IOConfiguration({ liveTuning, onStatusChange }: IOConfigurationProps) {
  const { sensors, addSensor, updateSensor, removeSensor } = useSensors();
  const { selectedECU } = useECU();

  const [analogInputs, setAnalogInputs] = useState([
    { channel: "AN0", function: "None", min: 0, max: 5, value: 0, showOnDash: false, pinNumber: "", specs: "", autoConfigured: false },
    { channel: "AN1", function: "None", min: 0, max: 5, value: 0, showOnDash: false, pinNumber: "", specs: "", autoConfigured: false },
    { channel: "AN2", function: "None", min: 0, max: 5, value: 0, showOnDash: false, pinNumber: "", specs: "", autoConfigured: false },
    { channel: "AN3", function: "None", min: 0, max: 5, value: 0, showOnDash: false, pinNumber: "", specs: "", autoConfigured: false },
    { channel: "AN4", function: "None", min: 0, max: 5, value: 0, showOnDash: false, pinNumber: "", specs: "", autoConfigured: false },
    { channel: "AN5", function: "None", min: 0, max: 5, value: 0, showOnDash: false, pinNumber: "", specs: "", autoConfigured: false },
    { channel: "AN6", function: "None", min: 0, max: 5, value: 0, showOnDash: false, pinNumber: "", specs: "", autoConfigured: false },
    { channel: "AN7", function: "None", min: 0, max: 5, value: 0, showOnDash: false, pinNumber: "", specs: "", autoConfigured: false },
  ]);

  const [digitalInputs, setDigitalInputs] = useState([
    { channel: "DI0", function: "None", pullMode: "Pull-up", state: "LOW", showOnDash: false, pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DI1", function: "None", pullMode: "Pull-up", state: "LOW", showOnDash: false, pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DI2", function: "None", pullMode: "Pull-up", state: "LOW", showOnDash: false, pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DI3", function: "None", pullMode: "Pull-up", state: "LOW", showOnDash: false, pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DI4", function: "None", pullMode: "Pull-up", state: "LOW", showOnDash: false, pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DI5", function: "None", pullMode: "Pull-up", state: "LOW", showOnDash: false, pinNumber: "", specs: "", autoConfigured: false },
  ]);

  const [digitalOutputs, setDigitalOutputs] = useState([
    { channel: "DO0", function: "None", mode: "Push-Pull", state: "OFF", pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DO1", function: "None", mode: "Push-Pull", state: "OFF", pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DO2", function: "None", mode: "Push-Pull", state: "OFF", pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DO3", function: "None", mode: "Push-Pull", state: "OFF", pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DO4", function: "None", mode: "Push-Pull", state: "OFF", pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DO5", function: "None", mode: "Push-Pull", state: "OFF", pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DO6", function: "None", mode: "Push-Pull", state: "OFF", pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DO7", function: "None", mode: "Push-Pull", state: "OFF", pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DO8", function: "None", mode: "Push-Pull", state: "OFF", pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DO9", function: "None", mode: "Push-Pull", state: "OFF", pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DO10", function: "None", mode: "Push-Pull", state: "OFF", pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DO11", function: "None", mode: "Push-Pull", state: "OFF", pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DO12", function: "None", mode: "Push-Pull", state: "OFF", pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DO13", function: "None", mode: "Push-Pull", state: "OFF", pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DO14", function: "None", mode: "Push-Pull", state: "OFF", pinNumber: "", specs: "", autoConfigured: false },
    { channel: "DO15", function: "None", mode: "Push-Pull", state: "OFF", pinNumber: "", specs: "", autoConfigured: false },
  ]);

  // Auto-configure I/O when ECU is selected
  useEffect(() => {
    if (selectedECU) {
      autoConfigureIO();
    } else {
      // Reset to manual configuration
      resetToManualConfiguration();
    }
  }, [selectedECU]);

  const autoConfigureIO = () => {
    if (!selectedECU) return;

    // Map pin types to friendly function names
    const getFunctionName = (pin: Pin): string => {
      const typeMap: Record<PinType, string> = {
        'injector': pin.function,
        'ignition': pin.function,
        'fuel_pump': 'Fuel Pump',
        'idle_control': 'Idle Control',
        'tachometer': 'Tachometer',
        'fan': 'Fan Control',
        'boost': 'Boost Control',
        'vvt': 'VVT Control',
        'auxiliary': pin.function,
        'crank_trigger': 'Crank Trigger',
        'cam_trigger': 'Cam Trigger',
        'map_sensor': 'MAP Sensor',
        'tps': 'TPS',
        'clt': 'CLT',
        'iat': 'IAT',
        'o2_sensor': 'O2 Sensor',
        'battery_voltage': 'Battery Voltage',
        'flex_fuel': 'Flex Fuel',
        'launch_control': 'Launch Control',
        'knock_sensor': 'Knock Sensor',
        'spare': 'None',
      };
      return typeMap[pin.type] || pin.function;
    };

    // Configure analog inputs from ECU inputs
    const analogInputTypes: PinType[] = ['map_sensor', 'tps', 'clt', 'iat', 'o2_sensor', 'battery_voltage', 'knock_sensor'];
    const ecuAnalogInputs = selectedECU.inputs.filter(pin => 
      analogInputTypes.includes(pin.type) && pin.enabled
    );

    const newAnalogInputs = analogInputs.map((input, index) => {
      if (index < ecuAnalogInputs.length) {
        const ecuPin = ecuAnalogInputs[index];
        const functionName = getFunctionName(ecuPin);
        const shouldShowOnDash = ecuPin.type === 'o2_sensor' || ecuPin.type === 'clt' || ecuPin.type === 'tps';
        
        // Add sensor to context
        const sensorId = `analog-${input.channel}`;
        const existing = sensors.find(s => s.id === sensorId);
        if (existing) {
          updateSensor(sensorId, { 
            name: functionName,
            showOnDashboard: shouldShowOnDash 
          });
        } else {
          addSensor({
            id: sensorId,
            name: functionName,
            channel: input.channel,
            value: Math.round(Math.random() * 100),
            unit: getUnitForSensor(functionName),
            min: input.min,
            max: input.max,
            showOnDashboard: shouldShowOnDash,
          });
        }
        
        return {
          ...input,
          function: functionName,
          pinNumber: ecuPin.pinNumber,
          specs: ecuPin.specs,
          autoConfigured: true,
          showOnDash: shouldShowOnDash,
        };
      }
      return { ...input, function: "None", pinNumber: "", specs: "", autoConfigured: false };
    });
    setAnalogInputs(newAnalogInputs);

    // Configure digital inputs from ECU inputs
    const digitalInputTypes: PinType[] = ['crank_trigger', 'cam_trigger', 'flex_fuel', 'launch_control'];
    const ecuDigitalInputs = selectedECU.inputs.filter(pin => 
      digitalInputTypes.includes(pin.type) && pin.enabled
    );

    const newDigitalInputs = digitalInputs.map((input, index) => {
      if (index < ecuDigitalInputs.length) {
        const ecuPin = ecuDigitalInputs[index];
        const functionName = getFunctionName(ecuPin);
        
        // Add sensor to context
        const sensorId = `digital-${input.channel}`;
        const existing = sensors.find(s => s.id === sensorId);
        if (existing) {
          updateSensor(sensorId, { 
            name: functionName,
            showOnDashboard: false 
          });
        } else {
          addSensor({
            id: sensorId,
            name: functionName,
            channel: input.channel,
            value: Math.round(Math.random() * 100),
            unit: getUnitForSensor(functionName),
            min: 0,
            max: 100,
            showOnDashboard: false,
          });
        }
        
        return {
          ...input,
          function: functionName,
          pinNumber: ecuPin.pinNumber,
          specs: ecuPin.specs,
          autoConfigured: true,
          showOnDash: false,
        };
      }
      return { ...input, function: "None", pinNumber: "", specs: "", autoConfigured: false };
    });
    setDigitalInputs(newDigitalInputs);

    // Configure digital outputs from ECU outputs
    const ecuOutputs = selectedECU.outputs.filter(pin => pin.enabled);
    const newDigitalOutputs = digitalOutputs.map((output, index) => {
      if (index < ecuOutputs.length) {
        const ecuPin = ecuOutputs[index];
        return {
          ...output,
          function: getFunctionName(ecuPin),
          pinNumber: ecuPin.pinNumber,
          specs: ecuPin.specs,
          autoConfigured: true,
          mode: ecuPin.type === 'idle_control' || ecuPin.type === 'boost' || ecuPin.type === 'vvt' ? 'PWM' : 'Push-Pull',
        };
      }
      return { ...output, function: "None", pinNumber: "", specs: "", autoConfigured: false };
    });
    setDigitalOutputs(newDigitalOutputs);
  };

  const resetToManualConfiguration = () => {
    setAnalogInputs(prev => prev.map(input => ({
      ...input,
      function: "None",
      pinNumber: "",
      specs: "",
      autoConfigured: false,
      showOnDash: false,
    })));
    
    setDigitalInputs(prev => prev.map(input => ({
      ...input,
      function: "None",
      pinNumber: "",
      specs: "",
      autoConfigured: false,
      showOnDash: false,
    })));
    
    setDigitalOutputs(prev => prev.map(output => ({
      ...output,
      function: "None",
      pinNumber: "",
      specs: "",
      autoConfigured: false,
    })));
  };

  const inputOptions = [
    "None",
    "TPS",
    "MAP",
    "IAT",
    "CLT",
    "O2 Sensor",
    "Knock Sensor",
    "Oil Pressure",
    "Oil Temp",
    "Fuel Pressure",
    "Battery Voltage",
    "Boost Pressure",
  ];

  const digitalInputOptions = [
    "None",
    "VSS",
    "Clutch",
    "Brake",
    "Launch Button",
    "Flex Fuel",
    "Cam Position",
  ];

  const outputOptions = [
    "None",
    "Injector 1",
    "Injector 2",
    "Injector 3",
    "Injector 4",
    "Injector 5",
    "Injector 6",
    "Injector 7",
    "Injector 8",
    "Ignition 1",
    "Ignition 2",
    "Ignition 3",
    "Ignition 4",
    "Ignition 5",
    "Ignition 6",
    "Ignition 7",
    "Ignition 8",
    "VTEC Solenoid",
    "Boost Solenoid",
    "Fuel Pump",
    "Fan Control",
    "Shift Light",
    "Check Engine",
    "Nitrous",
    "Water Injection",
  ];

  const handleAnalogFunctionChange = (index: number, value: string) => {
    const updated = [...analogInputs];
    updated[index].function = value;
    setAnalogInputs(updated);

    // Update sensor context
    const sensorId = `analog-${updated[index].channel}`;
    if (value !== "None") {
      const existing = sensors.find(s => s.id === sensorId);
      if (existing) {
        updateSensor(sensorId, { name: value });
      } else {
        addSensor({
          id: sensorId,
          name: value,
          channel: updated[index].channel,
          value: Math.round(Math.random() * 100),
          unit: getUnitForSensor(value),
          min: updated[index].min,
          max: updated[index].max,
          showOnDashboard: updated[index].showOnDash,
        });
      }
      // Notify status bar
      onStatusChange?.(`${updated[index].channel}: ${value} [CONFIGURED]`);
    } else {
      removeSensor(sensorId);
      onStatusChange?.(`${updated[index].channel}: Removed`);
    }
  };

  const handleDigitalFunctionChange = (index: number, value: string) => {
    const updated = [...digitalInputs];
    updated[index].function = value;
    setDigitalInputs(updated);

    const sensorId = `digital-${updated[index].channel}`;
    if (value !== "None") {
      const existing = sensors.find(s => s.id === sensorId);
      if (existing) {
        updateSensor(sensorId, { name: value });
      } else {
        addSensor({
          id: sensorId,
          name: value,
          channel: updated[index].channel,
          value: Math.round(Math.random() * 100),
          unit: getUnitForSensor(value),
          min: 0,
          max: 100,
          showOnDashboard: updated[index].showOnDash,
        });
      }
    } else {
      removeSensor(sensorId);
    }
  };

  const handleDashboardToggle = (type: "analog" | "digital", index: number, checked: boolean) => {
    if (type === "analog") {
      const updated = [...analogInputs];
      updated[index].showOnDash = checked;
      setAnalogInputs(updated);
      
      const sensorId = `analog-${updated[index].channel}`;
      const existing = sensors.find(s => s.id === sensorId);
      if (existing) {
        updateSensor(sensorId, { showOnDashboard: checked });
      } else if (updated[index].function !== "None") {
        // Create sensor if it doesn't exist and function is not None
        addSensor({
          id: sensorId,
          name: updated[index].function,
          channel: updated[index].channel,
          value: Math.round(Math.random() * 100),
          unit: getUnitForSensor(updated[index].function),
          min: updated[index].min,
          max: updated[index].max,
          showOnDashboard: checked,
        });
      }
    } else {
      const updated = [...digitalInputs];
      updated[index].showOnDash = checked;
      setDigitalInputs(updated);

      const sensorId = `digital-${updated[index].channel}`;
      const existing = sensors.find(s => s.id === sensorId);
      if (existing) {
        updateSensor(sensorId, { showOnDashboard: checked });
      } else if (updated[index].function !== "None") {
        // Create sensor if it doesn't exist and function is not None
        addSensor({
          id: sensorId,
          name: updated[index].function,
          channel: updated[index].channel,
          value: Math.round(Math.random() * 100),
          unit: getUnitForSensor(updated[index].function),
          min: 0,
          max: 100,
          showOnDashboard: checked,
        });
      }
    }
  };

  const getUnitForSensor = (sensorName: string): string => {
    const unitMap: Record<string, string> = {
      "TPS": "%",
      "MAP": "kPa",
      "IAT": "°C",
      "CLT": "°C",
      "O2 Sensor": "AFR",
      "Oil Pressure": "psi",
      "Oil Temp": "°C",
      "Fuel Pressure": "psi",
      "Battery Voltage": "V",
      "Boost Pressure": "psi",
      "VSS": "km/h",
    };
    return unitMap[sensorName] || "";
  };

  return (
    <div className="h-full bg-[#1a1a1a] p-6 overflow-auto">
      <div className="max-w-7xl mx-auto space-y-4">
        {/* ECU Selector */}
        <ECUSelector />

        {/* Analog Inputs */}
        <div className="bg-[#1a1a1a] border border-[#333333] p-4">
          <h2 className="text-white mb-4">
            Analog Inputs
            {selectedECU && (
              <span className="text-xs text-[#888888] ml-2">
                (Auto-configured from {selectedECU.name})
              </span>
            )}
          </h2>
          <table className="w-full border-collapse text-xs">
            <thead>
              <tr className="bg-[#2a2a2a]">
                <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                  Channel
                </th>
                {selectedECU && (
                  <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                    ECU Pin
                  </th>
                )}
                <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                  Function
                </th>
                {selectedECU && (
                  <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                    Specs
                  </th>
                )}
                <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                  Min Voltage
                </th>
                <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                  Max Voltage
                </th>
                <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                  Current Value
                </th>
                <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                  Show on Dash
                </th>
              </tr>
            </thead>
            <tbody>
              {analogInputs.map((input, i) => (
                <tr key={i} className={input.autoConfigured ? "bg-[#0a2a0a]" : ""}>
                  <td className="border border-[#444444] p-2 text-[#CCCCCC]">
                    <div className="flex items-center gap-2">
                      {input.channel}
                      {input.autoConfigured && (
                        <Badge variant="outline" className="bg-[#00FF00]/10 text-[#00FF00] border-[#00FF00] text-xs px-1 py-0">
                          AUTO
                        </Badge>
                      )}
                    </div>
                  </td>
                  {selectedECU && (
                    <td className="border border-[#444444] p-2 text-[#888888]">
                      {input.pinNumber || "-"}
                    </td>
                  )}
                  <td className="border border-[#444444] p-0">
                    <select 
                      className="w-full bg-black text-white p-2 border-0 outline-none"
                      value={input.function}
                      onChange={(e) => handleAnalogFunctionChange(i, e.target.value)}
                    >
                      {inputOptions.map((opt) => (
                        <option key={opt} value={opt}>
                          {opt}
                        </option>
                      ))}
                    </select>
                  </td>
                  {selectedECU && (
                    <td className="border border-[#444444] p-2 text-[#888888] text-xs">
                      {input.specs || "-"}
                    </td>
                  )}
                  <td className="border border-[#444444] p-0">
                    <input
                      type="number"
                      value={input.min}
                      step="0.1"
                      className="w-full bg-black text-white p-2 border-0 outline-none text-center"
                      onChange={(e) => {
                        const updated = [...analogInputs];
                        updated[i].min = parseFloat(e.target.value);
                        setAnalogInputs(updated);
                      }}
                    />
                  </td>
                  <td className="border border-[#444444] p-0">
                    <input
                      type="number"
                      value={input.max}
                      step="0.1"
                      className="w-full bg-black text-white p-2 border-0 outline-none text-center"
                      onChange={(e) => {
                        const updated = [...analogInputs];
                        updated[i].max = parseFloat(e.target.value);
                        setAnalogInputs(updated);
                      }}
                    />
                  </td>
                  <td className="border border-[#444444] p-2 text-[#00FF00] text-center font-data">
                    {(Math.random() * 5).toFixed(2)}V
                  </td>
                  <td className="border border-[#444444] p-2 text-center">
                    <Checkbox
                      checked={input.showOnDash}
                      onCheckedChange={(checked) => handleDashboardToggle("analog", i, checked as boolean)}
                      disabled={input.function === "None"}
                    />
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>

        {/* Digital Inputs */}
        <div className="bg-[#1a1a1a] border border-[#333333] p-4">
          <h2 className="text-white mb-4">
            Digital Inputs
            {selectedECU && (
              <span className="text-xs text-[#888888] ml-2">
                (Auto-configured from {selectedECU.name})
              </span>
            )}
          </h2>
          <table className="w-full border-collapse text-xs">
            <thead>
              <tr className="bg-[#2a2a2a]">
                <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                  Channel
                </th>
                {selectedECU && (
                  <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                    ECU Pin
                  </th>
                )}
                <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                  Function
                </th>
                {selectedECU && (
                  <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                    Specs
                  </th>
                )}
                <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                  Pull-up/Pull-down
                </th>
                <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                  State
                </th>
                <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                  Show on Dash
                </th>
              </tr>
            </thead>
            <tbody>
              {digitalInputs.map((input, i) => (
                <tr key={i} className={input.autoConfigured ? "bg-[#0a2a0a]" : ""}>
                  <td className="border border-[#444444] p-2 text-[#CCCCCC]">
                    <div className="flex items-center gap-2">
                      {input.channel}
                      {input.autoConfigured && (
                        <Badge variant="outline" className="bg-[#00FF00]/10 text-[#00FF00] border-[#00FF00] text-xs px-1 py-0">
                          AUTO
                        </Badge>
                      )}
                    </div>
                  </td>
                  {selectedECU && (
                    <td className="border border-[#444444] p-2 text-[#888888]">
                      {input.pinNumber || "-"}
                    </td>
                  )}
                  <td className="border border-[#444444] p-0">
                    <select 
                      className="w-full bg-black text-white p-2 border-0 outline-none"
                      value={input.function}
                      onChange={(e) => handleDigitalFunctionChange(i, e.target.value)}
                    >
                      {digitalInputOptions.map((opt) => (
                        <option key={opt} value={opt}>
                          {opt}
                        </option>
                      ))}
                    </select>
                  </td>
                  {selectedECU && (
                    <td className="border border-[#444444] p-2 text-[#888888] text-xs">
                      {input.specs || "-"}
                    </td>
                  )}
                  <td className="border border-[#444444] p-0">
                    <select 
                      className="w-full bg-black text-white p-2 border-0 outline-none"
                      value={input.pullMode}
                      onChange={(e) => {
                        const updated = [...digitalInputs];
                        updated[i].pullMode = e.target.value;
                        setDigitalInputs(updated);
                      }}
                    >
                      <option>Pull-up</option>
                      <option>Pull-down</option>
                      <option>None</option>
                    </select>
                  </td>
                  <td className="border border-[#444444] p-2 text-center">
                    <span
                      className={`px-2 py-1 ${
                        Math.random() > 0.5
                          ? "bg-[#00FF00] text-black"
                          : "bg-[#666666] text-white"
                      }`}
                    >
                      {Math.random() > 0.5 ? "HIGH" : "LOW"}
                    </span>
                  </td>
                  <td className="border border-[#444444] p-2 text-center">
                    <Checkbox
                      checked={input.showOnDash}
                      onCheckedChange={(checked) => handleDashboardToggle("digital", i, checked as boolean)}
                      disabled={input.function === "None"}
                    />
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>

        {/* Digital Outputs */}
        <div className="bg-[#1a1a1a] border border-[#333333] p-4">
          <h2 className="text-white mb-4">
            Digital Outputs (16 Channels for V8 Support)
            {selectedECU && (
              <span className="text-xs text-[#888888] ml-2">
                (Auto-configured from {selectedECU.name})
              </span>
            )}
          </h2>
          <table className="w-full border-collapse text-xs">
            <thead>
              <tr className="bg-[#2a2a2a]">
                <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                  Channel
                </th>
                {selectedECU && (
                  <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                    ECU Pin
                  </th>
                )}
                <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                  Function
                </th>
                {selectedECU && (
                  <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                    Specs
                  </th>
                )}
                <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                  Mode
                </th>
                <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                  State
                </th>
                <th className="border border-[#444444] p-2 text-left text-[#CCCCCC]">
                  Test
                </th>
              </tr>
            </thead>
            <tbody>
              {digitalOutputs.map((output, i) => (
                <tr key={i} className={output.autoConfigured ? "bg-[#0a2a0a]" : ""}>
                  <td className="border border-[#444444] p-2 text-[#CCCCCC]">
                    <div className="flex items-center gap-2">
                      {output.channel}
                      {output.autoConfigured && (
                        <Badge variant="outline" className="bg-[#00FF00]/10 text-[#00FF00] border-[#00FF00] text-xs px-1 py-0">
                          AUTO
                        </Badge>
                      )}
                    </div>
                  </td>
                  {selectedECU && (
                    <td className="border border-[#444444] p-2 text-[#888888]">
                      {output.pinNumber || "-"}
                    </td>
                  )}
                  <td className="border border-[#444444] p-0">
                    <select 
                      className="w-full bg-black text-white p-2 border-0 outline-none"
                      value={output.function}
                      onChange={(e) => {
                        const updated = [...digitalOutputs];
                        updated[i].function = e.target.value;
                        setDigitalOutputs(updated);
                      }}
                    >
                      {outputOptions.map((opt) => (
                        <option key={opt} value={opt}>
                          {opt}
                        </option>
                      ))}
                    </select>
                  </td>
                  {selectedECU && (
                    <td className="border border-[#444444] p-2 text-[#888888] text-xs">
                      {output.specs || "-"}
                    </td>
                  )}
                  <td className="border border-[#444444] p-0">
                    <select 
                      className="w-full bg-black text-white p-2 border-0 outline-none"
                      value={output.mode}
                      onChange={(e) => {
                        const updated = [...digitalOutputs];
                        updated[i].mode = e.target.value;
                        setDigitalOutputs(updated);
                      }}
                    >
                      <option>Push-Pull</option>
                      <option>Open Drain</option>
                      <option>PWM</option>
                    </select>
                  </td>
                  <td className="border border-[#444444] p-2 text-center">
                    <span className="text-[#00FF00]">
                      {Math.random() > 0.5 ? "ON" : "OFF"}
                    </span>
                  </td>
                  <td className="border border-[#444444] p-2 text-center">
                    <button className="bg-[#3a3a3a] hover:bg-[#4a4a4a] border border-[#555555] px-3 py-1 text-white">
                      Test
                    </button>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
}
