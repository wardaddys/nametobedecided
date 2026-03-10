import { useEffect, useRef } from 'react';

interface Surface3DProps {
  data: number[][];
  selectedCell: { row: number; col: number } | null;
  rpmAxis: number[];
  mapAxis: number[];
  title: string;
  unit: string;
}

export function Surface3D({ data, selectedCell, rpmAxis, mapAxis, title, unit }: Surface3DProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // Set canvas size
    const width = canvas.width;
    const height = canvas.height;

    // Clear canvas
    ctx.fillStyle = '#000000';
    ctx.fillRect(0, 0, width, height);

    // 3D projection parameters
    const angleX = 0.7;
    const angleZ = 0.5;
    const scale = 20;
    const offsetX = width * 0.3;
    const offsetY = height * 0.7;

    // Find min and max for color mapping
    const flatData = data.flat();
    const minValue = Math.min(...flatData);
    const maxValue = Math.max(...flatData);

    // Helper function to get color based on value
    const getColor = (value: number): string => {
      const normalized = (value - minValue) / (maxValue - minValue);
      
      if (normalized < 0.25) {
        // Blue to Cyan
        const t = normalized / 0.25;
        const r = 0;
        const g = Math.floor(255 * t);
        const b = 255;
        return `rgb(${r}, ${g}, ${b})`;
      } else if (normalized < 0.5) {
        // Cyan to Green
        const t = (normalized - 0.25) / 0.25;
        const r = 0;
        const g = 255;
        const b = Math.floor(255 * (1 - t));
        return `rgb(${r}, ${g}, ${b})`;
      } else if (normalized < 0.75) {
        // Green to Yellow
        const t = (normalized - 0.5) / 0.25;
        const r = Math.floor(255 * t);
        const g = 255;
        const b = 0;
        return `rgb(${r}, ${g}, ${b})`;
      } else {
        // Yellow to Red
        const t = (normalized - 0.75) / 0.25;
        const r = 255;
        const g = Math.floor(255 * (1 - t));
        const b = 0;
        return `rgb(${r}, ${g}, ${b})`;
      }
    };

    // 3D projection function
    const project = (x: number, y: number, z: number) => {
      const rotatedX = x * Math.cos(angleZ) - y * Math.sin(angleZ);
      const rotatedY = x * Math.sin(angleZ) + y * Math.cos(angleZ);
      
      const projectedX = rotatedX;
      const projectedY = rotatedY * Math.cos(angleX) - z * Math.sin(angleX);
      
      return {
        x: offsetX + projectedX * scale,
        y: offsetY - projectedY * scale,
      };
    };

    const rows = data.length;
    const cols = data[0].length;

    // Draw surface with proper sorting for depth
    const quads: Array<{
      points: Array<{ x: number; y: number }>;
      color: string;
      depth: number;
      isSelected: boolean;
    }> = [];

    for (let row = 0; row < rows - 1; row++) {
      for (let col = 0; col < cols - 1; col++) {
        const x1 = col;
        const y1 = row;
        const z1 = data[row][col] * 0.03;

        const x2 = col + 1;
        const y2 = row;
        const z2 = data[row][col + 1] * 0.03;

        const x3 = col + 1;
        const y3 = row + 1;
        const z3 = data[row + 1][col + 1] * 0.03;

        const x4 = col;
        const y4 = row + 1;
        const z4 = data[row + 1][col] * 0.03;

        const p1 = project(x1, y1, z1);
        const p2 = project(x2, y2, z2);
        const p3 = project(x3, y3, z3);
        const p4 = project(x4, y4, z4);

        const avgValue = (data[row][col] + data[row][col + 1] + data[row + 1][col + 1] + data[row + 1][col]) / 4;
        const avgDepth = (z1 + z2 + z3 + z4) / 4 + y1 + y2 + y3 + y4;

        const isSelected = selectedCell !== null && 
          ((row === selectedCell.row && col === selectedCell.col) ||
           (row === selectedCell.row && col === selectedCell.col - 1) ||
           (row === selectedCell.row - 1 && col === selectedCell.col) ||
           (row === selectedCell.row - 1 && col === selectedCell.col - 1));

        quads.push({
          points: [p1, p2, p3, p4],
          color: getColor(avgValue),
          depth: avgDepth,
          isSelected,
        });
      }
    }

    // Sort by depth (back to front)
    quads.sort((a, b) => b.depth - a.depth);

    // Draw quads
    quads.forEach(quad => {
      ctx.beginPath();
      ctx.moveTo(quad.points[0].x, quad.points[0].y);
      quad.points.forEach(p => ctx.lineTo(p.x, p.y));
      ctx.closePath();

      if (quad.isSelected) {
        ctx.fillStyle = '#00FF00';
        ctx.strokeStyle = '#00FF00';
        ctx.lineWidth = 4;
      } else {
        ctx.fillStyle = quad.color;
        ctx.strokeStyle = 'rgba(0, 0, 0, 0.3)';
        ctx.lineWidth = 0.5;
      }
      
      ctx.fill();
      ctx.stroke();
    });

    // Draw axes
    ctx.strokeStyle = '#FFFFFF';
    ctx.lineWidth = 2;
    
    // X axis (RPM)
    const xStart = project(0, 0, 0);
    const xEnd = project(cols, 0, 0);
    ctx.beginPath();
    ctx.moveTo(xStart.x, xStart.y);
    ctx.lineTo(xEnd.x, xEnd.y);
    ctx.stroke();

    // Y axis (MAP)
    const yStart = project(0, 0, 0);
    const yEnd = project(0, rows, 0);
    ctx.beginPath();
    ctx.moveTo(yStart.x, yStart.y);
    ctx.lineTo(yEnd.x, yEnd.y);
    ctx.stroke();

    // Z axis (Value)
    const zStart = project(0, 0, 0);
    const zEnd = project(0, 0, maxValue * 0.03);
    ctx.beginPath();
    ctx.moveTo(zStart.x, zStart.y);
    ctx.lineTo(zEnd.x, zEnd.y);
    ctx.stroke();

    // Draw axis labels
    ctx.fillStyle = '#CCCCCC';
    ctx.font = '12px Arial';
    
    // RPM label
    ctx.fillText('RPM', xEnd.x + 10, xEnd.y);
    
    // MAP label
    ctx.fillText('MAP (mbar)', yEnd.x - 60, yEnd.y + 20);
    
    // Value label
    ctx.fillText(unit, zEnd.x - 40, zEnd.y - 10);

    // Draw grid lines
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.1)';
    ctx.lineWidth = 1;

    // RPM grid lines
    for (let i = 0; i <= cols; i += 2) {
      const start = project(i, 0, 0);
      const end = project(i, rows, 0);
      ctx.beginPath();
      ctx.moveTo(start.x, start.y);
      ctx.lineTo(end.x, end.y);
      ctx.stroke();

      // RPM value labels
      if (i < rpmAxis.length) {
        ctx.fillStyle = '#888888';
        ctx.font = '10px Arial';
        ctx.fillText(rpmAxis[i].toString(), start.x - 15, start.y + 15);
      }
    }

    // MAP grid lines
    for (let i = 0; i <= rows; i += 2) {
      const start = project(0, i, 0);
      const end = project(cols, i, 0);
      ctx.beginPath();
      ctx.moveTo(start.x, start.y);
      ctx.lineTo(end.x, end.y);
      ctx.stroke();

      // MAP value labels
      if (i < mapAxis.length) {
        ctx.fillStyle = '#888888';
        ctx.font = '10px Arial';
        ctx.fillText(mapAxis[i].toString(), start.x - 50, start.y + 5);
      }
    }

    // Draw title
    ctx.fillStyle = '#FFFFFF';
    ctx.font = 'bold 14px Arial';
    ctx.fillText(title, 10, 25);

    // Draw color legend
    const legendWidth = 20;
    const legendHeight = 200;
    const legendX = width - 40;
    const legendY = 50;

    for (let i = 0; i < legendHeight; i++) {
      const value = minValue + (maxValue - minValue) * (1 - i / legendHeight);
      ctx.fillStyle = getColor(value);
      ctx.fillRect(legendX, legendY + i, legendWidth, 1);
    }

    ctx.strokeStyle = '#FFFFFF';
    ctx.lineWidth = 1;
    ctx.strokeRect(legendX, legendY, legendWidth, legendHeight);

    // Legend labels
    ctx.fillStyle = '#CCCCCC';
    ctx.font = '11px Arial';
    ctx.fillText(maxValue.toFixed(1), legendX + 25, legendY + 5);
    ctx.fillText(minValue.toFixed(1), legendX + 25, legendY + legendHeight);

  }, [data, selectedCell, rpmAxis, mapAxis, title, unit]);

  return (
    <div className="relative w-full h-full bg-black border border-[#333333]">
      <canvas
        ref={canvasRef}
        width={800}
        height={600}
        className="w-full h-full"
        style={{ imageRendering: 'crisp-edges' }}
      />
    </div>
  );
}
