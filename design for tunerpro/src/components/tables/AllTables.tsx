import { useState, useEffect, useMemo } from 'react';
import { useEngineConfig } from '../context/EngineConfigContext';
import { 
  ALL_TABLE_DEFINITIONS, 
  getVisibleTables, 
  getTablesByCategory,
  TableDefinition,
  TableCategory 
} from './TableDefinitions';
import { Enhanced3DViewer } from './Enhanced3DViewer';
import { HondataHeatmap2D } from './HondataHeatmap2D';
import { Button } from '../ui/button';
import { ScrollArea } from '../ui/scroll-area';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '../ui/tabs';
import { Alert, AlertDescription } from '../ui/alert';
import { 
  ChevronDown, 
  ChevronRight, 
  Copy, 
  Download, 
  Upload, 
  RotateCcw,
  Save,
  Grid3x3,
  Table as TableIcon,
  Plus,
  Minus,
  Search,
  AlertTriangle,
  Info,
} from 'lucide-react';
import { Input } from '../ui/input';
import { 
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '../ui/select';

interface AllTablesProps {
  liveTuning: boolean;
}

interface TableState {
  data: number[][];
  selectedCells: Set<string>;
  activeCell: { row: number; col: number } | null;
}

type CamProfile = 'low' | 'high';

export function AllTables({ liveTuning }: AllTablesProps) {
  const { config } = useEngineConfig();
  const [expandedCategories, setExpandedCategories] = useState<Set<TableCategory>>(
    new Set(['fuel', 'ignition', 'boost', 'compensation', 'valve-idle', 'protection', 'vtec'])
  );
  const [viewMode, setViewMode] = useState<'2d' | '3d'>('3d');
  const [currentProfile, setCurrentProfile] = useState<CamProfile>('low');
  const [incrementValue, setIncrementValue] = useState(0.5);
  const [selectedTableId, setSelectedTableId] = useState<string>('');
  const [searchQuery, setSearchQuery] = useState('');
  
  // Track table states for all tables
  const [tableStates, setTableStates] = useState<Map<string, { low: TableState; high: TableState }>>(new Map());
  
  // Get visible tables based on engine configuration
  const visibleTables = useMemo(() => getVisibleTables(config.features), [config.features]);
  
  // Set initial selected table when visible tables change
  useEffect(() => {
    if (visibleTables.length > 0 && !selectedTableId) {
      setSelectedTableId(visibleTables[0].id);
    }
  }, [visibleTables.length, selectedTableId]);

  // Initialize table states
  useEffect(() => {
    const newStates = new Map(tableStates);
    let hasChanges = false;
    
    visibleTables.forEach(table => {
      if (!newStates.has(table.id)) {
        newStates.set(table.id, {
          low: {
            data: table.defaultData.map(row => [...row]),
            selectedCells: new Set(),
            activeCell: null,
          },
          high: {
            data: table.defaultData.map(row => [...row]),
            selectedCells: new Set(),
            activeCell: null,
          },
        });
        hasChanges = true;
      }
    });
    
    if (hasChanges) {
      setTableStates(newStates);
    }
  }, [visibleTables.length]);

  // Simulate live ECU data
  useEffect(() => {
    if (!liveTuning) {
      // Clear all active cells
      setTableStates(prev => {
        const newStates = new Map(prev);
        newStates.forEach((state, key) => {
          state.low.activeCell = null;
          state.high.activeCell = null;
        });
        return newStates;
      });
      return;
    }

    const interval = setInterval(() => {
      // Simulate operating point moving across tables
      const time = Date.now() / 1000;
      const simulatedRPM = 3000 + Math.sin(time * 0.5) * 2000;
      const simulatedLoad = 80 + Math.sin(time * 0.3) * 40;

      setTableStates(prev => {
        const newStates = new Map(prev);
        
        // Update active cells for fuel and ignition tables
        ['ve-table', 'afr-target', 'ignition-advance'].forEach(tableId => {
          const tableState = newStates.get(tableId);
          if (!tableState) return;
          
          const tableDef = ALL_TABLE_DEFINITIONS.find(t => t.id === tableId);
          if (!tableDef) return;
          
          // Find closest RPM
          let closestRpmIdx = 0;
          let minRpmDiff = Math.abs(tableDef.xAxis.values[0] - simulatedRPM);
          for (let i = 1; i < tableDef.xAxis.values.length; i++) {
            const diff = Math.abs(tableDef.xAxis.values[i] - simulatedRPM);
            if (diff < minRpmDiff) {
              minRpmDiff = diff;
              closestRpmIdx = i;
            }
          }
          
          // Find closest Load
          let closestLoadIdx = 0;
          let minLoadDiff = Math.abs(tableDef.yAxis.values[0] - simulatedLoad);
          for (let i = 1; i < tableDef.yAxis.values.length; i++) {
            const diff = Math.abs(tableDef.yAxis.values[i] - simulatedLoad);
            if (diff < minLoadDiff) {
              minLoadDiff = diff;
              closestLoadIdx = i;
            }
          }
          
          tableState[currentProfile].activeCell = { 
            row: closestLoadIdx, 
            col: closestRpmIdx 
          };
        });
        
        return newStates;
      });
    }, 100);

    return () => clearInterval(interval);
  }, [liveTuning, currentProfile]);

  const toggleCategory = (category: TableCategory) => {
    setExpandedCategories(prev => {
      const newSet = new Set(prev);
      if (newSet.has(category)) {
        newSet.delete(category);
      } else {
        newSet.add(category);
      }
      return newSet;
    });
  };

  const handleCellClick = (tableId: string, profile: CamProfile, row: number, col: number) => {
    setTableStates(prev => {
      const newStates = new Map(prev);
      const tableState = newStates.get(tableId);
      if (!tableState) return prev;
      
      const cellKey = `${row}-${col}`;
      const currentSelected = tableState[profile].selectedCells;
      const newSelected = new Set(currentSelected);
      
      if (newSelected.has(cellKey)) {
        newSelected.delete(cellKey);
      } else {
        newSelected.add(cellKey);
      }
      
      tableState[profile].selectedCells = newSelected;
      return newStates;
    });
  };

  const handleCellValueChange = (
    tableId: string, 
    profile: CamProfile, 
    row: number, 
    col: number, 
    value: number
  ) => {
    setTableStates(prev => {
      const newStates = new Map(prev);
      const tableState = newStates.get(tableId);
      if (!tableState) return prev;
      
      const newData = tableState[profile].data.map((r, rIdx) =>
        rIdx === row ? r.map((c, cIdx) => cIdx === col ? value : c) : r
      );
      
      tableState[profile].data = newData;
      return newStates;
    });
  };

  const handleBulkAdjust = (tableId: string, profile: CamProfile, delta: number) => {
    setTableStates(prev => {
      const newStates = new Map(prev);
      const tableState = newStates.get(tableId);
      if (!tableState) return prev;
      
      const selectedCells = tableState[profile].selectedCells;
      if (selectedCells.size === 0) return prev;
      
      const newData = tableState[profile].data.map((row, rIdx) =>
        row.map((cell, cIdx) => {
          const cellKey = `${rIdx}-${cIdx}`;
          return selectedCells.has(cellKey) ? cell + delta : cell;
        })
      );
      
      tableState[profile].data = newData;
      return newStates;
    });
  };

  const adjustSelectedCells = (delta: number) => {
    if (!selectedTableId) return;
    handleBulkAdjust(selectedTableId, currentProfile, delta);
  };

  const renderTable = (table: TableDefinition) => {
    const tableState = tableStates.get(table.id);
    if (!tableState) return null;

    const needsProfileTabs = config.features.requiresDualMaps && table.profileSpecific;

    return (
      <div key={table.id} className="bg-[#2a2a2a] border border-[#444444] rounded-lg overflow-hidden">
        {/* Table Header */}
        <div className="bg-[#1a1a1a] border-b border-[#444444] px-4 py-3 flex-shrink-0">
          <div className="flex items-start justify-between">
            <div>
              <h3 className="text-sm text-white mb-1">{table.name}</h3>
              <p className="text-xs text-[#888888]">{table.description}</p>
            </div>
            <div className="flex items-center gap-2">
              {/* View Mode Toggle */}
              <div className="flex gap-0.5 bg-[#3a3a3a] rounded border border-[#555555]">
                <button
                  onClick={() => setViewMode('2d')}
                  className={`px-2 py-1 text-xs transition-colors flex items-center gap-1 ${
                    viewMode === '2d'
                      ? 'bg-[#0066CC] text-white'
                      : 'text-[#CCCCCC] hover:bg-[#444444]'
                  }`}
                >
                  <TableIcon className="w-3 h-3" />
                  2D
                </button>
                <button
                  onClick={() => setViewMode('3d')}
                  className={`px-2 py-1 text-xs transition-colors flex items-center gap-1 ${
                    viewMode === '3d'
                      ? 'bg-[#0066CC] text-white'
                      : 'text-[#CCCCCC] hover:bg-[#444444]'
                  }`}
                >
                  <Grid3x3 className="w-3 h-3" />
                  3D
                </button>
              </div>
              
              {/* Action Buttons */}
              <button className="p-1.5 text-[#CCCCCC] hover:bg-[#444444] rounded border border-[#555555]">
                <Copy className="w-3.5 h-3.5" />
              </button>
              <button className="p-1.5 text-[#CCCCCC] hover:bg-[#444444] rounded border border-[#555555]">
                <Download className="w-3.5 h-3.5" />
              </button>
              <button className="p-1.5 text-[#CCCCCC] hover:bg-[#444444] rounded border border-[#555555]">
                <Upload className="w-3.5 h-3.5" />
              </button>
              <button className="p-1.5 text-[#CCCCCC] hover:bg-[#444444] rounded border border-[#555555]">
                <RotateCcw className="w-3.5 h-3.5" />
              </button>
              <button className="p-1.5 text-[#00AA00] hover:bg-[#00AA00]/20 rounded border border-[#00AA00]">
                <Save className="w-3.5 h-3.5" />
              </button>
            </div>
          </div>
        </div>

        {/* Table Content */}
        {needsProfileTabs ? (
          <Tabs value={currentProfile} onValueChange={(v) => setCurrentProfile(v as CamProfile)}>
            <div className="bg-[#1a1a1a] border-b border-[#444444] px-4 py-2">
              <TabsList className="bg-[#2a2a2a]">
                <TabsTrigger value="low" className="data-[state=active]:bg-[#0066CC]">
                  Low Cam Profile
                </TabsTrigger>
                <TabsTrigger value="high" className="data-[state=active]:bg-[#FF6600]">
                  High Cam Profile
                </TabsTrigger>
              </TabsList>
            </div>
            
            <TabsContent value="low" className="m-0">
              {renderTableContent(table, tableState.low, 'low')}
            </TabsContent>
            
            <TabsContent value="high" className="m-0">
              {renderTableContent(table, tableState.high, 'high')}
            </TabsContent>
          </Tabs>
        ) : (
          renderTableContent(table, tableState.low, 'low')
        )}
      </div>
    );
  };

  const renderTableContent = (table: TableDefinition, state: TableState, profile: CamProfile) => {
    return (
      <div className="p-4">
        {/* New 3-Column Layout: Grid | Controls | Graph */}
        <div className="grid grid-cols-12 gap-4">
          {/* LEFT: Editable Grid - 6 columns */}
          <div className="col-span-6">
            <div className="h-[500px] bg-[#1a1a1a] border border-[#444444] rounded overflow-auto">
              <table className="w-full border-collapse text-xs">
                <thead className="sticky top-0 z-10">
                  <tr>
                    <th className="border border-[#444444] bg-[#2a2a2a] px-2 py-1.5 text-[#CCCCCC] sticky left-0 z-20 text-[10px]">
                      {table.yAxis.label}/{table.xAxis.label}
                    </th>
                    {table.xAxis.values.map((value, idx) => (
                      <th key={idx} className="border border-[#444444] bg-[#2a2a2a] px-1.5 py-1 text-[#CCCCCC] text-center min-w-[45px] text-[10px]">
                        {value}
                      </th>
                    ))}
                  </tr>
                </thead>
                <tbody>
                  {state.data.map((row, rowIdx) => {
                    // Calculate min/max for color mapping
                    const flatData = state.data.flat();
                    const minValue = Math.min(...flatData);
                    const maxValue = Math.max(...flatData);
                    
                    return (
                      <tr key={rowIdx}>
                        <td className="border border-[#444444] bg-[#2a2a2a] px-1.5 py-1 text-[#CCCCCC] sticky left-0 z-10 text-[10px]">
                          {table.yAxis.values[rowIdx]}
                        </td>
                        {row.map((cell, colIdx) => {
                          const cellKey = `${rowIdx}-${colIdx}`;
                          const isSelected = state.selectedCells.has(cellKey);
                          const isActive = state.activeCell?.row === rowIdx && state.activeCell?.col === colIdx;

                          // O2-style subtle color mapping: Green to Yellow gradient
                          const normalized = (cell - minValue) / (maxValue - minValue);
                          let cellColor = '#2d5c2d'; // Default green (low values)
                          
                          if (normalized < 0.2) {
                            cellColor = '#2d5c2d'; // Deep green
                          } else if (normalized < 0.4) {
                            cellColor = '#2d5c3d'; // Green
                          } else if (normalized < 0.6) {
                            cellColor = '#3d5c2d'; // Light green
                          } else if (normalized < 0.8) {
                            cellColor = '#4d5c2d'; // Yellow-green
                          } else {
                            cellColor = '#5c5c2d'; // Yellow (high values)
                          }

                          return (
                            <td
                              key={colIdx}
                              className="border border-[#444444] p-0 text-center cursor-pointer hover:brightness-125 transition-all"
                              style={{
                                backgroundColor: isActive ? '#00FFFF' : isSelected ? '#0088FF' : cellColor
                              }}
                              onClick={() => handleCellClick(table.id, profile, rowIdx, colIdx)}
                            >
                              <input
                                type="number"
                                value={cell.toFixed(table.zAxis.unit === 'bool' ? 0 : 2)}
                                onChange={(e) => {
                                  const val = parseFloat(e.target.value);
                                  if (!isNaN(val)) {
                                    handleCellValueChange(table.id, profile, rowIdx, colIdx, val);
                                  }
                                }}
                                className="w-full bg-transparent text-center px-1.5 py-1 border-0 outline-none font-mono text-[10px] text-white"
                                step="0.1"
                              />
                            </td>
                          );
                        })}
                      </tr>
                    );
                  })}
                </tbody>
              </table>
            </div>
          </div>

          {/* MIDDLE: Controls Panel - 2 columns */}
          <div className="col-span-2 flex flex-col gap-3">
            {/* Table Info */}
            <div className="bg-[#2a2a2a] border border-[#444444] rounded p-3">
              <div className="space-y-2">
                <div className="flex items-center justify-between">
                  <span className="text-xs text-[#888888]">Table Size:</span>
                  <span className="text-xs text-[#00CCFF]">{table.xAxis.values.length}x{table.yAxis.values.length}</span>
                </div>
                <div className="h-px bg-[#444444]" />
                <div className="space-y-1">
                  <div className="text-xs text-[#888888]">
                    <span className="text-[#00CCFF]">X:</span> {table.xAxis.label} ({table.xAxis.unit})
                  </div>
                  <div className="text-xs text-[#888888]">
                    <span className="text-[#00CCFF]">Y:</span> {table.yAxis.label} ({table.yAxis.unit})
                  </div>
                  <div className="text-xs text-[#888888]">
                    <span className="text-[#00CCFF]">Z:</span> {table.zAxis.label} ({table.zAxis.unit})
                  </div>
                </div>
              </div>
            </div>

            {/* Selection Info */}
            {state.selectedCells.size > 0 && (
              <div className="bg-[#2a2a2a] border border-[#0066CC] rounded p-3">
                <div className="text-xs text-[#00CCFF] mb-2">
                  {state.selectedCells.size} cell{state.selectedCells.size !== 1 ? 's' : ''} selected
                </div>
                <button
                  onClick={() => {
                    setTableStates(prev => {
                      const newStates = new Map(prev);
                      const tableState = newStates.get(table.id);
                      if (tableState) {
                        tableState[profile].selectedCells = new Set();
                      }
                      return newStates;
                    });
                  }}
                  className="w-full px-2 py-1 text-xs text-[#888888] hover:text-white hover:bg-[#444444] rounded border border-[#555555]"
                >
                  Clear Selection
                </button>
              </div>
            )}

            {/* Increment Controls */}
            <div className="bg-[#2a2a2a] border border-[#444444] rounded p-3">
              <div className="space-y-2">
                <span className="text-xs text-[#888888]">Adjust Selected:</span>
                <div className="flex items-center gap-2">
                  <Input
                    type="number"
                    value={incrementValue}
                    onChange={(e) => setIncrementValue(parseFloat(e.target.value) || 0.5)}
                    className="flex-1 h-8 text-xs bg-[#1a1a1a] border-[#555555] text-center text-white"
                    step="0.1"
                  />
                </div>
                <div className="grid grid-cols-2 gap-2">
                  <button 
                    onClick={() => adjustSelectedCells(incrementValue)}
                    disabled={state.selectedCells.size === 0}
                    className="px-2 py-1.5 text-xs bg-[#00AA00]/20 text-[#00FF00] border border-[#00AA00] hover:bg-[#00AA00]/30 rounded flex items-center justify-center gap-1 disabled:opacity-30 disabled:cursor-not-allowed"
                  >
                    <Plus className="w-3 h-3" />
                    Add
                  </button>
                  <button 
                    onClick={() => adjustSelectedCells(-incrementValue)}
                    disabled={state.selectedCells.size === 0}
                    className="px-2 py-1.5 text-xs bg-[#AA0000]/20 text-[#FF6666] border border-[#AA0000] hover:bg-[#AA0000]/30 rounded flex items-center justify-center gap-1 disabled:opacity-30 disabled:cursor-not-allowed"
                  >
                    <Minus className="w-3 h-3" />
                    Subtract
                  </button>
                </div>
              </div>
            </div>

            {/* Quick Actions */}
            <div className="bg-[#2a2a2a] border border-[#444444] rounded p-3">
              <div className="space-y-1.5">
                <span className="text-xs text-[#888888] mb-1 block">Quick Actions:</span>
                <button className="w-full px-2 py-1.5 text-xs bg-[#1a1a1a] border border-[#555555] text-[#CCCCCC] hover:bg-[#333333] rounded flex items-center justify-center gap-1">
                  <Copy className="w-3 h-3" />
                  Copy Table
                </button>
                <button className="w-full px-2 py-1.5 text-xs bg-[#1a1a1a] border border-[#555555] text-[#CCCCCC] hover:bg-[#333333] rounded flex items-center justify-center gap-1">
                  Smooth Values
                </button>
                <button className="w-full px-2 py-1.5 text-xs bg-[#1a1a1a] border border-[#555555] text-[#CCCCCC] hover:bg-[#333333] rounded flex items-center justify-center gap-1">
                  Interpolate
                </button>
                <button className="w-full px-2 py-1.5 text-xs bg-[#1a1a1a] border border-[#555555] text-[#CCCCCC] hover:bg-[#333333] rounded flex items-center justify-center gap-1">
                  <RotateCcw className="w-3 h-3" />
                  Reset
                </button>
              </div>
            </div>
          </div>

          {/* RIGHT: 3D/2D Visualization - 4 columns */}
          <div className="col-span-4">
            <div className="h-[500px] relative bg-[#1a1a1a] border border-[#444444] rounded overflow-hidden">
              {viewMode === '3d' ? (
                <Enhanced3DViewer
                  data={state.data}
                  selectedCell={state.activeCell}
                  rpmAxis={table.xAxis.values}
                  mapAxis={table.yAxis.values}
                  title={table.name}
                  unit={table.zAxis.unit}
                />
              ) : (
                <HondataHeatmap2D
                  tableData={state.data}
                  definition={table}
                  activeCell={state.activeCell}
                  selectedCells={state.selectedCells}
                  onCellClick={(row, col) => handleCellClick(table.id, profile, row, col)}
                />
              )}
            </div>
          </div>
        </div>
      </div>
    );
  };

  const categories: Array<{ id: TableCategory; name: string; icon: string }> = [
    { id: 'fuel', name: 'Fuel Tables', icon: '⛽' },
    { id: 'ignition', name: 'Ignition Tables', icon: '⚡' },
    { id: 'boost', name: 'Boost Control', icon: '🚀' },
    { id: 'compensation', name: 'Compensation Tables', icon: '🌡️' },
    { id: 'valve-idle', name: 'Valve Timing & Idle', icon: '⚙️' },
    { id: 'protection', name: 'Protection & Limiters', icon: '🛡️' },
    { id: 'vtec', name: 'VTEC Control', icon: '🏁' },
  ];

  // Group tables by category for dropdown
  const tablesByCategory = useMemo(() => {
    const categories = new Map<string, { name: string; tables: TableDefinition[] }>();
    const categoryInfo: Record<TableCategory, string> = {
      'fuel': 'Fuel Tables',
      'ignition': 'Ignition Tables',
      'boost': 'Boost Control',
      'compensation': 'Compensation',
      'valve-idle': 'Valve & Idle',
      'protection': 'Protection',
      'vtec': 'VTEC Control',
    };

    visibleTables.forEach(table => {
      if (!categories.has(table.category)) {
        categories.set(table.category, {
          name: categoryInfo[table.category] || table.category,
          tables: []
        });
      }
      categories.get(table.category)?.tables.push(table);
    });

    return categories;
  }, [visibleTables]);

  return (
    <div className="h-full w-full bg-[#1a1a1a] flex flex-col">
      {/* Header */}
      <div className="border-b border-[#444444] bg-[#1a1a1a] px-4 py-3 flex-shrink-0">
        <div className="flex items-center justify-between gap-4 mb-3">
          <div className="flex-1">
            <h2 className="text-white mb-1">All Tuning Tables</h2>
            <p className="text-xs text-[#888888]">
              Comprehensive table editor with conditional visibility • {visibleTables.length} tables active
            </p>
          </div>
          
          {liveTuning && (
            <div className="flex items-center gap-2 px-3 py-1.5 rounded bg-[#00AA00]/20 border border-[#00AA00]">
              <div className="w-2 h-2 rounded-full bg-[#00FF00] animate-pulse" />
              <span className="text-xs text-[#00FF00]">LIVE TUNING</span>
            </div>
          )}
        </div>

        {/* VTEC Warning Alert */}
        {config.features.requiresDualMaps && (
          <Alert className="bg-[#FF6600]/10 border-[#FF6600]/30 mb-3">
            <AlertTriangle className="h-4 w-4 text-[#FF6600]" />
            <AlertDescription className="text-xs text-[#FF6600]">
              <strong>VTEC enabled:</strong> Dual profiles active (Low Cam / High Cam). Check for discontinuity at engagement RPM to ensure smooth transitions.
            </AlertDescription>
          </Alert>
        )}

        {/* Table Selector Dropdown */}
        <div className="flex items-center gap-3">
          <div className="flex-1 max-w-md">
            <Select value={selectedTableId} onValueChange={setSelectedTableId}>
              <SelectTrigger className="bg-[#2a2a2a] border-[#555555] text-white h-9">
                <SelectValue placeholder="Select a table to edit..." />
              </SelectTrigger>
              <SelectContent className="bg-[#2a2a2a] border-[#555555] max-h-[400px]">
                {Array.from(tablesByCategory.entries()).map(([categoryId, { name, tables }]) => (
                  <div key={categoryId}>
                    <div className="px-2 py-1.5 text-xs text-[#888888] bg-[#1a1a1a] border-b border-[#444444]">
                      {name}
                    </div>
                    {tables.map(table => (
                      <SelectItem 
                        key={table.id} 
                        value={table.id}
                        className="text-white hover:bg-[#333333] focus:bg-[#0066CC] focus:text-white cursor-pointer pl-6"
                      >
                        {table.name}
                      </SelectItem>
                    ))}
                  </div>
                ))}
              </SelectContent>
            </Select>
          </div>

          <div className="relative flex-1 max-w-xs">
            <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-4 h-4 text-[#888888]" />
            <Input
              type="text"
              placeholder="Search tables..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className="bg-[#2a2a2a] border-[#555555] text-white pl-9 h-9"
            />
          </div>
        </div>
      </div>

      {/* Main Content - Scrollable */}
      <div className="flex-1 overflow-auto">
        <div className="p-4">
          {visibleTables.length === 0 && (
            <div className="flex items-center justify-center h-64">
              <div className="text-center">
                <p className="text-white mb-2">No tables available</p>
                <p className="text-sm text-[#888888]">
                  Configure your engine type in the Setup tab to enable additional tables
                </p>
              </div>
            </div>
          )}
          
          {/* Display Selected Table */}
          {selectedTableId && (() => {
            const selectedTable = visibleTables.find(t => t.id === selectedTableId);
            return selectedTable ? renderTable(selectedTable) : null;
          })()}
        </div>
      </div>
    </div>
  );
}
