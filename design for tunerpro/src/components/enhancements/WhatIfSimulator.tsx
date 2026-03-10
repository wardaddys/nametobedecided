import { useState } from "react";
import { TrendingUp, Droplets, Zap, Wind, AlertCircle } from "lucide-react";
import { Slider } from "../ui/slider";
import { Card } from "../ui/card";

interface SimulationResult {
  parameter: string;
  current: number;
  predicted: number;
  change: number;
  unit: string;
  status: "safe" | "warning" | "danger";
}

/**
 * What-If Simulator Panel
 * Interactive forecast tool for tuning parameter changes
 */
export function WhatIfSimulator() {
  const [boostTarget, setBoostTarget] = useState(10);
  const [fuelEnrichment, setFuelEnrichment] = useState(100);
  const [ignitionAdvance, setIgnitionAdvance] = useState(20);

  // Simulate predictions based on adjustments
  const calculatePredictions = (): SimulationResult[] => {
    // Simple mock formulas - in real app, these would use actual ECU calculations
    const baseAFR = 14.7;
    const basePower = 200; // HP
    const baseEGT = 750; // °C
    const baseMAPkPa = 100;

    // Boost impact
    const boostMultiplier = boostTarget / 10;
    const predictedPower = basePower * (1 + (boostTarget - 10) * 0.08);
    const predictedMAP = baseMAPkPa + (boostTarget - 10) * 6.89; // PSI to kPa conversion factor

    // Fuel enrichment impact
    const fuelMultiplier = fuelEnrichment / 100;
    const predictedAFR = baseAFR / fuelMultiplier;

    // Ignition advance impact on EGT
    const advanceOffset = ignitionAdvance - 20;
    const predictedEGT = baseEGT + advanceOffset * 5;

    // Knock risk calculation
    const knockRisk = (boostTarget > 15 ? 30 : 0) + (ignitionAdvance > 30 ? 25 : 0) + (predictedAFR > 15 ? 40 : 0);

    return [
      {
        parameter: "Engine Power",
        current: basePower,
        predicted: predictedPower,
        change: ((predictedPower - basePower) / basePower) * 100,
        unit: "HP",
        status: boostTarget > 20 ? "danger" : boostTarget > 15 ? "warning" : "safe",
      },
      {
        parameter: "Air-Fuel Ratio",
        current: baseAFR,
        predicted: predictedAFR,
        change: ((predictedAFR - baseAFR) / baseAFR) * 100,
        unit: ":1",
        status: predictedAFR > 15 || predictedAFR < 12 ? "danger" : predictedAFR > 14.9 || predictedAFR < 13 ? "warning" : "safe",
      },
      {
        parameter: "Manifold Pressure",
        current: baseMAPkPa,
        predicted: predictedMAP,
        change: ((predictedMAP - baseMAPkPa) / baseMAPkPa) * 100,
        unit: "kPa",
        status: predictedMAP > 200 ? "danger" : predictedMAP > 150 ? "warning" : "safe",
      },
      {
        parameter: "Exhaust Gas Temp",
        current: baseEGT,
        predicted: predictedEGT,
        change: ((predictedEGT - baseEGT) / baseEGT) * 100,
        unit: "°C",
        status: predictedEGT > 900 ? "danger" : predictedEGT > 850 ? "warning" : "safe",
      },
      {
        parameter: "Knock Risk",
        current: 5,
        predicted: knockRisk,
        change: knockRisk - 5,
        unit: "%",
        status: knockRisk > 50 ? "danger" : knockRisk > 25 ? "warning" : "safe",
      },
    ];
  };

  const predictions = calculatePredictions();

  const getStatusColor = (status: "safe" | "warning" | "danger") => {
    switch (status) {
      case "safe": return "text-[#00FF00]";
      case "warning": return "text-[#FFFF00]";
      case "danger": return "text-[#FF0000]";
    }
  };

  const getStatusBg = (status: "safe" | "warning" | "danger") => {
    switch (status) {
      case "safe": return "bg-[#00FF00]/10 border-[#00FF00]";
      case "warning": return "bg-[#FFFF00]/10 border-[#FFFF00]";
      case "danger": return "bg-[#FF0000]/10 border-[#FF0000]";
    }
  };

  return (
    <Card className="bg-[#1a1a1a] border-2 border-[#0066CC] p-6">
      {/* Header */}
      <div className="flex items-center gap-3 mb-6">
        <TrendingUp className="w-6 h-6 text-[#0066CC]" />
        <div>
          <h3 className="text-white text-lg">What-If Simulator</h3>
          <p className="text-[#888888] text-sm">Preview parameter changes before applying</p>
        </div>
      </div>

      {/* Input Sliders */}
      <div className="space-y-6 mb-6">
        {/* Boost Target */}
        <div>
          <div className="flex items-center justify-between mb-2">
            <label className="text-[#CCCCCC] text-sm flex items-center gap-2">
              <Wind className="w-4 h-4 text-[#00CCFF]" />
              Boost Target
            </label>
            <span className="text-[#00CCFF] font-mono">{boostTarget} PSI</span>
          </div>
          <Slider
            value={[boostTarget]}
            onValueChange={(val) => setBoostTarget(val[0])}
            min={0}
            max={30}
            step={0.5}
            className="[&_[role=slider]]:bg-[#00CCFF] [&_[role=slider]]:border-[#00CCFF]"
          />
          <div className="flex justify-between text-xs text-[#666666] mt-1">
            <span>0 PSI (NA)</span>
            <span className="text-[#FFFF00]">15 PSI</span>
            <span className="text-[#FF0000]">30 PSI</span>
          </div>
        </div>

        {/* Fuel Enrichment */}
        <div>
          <div className="flex items-center justify-between mb-2">
            <label className="text-[#CCCCCC] text-sm flex items-center gap-2">
              <Droplets className="w-4 h-4 text-[#FFD700]" />
              Fuel Enrichment
            </label>
            <span className="text-[#FFD700] font-mono">{fuelEnrichment}%</span>
          </div>
          <Slider
            value={[fuelEnrichment]}
            onValueChange={(val) => setFuelEnrichment(val[0])}
            min={80}
            max={150}
            step={1}
            className="[&_[role=slider]]:bg-[#FFD700] [&_[role=slider]]:border-[#FFD700]"
          />
          <div className="flex justify-between text-xs text-[#666666] mt-1">
            <span className="text-[#FF0000]">80% (Lean)</span>
            <span>100% (Stoich)</span>
            <span>150% (Rich)</span>
          </div>
        </div>

        {/* Ignition Advance */}
        <div>
          <div className="flex items-center justify-between mb-2">
            <label className="text-[#CCCCCC] text-sm flex items-center gap-2">
              <Zap className="w-4 h-4 text-[#FF00FF]" />
              Ignition Advance
            </label>
            <span className="text-[#FF00FF] font-mono">{ignitionAdvance}° BTDC</span>
          </div>
          <Slider
            value={[ignitionAdvance]}
            onValueChange={(val) => setIgnitionAdvance(val[0])}
            min={5}
            max={40}
            step={0.5}
            className="[&_[role=slider]]:bg-[#FF00FF] [&_[role=slider]]:border-[#FF00FF]"
          />
          <div className="flex justify-between text-xs text-[#666666] mt-1">
            <span>5° (Retarded)</span>
            <span>20° (Normal)</span>
            <span className="text-[#FF0000]">40° (Advanced)</span>
          </div>
        </div>
      </div>

      {/* Predictions */}
      <div className="bg-black/40 border border-[#333333] rounded p-4">
        <h4 className="text-[#0066CC] text-sm mb-3 flex items-center gap-2">
          <TrendingUp className="w-4 h-4" />
          Predicted Results
        </h4>
        <div className="space-y-3">
          {predictions.map((result, index) => (
            <div key={index} className={`p-3 rounded border ${getStatusBg(result.status)}`}>
              <div className="flex items-center justify-between">
                <div className="flex-1">
                  <div className="text-white text-sm mb-1">{result.parameter}</div>
                  <div className="flex items-center gap-4 text-xs">
                    <span className="text-[#888888]">
                      Current: <span className="font-mono">{result.current.toFixed(1)}{result.unit}</span>
                    </span>
                    <span className="text-[#888888]">→</span>
                    <span className={getStatusColor(result.status)}>
                      Predicted: <span className="font-mono">{result.predicted.toFixed(1)}{result.unit}</span>
                    </span>
                  </div>
                </div>
                <div className={`text-right ${getStatusColor(result.status)}`}>
                  <div className="font-mono">
                    {result.change > 0 ? "+" : ""}{result.change.toFixed(1)}
                    {result.parameter === "Knock Risk" ? "%" : "%"}
                  </div>
                  <div className="text-xs">
                    {result.status === "safe" ? "✓ Safe" : result.status === "warning" ? "⚠ Caution" : "⛔ Danger"}
                  </div>
                </div>
              </div>
            </div>
          ))}
        </div>
      </div>

      {/* Overall Assessment */}
      <div className="mt-4 p-3 bg-[#0066CC]/10 border border-[#0066CC] rounded">
        <div className="flex items-start gap-2">
          <AlertCircle className="w-4 h-4 text-[#0066CC] mt-0.5" />
          <div className="text-xs text-[#CCCCCC]">
            <strong className="text-[#0066CC]">Note:</strong> These predictions are estimates based on theoretical models.
            Always verify changes with dyno testing and monitor knock sensors during tuning.
          </div>
        </div>
      </div>
    </Card>
  );
}
