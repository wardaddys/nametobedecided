import { useCallback } from 'react';
import { TableDefinition } from './TableDefinitions';

interface HondataHeatmap2DProps {
  tableData: number[][];
  definition: TableDefinition;
  activeCell?: { row: number; col: number } | null;
  selectedCells?: Set<string>;
  onCellClick?: (row: number, col: number) => void;
  compact?: boolean;
}

export function HondataHeatmap2D({ 
  tableData, 
  definition,
  activeCell,
  selectedCells,
  onCellClick,
  compact = false,
}: HondataHeatmap2DProps) {
  
  // Hondata color mapping: Blue (low) -> Cyan -> Green -> Yellow -> Orange -> Red (high)
  const getHondataColor = useCallback((value: number, minValue: number, maxValue: number): string => {
    const normalized = (value - minValue) / (maxValue - minValue);
    
    // Hondata spectral gradient matching their software
    if (normalized < 0.2) {
      // Blue -> Cyan
      const t = normalized / 0.2;
      const g = Math.floor(100 + 155 * t);
      return `rgb(0, ${g}, 255)`;
    } else if (normalized < 0.4) {
      // Cyan -> Green
      const t = (normalized - 0.2) / 0.2;
      const b = Math.floor(255 * (1 - t));
      return `rgb(0, 255, ${b})`;
    } else if (normalized < 0.6) {
      // Green -> Yellow
      const t = (normalized - 0.4) / 0.2;
      const r = Math.floor(255 * t);
      return `rgb(${r}, 255, 0)`;
    } else if (normalized < 0.8) {
      // Yellow -> Orange
      const t = (normalized - 0.6) / 0.2;
      const g = Math.floor(255 - 105 * t);
      return `rgb(255, ${g}, 0)`;
    } else {
      // Orange -> Red
      const t = (normalized - 0.8) / 0.2;
      const g = Math.floor(150 * (1 - t));
      return `rgb(255, ${g}, 0)`;
    }
  }, []);

  const flatData = tableData.flat();
  const minValue = Math.min(...flatData);
  const maxValue = Math.max(...flatData);

  const rows = tableData.length;
  const cols = tableData[0]?.length || 0;

  return (
    <div className="flex flex-col h-full w-full bg-black border border-[#444444] rounded overflow-hidden">
      {/* Header */}
      {!compact && (
        <div className="bg-[#1a1a1a] border-b border-[#444444] px-3 py-2">
          <div className="text-xs text-[#00CCFF]">{definition.name}</div>
          <div className="text-[10px] text-[#888888] mt-0.5">
            {definition.zAxis.label} ({definition.zAxis.unit}): {minValue.toFixed(1)} - {maxValue.toFixed(1)}
          </div>
        </div>
      )}

      {/* 2D Heatmap Grid */}
      <div className="flex-1 overflow-auto p-2">
        <div className="inline-block min-w-full">
          <table className="border-collapse" style={{ fontSize: compact ? '9px' : '10px' }}>
            <thead>
              <tr>
                <th className="bg-[#0066CC] border border-[#004488] px-2 py-1 text-white sticky left-0 z-20">
                  <div className="flex flex-col items-center">
                    <span>{definition.yAxis.label}</span>
                    <span className="text-[9px] opacity-70">({definition.yAxis.unit})</span>
                  </div>
                  <div className="text-[9px] mt-1 opacity-70">/ {definition.xAxis.label} ({definition.xAxis.unit})</div>
                </th>
                {definition.xAxis.values.map((value, idx) => (
                  <th 
                    key={idx} 
                    className="bg-[#0066CC] border border-[#004488] px-2 py-1 text-white text-center min-w-[45px]"
                  >
                    {value}
                  </th>
                ))}
              </tr>
            </thead>
            <tbody>
              {tableData.map((row, rowIdx) => (
                <tr key={rowIdx}>
                  <td className="bg-[#0066CC] border border-[#004488] px-2 py-1 text-white sticky left-0 z-10">
                    {definition.yAxis.values[rowIdx]}
                  </td>
                  {row.map((cellValue, colIdx) => {
                    const cellKey = `${rowIdx}-${colIdx}`;
                    const isSelected = selectedCells?.has(cellKey);
                    const isActive = activeCell?.row === rowIdx && activeCell?.col === colIdx;
                    const bgColor = getHondataColor(cellValue, minValue, maxValue);

                    return (
                      <td
                        key={colIdx}
                        className={`border border-black text-center cursor-pointer transition-all ${
                          isActive
                            ? 'ring-2 ring-cyan-400 ring-inset'
                            : isSelected
                            ? 'ring-2 ring-blue-500 ring-inset'
                            : ''
                        }`}
                        style={{ 
                          backgroundColor: bgColor,
                          color: '#FFFFFF',
                          textShadow: '0 0 2px rgba(0,0,0,0.8)',
                        }}
                        onClick={() => onCellClick?.(rowIdx, colIdx)}
                      >
                        <div className="px-1.5 py-1">
                          {cellValue.toFixed(definition.zAxis.unit === 'bool' ? 0 : 1)}
                        </div>
                      </td>
                    );
                  })}
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>

      {/* Color Legend */}
      <div className="bg-[#1a1a1a] border-t border-[#444444] px-3 py-2">
        <div className="flex items-center gap-2">
          <span className="text-[10px] text-[#888888]">Scale:</span>
          <div className="flex-1 h-4 rounded overflow-hidden" style={{
            background: `linear-gradient(to right, 
              rgb(0, 100, 255) 0%,
              rgb(0, 255, 255) 20%,
              rgb(0, 255, 0) 40%,
              rgb(255, 255, 0) 60%,
              rgb(255, 150, 0) 80%,
              rgb(255, 0, 0) 100%
            )`
          }} />
          <div className="flex gap-3 text-[10px] text-[#CCCCCC]">
            <span>{minValue.toFixed(1)}</span>
            <span>{maxValue.toFixed(1)} {definition.zAxis.unit}</span>
          </div>
        </div>
      </div>
    </div>
  );
}
