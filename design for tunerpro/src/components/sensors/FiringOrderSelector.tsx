// Firing Order Selector Component for Engine Setup Tab
// Integrates comprehensive Speeduino firing order database

import { useState } from "react";
import { Card } from "../ui/card";
import { Label } from "../ui/label";
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "../ui/select";
import { Badge } from "../ui/badge";
import { Alert, AlertDescription } from "../ui/alert";
import { Target, CheckCircle2, AlertTriangle, Info } from "lucide-react";
import {
  FIRING_ORDER_DATABASE,
  getFiringOrdersByCylinders,
  getFiringOrdersByConfig,
  getAvailableLayouts,
  SUPPORTED_CYLINDER_COUNTS,
  LAYOUT_DISPLAY_NAMES,
  FiringOrderDefinition,
} from "./FiringOrderDatabase";

interface FiringOrderSelectorProps {
  onSelect?: (firingOrder: FiringOrderDefinition | null) => void;
}

export function FiringOrderSelector({ onSelect }: FiringOrderSelectorProps) {
  const [cylinders, setCylinders] = useState<number | null>(null);
  const [layout, setLayout] = useState<"inline" | "v" | "flat" | "rotary" | null>(null);
  const [selectedFiringOrder, setSelectedFiringOrder] = useState<FiringOrderDefinition | null>(null);

  const availableLayouts = cylinders ? getAvailableLayouts(cylinders) : [];
  const availableFiringOrders = cylinders && layout ? getFiringOrdersByConfig(cylinders, layout) : [];

  const handleCylinderChange = (value: string) => {
    const cylCount = parseInt(value);
    setCylinders(cylCount);
    setLayout(null);
    setSelectedFiringOrder(null);
    if (onSelect) onSelect(null);
  };

  const handleLayoutChange = (value: string) => {
    setLayout(value as any);
    setSelectedFiringOrder(null);
    if (onSelect) onSelect(null);
  };

  const handleFiringOrderChange = (value: string) => {
    const fo = FIRING_ORDER_DATABASE.find((f) => f.id === value);
    setSelectedFiringOrder(fo || null);
    if (onSelect) onSelect(fo || null);
  };

  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#00CCFF]/40 p-6 smooth-transition hover:neon-glow-cyan">
      <div className="flex items-center justify-between mb-4">
        <h3 className="text-white flex items-center gap-2">
          <Target className="w-5 h-5 text-[#00CCFF] pulse-glow-cyan" />
          Firing Order Configuration
        </h3>
        {selectedFiringOrder && (
          <Badge className="bg-[#00FF00] text-black">
            <CheckCircle2 className="w-3 h-3 mr-1" />
            Configured
          </Badge>
        )}
      </div>

      <p className="text-xs text-[#888888] mb-4">
        Select your engine configuration to auto-populate firing order. Only Speeduino-supported patterns shown.
      </p>

      <div className="space-y-4">
        {/* Step 1: Cylinder Count */}
        <div>
          <Label className="text-[#CCCCCC] mb-2 block">
            Step 1: Number of Cylinders
          </Label>
          <Select value={cylinders?.toString() || ""} onValueChange={handleCylinderChange}>
            <SelectTrigger className="bg-black border-[#555555] text-white">
              <SelectValue placeholder="Select cylinder count..." />
            </SelectTrigger>
            <SelectContent className="bg-[#1a1a1a] border-[#555555]">
              {SUPPORTED_CYLINDER_COUNTS.map((count) => (
                <SelectItem key={count} value={count.toString()} className="text-white hover:bg-[#333333]">
                  {count} Cylinder{count !== 1 ? "s" : ""}
                </SelectItem>
              ))}
            </SelectContent>
          </Select>
        </div>

        {/* Step 2: Engine Layout */}
        {cylinders && availableLayouts.length > 0 && (
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">
              Step 2: Engine Layout
            </Label>
            <Select value={layout || ""} onValueChange={handleLayoutChange}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue placeholder="Select engine layout..." />
              </SelectTrigger>
              <SelectContent className="bg-[#1a1a1a] border-[#555555]">
                {availableLayouts.map((layoutType) => (
                  <SelectItem
                    key={layoutType}
                    value={layoutType}
                    className="text-white hover:bg-[#333333]"
                  >
                    {LAYOUT_DISPLAY_NAMES[layoutType]}
                  </SelectItem>
                ))}
              </SelectContent>
            </Select>
          </div>
        )}

        {/* Step 3: Firing Order */}
        {cylinders && layout && availableFiringOrders.length > 0 && (
          <div>
            <Label className="text-[#CCCCCC] mb-2 block">
              Step 3: Firing Order Pattern
            </Label>
            <Select value={selectedFiringOrder?.id || ""} onValueChange={handleFiringOrderChange}>
              <SelectTrigger className="bg-black border-[#555555] text-white">
                <SelectValue placeholder="Select firing order..." />
              </SelectTrigger>
              <SelectContent className="bg-[#1a1a1a] border-[#555555] max-h-[400px]">
                {availableFiringOrders.map((fo) => (
                  <SelectItem
                    key={fo.id}
                    value={fo.id}
                    className="text-white hover:bg-[#333333] cursor-pointer"
                  >
                    <div className="flex flex-col">
                      <div className="flex items-center gap-2">
                        <span className="text-[#00CCFF] font-mono font-semibold">
                          {fo.firingOrder}
                        </span>
                        <span className="text-[#888888]">•</span>
                        <span className="text-xs text-[#888888]">{fo.description}</span>
                      </div>
                      <div className="text-xs text-[#666666] mt-1">
                        {fo.commonApplications.slice(0, 3).join(" • ")}
                      </div>
                    </div>
                  </SelectItem>
                ))}
              </SelectContent>
            </Select>
          </div>
        )}

        {/* Selected Firing Order Details */}
        {selectedFiringOrder && (
          <Card className="bg-black/60 border border-[#00FF00]/30 p-4 fade-in neon-glow-green">
            <div className="grid grid-cols-2 gap-4">
              <div>
                <h5 className="text-sm text-[#00FF00] mb-2 flex items-center gap-2">
                  <CheckCircle2 className="w-4 h-4" />
                  Selected Configuration
                </h5>
                <div className="space-y-1 text-xs">
                  <div className="flex justify-between">
                    <span className="text-[#888888]">Cylinders:</span>
                    <span className="text-white font-mono">{selectedFiringOrder.cylinders}</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[#888888]">Layout:</span>
                    <span className="text-white">{LAYOUT_DISPLAY_NAMES[selectedFiringOrder.layout]}</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[#888888]">Firing Order:</span>
                    <span className="text-white font-mono font-semibold">{selectedFiringOrder.firingOrder}</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[#888888]">Speeduino:</span>
                    <span className="text-[#00FF00]">✓ Supported</span>
                  </div>
                </div>
              </div>
              <div>
                <h5 className="text-sm text-[#00CCFF] mb-2">Common Applications</h5>
                <ul className="text-xs text-[#CCCCCC] space-y-1">
                  {selectedFiringOrder.commonApplications.map((app, idx) => (
                    <li key={idx}>• {app}</li>
                  ))}
                </ul>
              </div>
            </div>

            <Alert className="mt-4 bg-[#0088FF]/10 border-[#0088FF]">
              <Info className="w-4 h-4" />
              <AlertDescription className="text-xs">
                This firing order will be used to configure injector and ignition outputs. Ensure your trigger
                system is properly calibrated for accurate cylinder identification.
              </AlertDescription>
            </Alert>
          </Card>
        )}

        {/* No Results Message */}
        {cylinders && layout && availableFiringOrders.length === 0 && (
          <Alert className="bg-[#FF9900]/10 border-[#FF9900]">
            <AlertTriangle className="w-4 h-4" />
            <AlertDescription className="text-xs">
              No firing orders found for this configuration. This combination may not be supported by Speeduino.
            </AlertDescription>
          </Alert>
        )}
      </div>

      {/* Quick Reference Guide */}
      {!selectedFiringOrder && (
        <div className="mt-6 pt-4 border-t border-[#333333]">
          <h5 className="text-xs text-[#00CCFF] font-semibold mb-2">💡 Quick Reference:</h5>
          <div className="grid grid-cols-2 gap-2 text-xs text-[#CCCCCC]">
            <div>
              <span className="text-[#888888]">Most Common I4:</span>
              <span className="ml-2 font-mono">1-3-4-2</span>
            </div>
            <div>
              <span className="text-[#888888]">Most Common I6:</span>
              <span className="ml-2 font-mono">1-5-3-6-2-4</span>
            </div>
            <div>
              <span className="text-[#888888]">Most Common V8:</span>
              <span className="ml-2 font-mono">1-8-4-3-6-5-7-2</span>
            </div>
            <div>
              <span className="text-[#888888]">Honda K-Series:</span>
              <span className="ml-2 font-mono">1-3-4-2</span>
            </div>
          </div>
        </div>
      )}
    </Card>
  );
}
