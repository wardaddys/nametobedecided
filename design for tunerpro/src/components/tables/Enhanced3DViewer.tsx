import { useEffect, useRef, useState, useCallback } from 'react';
import { RotateCcw, ZoomIn, ZoomOut, Grid3x3, Sun, Eye } from 'lucide-react';

interface Enhanced3DViewerProps {
  data: number[][];
  selectedCell: { row: number; col: number } | null;
  rpmAxis: number[];
  mapAxis: number[];
  title: string;
  unit: string;
}

interface ViewState {
  rotationX: number;
  rotationZ: number;
  zoom: number;
  panX: number;
  panY: number;
}

export function Enhanced3DViewer({ 
  data, 
  selectedCell, 
  rpmAxis, 
  mapAxis, 
  title, 
  unit 
}: Enhanced3DViewerProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const [viewState, setViewState] = useState<ViewState>({
    rotationX: 0.6,
    rotationZ: -0.4,
    zoom: 1.0,
    panX: 0,
    panY: 0,
  });
  
  const [isDragging, setIsDragging] = useState(false);
  const [lastMousePos, setLastMousePos] = useState({ x: 0, y: 0 });
  const [showGrid, setShowGrid] = useState(true);
  const [showLighting, setShowLighting] = useState(true);
  const [colorScheme, setColorScheme] = useState<'performance' | 'thermal' | 'rainbow'>('performance');

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

  // Reset view to optimal angle
  const resetView = () => {
    setViewState({
      rotationX: 0.6,
      rotationZ: -0.4,
      zoom: 1.0,
      panX: 0,
      panY: 0,
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

  // Hondata-style color mapping: Blue (low) -> Cyan -> Green -> Yellow -> Orange -> Red (high)
  const getColor = useCallback((value: number, minValue: number, maxValue: number): string => {
    const normalized = (value - minValue) / (maxValue - minValue);
    
    if (colorScheme === 'thermal') {
      // Simple thermal: Dark Red -> Bright Red -> Yellow -> White
      if (normalized < 0.33) {
        const t = normalized / 0.33;
        const r = Math.floor(100 + 155 * t);
        return `rgb(${r}, 0, 0)`;
      } else if (normalized < 0.66) {
        const t = (normalized - 0.33) / 0.33;
        const r = 255;
        const g = Math.floor(255 * t);
        return `rgb(${r}, ${g}, 0)`;
      } else {
        const t = (normalized - 0.66) / 0.34;
        const val = Math.floor(255 * t);
        return `rgb(255, 255, ${val})`;
      }
    }
    
    if (colorScheme === 'rainbow') {
      // Classic rainbow spectrum
      if (normalized < 0.25) {
        const t = normalized / 0.25;
        return `rgb(0, ${Math.floor(255 * t)}, 255)`;
      } else if (normalized < 0.5) {
        const t = (normalized - 0.25) / 0.25;
        return `rgb(0, 255, ${Math.floor(255 * (1 - t))})`;
      } else if (normalized < 0.75) {
        const t = (normalized - 0.5) / 0.25;
        return `rgb(${Math.floor(255 * t)}, 255, 0)`;
      } else {
        const t = (normalized - 0.75) / 0.25;
        return `rgb(255, ${Math.floor(255 * (1 - t))}, 0)`;
      }
    }
    
    // Hondata default: Blue -> Cyan -> Green -> Yellow -> Orange -> Red
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
  }, [colorScheme]);

  // Render 3D surface
  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const width = canvas.width;
    const height = canvas.height;

    // Clear canvas with gradient background
    const gradient = ctx.createLinearGradient(0, 0, 0, height);
    gradient.addColorStop(0, '#0a0a0a');
    gradient.addColorStop(1, '#1a1a1a');
    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, width, height);

    // Calculate scale based on zoom - ADJUSTED for better centering
    const scale = 12 * viewState.zoom;
    const offsetX = width * 0.5 + viewState.panX;
    const offsetY = height * 0.55 + viewState.panY; // Centered vertically

    // Find min and max for color mapping
    const flatData = data.flat();
    const minValue = Math.min(...flatData);
    const maxValue = Math.max(...flatData);

    // 3D projection function
    const project = (x: number, y: number, z: number) => {
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
      const projectedY = rotatedY * cosX - z * sinX;
      const projectedZ = rotatedY * sinX + z * cosX;
      
      return {
        x: offsetX + projectedX * scale,
        y: offsetY - projectedY * scale,
        z: projectedZ, // For depth sorting
      };
    };

    const rows = data.length;
    const cols = data[0]?.length || 0;

    if (rows === 0 || cols === 0) return;

    // Create quads with depth information
    interface Quad {
      points: Array<{ x: number; y: number }>;
      color: string;
      depth: number;
      isSelected: boolean;
      value: number;
    }

    const quads: Quad[] = [];

    for (let row = 0; row < rows - 1; row++) {
      for (let col = 0; col < cols - 1; col++) {
        const x = col - cols / 2;
        const y = row - rows / 2;
        
        const z1 = data[row][col];
        const z2 = data[row][col + 1];
        const z3 = data[row + 1][col + 1];
        const z4 = data[row + 1][col];
        
        const avgZ = (z1 + z2 + z3 + z4) / 4;
        
        const p1 = project(x, y, z1);
        const p2 = project(x + 1, y, z2);
        const p3 = project(x + 1, y + 1, z3);
        const p4 = project(x, y + 1, z4);
        
        const isSelected = selectedCell && 
          ((row === selectedCell.row && col === selectedCell.col) ||
           (row === selectedCell.row && col === selectedCell.col - 1) ||
           (row === selectedCell.row - 1 && col === selectedCell.col) ||
           (row === selectedCell.row - 1 && col === selectedCell.col - 1));
        
        quads.push({
          points: [p1, p2, p3, p4],
          color: getColor(avgZ, minValue, maxValue),
          depth: (p1.z + p2.z + p3.z + p4.z) / 4,
          isSelected: !!isSelected,
          value: avgZ,
        });
      }
    }

    // Sort quads by depth (painter's algorithm)
    quads.sort((a, b) => b.depth - a.depth);

    // Draw quads
    quads.forEach(quad => {
      ctx.beginPath();
      ctx.moveTo(quad.points[0].x, quad.points[0].y);
      quad.points.slice(1).forEach(p => ctx.lineTo(p.x, p.y));
      ctx.closePath();
      
      // Apply lighting effect
      if (showLighting) {
        const lightFactor = 0.6 + (quad.depth / 100) * 0.4;
        const rgb = quad.color.match(/\d+/g)?.map(Number) || [255, 255, 255];
        ctx.fillStyle = `rgba(${rgb[0] * lightFactor}, ${rgb[1] * lightFactor}, ${rgb[2] * lightFactor}, 0.95)`;
      } else {
        ctx.fillStyle = quad.color;
      }
      
      ctx.fill();
      
      // Draw grid lines
      if (showGrid) {
        ctx.strokeStyle = 'rgba(100, 100, 100, 0.4)';
        ctx.lineWidth = 0.5;
        ctx.stroke();
      }
      
      // Highlight selected cell
      if (quad.isSelected) {
        ctx.strokeStyle = '#00FFFF';
        ctx.lineWidth = 3;
        ctx.stroke();
      }
    });

    // Draw axes labels
    ctx.fillStyle = '#FFFFFF';
    ctx.font = '12px Inter';
    ctx.textAlign = 'center';
    ctx.fillText('RPM →', width * 0.8, height * 0.9);
    ctx.fillText('MAP ↑', width * 0.1, height * 0.2);

    // Draw title and value range
    ctx.fillStyle = '#00CCFF';
    ctx.font = 'bold 14px Inter';
    ctx.textAlign = 'left';
    ctx.fillText(title, 10, 20);
    
    ctx.fillStyle = '#CCCCCC';
    ctx.font = '11px Inter';
    ctx.fillText(`Range: ${minValue.toFixed(1)} - ${maxValue.toFixed(1)} ${unit}`, 10, 40);

  }, [data, selectedCell, viewState, showGrid, showLighting, getColor, title, unit]);

  return (
    <div className="flex flex-col h-full bg-[#0a0a0a] rounded border border-[#444444]">
      {/* Control Bar */}
      <div className="bg-[#2a2a2a] border-b border-[#444444] px-3 py-2 flex items-center justify-between gap-2 flex-wrap">
        <div className="flex items-center gap-2">
          <button
            onClick={resetView}
            className="bg-[#3a3a3a] hover:bg-[#4a4a4a] border border-[#555555] px-2 py-1 rounded flex items-center gap-1 text-xs text-white"
            title="Reset View"
          >
            <RotateCcw className="w-3 h-3" />
            Reset
          </button>
          <button
            onClick={zoomIn}
            className="bg-[#3a3a3a] hover:bg-[#4a4a4a] border border-[#555555] px-2 py-1 rounded text-white"
            title="Zoom In"
          >
            <ZoomIn className="w-3 h-3" />
          </button>
          <button
            onClick={zoomOut}
            className="bg-[#3a3a3a] hover:bg-[#4a4a4a] border border-[#555555] px-2 py-1 rounded text-white"
            title="Zoom Out"
          >
            <ZoomOut className="w-3 h-3" />
          </button>
          <span className="text-xs text-[#888888]">
            Zoom: {(viewState.zoom * 100).toFixed(0)}%
          </span>
        </div>

        <div className="flex items-center gap-2">
          <button
            onClick={() => setShowGrid(!showGrid)}
            className={`border border-[#555555] px-2 py-1 rounded flex items-center gap-1 text-xs ${
              showGrid ? 'bg-[#0066CC] text-white' : 'bg-[#3a3a3a] text-[#CCCCCC]'
            }`}
            title="Toggle Grid"
          >
            <Grid3x3 className="w-3 h-3" />
            Grid
          </button>
          <button
            onClick={() => setShowLighting(!showLighting)}
            className={`border border-[#555555] px-2 py-1 rounded flex items-center gap-1 text-xs ${
              showLighting ? 'bg-[#0066CC] text-white' : 'bg-[#3a3a3a] text-[#CCCCCC]'
            }`}
            title="Toggle Lighting"
          >
            <Sun className="w-3 h-3" />
            Light
          </button>
          <select
            value={colorScheme}
            onChange={(e) => setColorScheme(e.target.value as any)}
            className="bg-[#3a3a3a] border border-[#555555] text-white text-xs px-2 py-1 rounded"
          >
            <option value="performance">Hondata</option>
            <option value="thermal">Thermal</option>
            <option value="rainbow">Rainbow</option>
          </select>
        </div>
      </div>

      {/* Canvas */}
      <div className="flex-1 relative">
        <canvas
          ref={canvasRef}
          width={800}
          height={600}
          className="w-full h-full cursor-grab active:cursor-grabbing"
          onMouseDown={handleMouseDown}
          onMouseMove={handleMouseMove}
          onMouseUp={handleMouseUp}
          onMouseLeave={handleMouseUp}
          onWheel={handleWheel}
        />
        
        {/* Instructions Overlay */}
        <div className="absolute bottom-2 left-2 bg-black/70 border border-[#444444] rounded px-3 py-2 text-xs text-[#CCCCCC]">
          <div className="flex items-center gap-1 mb-1">
            <Eye className="w-3 h-3 text-[#00CCFF]" />
            <span className="text-white">Controls:</span>
          </div>
          <div>• Drag: Rotate view</div>
          <div>• Scroll: Zoom in/out</div>
          <div>• Click Reset: Default view</div>
        </div>
      </div>
    </div>
  );
}
