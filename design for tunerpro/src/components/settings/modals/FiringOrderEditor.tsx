import { useState } from "react";
import { Card } from "../../ui/card";
import { Label } from "../../ui/label";
import { Input } from "../../ui/input";
import { Button } from "../../ui/button";
import { X } from "lucide-react";

interface FiringOrderEditorProps {
  numCylinders: number;
  onClose: () => void;
  onApply: () => void;
}

export function FiringOrderEditor({ numCylinders, onClose, onApply }: FiringOrderEditorProps) {
  const cylLabels = ["A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L"];
  const [firingOrder, setFiringOrder] = useState<number[]>(
    numCylinders === 4 ? [1, 3, 4, 2] :
    numCylinders === 6 ? [1, 5, 3, 6, 2, 4] :
    numCylinders === 8 ? [1, 8, 4, 3, 6, 5, 7, 2] :
    Array.from({ length: numCylinders }, (_, i) => i + 1)
  );
  const [injectorTrim, setInjectorTrim] = useState<number[]>(Array(numCylinders).fill(0));

  const handleFiringOrderChange = (index: number, value: string) => {
    const newOrder = [...firingOrder];
    newOrder[index] = parseInt(value) || 0;
    setFiringOrder(newOrder);
  };

  const handleTrimChange = (index: number, value: number) => {
    const newTrim = [...injectorTrim];
    newTrim[index] = Math.max(-10, Math.min(10, value));
    setInjectorTrim(newTrim);
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/80 backdrop-blur-sm">
      <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#00FF88] w-full max-w-2xl p-6 m-4 max-h-[90vh] overflow-y-auto">
        {/* Header */}
        <div className="flex items-center justify-between mb-6">
          <div className="flex items-center gap-3">
            <span className="text-2xl">🔢</span>
            <h3 className="text-white text-xl font-semibold">Firing Order Configuration</h3>
          </div>
          <button
            onClick={onClose}
            className="text-[#888888] hover:text-white transition-colors"
          >
            <X className="w-5 h-5" />
          </button>
        </div>

        {/* Firing Order */}
        <div className="mb-6">
          <Label className="text-[#CCCCCC] mb-3 block">Firing Order</Label>
          <div className="grid grid-cols-8 gap-2">
            {Array.from({ length: numCylinders }).map((_, index) => (
              <div key={index} className="text-center">
                <div className="text-xs text-[#888888] mb-1">Cyl {cylLabels[index]}</div>
                <Input
                  type="number"
                  min="1"
                  max={numCylinders}
                  value={firingOrder[index]}
                  onChange={(e) => handleFiringOrderChange(index, e.target.value)}
                  className="bg-black border-[#555555] text-white font-mono text-center p-2"
                />
              </div>
            ))}
          </div>
        </div>

        {/* Injector Trim */}
        <div>
          <Label className="text-[#CCCCCC] mb-3 block">Injector Trim (%)</Label>
          <div className="space-y-3">
            {Array.from({ length: numCylinders }).map((_, index) => (
              <div key={index} className="flex items-center gap-3">
                <span className="text-white text-sm w-12">Cyl {cylLabels[index]}:</span>
                <input
                  type="range"
                  min="-10"
                  max="10"
                  step="0.5"
                  value={injectorTrim[index]}
                  onChange={(e) => handleTrimChange(index, parseFloat(e.target.value))}
                  className="flex-1 h-2 bg-[#333333] rounded-lg appearance-none cursor-pointer accent-[#00FF88]"
                  style={{
                    background: `linear-gradient(to right, 
                      #333333 0%, 
                      #333333 ${((injectorTrim[index] + 10) / 20) * 100}%, 
                      #00FF88 ${((injectorTrim[index] + 10) / 20) * 100}%, 
                      #00FF88 ${((injectorTrim[index] + 10) / 20) * 100}%, 
                      #333333 ${((injectorTrim[index] + 10) / 20) * 100}%)`
                  }}
                />
                <Input
                  type="number"
                  step="0.5"
                  value={injectorTrim[index]}
                  onChange={(e) => handleTrimChange(index, parseFloat(e.target.value))}
                  className="w-20 bg-black border-[#555555] text-white font-mono text-center"
                />
                <span className="text-[#888888] text-sm w-16">[-10 to +10]</span>
              </div>
            ))}
          </div>
        </div>

        {/* Info */}
        <div className="mt-6 p-3 bg-[#00CCFF]/10 border border-[#00CCFF] rounded">
          <p className="text-xs text-[#CCCCCC]">
            <strong className="text-[#00CCFF]">Firing Order:</strong> Defines the sequence in which cylinders fire.
            Common patterns: 4-cyl: 1-3-4-2, 6-cyl: 1-5-3-6-2-4, V8: 1-8-4-3-6-5-7-2
          </p>
          <p className="text-xs text-[#CCCCCC] mt-2">
            <strong className="text-[#00CCFF]">Injector Trim:</strong> Individual cylinder fuel correction to balance AFR across all cylinders. Use +/- % to add or remove fuel.
          </p>
        </div>

        {/* Actions */}
        <div className="flex gap-3 mt-6">
          <Button
            variant="outline"
            onClick={onClose}
            className="flex-1 border-[#555555]"
          >
            Cancel
          </Button>
          <Button
            onClick={onApply}
            className="flex-1 bg-gradient-to-r from-[#00FF88] to-[#00CC66] hover:from-[#00DD77] hover:to-[#00BB55] text-black font-semibold"
          >
            Apply
          </Button>
        </div>
      </Card>
    </div>
  );
}
