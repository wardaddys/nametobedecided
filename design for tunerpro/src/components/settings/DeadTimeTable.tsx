import { useState } from 'react';
import { Info, Download, Upload } from 'lucide-react';
import { InjectorSpec, interpolateDeadTime } from '../database/InjectorDatabase';

export interface DeadTimeData {
  // 2D table: voltage (rows) vs pulse width (columns)
  voltages: number[]; // e.g., [7, 9, 11, 13.5, 14.5]
  pulseWidths: number[]; // e.g., [0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 5.0]
  latencies: number[][]; // ms values
}

interface DeadTimeTableProps {
  data: DeadTimeData;
  onChange: (data: DeadTimeData) => void;
  injector?: InjectorSpec;
  onLoadFromInjector?: () => void;
}

export const DEFAULT_DEAD_TIME_DATA: DeadTimeData = {
  voltages: [7, 9, 11, 13.5, 14.5],
  pulseWidths: [0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 5.0],
  latencies: [
    [1.20, 1.15, 1.10, 1.05, 1.00, 0.95, 0.90, 0.85, 0.75], // 7V
    [1.05, 1.00, 0.95, 0.90, 0.85, 0.80, 0.75, 0.70, 0.60], // 9V
    [0.90, 0.85, 0.80, 0.75, 0.70, 0.65, 0.60, 0.55, 0.45], // 11V
    [0.75, 0.70, 0.65, 0.60, 0.55, 0.50, 0.45, 0.40, 0.30], // 13.5V
    [0.70, 0.65, 0.60, 0.55, 0.50, 0.45, 0.40, 0.35, 0.25], // 14.5V
  ],
};

export function DeadTimeTable({ data, onChange, injector, onLoadFromInjector }: DeadTimeTableProps) {
  const [selectedCell, setSelectedCell] = useState<{ row: number; col: number } | null>(null);
  const [editValue, setEditValue] = useState('');

  const handleCellClick = (row: number, col: number) => {
    setSelectedCell({ row, col });
    setEditValue(data.latencies[row][col].toFixed(2));
  };

  const handleCellChange = (value: string) => {
    setEditValue(value);
  };

  const handleCellBlur = () => {
    if (selectedCell) {
      const newValue = parseFloat(editValue);
      if (!isNaN(newValue) && newValue >= 0 && newValue <= 10) {
        const newLatencies = data.latencies.map((row, i) =>
          i === selectedCell.row
            ? row.map((val, j) => (j === selectedCell.col ? newValue : val))
            : row
        );
        onChange({ ...data, latencies: newLatencies });
      }
    }
    setSelectedCell(null);
  };

  const handleKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter') {
      handleCellBlur();
    } else if (e.key === 'Escape') {
      setSelectedCell(null);
    }
  };

  // Calculate color for cell based on latency value
  const getColorForLatency = (latency: number): string => {
    if (latency < 0.5) return 'bg-[#004400]'; // Dark green - good
    if (latency < 0.8) return 'bg-[#006600]'; // Green - good
    if (latency < 1.0) return 'bg-[#444400]'; // Yellow-ish - moderate
    if (latency < 1.5) return 'bg-[#664400]'; // Orange-ish - concerning
    return 'bg-[#440000]'; // Dark red - high latency
  };

  return (
    <div className="space-y-4">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div className="flex items-center gap-2">
          <h3 className="text-white">Dead Time Calibration Table</h3>
          <button className="text-[#00CCFF] hover:text-white">
            <Info className="w-4 h-4" />
          </button>
        </div>
        <div className="flex items-center gap-2">
          {injector && onLoadFromInjector && (
            <button
              onClick={onLoadFromInjector}
              className="bg-[#0066CC] hover:bg-[#0088EE] text-white px-3 py-1 rounded text-xs flex items-center gap-1"
            >
              <Download className="w-3 h-3" />
              Load from {injector.brand}
            </button>
          )}
          <button className="bg-[#3a3a3a] hover:bg-[#4a4a4a] text-white px-3 py-1 rounded text-xs flex items-center gap-1">
            <Upload className="w-3 h-3" />
            Import CSV
          </button>
        </div>
      </div>

      {/* Info Box */}
      <div className="bg-[#2a2a2a] border border-[#444444] rounded p-3 text-xs">
        <p className="text-[#CCCCCC] mb-2">
          <span className="text-[#00CCFF]">Dead Time:</span> Time delay between ECU command and actual fuel spray.
          Lower voltage = longer dead time. Critical for accurate fuel delivery at idle.
        </p>
        <div className="flex items-center gap-4">
          <div className="flex items-center gap-2">
            <div className="w-3 h-3 bg-[#004400]" />
            <span className="text-[#888888]">{'<'}0.5ms Excellent</span>
          </div>
          <div className="flex items-center gap-2">
            <div className="w-3 h-3 bg-[#444400]" />
            <span className="text-[#888888]">0.8-1.0ms Moderate</span>
          </div>
          <div className="flex items-center gap-2">
            <div className="w-3 h-3 bg-[#440000]" />
            <span className="text-[#888888]">{'>'}1.5ms High</span>
          </div>
        </div>
      </div>

      {/* Table */}
      <div className="bg-[#1a1a1a] border border-[#444444] rounded overflow-hidden">
        <div className="overflow-x-auto">
          <table className="w-full text-xs border-collapse">
            <thead>
              <tr className="bg-[#0066CC]">
                <th className="border border-[#444444] px-3 py-2 text-white text-left sticky left-0 bg-[#0066CC]">
                  Voltage (V)
                </th>
                {data.pulseWidths.map((pw, i) => (
                  <th key={i} className="border border-[#444444] px-3 py-2 text-white text-center min-w-[70px]">
                    {pw.toFixed(1)} ms
                  </th>
                ))}
              </tr>
            </thead>
            <tbody>
              {data.voltages.map((voltage, rowIndex) => (
                <tr key={rowIndex} className="hover:bg-[#2a2a2a]">
                  <td className="border border-[#444444] px-3 py-2 text-white sticky left-0 bg-[#2a2a2a]">
                    {voltage.toFixed(1)} V
                  </td>
                  {data.latencies[rowIndex].map((latency, colIndex) => {
                    const isSelected = selectedCell?.row === rowIndex && selectedCell?.col === colIndex;
                    return (
                      <td
                        key={colIndex}
                        className={`border border-[#444444] px-2 py-2 text-center cursor-pointer ${
                          getColorForLatency(latency)
                        } ${isSelected ? 'ring-2 ring-[#00FFFF]' : ''}`}
                        onClick={() => handleCellClick(rowIndex, colIndex)}
                      >
                        {isSelected ? (
                          <input
                            type="number"
                            value={editValue}
                            onChange={(e) => handleCellChange(e.target.value)}
                            onBlur={handleCellBlur}
                            onKeyDown={handleKeyDown}
                            className="w-full bg-transparent text-white text-center outline-none"
                            autoFocus
                            step="0.01"
                            min="0"
                            max="10"
                          />
                        ) : (
                          <span className="text-white">{latency.toFixed(2)}</span>
                        )}
                      </td>
                    );
                  })}
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>

      {/* Live Preview */}
      <div className="bg-[#2a2a2a] border border-[#444444] rounded p-3">
        <h4 className="text-white text-xs mb-2">Live Preview - Fuel Error Without Compensation</h4>
        <div className="grid grid-cols-3 gap-2 text-xs">
          <div className="bg-[#1a1a1a] p-2 rounded">
            <div className="text-[#888888] mb-1">@ Idle (1ms PW, 12V)</div>
            <div className="text-[#FF3B30]">
              Error: {selectedCell 
                ? ((data.latencies[1][1] / 1.0) * 100 - 100).toFixed(1) 
                : (0.85 / 1.0 * 100 - 100).toFixed(1)}%
            </div>
          </div>
          <div className="bg-[#1a1a1a] p-2 rounded">
            <div className="text-[#888888] mb-1">@ Cruise (3ms PW, 14V)</div>
            <div className="text-[#FFD700]">
              Error: {selectedCell 
                ? ((data.latencies[4][5] / 3.0) * 100 - 100).toFixed(1) 
                : (0.45 / 3.0 * 100 - 100).toFixed(1)}%
            </div>
          </div>
          <div className="bg-[#1a1a1a] p-2 rounded">
            <div className="text-[#888888] mb-1">@ WOT (10ms PW, 13V)</div>
            <div className="text-[#00FF00]">
              Error: {selectedCell 
                ? ((data.latencies[3][8] / 10.0) * 100 - 100).toFixed(1) 
                : (0.30 / 10.0 * 100 - 100).toFixed(1)}%
            </div>
          </div>
        </div>
      </div>

      {/* Instructions */}
      <div className="text-xs text-[#888888]">
        <p>• Click any cell to edit • Green = low latency (good) • Red = high latency (concerning)</p>
        <p>• Lower voltage creates longer dead time due to slower solenoid response</p>
        <p>• Accurate dead time critical for idle stability and low-pulse-width accuracy</p>
      </div>
    </div>
  );
}
