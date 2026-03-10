interface BarIndicatorProps {
  label: string;
  value: number;
  unit: string;
  maxBars?: number;
  align?: 'left' | 'right';
  compact?: boolean;
}

export function BarIndicator({ 
  label, 
  value, 
  unit, 
  maxBars = 8,
  align = 'left',
  compact = false
}: BarIndicatorProps) {
  // Calculate percentage for fluid fill (0-100%)
  const percentage = Math.min(100, Math.max(0, (value / 120) * 100));
  
  // Determine color based on percentage
  const getBarColor = () => {
    if (percentage >= 75) return '#DC143C'; // Red
    if (percentage >= 50) return '#FFD700'; // Yellow
    return '#00FF00'; // Green
  };

  const barColor = getBarColor();

  if (compact) {
    return (
      <div className={`flex flex-col gap-1 ${align === 'right' ? 'items-start' : 'items-end'}`}>
        {label && (
          <div 
            className="text-[#AAAAAA] text-xs tracking-wider"
            style={{
              fontFamily: 'Arial, sans-serif',
              fontSize: '8px',
              textTransform: 'uppercase',
              textShadow: '0 0 4px rgba(255, 255, 255, 0.2)',
            }}
          >
            {label}
          </div>
        )}
        
        {/* Compact value display */}
        <div 
          className="text-[#FFD700]"
          style={{
            fontFamily: 'Courier New, monospace',
            fontSize: '14px',
            textShadow: '0 0 6px rgba(255, 215, 0, 0.4)',
            lineHeight: '1',
          }}
        >
          {Math.round(value)}
          {unit && <span className="text-xs ml-1">{unit}</span>}
        </div>

        {/* Fluid bar - compact */}
        <div 
          className="relative w-24 h-3 bg-[#1a1a1a] border border-[#2a2a2a]"
          style={{
            boxShadow: 'inset 0 1px 3px rgba(0,0,0,0.5)',
          }}
        >
          <div
            className="absolute top-0 left-0 h-full transition-all duration-200"
            style={{
              width: `${percentage}%`,
              backgroundColor: barColor,
              boxShadow: `0 0 6px ${barColor}`,
            }}
          />
        </div>
      </div>
    );
  }

  return (
    <div className={`flex flex-col gap-2 ${align === 'right' ? 'items-start' : 'items-end'}`}>
      {label && (
        <div 
          className="text-[#CCCCCC] text-xs tracking-wider"
          style={{
            fontFamily: 'Arial, sans-serif',
            fontSize: '10px',
            textTransform: 'uppercase',
            textShadow: '0 0 6px rgba(255, 255, 255, 0.3)',
          }}
        >
          {label}
        </div>
      )}
      
      {/* Value display */}
      <div 
        className="text-[#FFD700] font-bold"
        style={{
          fontFamily: 'Courier New, monospace',
          fontSize: '32px',
          textShadow: '0 0 8px rgba(255, 215, 0, 0.4)',
          lineHeight: '1',
        }}
      >
        {Math.round(value)}
        {unit && <span className="text-sm ml-1">{unit}</span>}
      </div>

      {/* Fluid fill bar */}
      <div 
        className="relative w-32 h-8 bg-[#1a1a1a] border border-[#2a2a2a]"
        style={{
          boxShadow: 'inset 0 2px 4px rgba(0,0,0,0.5)',
        }}
      >
        <div
          className="absolute top-0 left-0 h-full transition-all duration-200"
          style={{
            width: `${percentage}%`,
            backgroundColor: barColor,
            boxShadow: `0 0 8px ${barColor}`,
          }}
        />
        {/* Segment lines for visual reference */}
        {Array.from({ length: 7 }).map((_, i) => (
          <div
            key={i}
            className="absolute top-0 h-full w-px bg-[#0a0a0a] opacity-30"
            style={{
              left: `${((i + 1) * 12.5)}%`,
            }}
          />
        ))}
      </div>
    </div>
  );
}
