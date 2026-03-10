import { useEffect, useRef } from 'react';

interface Heatmap2DProps {
  data: number[][];
  selectedCells: Set<string>;
  activeCell: { row: number; col: number } | null;
  rpmAxis: number[];
  mapAxis: number[];
  title: string;
  unit: string;
  onCellClick?: (row: number, col: number) => void;
}

export function Heatmap2D({ data, selectedCells, activeCell, rpmAxis, mapAxis, title, unit, onCellClick }: Heatmap2DProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const width = canvas.width;
    const height = canvas.height;

    // Clear canvas
    ctx.fillStyle = '#000000';
    ctx.fillRect(0, 0, width, height);

    const rows = data.length;
    const cols = data[0].length;

    // Calculate cell dimensions
    const leftMargin = 80;
    const topMargin = 80;
    const rightMargin = 60;
    const bottomMargin = 60;
    
    const cellWidth = (width - leftMargin - rightMargin) / cols;
    const cellHeight = (height - topMargin - bottomMargin) / rows;

    // Find min and max for color mapping
    const flatData = data.flat();
    const minValue = Math.min(...flatData);
    const maxValue = Math.max(...flatData);

    // Helper function to get color based on value
    const getColor = (value: number): string => {
      const normalized = (value - minValue) / (maxValue - minValue);
      
      if (normalized < 0.2) {
        const t = normalized / 0.2;
        return `rgb(0, ${Math.floor(100 * t)}, ${Math.floor(150 + 105 * t)})`;
      } else if (normalized < 0.4) {
        const t = (normalized - 0.2) / 0.2;
        return `rgb(0, ${Math.floor(100 + 155 * t)}, ${Math.floor(255 * (1 - t))})`;
      } else if (normalized < 0.6) {
        const t = (normalized - 0.4) / 0.2;
        return `rgb(${Math.floor(255 * t)}, 255, 0)`;
      } else if (normalized < 0.8) {
        const t = (normalized - 0.6) / 0.2;
        return `rgb(255, ${Math.floor(255 * (1 - t))}, 0)`;
      } else {
        return `rgb(255, 0, 0)`;
      }
    };

    // Draw heatmap cells
    data.forEach((row, rowIdx) => {
      row.forEach((value, colIdx) => {
        const x = leftMargin + colIdx * cellWidth;
        const y = topMargin + rowIdx * cellHeight;
        
        const cellKey = `${rowIdx}-${colIdx}`;
        const isSelected = selectedCells.has(cellKey);
        const isActive = activeCell?.row === rowIdx && activeCell?.col === colIdx;

        // Draw cell
        if (isActive) {
          ctx.fillStyle = '#00FF00';
        } else if (isSelected) {
          ctx.fillStyle = '#0088FF';
        } else {
          ctx.fillStyle = getColor(value);
        }
        
        ctx.fillRect(x, y, cellWidth, cellHeight);

        // Draw cell border
        ctx.strokeStyle = isActive ? '#00FF00' : isSelected ? '#00CCFF' : 'rgba(0, 0, 0, 0.3)';
        ctx.lineWidth = isActive ? 3 : isSelected ? 2 : 1;
        ctx.strokeRect(x, y, cellWidth, cellHeight);

        // Draw value in cell
        ctx.fillStyle = isActive || isSelected ? '#000000' : '#FFFFFF';
        ctx.font = 'bold 11px monospace';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText(value.toFixed(1), x + cellWidth / 2, y + cellHeight / 2);
      });
    });

    // Draw RPM axis labels (top)
    ctx.fillStyle = '#00FF00';
    ctx.font = 'bold 12px Arial';
    ctx.textAlign = 'center';
    rpmAxis.forEach((rpm, idx) => {
      const x = leftMargin + idx * cellWidth + cellWidth / 2;
      ctx.fillText(rpm.toString(), x, topMargin - 30);
    });

    // Draw RPM axis title
    ctx.fillStyle = '#CCCCCC';
    ctx.font = 'bold 14px Arial';
    ctx.fillText('RPM', width / 2, topMargin - 55);

    // Draw MAP axis labels (left)
    ctx.fillStyle = '#00CCFF';
    ctx.font = 'bold 12px Arial';
    ctx.textAlign = 'right';
    ctx.textBaseline = 'middle';
    mapAxis.forEach((map, idx) => {
      const y = topMargin + idx * cellHeight + cellHeight / 2;
      ctx.fillText(map.toString(), leftMargin - 15, y);
    });

    // Draw MAP axis title (rotated)
    ctx.save();
    ctx.translate(20, height / 2);
    ctx.rotate(-Math.PI / 2);
    ctx.fillStyle = '#CCCCCC';
    ctx.font = 'bold 14px Arial';
    ctx.textAlign = 'center';
    ctx.fillText('MAP (mbar)', 0, 0);
    ctx.restore();

    // Draw color legend
    const legendWidth = 30;
    const legendHeight = height - topMargin - bottomMargin;
    const legendX = width - rightMargin + 15;
    const legendY = topMargin;

    for (let i = 0; i < legendHeight; i++) {
      const value = minValue + (maxValue - minValue) * (1 - i / legendHeight);
      ctx.fillStyle = getColor(value);
      ctx.fillRect(legendX, legendY + i, legendWidth, 1);
    }

    ctx.strokeStyle = '#666666';
    ctx.lineWidth = 1;
    ctx.strokeRect(legendX, legendY, legendWidth, legendHeight);

    // Legend labels
    ctx.fillStyle = '#CCCCCC';
    ctx.font = '11px Arial';
    ctx.textAlign = 'left';
    ctx.fillText(maxValue.toFixed(1) + ' ' + unit, legendX + legendWidth + 5, legendY + 10);
    ctx.fillText(minValue.toFixed(1) + ' ' + unit, legendX + legendWidth + 5, legendY + legendHeight - 5);

    // Draw title
    ctx.fillStyle = '#FFFFFF';
    ctx.font = 'bold 16px Arial';
    ctx.textAlign = 'center';
    ctx.fillText(title, width / 2, 25);

  }, [data, selectedCells, activeCell, rpmAxis, mapAxis, title, unit]);

  const handleCanvasClick = (e: React.MouseEvent<HTMLCanvasElement>) => {
    if (!onCellClick) return;

    const canvas = canvasRef.current;
    if (!canvas) return;

    const rect = canvas.getBoundingClientRect();
    const x = ((e.clientX - rect.left) / rect.width) * canvas.width;
    const y = ((e.clientY - rect.top) / rect.height) * canvas.height;

    const leftMargin = 80;
    const topMargin = 80;
    const rightMargin = 60;
    const bottomMargin = 60;
    
    const cellWidth = (canvas.width - leftMargin - rightMargin) / data[0].length;
    const cellHeight = (canvas.height - topMargin - bottomMargin) / data.length;

    if (x >= leftMargin && y >= topMargin) {
      const col = Math.floor((x - leftMargin) / cellWidth);
      const row = Math.floor((y - topMargin) / cellHeight);

      if (row >= 0 && row < data.length && col >= 0 && col < data[0].length) {
        onCellClick(row, col);
      }
    }
  };

  return (
    <canvas
      ref={canvasRef}
      width={900}
      height={700}
      className="w-full h-full cursor-crosshair"
      onClick={handleCanvasClick}
    />
  );
}
