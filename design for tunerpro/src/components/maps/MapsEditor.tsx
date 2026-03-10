import { useState, useEffect, useRef } from "react";
import { useVTEC } from "../context/VTECContext";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "../ui/tabs";
import { Button } from "../ui/button";
import { Surface3D } from "./Surface3D";
import { Heatmap2D } from "./Heatmap2D";
import { AxisConfig } from "./AxisConfig";
import { Input } from "../ui/input";
import { 
  Download, 
  Upload, 
  RotateCcw, 
  Copy, 
  Plus, 
  Minus, 
  Grid3x3, 
  Grid2x2,
  Settings2,
  FileText
} from "lucide-react";

interface MapsEditorProps {
  liveTuning: boolean;
}

export function MapsEditor({ liveTuning }: MapsEditorProps) {
  const { vtecEnabled, currentMap, setCurrentMap } = useVTEC();
  const [activeTab, setActiveTab] = useState<"fuel" | "ignition">("fuel");
  const [selectedCells, setSelectedCells] = useState<Set<string>>(new Set());
  const [viewMode, setViewMode] = useState<"2d" | "3d">("3d");
  const [showAxisConfig, setShowAxisConfig] = useState(false);
  const [incrementValue, setIncrementValue] = useState(0.25);
  const [isRunning, setIsRunning] = useState(false);
  const [activeCell, setActiveCell] = useState<{ row: number; col: number } | null>(null);
  const lastCellRef = useRef<string | null>(null);

  // Automatically sync ECU simulation with Live Tuning state
  useEffect(() => {
    setIsRunning(liveTuning);
  }, [liveTuning]);

  // RPM axis (columns) - 10 columns - now editable
  const [rpmAxis, setRpmAxis] = useState([1000, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 7000, 8000]);

  // MAP axis (rows) - 16 rows - now editable
  const [mapAxis, setMapAxis] = useState([200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700]);

  // Fuel table data (AFR values) - 16 rows x 10 columns
  const [fuelTableLow, setFuelTableLow] = useState<number[][]>([
    [18.50, 18.50, 18.50, 18.50, 13.00, 8.00, 3.25, 0.00, -2.00, -3.00],
    [18.50, 18.50, 18.50, 18.50, 13.00, 8.00, 3.25, 0.00, -2.00, -3.00],
    [23.75, 23.75, 23.75, 21.25, 16.50, 12.00, 6.00, 3.00, 0.00, -1.00],
    [30.25, 30.25, 30.25, 28.00, 24.50, 19.00, 14.00, 11.00, 8.00, 4.50],
    [30.25, 30.75, 30.75, 30.50, 27.50, 22.50, 19.00, 15.75, 13.50, 9.50],
    [34.50, 34.50, 34.50, 32.25, 29.25, 25.50, 21.00, 18.25, 13.00, 12.00],
    [36.75, 36.75, 36.75, 35.75, 30.50, 28.00, 24.50, 21.25, 17.00, 14.00],
    [38.00, 40.00, 40.50, 37.25, 34.50, 31.50, 25.00, 21.25, 16.75, 13.25],
    [40.50, 43.25, 43.25, 40.75, 37.50, 34.50, 28.00, 23.00, 18.00, 16.00],
    [44.00, 44.00, 44.00, 42.00, 40.00, 38.00, 34.00, 31.50, 28.25, 26.25],
    [44.00, 44.00, 44.00, 42.00, 40.00, 38.00, 34.00, 31.50, 28.25, 26.25],
    [44.00, 44.00, 44.00, 42.50, 40.50, 38.75, 35.00, 32.75, 29.25, 28.25],
    [44.75, 44.75, 44.75, 42.75, 40.75, 39.00, 35.50, 33.00, 30.25, 29.25],
    [44.75, 44.75, 44.75, 42.75, 40.75, 39.00, 35.50, 33.00, 30.25, 29.25],
    [44.75, 44.75, 44.75, 42.75, 40.75, 39.00, 35.50, 33.00, 30.25, 30.25],
    [44.75, 44.75, 44.75, 42.75, 40.75, 39.00, 35.50, 33.00, 30.25, 30.25],
  ]);

  const [fuelTableHigh, setFuelTableHigh] = useState<number[][]>([
    [18.50, 18.50, 18.50, 18.50, 13.00, 8.00, 3.25, 0.00, -2.00, -3.00],
    [18.50, 18.50, 18.50, 18.50, 13.00, 8.00, 3.25, 0.00, -2.00, -3.00],
    [23.75, 23.75, 23.75, 21.25, 16.50, 12.20, 6.00, 3.00, 0.00, -1.00],
    [30.25, 30.25, 30.25, 28.00, 24.50, 20.50, 14.00, 11.00, 8.00, 4.50],
    [34.50, 34.50, 34.50, 32.25, 29.25, 26.50, 21.00, 16.25, 11.00, 12.00],
    [34.00, 34.00, 34.00, 32.25, 29.25, 25.00, 19.50, 14.75, 11.00, 9.50],
    [44.00, 44.00, 44.00, 41.75, 38.50, 36.50, 29.75, 24.75, 20.75, 19.75],
    [44.00, 44.00, 44.00, 42.00, 40.00, 38.00, 32.75, 29.50, 26.00, 26.00],
    [44.00, 44.00, 44.00, 42.00, 40.00, 38.00, 34.00, 31.50, 28.25, 28.25],
    [44.00, 44.00, 44.00, 42.00, 40.00, 38.00, 34.00, 31.50, 28.25, 28.25],
    [44.00, 44.00, 44.00, 42.00, 40.00, 38.00, 34.00, 31.50, 28.25, 28.25],
    [44.75, 44.75, 44.75, 42.50, 40.75, 38.75, 35.00, 32.75, 29.25, 29.25],
    [44.75, 44.75, 44.75, 42.75, 40.75, 38.75, 35.00, 32.75, 29.25, 29.25],
    [44.75, 44.75, 44.75, 42.75, 40.75, 39.00, 35.50, 33.00, 30.25, 30.25],
    [44.75, 44.75, 44.75, 42.75, 40.75, 39.00, 36.00, 33.50, 30.25, 30.25],
    [44.75, 44.75, 44.75, 42.75, 40.75, 39.00, 36.00, 33.50, 30.25, 30.25],
  ]);

  // Ignition table data (degrees) - 16 rows x 10 columns
  const [ignitionTableLow, setIgnitionTableLow] = useState<number[][]>([
    [10, 12, 15, 18, 20, 22, 24, 26, 28, 30],
    [12, 14, 16, 19, 22, 24, 26, 28, 30, 32],
    [14, 16, 18, 21, 24, 26, 28, 30, 32, 34],
    [16, 18, 20, 23, 26, 28, 30, 32, 34, 35],
    [18, 20, 22, 25, 28, 30, 32, 34, 35, 36],
    [20, 22, 24, 27, 30, 32, 34, 35, 36, 37],
    [22, 24, 26, 29, 32, 34, 35, 36, 37, 38],
    [24, 26, 28, 31, 34, 35, 36, 37, 38, 38],
    [26, 28, 30, 32, 35, 36, 37, 38, 38, 38],
    [28, 29, 31, 33, 35, 36, 37, 38, 38, 38],
    [28, 29, 31, 33, 35, 36, 37, 38, 38, 38],
    [28, 29, 31, 33, 35, 36, 37, 38, 38, 38],
    [28, 29, 31, 33, 35, 36, 37, 38, 38, 38],
    [28, 29, 31, 33, 35, 36, 37, 38, 38, 38],
    [28, 29, 31, 33, 35, 36, 37, 38, 38, 38],
    [28, 29, 31, 33, 35, 36, 37, 38, 38, 38],
  ]);

  const [ignitionTableHigh, setIgnitionTableHigh] = useState<number[][]>([
    [10, 12, 15, 18, 20, 22, 24, 26, 28, 30],
    [12, 14, 16, 19, 22, 24, 26, 28, 30, 32],
    [14, 16, 18, 21, 24, 26, 28, 30, 32, 34],
    [16, 18, 20, 23, 26, 28, 30, 32, 34, 35],
    [18, 20, 22, 25, 28, 30, 32, 34, 36, 37],
    [20, 22, 24, 27, 30, 32, 34, 36, 37, 38],
    [22, 24, 26, 29, 32, 34, 36, 37, 38, 39],
    [24, 26, 28, 31, 34, 36, 37, 38, 39, 40],
    [26, 28, 30, 32, 35, 37, 38, 39, 40, 40],
    [28, 29, 31, 33, 36, 38, 39, 40, 40, 40],
    [28, 29, 31, 34, 36, 38, 39, 40, 40, 40],
    [28, 30, 32, 34, 37, 38, 39, 40, 40, 40],
    [28, 30, 32, 35, 37, 38, 39, 40, 40, 40],
    [28, 30, 32, 35, 37, 38, 39, 40, 40, 40],
    [28, 30, 32, 35, 37, 38, 39, 40, 40, 40],
    [28, 30, 32, 35, 37, 38, 39, 40, 40, 40],
  ]);

  // Simulate ECU running - active cell tracking
  useEffect(() => {
    if (!isRunning) {
      setActiveCell(null);
      return;
    }

    const interval = setInterval(() => {
      // Simulate RPM sweeping between 2000-7000
      const time = Date.now() / 1000;
      const rpm = 3000 + Math.sin(time * 0.5) * 2000 + Math.random() * 500;
      
      // Simulate MAP pressure varying
      const map = 600 + Math.sin(time * 0.3) * 400 + Math.random() * 200;

      // Find closest cell
      let closestRpmIdx = 0;
      let minRpmDiff = Math.abs(rpmAxis[0] - rpm);
      for (let i = 1; i < rpmAxis.length; i++) {
        const diff = Math.abs(rpmAxis[i] - rpm);
        if (diff < minRpmDiff) {
          minRpmDiff = diff;
          closestRpmIdx = i;
        }
      }

      let closestMapIdx = 0;
      let minMapDiff = Math.abs(mapAxis[0] - map);
      for (let i = 1; i < mapAxis.length; i++) {
        const diff = Math.abs(mapAxis[i] - map);
        if (diff < minMapDiff) {
          minMapDiff = diff;
          closestMapIdx = i;
        }
      }

      setActiveCell({ row: closestMapIdx, col: closestRpmIdx });
    }, 100);

    return () => clearInterval(interval);
  }, [isRunning, rpmAxis, mapAxis]);

  const getCurrentTable = () => {
    if (activeTab === "fuel") {
      return vtecEnabled && currentMap === "high" ? fuelTableHigh : fuelTableLow;
    } else {
      return vtecEnabled && currentMap === "high" ? ignitionTableHigh : ignitionTableLow;
    }
  };

  const getCurrentSetTable = () => {
    if (activeTab === "fuel") {
      return vtecEnabled && currentMap === "high" ? setFuelTableHigh : setFuelTableLow;
    } else {
      return vtecEnabled && currentMap === "high" ? setIgnitionTableHigh : setIgnitionTableLow;
    }
  };

  const handleCellChange = (rowIndex: number, colIndex: number, value: string) => {
    const numValue = parseFloat(value);
    if (isNaN(numValue)) return;

    const currentTable = getCurrentTable();
    const setTable = getCurrentSetTable();
    const newTable = currentTable.map((row, rIdx) =>
      rIdx === rowIndex ? row.map((cell, cIdx) => (cIdx === colIndex ? numValue : cell)) : row
    );
    setTable(newTable);
  };

  const handleCellClick = (rowIndex: number, colIndex: number, ctrlKey: boolean) => {
    const cellKey = `${rowIndex}-${colIndex}`;
    
    if (ctrlKey) {
      // Multi-select with Ctrl
      const newSelection = new Set(selectedCells);
      if (newSelection.has(cellKey)) {
        newSelection.delete(cellKey);
      } else {
        newSelection.add(cellKey);
      }
      setSelectedCells(newSelection);
      lastCellRef.current = cellKey;
    } else {
      // Single select
      setSelectedCells(new Set([cellKey]));
      lastCellRef.current = cellKey;
    }
  };

  const handleIncrement = () => {
    if (selectedCells.size === 0) return;

    const currentTable = getCurrentTable();
    const setTable = getCurrentSetTable();
    const newTable = currentTable.map((row, rowIdx) =>
      row.map((cell, colIdx) => {
        const cellKey = `${rowIdx}-${colIdx}`;
        return selectedCells.has(cellKey) ? cell + incrementValue : cell;
      })
    );
    setTable(newTable);
  };

  const handleDecrement = () => {
    if (selectedCells.size === 0) return;

    const currentTable = getCurrentTable();
    const setTable = getCurrentSetTable();
    const newTable = currentTable.map((row, rowIdx) =>
      row.map((cell, colIdx) => {
        const cellKey = `${rowIdx}-${colIdx}`;
        return selectedCells.has(cellKey) ? cell - incrementValue : cell;
      })
    );
    setTable(newTable);
  };

  return (
    <div className="h-full bg-[#1a1a1a] flex flex-col">
      {/* Clean Toolbar - AEM Style */}
      <div className="border-b border-[#444444] bg-[#1a1a1a] px-3 py-2">
        <div className="flex items-center justify-between">
          {/* Left - Map Selection */}
          <div className="flex items-center gap-3">
            <div className="flex gap-1 bg-[#2a2a2a] rounded border border-[#444444]">
              <button
                onClick={() => setActiveTab("fuel")}
                className={`px-3 py-1.5 text-xs transition-colors ${
                  activeTab === "fuel"
                    ? "bg-[#0066CC] text-white"
                    : "text-[#CCCCCC] hover:bg-[#333333]"
                }`}
              >
                Fuel Map
              </button>
              <button
                onClick={() => setActiveTab("ignition")}
                className={`px-3 py-1.5 text-xs transition-colors ${
                  activeTab === "ignition"
                    ? "bg-[#FF6600] text-white"
                    : "text-[#CCCCCC] hover:bg-[#333333]"
                }`}
              >
                Ignition
              </button>
            </div>

            {vtecEnabled && (
              <div className="flex gap-1 bg-[#2a2a2a] rounded border border-[#444444]">
                <button
                  onClick={() => setCurrentMap("low")}
                  className={`px-3 py-1.5 text-xs transition-colors ${
                    currentMap === "low"
                      ? "bg-[#0066CC] text-white"
                      : "text-[#CCCCCC] hover:bg-[#333333]"
                  }`}
                >
                  Low Cam
                </button>
                <button
                  onClick={() => setCurrentMap("high")}
                  className={`px-3 py-1.5 text-xs transition-colors ${
                    currentMap === "high"
                      ? "bg-[#0066CC] text-white"
                      : "text-[#CCCCCC] hover:bg-[#333333]"
                  }`}
                >
                  High Cam
                </button>
              </div>
            )}

            {isRunning && (
              <div className="flex items-center gap-2 px-2 py-1 rounded bg-[#00AA00]/20 border border-[#00AA00]">
                <div className="w-2 h-2 rounded-full bg-[#00FF00] animate-pulse" />
                <span className="text-[10px] text-[#00FF00]">LIVE</span>
              </div>
            )}
          </div>

          {/* Right - Controls */}
          <div className="flex items-center gap-2">
            {/* View Toggle */}
            <div className="flex gap-0.5 bg-[#2a2a2a] rounded border border-[#444444]">
              <button
                onClick={() => setViewMode("2d")}
                className={`px-2 py-1.5 text-xs transition-colors flex items-center gap-1 ${
                  viewMode === "2d"
                    ? "bg-[#3a3a3a] text-white"
                    : "text-[#CCCCCC] hover:bg-[#333333]"
                }`}
                title="Table View"
              >
                <FileText className="w-3.5 h-3.5" />
              </button>
              <button
                onClick={() => setViewMode("3d")}
                className={`px-2 py-1.5 text-xs transition-colors flex items-center gap-1 ${
                  viewMode === "3d"
                    ? "bg-[#3a3a3a] text-white"
                    : "text-[#CCCCCC] hover:bg-[#333333]"
                }`}
                title="3D Graph"
              >
                <Grid3x3 className="w-3.5 h-3.5" />
              </button>
            </div>

            <div className="h-4 w-px bg-[#444444]" />

            {/* Action Buttons */}
            <button 
              className="px-2 py-1.5 text-xs text-[#CCCCCC] hover:bg-[#333333] rounded border border-[#444444] flex items-center gap-1"
              onClick={() => setShowAxisConfig(!showAxisConfig)}
              title="Axis Configuration"
            >
              <Settings2 className="w-3.5 h-3.5" />
            </button>
            <button className="px-2 py-1.5 text-xs text-[#CCCCCC] hover:bg-[#333333] rounded border border-[#444444] flex items-center gap-1" title="Import">
              <Upload className="w-3.5 h-3.5" />
            </button>
            <button className="px-2 py-1.5 text-xs text-[#CCCCCC] hover:bg-[#333333] rounded border border-[#444444] flex items-center gap-1" title="Export">
              <Download className="w-3.5 h-3.5" />
            </button>
            <button className="px-2 py-1.5 text-xs text-[#CCCCCC] hover:bg-[#333333] rounded border border-[#444444] flex items-center gap-1" title="Copy">
              <Copy className="w-3.5 h-3.5" />
            </button>
            <button className="px-2 py-1.5 text-xs text-[#CCCCCC] hover:bg-[#333333] rounded border border-[#444444] flex items-center gap-1" title="Reset">
              <RotateCcw className="w-3.5 h-3.5" />
            </button>
          </div>
        </div>

        {/* Edit Controls */}
        {selectedCells.size > 0 && (
          <div className="mt-2 flex items-center gap-3 bg-[#2a2a2a] p-2 rounded border border-[#444444]">
            <span className="text-xs text-[#888888]">
              {selectedCells.size} cell{selectedCells.size !== 1 ? "s" : ""} selected
            </span>

            <div className="h-3 w-px bg-[#444444]" />

            <div className="flex items-center gap-2">
              <span className="text-xs text-[#888888]">Step:</span>
              <Input
                type="number"
                value={incrementValue}
                onChange={(e) => setIncrementValue(Number(e.target.value))}
                className="w-16 h-6 bg-[#1a1a1a] border-[#444444] text-white text-xs px-2"
                step="0.25"
              />
            </div>

            <button
              onClick={handleIncrement}
              className="px-2 py-1 text-xs bg-[#00AA00]/20 text-[#00FF00] border border-[#00AA00] hover:bg-[#00AA00]/30 rounded flex items-center gap-1"
            >
              <Plus className="w-3 h-3" />
              Add
            </button>

            <button
              onClick={handleDecrement}
              className="px-2 py-1 text-xs bg-[#AA0000]/20 text-[#FF6666] border border-[#AA0000] hover:bg-[#AA0000]/30 rounded flex items-center gap-1"
            >
              <Minus className="w-3 h-3" />
              Subtract
            </button>

            <button
              onClick={() => setSelectedCells(new Set())}
              className="px-2 py-1 text-xs text-[#888888] hover:text-white hover:bg-[#333333] rounded"
            >
              Clear
            </button>
          </div>
        )}
      </div>

      {/* Axis Config Panel */}
      {showAxisConfig && (
        <AxisConfig
          rpmAxis={rpmAxis}
          mapAxis={mapAxis}
          onRpmAxisChange={setRpmAxis}
          onMapAxisChange={setMapAxis}
          onClose={() => setShowAxisConfig(false)}
        />
      )}

      {/* Main Content */}
      <div className="flex-1 flex overflow-hidden">
        {/* Left Side - Table like AEM - MORE SPACE */}
        <div className="w-[750px] border-r border-[#444444] bg-[#1a1a1a] overflow-auto">
          <div className="p-2">
            <div className="bg-[#2a2a2a] border border-[#444444] rounded overflow-hidden">
              <table className="w-full border-collapse text-xs">
                <thead>
                  <tr className="bg-[#0066CC]">
                    <th className="border border-[#005599] px-2 py-1.5 text-white sticky left-0 bg-[#0066CC] z-10">
                      MAP/RPM
                    </th>
                    {rpmAxis.map((rpm, idx) => (
                      <th key={idx} className="border border-[#005599] px-2 py-1.5 text-white text-center min-w-[50px]">
                        {rpm}
                      </th>
                    ))}
                  </tr>
                </thead>
                <tbody>
                  {getCurrentTable().map((row, rowIdx) => (
                    <tr key={rowIdx} className="hover:bg-[#333333] transition-colors">
                      <td className="border border-[#444444] px-2 py-1.5 text-[#CCCCCC] sticky left-0 bg-[#2a2a2a] z-10">
                        {mapAxis[rowIdx]}
                      </td>
                      {row.map((cell, colIdx) => {
                        const cellKey = `${rowIdx}-${colIdx}`;
                        const isSelected = selectedCells.has(cellKey);
                        const isActive = activeCell?.row === rowIdx && activeCell?.col === colIdx;

                        return (
                          <td
                            key={colIdx}
                            className={`border border-[#444444] p-0 text-center cursor-pointer transition-colors ${
                              isActive
                                ? "bg-[#00FF00]"
                                : isSelected
                                ? "bg-[#0088FF]"
                                : "hover:bg-[#333333]"
                            }`}
                            onClick={(e) => handleCellClick(rowIdx, colIdx, e.ctrlKey || e.metaKey)}
                          >
                            <input
                              type="number"
                              value={cell.toFixed(2)}
                              onChange={(e) => handleCellChange(rowIdx, colIdx, e.target.value)}
                              className={`w-full bg-transparent text-center px-2 py-1.5 border-0 outline-none font-mono ${
                                isActive
                                  ? "text-black"
                                  : isSelected
                                  ? "text-white"
                                  : "text-[#CCCCCC]"
                              }`}
                              step="0.25"
                            />
                          </td>
                        );
                      })}
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          </div>
        </div>

        {/* Right Side - Visualization */}
        <div className="flex-1 bg-[#1a1a1a] p-3">
          <div className="w-full h-full bg-black border border-[#444444] rounded overflow-hidden">
            {viewMode === "3d" ? (
              <Surface3D
                data={getCurrentTable()}
                selectedCell={activeCell || (selectedCells.size === 1 ? (() => {
                  const [key] = selectedCells;
                  const [row, col] = key.split('-').map(Number);
                  return { row, col };
                })() : null)}
                rpmAxis={rpmAxis}
                mapAxis={mapAxis}
                title={`${activeTab === "fuel" ? "Fuel Map" : "Ignition Map"}${vtecEnabled ? ` - ${currentMap === "low" ? "Low Cam" : "High Cam"}` : ""}`}
                unit={activeTab === "fuel" ? "AFR" : "°"}
              />
            ) : (
              <Heatmap2D
                data={getCurrentTable()}
                selectedCells={selectedCells}
                activeCell={activeCell}
                rpmAxis={rpmAxis}
                mapAxis={mapAxis}
                title={`${activeTab === "fuel" ? "Fuel Map" : "Ignition Map"}${vtecEnabled ? ` - ${currentMap === "low" ? "Low Cam" : "High Cam"}` : ""}`}
                unit={activeTab === "fuel" ? "AFR" : "°"}
                onCellClick={(row, col) => handleCellClick(row, col, false)}
              />
            )}
          </div>
        </div>
      </div>
    </div>
  );
}
