import { useMemo, useRef, useState } from "react";
import { motion } from "motion/react";
import { Line } from "react-chartjs-2";
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  ChartOptions
} from 'chart.js';
import { ZoomIn, ZoomOut, Maximize2, Download } from "lucide-react";
import { LogDataPoint, LogChannel, STANDARD_CHANNELS } from "./LoggingTypes";

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend
);

interface LogGraphProps {
  data: LogDataPoint[];
  channels: string[];
  title?: string;
  height?: number;
}

export function LogGraph({ data, channels, title = "Log Data", height = 400 }: LogGraphProps) {
  const [zoomLevel, setZoomLevel] = useState(1);
  const chartRef = useRef<any>(null);

  const chartData = useMemo(() => {
    if (!data.length) return { labels: [], datasets: [] };

    // Generate time labels
    const startTime = data[0].timestamp;
    const labels = data.map(point => {
      const elapsed = (point.timestamp - startTime) / 1000; // seconds
      return elapsed.toFixed(1);
    });

    // Create datasets for each channel
    const datasets = channels.map(channelId => {
      const channel = STANDARD_CHANNELS.find(c => c.id === channelId);
      if (!channel) return null;

      return {
        label: `${channel.name} (${channel.unit})`,
        data: data.map(point => point.values[channelId] || null),
        borderColor: channel.color,
        backgroundColor: channel.color + '33',
        borderWidth: 2,
        tension: 0.3,
        pointRadius: 0,
        pointHoverRadius: 4,
        fill: false
      };
    }).filter(Boolean);

    return { labels, datasets };
  }, [data, channels]);

  const options: ChartOptions<'line'> = {
    responsive: true,
    maintainAspectRatio: false,
    interaction: {
      mode: 'index',
      intersect: false,
    },
    plugins: {
      legend: {
        position: 'top',
        labels: {
          color: '#E6EEF3',
          font: {
            size: 11
          },
          usePointStyle: true,
          padding: 15
        }
      },
      title: {
        display: true,
        text: title,
        color: '#E6EEF3',
        font: {
          size: 14,
          weight: 'normal'
        }
      },
      tooltip: {
        backgroundColor: 'rgba(0, 0, 0, 0.9)',
        titleColor: '#00C4B4',
        bodyColor: '#E6EEF3',
        borderColor: '#333333',
        borderWidth: 1,
        padding: 10,
        displayColors: true,
        callbacks: {
          title: (context) => {
            return `Time: ${context[0].label}s`;
          },
          label: (context) => {
            const channel = STANDARD_CHANNELS.find(c => 
              context.dataset.label?.includes(c.name)
            );
            if (channel) {
              return `${channel.name}: ${Number(context.parsed.y).toFixed(channel.decimals)} ${channel.unit}`;
            }
            return context.dataset.label || '';
          }
        }
      }
    },
    scales: {
      x: {
        title: {
          display: true,
          text: 'Time (seconds)',
          color: '#888888',
          font: { size: 11 }
        },
        grid: {
          color: '#333333',
          lineWidth: 1
        },
        ticks: {
          color: '#888888',
          font: { size: 10 }
        }
      },
      y: {
        title: {
          display: true,
          text: 'Value',
          color: '#888888',
          font: { size: 11 }
        },
        grid: {
          color: '#333333',
          lineWidth: 1
        },
        ticks: {
          color: '#888888',
          font: { size: 10 }
        }
      }
    },
    elements: {
      line: {
        borderWidth: 2
      }
    }
  };

  const handleZoomIn = () => {
    setZoomLevel(prev => Math.min(prev * 1.5, 10));
  };

  const handleZoomOut = () => {
    setZoomLevel(prev => Math.max(prev / 1.5, 0.5));
  };

  const handleReset = () => {
    setZoomLevel(1);
  };

  const handleExport = () => {
    if (chartRef.current) {
      const url = chartRef.current.toBase64Image();
      const link = document.createElement('a');
      link.download = `log-graph-${Date.now()}.png`;
      link.href = url;
      link.click();
    }
  };

  return (
    <div className="h-full flex flex-col bg-[#1a1a1a] border border-[#333333] rounded">
      {/* Toolbar */}
      <div className="flex items-center justify-between p-3 border-b border-[#333333]">
        <div className="flex items-center gap-2">
          <button
            onClick={handleZoomIn}
            className="p-1.5 bg-[#2a2a2a] hover:bg-[#333333] text-[#E6EEF3] rounded transition-colors"
            title="Zoom In"
          >
            <ZoomIn className="w-4 h-4" />
          </button>
          <button
            onClick={handleZoomOut}
            className="p-1.5 bg-[#2a2a2a] hover:bg-[#333333] text-[#E6EEF3] rounded transition-colors"
            title="Zoom Out"
          >
            <ZoomOut className="w-4 h-4" />
          </button>
          <button
            onClick={handleReset}
            className="px-2 py-1.5 bg-[#2a2a2a] hover:bg-[#333333] text-[#E6EEF3] rounded transition-colors text-xs"
            title="Reset Zoom"
          >
            Reset
          </button>
          <div className="text-xs text-[#666666] ml-2">
            Zoom: {(zoomLevel * 100).toFixed(0)}%
          </div>
        </div>

        <div className="flex items-center gap-2">
          <button
            onClick={handleExport}
            className="flex items-center gap-1.5 px-2 py-1.5 bg-[#2a2a2a] hover:bg-[#333333] text-[#E6EEF3] rounded transition-colors text-xs"
          >
            <Download className="w-3.5 h-3.5" />
            Export PNG
          </button>
        </div>
      </div>

      {/* Graph */}
      <div className="flex-1 p-4" style={{ height }}>
        {data.length > 0 ? (
          <Line ref={chartRef} data={chartData} options={options} />
        ) : (
          <div className="h-full flex items-center justify-center text-[#666666]">
            <div className="text-center">
              <Maximize2 className="w-12 h-12 mx-auto mb-3 opacity-30" />
              <p>No data to display</p>
              <p className="text-sm mt-1">Start recording or load a log file</p>
            </div>
          </div>
        )}
      </div>
    </div>
  );
}
