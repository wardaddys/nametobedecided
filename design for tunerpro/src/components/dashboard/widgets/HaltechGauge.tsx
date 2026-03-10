import { useEffect, useState } from "react";

interface HaltechGaugeProps {
  value: number;
  min: number;
  max: number;
  majorTicks: number[];
  greenZone: [number, number];
  orangeZone: [number, number];
  redZone: [number, number];
  centerLabel: string;
  bottomLabel: string;
  size?: number;
  showDigitalReadout?: boolean;
  digitalValue?: number;
  digitalLabel?: string;
  secondaryValue?: number;
  secondaryLabel?: string;
  compact?: boolean;
}

export function HaltechGauge({
  value,
  min,
  max,
  majorTicks,
  greenZone,
  orangeZone,
  redZone,
  centerLabel,
  bottomLabel,
  size = 400,
  showDigitalReadout = false,
  digitalValue,
  digitalLabel,
  secondaryValue,
  secondaryLabel,
  compact = false,
}: HaltechGaugeProps) {
  const [displayValue, setDisplayValue] = useState(value);

  useEffect(() => {
    setDisplayValue(value);
  }, [value]);

  // Calculate needle angle (240 degree sweep, starting from bottom left)
  const percentage = (displayValue - min) / (max - min);
  const sweepAngle = 240; // degrees of arc
  const startAngle = 150; // start angle (bottom left)
  const needleAngle = startAngle + percentage * sweepAngle;

  // Calculate arc paths for colored zones
  const getArcPath = (startVal: number, endVal: number, radius: number) => {
    const startPercent = (startVal - min) / (max - min);
    const endPercent = (endVal - min) / (max - min);
    const startA = startAngle + startPercent * sweepAngle;
    const endA = startAngle + endPercent * sweepAngle;

    const centerX = size / 2;
    const centerY = size / 2;

    const startRad = (startA - 90) * Math.PI / 180;
    const endRad = (endA - 90) * Math.PI / 180;

    const innerRadius = radius - 20; // Slimmer color zones
    
    const x1 = centerX + radius * Math.cos(startRad);
    const y1 = centerY + radius * Math.sin(startRad);
    const x2 = centerX + radius * Math.cos(endRad);
    const y2 = centerY + radius * Math.sin(endRad);
    const x3 = centerX + innerRadius * Math.cos(endRad);
    const y3 = centerY + innerRadius * Math.sin(endRad);
    const x4 = centerX + innerRadius * Math.cos(startRad);
    const y4 = centerY + innerRadius * Math.sin(startRad);

    const largeArc = endA - startA > 180 ? 1 : 0;

    return `M ${x1} ${y1} A ${radius} ${radius} 0 ${largeArc} 1 ${x2} ${y2} L ${x3} ${y3} A ${innerRadius} ${innerRadius} 0 ${largeArc} 0 ${x4} ${y4} Z`;
  };

  // Calculate tick positions
  const getTicks = () => {
    return majorTicks.map(tickValue => {
      const tickPercent = (tickValue - min) / (max - min);
      const tickAngle = startAngle + tickPercent * sweepAngle;
      const tickRad = (tickAngle - 90) * Math.PI / 180;
      
      const centerX = size / 2;
      const centerY = size / 2;
      const radius = size * 0.42;
      
      const x = centerX + radius * Math.cos(tickRad);
      const y = centerY + radius * Math.sin(tickRad);

      return { value: tickValue, x, y, angle: tickAngle };
    });
  };

  const ticks = getTicks();

  return (
    <div className="relative" style={{ width: size, height: size }}>
      <svg width={size} height={size} viewBox={`0 0 ${size} ${size}`}>
        {/* Outer bezel ring - Teal */}
        <circle
          cx={size / 2}
          cy={size / 2}
          r={size * 0.48}
          fill="none"
          stroke="#00d4ff"
          strokeWidth={4}
        />
        
        {/* Inner bezel ring - Darker Teal */}
        <circle
          cx={size / 2}
          cy={size / 2}
          r={size * 0.45}
          fill="none"
          stroke="#008BA3"
          strokeWidth={2}
        />

        {/* Gauge face background */}
        <circle
          cx={size / 2}
          cy={size / 2}
          r={size * 0.43}
          fill="#000000"
        />

        {/* Green arc zone */}
        <path
          d={getArcPath(greenZone[0], greenZone[1], size * 0.42)}
          fill="#00FF00"
        />

        {/* Orange arc zone */}
        <path
          d={getArcPath(orangeZone[0], orangeZone[1], size * 0.42)}
          fill="#FF8C00"
        />

        {/* Red arc zone */}
        <path
          d={getArcPath(redZone[0], redZone[1], size * 0.42)}
          fill="#DC143C"
        />

        {/* Tick marks and numbers */}
        {ticks.map((tick, i) => {
          // Always use white text for better visibility
          const textColor = '#FFFFFF';
          
          return (
            <g key={i}>
              <text
                x={tick.x}
                y={tick.y}
                textAnchor="middle"
                dominantBaseline="middle"
                fill={textColor}
                style={{
                  fontSize: `${compact ? size * 0.08 : size * 0.055}px`,
                  fontFamily: 'Arial, sans-serif',
                  fontWeight: 'bold',
                  textShadow: '0 0 3px rgba(0, 0, 0, 0.8)',
                }}
              >
                {tick.value}
              </text>
            </g>
          );
        })}

        {/* Center label */}
        {centerLabel && !compact && (
          <text
            x={size / 2}
            y={size / 2 - size * 0.05}
            textAnchor="middle"
            dominantBaseline="middle"
            fill="#FFD700"
            style={{
              fontSize: `${size * 0.055}px`,
              fontFamily: 'Arial, sans-serif',
              fontWeight: 'bold',
              fontStyle: 'italic',
              textShadow: '0 0 10px rgba(255, 215, 0, 0.5)',
            }}
          >
            {centerLabel}
          </text>
        )}

        {/* Digital readout center display */}
        {showDigitalReadout && digitalValue !== undefined && !compact && (
          <>
            <rect
              x={size / 2 - size * 0.15}
              y={size / 2 + size * 0.02}
              width={size * 0.3}
              height={size * 0.12}
              fill="#000000"
              stroke="#2a2a2a"
              strokeWidth={2}
              rx={4}
            />
            <text
              x={size / 2}
              y={size / 2 + size * 0.065}
              textAnchor="middle"
              dominantBaseline="middle"
              fill="#FFD700"
              style={{
                fontSize: `${size * 0.08}px`,
                fontFamily: 'Courier New, monospace',
                fontWeight: 'bold',
                textShadow: '0 0 8px rgba(255, 215, 0, 0.6)',
              }}
            >
              {Math.round(digitalValue)}
            </text>
          </>
        )}

        {/* Secondary values below digital readout */}
        {secondaryValue !== undefined && !compact && (
          <>
            <text
              x={size / 2}
              y={size / 2 + size * 0.14}
              textAnchor="middle"
              fill="#666666"
              style={{
                fontSize: `${size * 0.028}px`,
                fontFamily: 'Arial, sans-serif',
              }}
            >
              {digitalLabel || ''}
            </text>
            <text
              x={size / 2}
              y={size / 2 + size * 0.175}
              textAnchor="middle"
              fill="#888888"
              style={{
                fontSize: `${size * 0.032}px`,
                fontFamily: 'Courier New, monospace',
                fontWeight: 'bold',
              }}
            >
              {secondaryValue.toLocaleString()}
            </text>
            {secondaryLabel && (
              <text
                x={size / 2}
                y={size / 2 + size * 0.2}
                textAnchor="middle"
                fill="#555555"
                style={{
                  fontSize: `${size * 0.024}px`,
                  fontFamily: 'Arial, sans-serif',
                }}
              >
                {secondaryLabel}
              </text>
            )}
          </>
        )}

        {/* Needle pivot center */}
        <circle
          cx={size / 2}
          cy={size / 2}
          r={size * 0.025}
          fill="#DC143C"
          stroke="#000000"
          strokeWidth={2}
        />

        {/* Needle */}
        <g transform={`rotate(${needleAngle} ${size / 2} ${size / 2})`}>
          <polygon
            points={`${size / 2},${size / 2 - size * 0.32} ${size / 2 - size * 0.015},${size / 2 + size * 0.02} ${size / 2 + size * 0.015},${size / 2 + size * 0.02}`}
            fill="#DC143C"
            stroke="#8B0000"
            strokeWidth={1}
          />
        </g>

        {/* Bottom label */}
        <text
          x={size / 2}
          y={size * 0.88}
          textAnchor="middle"
          fill="#444444"
          style={{
            fontSize: `${size * 0.028}px`,
            fontFamily: 'Arial, sans-serif',
            letterSpacing: '0.1em',
          }}
        >
          {bottomLabel}
        </text>
      </svg>
    </div>
  );
}
