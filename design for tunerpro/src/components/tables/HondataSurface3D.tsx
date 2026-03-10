import { useEffect, useRef, useState, useCallback } from 'react';
import { RotateCcw, ZoomIn, ZoomOut, Grid3x3, Maximize2 } from 'lucide-react';
import { TableDefinition } from './TableDefinitions';

interface HondataSurface3DProps {
  tableData: number[][];
  definition: TableDefinition;
  activeCell?: { row: number; col: number } | null;
  isLive?: boolean;
  compact?: boolean;
}

interface ViewState {
  rotationX: number;
  rotationZ: number;
  zoom: number;
}

export function HondataSurface3D({ 
  tableData, 
  definition,
  activeCell,
  isLive = false,
  compact = false,
}: HondataSurface3DProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const [viewState, setViewState] = useState<ViewState>({
    rotationX: 0.6,
    rotationZ: 0.4,
    zoom: 1.0,
  });
  
  const [isDragging, setIsDragging] = useState(false);
  const [lastMousePos, setLastMousePos] = useState({ x: 0, y: 0 });
  const [showWireframe, setShowWireframe] = useState(true);

  // Mouse event handlers
  const handleMouseDown = useCallback((e: React.MouseEvent<HTMLCanvasElement>) => {
    setIsDragging(true);
    setLastMousePos({ x: e.clientX, y: e.clientY });
  }, []);

  const handleMouseMove = useCallback((e: React.MouseEvent<HTMLCanvasElement>) => {
    if (!isDragging) return;

    const deltaX = e.clientX - lastMousePos.x;
    const deltaY = e.clientY - lastMousePos.y;

    setViewState(prev => ({
      ...prev,
      rotationZ: prev.rotationZ + deltaX * 0.01,
      rotationX: Math.max(0.1, Math.min(Math.PI - 0.1, prev.rotationX + deltaY * 0.01)),
    }));

    setLastMousePos({ x: e.clientX, y: e.clientY });
  }, [isDragging, lastMousePos]);

  const handleMouseUp = useCallback(() => {
    setIsDragging(false);
  }, []);

  const handleWheel = useCallback((e: React.WheelEvent<HTMLCanvasElement>) => {
    e.preventDefault();
    const zoomDelta = e.deltaY > 0 ? 0.9 : 1.1;
    setViewState(prev => ({
      ...prev,
      zoom: Math.max(0.5, Math.min(3.0, prev.zoom * zoomDelta)),
    }));
  }, []);

  // Reset view
  const resetView = () => {
    setViewState({
      rotationX: 0.6,
      rotationZ: 0.4,
      zoom: 1.0,
    });
  };

  // Zoom controls
  const zoomIn = () => {
    setViewState(prev => ({
      ...prev,
      zoom: Math.min(3.0, prev.zoom * 1.2),
    }));
  };

  const zoomOut = () => {
    setViewState(prev => ({
      ...prev,
      zoom: Math.max(0.5, prev.zoom / 1.2),
    }));
  };

  // Hondata-style color mapping (red high -> green/blue low)
  const getHondataColor = useCallback((value: number, minValue: number, maxValue: number): string => {
    const normalized = (value - minValue) / (maxValue - minValue);
    
    // Hondata gradient: Blue (low) -> Green -> Yellow -> Orange -> Red (high)
    if (normalized < 0.25) {
      // Blue to Cyan
      const t = normalized / 0.25;
      return `rgb(${Math.floor(0 * (1-t) + 0 * t)}, ${Math.floor(100 * (1-t) + 200 * t)}, ${Math.floor(200 * (1-t) + 200 * t)})`;
    } else if (normalized < 0.5) {
      // Cyan to Green
      const t = (normalized - 0.25) / 0.25;
      return `rgb(${Math.floor(0 * (1-t) + 0 * t)}, ${Math.floor(200 * (1-t) + 220 * t)}, ${Math.floor(200 * (1-t) + 0 * t)})`;
    } else if (normalized < 0.75) {
      // Green to Yellow
      const t = (normalized - 0.5) / 0.25;
      return `rgb(${Math.floor(0 * (1-t) + 255 * t)}, ${Math.floor(220 * (1-t) + 220 * t)}, ${Math.floor(0 * (1-t) + 0 * t)})`;
    } else {
      // Yellow to Red
      const t = (normalized - 0.75) / 0.25;
      return `rgb(255, ${Math.floor(220 * (1-t) + 0 * t)}, 0)`;
    }
  }, []);

  // Render 3D surface with Hondata styling
  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const width = canvas.width;
    const height = canvas.height;

    // Clear with black background (Hondata style)
    ctx.fillStyle = '#000000';
    ctx.fillRect(0, 0, width, height);

    // Calculate scale based on zoom
    const scale = (compact ? 8 : 12) * viewState.zoom;
    const offsetX = width * 0.5;
    const offsetY = height * 0.7;

    // Find min and max for color mapping
    const flatData = tableData.flat();
    const minValue = Math.min(...flatData);
    const maxValue = Math.max(...flatData);

    // 3D projection function
    const project = (x: number, y: number, z: number) => {
      // Normalize Z value for consistent scaling
      const normalizedZ = ((z - minValue) / (maxValue - minValue)) * 10;
      
      // Apply rotation
      const cosX = Math.cos(viewState.rotationX);
      const sinX = Math.sin(viewState.rotationX);
      const cosZ = Math.cos(viewState.rotationZ);
      const sinZ = Math.sin(viewState.rotationZ);

      // Rotate around Z axis
      const rotatedX = x * cosZ - y * sinZ;
      const rotatedY = x * sinZ + y * cosZ;
      
      // Rotate around X axis
      const projectedX = rotatedX;
      const projectedY = rotatedY * cosX - normalizedZ * sinX;
      const projectedZ = rotatedY * sinX + normalizedZ * cosX;
      
      return {
        x: offsetX + projectedX * scale,
        y: offsetY - projectedY * scale,
        z: projectedZ,
      };
    };

    const rows = tableData.length;
    const cols = tableData[0]?.length || 0;

    if (rows === 0 || cols === 0) return;

    // Create quads with depth information
    interface Quad {
      points: Array<{ x: number; y: number }>;
      color: string;
      depth: number;
      isActive: boolean;
      value: number;
      gridPoints: Array<{ x: number; y: number }>;
    }

    const quads: Quad[] = [];

    for (let row = 0; row < rows - 1; row++) {
      for (let col = 0; col < cols - 1; col++) {
        const x = col - cols / 2;
        const y = row - rows / 2;
        
        const z1 = tableData[row][col];
        const z2 = tableData[row][col + 1];
        const z3 = tableData[row + 1][col + 1];
        const z4 = tableData[row + 1][col];
        
        const avgZ = (z1 + z2 + z3 + z4) / 4;
        
        const p1 = project(x, y, z1);
        const p2 = project(x + 1, y, z2);
        const p3 = project(x + 1, y + 1, z3);
        const p4 = project(x, y + 1, z4);
        
        const isActive = activeCell && 
          row === activeCell.row && col === activeCell.col;
        
        quads.push({
          points: [p1, p2, p3, p4],
          color: getHondataColor(avgZ, minValue, maxValue),
          depth: (p1.z + p2.z + p3.z + p4.z) / 4,
          isActive: !!isActive,
          value: avgZ,
          gridPoints: [p1, p2, p3, p4],
        });
      }
    }

    // Sort quads by depth (painter's algorithm)
    quads.sort((a, b) => b.depth - a.depth);

    // Draw colored faces first
    quads.forEach(quad => {
      ctx.beginPath();
      ctx.moveTo(quad.points[0].x, quad.points[0].y);
      quad.points.slice(1).forEach(p => ctx.lineTo(p.x, p.y));
      ctx.closePath();
      
      ctx.fillStyle = quad.color;
      ctx.globalAlpha = 0.85;
      ctx.fill();
      ctx.globalAlpha = 1.0;
    });

    // Draw black wireframe overlay (Hondata style)
    if (showWireframe) {
      quads.forEach(quad => {
        ctx.beginPath();
        ctx.moveTo(quad.gridPoints[0].x, quad.gridPoints[0].y);
        quad.gridPoints.slice(1).forEach(p => ctx.lineTo(p.x, p.y));
        ctx.closePath();
        
        ctx.strokeStyle = '#000000';
        ctx.lineWidth = 1.0;
        ctx.stroke();
      });
    }

    // Highlight active cell with blue marker
    if (activeCell && isLive) {
      const activeQuad = quads.find(q => q.isActive);
      if (activeQuad) {
        const centerX = activeQuad.points.reduce((sum, p) => sum + p.x, 0) / 4;
        const centerY = activeQuad.points.reduce((sum, p) => sum + p.y, 0) / 4;
        
        // Pulsing blue marker
        const pulseTime = Date.now() / 500;
        const pulseSize = 4 + Math.sin(pulseTime) * 2;
        
        ctx.beginPath();
        ctx.arc(centerX, centerY, pulseSize, 0, Math.PI * 2);
        ctx.fillStyle = '#00FFFF';
        ctx.fill();
        ctx.strokeStyle = '#0088FF';
        ctx.lineWidth = 2;
        ctx.stroke();
      }
    }

    // Draw axis labels with units
    ctx.fillStyle = '#CCCCCC';
    ctx.font = compact ? '9px Inter' : '11px Inter';
    ctx.textAlign = 'center';
    
    const xAxisLabel = `${definition.xAxis.label} (${definition.xAxis.unit})`;
    const yAxisLabel = `${definition.yAxis.label} (${definition.yAxis.unit})`;
    const zAxisLabel = `${definition.zAxis.label} (${definition.zAxis.unit})`;
    
    ctx.fillText(xAxisLabel, width * 0.8, height - 10);
    ctx.fillText(yAxisLabel, 20, height * 0.3);
    
    if (!compact) {
      // Draw title and range
      ctx.fillStyle = '#00CCFF';
      ctx.font = 'bold 12px Inter';
      ctx.textAlign = 'left';
      ctx.fillText(definition.name, 10, 18);
      
      ctx.fillStyle = '#888888';
      ctx.font = '10px Inter';
      ctx.fillText(`${zAxisLabel}: ${minValue.toFixed(1)} - ${maxValue.toFixed(1)}`, 10, 35);
    }

  }, [tableData, activeCell, viewState, showWireframe, getHondataColor, definition, compact, isLive]);

  // Auto-rotate when live
  useEffect(() => {
    if (!isLive) return;

    const interval = setInterval(() => {
      // Trigger re-render for pulse animation
      canvasRef.current?.getContext('2d');
    }, 50);

    return () => clearInterval(interval);
  }, [isLive]);

  if (compact) {
    return (
      <div className="w-full h-full bg-black border border-[#333333] rounded overflow-hidden">
        <canvas
          ref={canvasRef}
          width={400}
          height={300}
          className="w-full h-full cursor-grab active:cursor-grabbing"
          onMouseDown={handleMouseDown}
          onMouseMove={handleMouseMove}
          onMouseUp={handleMouseUp}
          onMouseLeave={handleMouseUp}
          onWheel={handleWheel}
        />
      </div>
    );
  }

  return (
    <div className="flex flex-col h-full w-full bg-black border border-[#444444] rounded overflow-hidden">
      {/* Compact Control Bar */}
      <div className="bg-[#1a1a1a] border-b border-[#444444] px-2 py-1.5 flex items-center justify-between">
        <div className="flex items-center gap-1.5">
          <button
            onClick={resetView}
            className="bg-[#2a2a2a] hover:bg-[#3a3a3a] border border-[#555555] px-1.5 py-1 rounded text-[#CCCCCC] text-xs flex items-center gap-1"
            title="Reset View"
          >
            <RotateCcw className="w-3 h-3" />
          </button>
          <button
            onClick={zoomIn}
            className="bg-[#2a2a2a] hover:bg-[#3a3a3a] border border-[#555555] p-1 rounded text-[#CCCCCC]"
            title="Zoom In"
          >
            <ZoomIn className="w-3 h-3" />
          </button>
          <button
            onClick={zoomOut}
            className="bg-[#2a2a2a] hover:bg-[#3a3a3a] border border-[#555555] p-1 rounded text-[#CCCCCC]"
            title="Zoom Out"
          >
            <ZoomOut className="w-3 h-3" />
          </button>
          <button
            onClick={() => setShowWireframe(!showWireframe)}
            className={`border border-[#555555] px-1.5 py-1 rounded text-xs flex items-center gap-1 ${
              showWireframe ? 'bg-[#0066CC] text-white' : 'bg-[#2a2a2a] text-[#CCCCCC]'
            }`}
            title="Toggle Wireframe"
          >
            <Grid3x3 className="w-3 h-3" />
          </button>
        </div>
        
        <span className="text-[10px] text-[#666666]">
          Drag: Rotate • Scroll: Zoom
        </span>
      </div>

      {/* Canvas */}
      <div className="flex-1 relative min-h-0">
        <canvas
          ref={canvasRef}
          width={800}
          height={600}
          className="w-full h-full cursor-grab active:cursor-grabbing"
          style={{ display: 'block' }}
          onMouseDown={handleMouseDown}
          onMouseMove={handleMouseMove}
          onMouseUp={handleMouseUp}
          onMouseLeave={handleMouseUp}
          onWheel={handleWheel}
        />
      </div>
    </div>
  );
}
