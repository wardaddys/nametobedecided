import { useState, useMemo } from 'react';
import { 
  BOOST_TABLE_DEFINITIONS, 
  BOOST_PARAMETERS, 
  FORCED_INDUCTION_CONFIGS,
  ForcedInductionType,
  calculateSetupCompletion 
} from './BoostDatabase';
import { HondataSurface3D } from '../tables/HondataSurface3D';
import { HondataHeatmap2D } from '../tables/HondataHeatmap2D';
import { ScrollArea } from '../ui/scroll-area';
import { Progress } from '../ui/progress';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '../ui/tabs';
import { 
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '../ui/select';
import {
  ChevronDown,
  ChevronRight,
  Info,
  AlertTriangle,
  Check,
  Save,
  Download,
  Upload,
  RotateCcw,
  Zap,
  TrendingUp,
  Shield,
  Settings as SettingsIcon,
} from 'lucide-react';
import { Input } from '../ui/input';
import { Switch } from '../ui/switch';
import { Button } from '../ui/button';
import {
  Tooltip,
  TooltipContent,
  TooltipProvider,
  TooltipTrigger,
} from '../ui/tooltip';
import { Alert, AlertDescription } from '../ui/alert';

interface ComprehensiveBoostSettingsProps {
  liveTuning: boolean;
  forcedInductionType: ForcedInductionType;
  onConfigChange?: (type: ForcedInductionType) => void;
}

interface TableState {
  data: number[][];
  selectedCells: Set<string>;
  activeCell: { row: number; col: number } | null;
}

export function ComprehensiveBoostSettings({ 
  liveTuning, 
  forcedInductionType,
  onConfigChange 
}: ComprehensiveBoostSettingsProps) {
  const [currentConfig, setCurrentConfig] = useState<ForcedInductionType>(forcedInductionType);
  const [viewMode, setViewMode] = useState<'2d' | '3d'>('3d');
  const [expandedCategories, setExpandedCategories] = useState<Set<string>>(
    new Set(['Target & Ramp', 'Control & Solenoid', 'Protection & Compensation'])
  );
  const [selectedTableId, setSelectedTableId] = useState<string>('boost-target');
  const [showNoviceMode, setShowNoviceMode] = useState(true);
  
  // Table states
  const [tableStates, setTableStates] = useState<Map<string, TableState>>(() => {
    const states = new Map();
    BOOST_TABLE_DEFINITIONS.forEach(table => {
      states.set(table.id, {
        data: table.defaultData.map(row => [...row]),
        selectedCells: new Set(),
        activeCell: null,
      });
    });
    return states;
  });

  // Parameter values
  const [paramValues, setParamValues] = useState<Map<string, any>>(() => {
    const values = new Map();
    BOOST_PARAMETERS.forEach(param => {
      values.set(param.id, param.default);
    });
    return values;
  });

  // Get applicable tables and params
  const applicableTables = useMemo(() => 
    BOOST_TABLE_DEFINITIONS.filter(t => t.applicableConfigs.includes(currentConfig)),
    [currentConfig]
  );

  const applicableParams = useMemo(() =>
    BOOST_PARAMETERS.filter(p => p.applicableConfigs.includes(currentConfig)),
    [currentConfig]
  );

  // Calculate setup completion
  const setupCompletion = useMemo(() => {
    const completedTables = applicableTables.map(t => t.id);
    const completedParams = applicableParams.map(p => p.id);
    return calculateSetupCompletion(currentConfig, completedTables, completedParams);
  }, [currentConfig, applicableTables, applicableParams]);

  // Group tables by category
  const tablesByCategory = useMemo(() => {
    const categories = new Map<string, typeof applicableTables>();
    applicableTables.forEach(table => {
      if (!categories.has(table.category)) {
        categories.set(table.category, []);
      }
      categories.get(table.category)?.push(table);
    });
    return categories;
  }, [applicableTables]);

  const configInfo = FORCED_INDUCTION_CONFIGS[currentConfig];

  const handleConfigChange = (newConfig: ForcedInductionType) => {
    setCurrentConfig(newConfig);
    onConfigChange?.(newConfig);
    // Auto-select first applicable table
    const firstTable = BOOST_TABLE_DEFINITIONS.find(t => t.applicableConfigs.includes(newConfig));
    if (firstTable) {
      setSelectedTableId(firstTable.id);
    }
  };

  const handleCellClick = (tableId: string, row: number, col: number) => {
    setTableStates(prev => {
      const newStates = new Map(prev);
      const tableState = newStates.get(tableId);
      if (!tableState) return prev;
      
      const cellKey = `${row}-${col}`;
      const newSelected = new Set(tableState.selectedCells);
      
      if (newSelected.has(cellKey)) {
        newSelected.delete(cellKey);
      } else {
        newSelected.add(cellKey);
      }
      
      tableState.selectedCells = newSelected;
      return newStates;
    });
  };

  const handleCellValueChange = (tableId: string, row: number, col: number, value: number) => {
    setTableStates(prev => {
      const newStates = new Map(prev);
      const tableState = newStates.get(tableId);
      if (!tableState) return prev;
      
      const newData = tableState.data.map((r, rIdx) =>
        rIdx === row ? r.map((c, cIdx) => cIdx === col ? value : c) : r
      );
      
      tableState.data = newData;
      return newStates;
    });
  };

  const toggleCategory = (category: string) => {
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

  const renderTable = (table: typeof BOOST_TABLE_DEFINITIONS[0]) => {
    const tableState = tableStates.get(table.id);
    if (!tableState) return null;

    // Convert to table definition format for 3D viewer
    const tableDefinition = {
      id: table.id,
      name: table.name,
      xAxis: table.xAxis,
      yAxis: table.yAxis,
      zAxis: table.zAxis,
      defaultData: table.defaultData,
    };

    return (
      <div key={table.id} className="bg-[#2a2a2a] border border-[#444444] rounded-lg overflow-hidden">
        {/* Table Header */}
        <div className="bg-[#1a1a1a] border-b border-[#444444] px-4 py-3">
          <div className="flex items-start justify-between gap-4">
            <div className="flex-1">
              <div className="flex items-center gap-2 mb-1">
                <span className="text-xl">{table.icon}</span>
                <h3 className="text-sm text-white">{table.name}</h3>
              </div>
              <p className="text-xs text-[#888888] mb-2">{table.description}</p>
              
              {/* Novice/Pro Info Toggle */}
              {showNoviceMode && (
                <div className="bg-[#0066CC]/10 border border-[#0066CC]/30 rounded p-2 mb-2">
                  <div className="flex items-start gap-2">
                    <Info className="w-4 h-4 text-[#00AAFF] flex-shrink-0 mt-0.5" />
                    <div>
                      <p className="text-xs text-[#00AAFF] mb-1">
                        <span className="font-semibold">Beginner:</span> {table.noviceExplanation}
                      </p>
                      <p className="text-xs text-[#888888]">
                        <span className="font-semibold">Pro:</span> {table.professionalDetails}
                      </p>
                    </div>
                  </div>
                </div>
              )}

              {/* Warnings */}
              {table.warningThresholds && (
                <Alert className="bg-[#FF6600]/10 border-[#FF6600]/30 mb-2">
                  <AlertTriangle className="h-4 w-4 text-[#FF6600]" />
                  <AlertDescription className="text-xs text-[#FF6600]">
                    {table.warningThresholds.message}
                  </AlertDescription>
                </Alert>
              )}
            </div>

            {/* Action Buttons */}
            <div className="flex items-center gap-2">
              <div className="flex gap-0.5 bg-[#3a3a3a] rounded border border-[#555555]">
                <button
                  onClick={() => setViewMode('2d')}
                  className={`px-2 py-1 text-xs transition-colors ${
                    viewMode === '2d'
                      ? 'bg-[#0066CC] text-white'
                      : 'text-[#CCCCCC] hover:bg-[#444444]'
                  }`}
                >
                  2D
                </button>
                <button
                  onClick={() => setViewMode('3d')}
                  className={`px-2 py-1 text-xs transition-colors ${
                    viewMode === '3d'
                      ? 'bg-[#0066CC] text-white'
                      : 'text-[#CCCCCC] hover:bg-[#444444]'
                  }`}
                >
                  3D
                </button>
              </div>
              
              <Button variant="ghost" size="sm" className="h-7 px-2">
                <RotateCcw className="w-3.5 h-3.5" />
              </Button>
              <Button variant="ghost" size="sm" className="h-7 px-2 text-[#00AA00] hover:text-[#00FF00]">
                <Save className="w-3.5 h-3.5" />
              </Button>
            </div>
          </div>
        </div>

        {/* Table Content: Side-by-side layout */}
        <div className="p-4">
          <div className="flex gap-3 h-[500px]">
            {/* Left: Data Table - 65% */}
            <div className="w-[65%] flex-shrink-0 bg-[#1a1a1a] border border-[#444444] rounded overflow-auto">
              <table className="w-full border-collapse text-xs">
                <thead className="sticky top-0 z-10">
                  <tr className="bg-[#0066CC]">
                    <th className="border border-[#005599] px-2 py-1.5 text-white sticky left-0 bg-[#0066CC] z-20">
                      {table.yAxis.label}/{table.xAxis.label}
                    </th>
                    {table.xAxis.values.map((value, idx) => (
                      <th key={idx} className="border border-[#005599] px-2 py-1.5 text-white text-center min-w-[50px]">
                        {value}
                      </th>
                    ))}
                  </tr>
                </thead>
                <tbody>
                  {tableState.data.map((row, rowIdx) => (
                    <tr key={rowIdx} className="hover:bg-[#333333] transition-colors">
                      <td className="border border-[#444444] px-2 py-1.5 text-[#CCCCCC] sticky left-0 bg-[#2a2a2a] z-10">
                        {table.yAxis.values[rowIdx]}
                      </td>
                      {row.map((cell, colIdx) => {
                        const cellKey = `${rowIdx}-${colIdx}`;
                        const isSelected = tableState.selectedCells.has(cellKey);
                        const isActive = tableState.activeCell?.row === rowIdx && tableState.activeCell?.col === colIdx;

                        return (
                          <td
                            key={colIdx}
                            className={`border border-[#444444] p-0 text-center cursor-pointer transition-colors ${
                              isActive
                                ? 'bg-[#00FFFF]'
                                : isSelected
                                ? 'bg-[#0088FF]'
                                : 'hover:bg-[#333333]'
                            }`}
                            onClick={() => handleCellClick(table.id, rowIdx, colIdx)}
                          >
                            <input
                              type="number"
                              value={cell.toFixed(table.zAxis.unit === 'bool' ? 0 : 1)}
                              onChange={(e) => {
                                const val = parseFloat(e.target.value);
                                if (!isNaN(val)) {
                                  handleCellValueChange(table.id, rowIdx, colIdx, val);
                                }
                              }}
                              className={`w-full bg-transparent text-center px-2 py-1.5 border-0 outline-none font-mono ${
                                isActive ? 'text-black' : isSelected ? 'text-white' : 'text-[#CCCCCC]'
                              }`}
                              step="0.1"
                            />
                          </td>
                        );
                      })}
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>

            {/* Right: 3D/2D Visualization - 35% */}
            <div className="flex-1 relative">
              {viewMode === '3d' ? (
                <HondataSurface3D
                  tableData={tableState.data}
                  definition={tableDefinition}
                  activeCell={tableState.activeCell}
                  isLive={liveTuning}
                />
              ) : (
                <HondataHeatmap2D
                  tableData={tableState.data}
                  definition={tableDefinition}
                  activeCell={tableState.activeCell}
                  selectedCells={tableState.selectedCells}
                  onCellClick={(row, col) => handleCellClick(table.id, row, col)}
                />
              )}
            </div>
          </div>
        </div>
      </div>
    );
  };

  return (
    <div className="h-full w-full bg-[#1a1a1a] flex flex-col overflow-hidden">
      {/* Header */}
      <div className="border-b border-[#444444] bg-[#1a1a1a] px-4 py-3 flex-shrink-0">
        <div className="flex items-center justify-between gap-4 mb-3">
          <div>
            <h2 className="text-white mb-1">Comprehensive Boost & Forced Induction Settings</h2>
            <p className="text-xs text-[#888888]">
              Professional-grade boost control with novice-friendly guidance
            </p>
          </div>

          {liveTuning && (
            <div className="flex items-center gap-2 px-3 py-1.5 rounded bg-[#00AA00]/20 border border-[#00AA00]">
              <div className="w-2 h-2 rounded-full bg-[#00FF00] animate-pulse" />
              <span className="text-xs text-[#00FF00]">LIVE TUNING</span>
            </div>
          )}
        </div>

        {/* Configuration Selector & Setup Progress */}
        <div className="grid grid-cols-3 gap-4 mb-3">
          <div>
            <label className="text-xs text-[#888888] mb-1.5 block">Forced Induction Type</label>
            <Select value={currentConfig} onValueChange={(v) => handleConfigChange(v as ForcedInductionType)}>
              <SelectTrigger className="bg-[#2a2a2a] border-[#555555] text-white h-9">
                <SelectValue />
              </SelectTrigger>
              <SelectContent className="bg-[#2a2a2a] border-[#555555]">
                {Object.entries(FORCED_INDUCTION_CONFIGS).map(([key, config]) => (
                  <SelectItem 
                    key={key} 
                    value={key}
                    className="text-white hover:bg-[#333333] focus:bg-[#0066CC]"
                  >
                    {config.name}
                  </SelectItem>
                ))}
              </SelectContent>
            </Select>
          </div>

          <div className="col-span-2">
            <div className="flex items-center justify-between mb-1.5">
              <label className="text-xs text-[#888888]">Setup Completion</label>
              <span className="text-xs text-[#00FF00]">{setupCompletion}%</span>
            </div>
            <Progress value={setupCompletion} className="h-2 bg-[#2a2a2a]" />
            <div className="flex items-center gap-1 mt-1">
              {setupCompletion === 100 && <Check className="w-3 h-3 text-[#00FF00]" />}
              <p className="text-xs text-[#888888]">
                {setupCompletion === 100 
                  ? '✓ Ready for dyno tuning' 
                  : `${applicableTables.length} tables to configure`}
              </p>
            </div>
          </div>
        </div>

        {/* Config Info Panel */}
        <div className="bg-[#2a2a2a] border border-[#444444] rounded-lg p-3">
          <div className="grid grid-cols-4 gap-4">
            <div>
              <p className="text-xs text-[#888888] mb-1">Configuration</p>
              <p className="text-sm text-white">{configInfo.name}</p>
              <p className="text-xs text-[#00AAFF] mt-1">{configInfo.noviceAnalogy}</p>
            </div>
            <div>
              <p className="text-xs text-[#888888] mb-1">Key Differences</p>
              <p className="text-xs text-[#CCCCCC]">{configInfo.keyDifferences}</p>
            </div>
            <div>
              <p className="text-xs text-[#888888] mb-1">Defaults</p>
              <p className="text-xs text-[#CCCCCC]">
                Target: {configInfo.defaultBoostTarget} psi<br />
                Ramp: {configInfo.defaultRampRate} kPa/s<br />
                PID: P={configInfo.defaultPID.p}, I={configInfo.defaultPID.i}, D={configInfo.defaultPID.d}
              </p>
            </div>
            <div>
              <p className="text-xs text-[#888888] mb-1">Complexity</p>
              <div className="flex items-center gap-2">
                <div className={`px-2 py-1 rounded text-xs ${
                  configInfo.setupComplexity === 'beginner' ? 'bg-[#00AA00]/20 text-[#00FF00]' :
                  configInfo.setupComplexity === 'intermediate' ? 'bg-[#FF6600]/20 text-[#FF6600]' :
                  configInfo.setupComplexity === 'advanced' ? 'bg-[#FF0000]/20 text-[#FF6666]' :
                  'bg-[#FF00FF]/20 text-[#FF00FF]'
                }`}>
                  {configInfo.setupComplexity.toUpperCase()}
                </div>
              </div>
            </div>
          </div>
        </div>

        {/* Table Selector & Novice Mode Toggle */}
        <div className="flex items-center gap-3 mt-3">
          <div className="flex-1 max-w-md">
            <Select value={selectedTableId} onValueChange={setSelectedTableId}>
              <SelectTrigger className="bg-[#2a2a2a] border-[#555555] text-white h-9">
                <SelectValue placeholder="Select a boost table..." />
              </SelectTrigger>
              <SelectContent className="bg-[#2a2a2a] border-[#555555] max-h-[400px]">
                {Array.from(tablesByCategory.entries()).map(([category, tables]) => (
                  <div key={category}>
                    <div className="px-2 py-1.5 text-xs text-[#888888] bg-[#1a1a1a] border-b border-[#444444]">
                      {category}
                    </div>
                    {tables.map(table => (
                      <SelectItem 
                        key={table.id} 
                        value={table.id}
                        className="text-white hover:bg-[#333333] focus:bg-[#0066CC] cursor-pointer pl-6"
                      >
                        <span className="mr-2">{table.icon}</span>
                        {table.name}
                      </SelectItem>
                    ))}
                  </div>
                ))}
              </SelectContent>
            </Select>
          </div>

          <div className="flex items-center gap-2">
            <label className="text-xs text-[#888888]">Novice Mode</label>
            <Switch checked={showNoviceMode} onCheckedChange={setShowNoviceMode} />
          </div>
        </div>
      </div>

      {/* Main Content */}
      <ScrollArea className="flex-1">
        <div className="p-4">
          {/* Selected Table */}
          {selectedTableId && (() => {
            const selectedTable = applicableTables.find(t => t.id === selectedTableId);
            return selectedTable ? renderTable(selectedTable) : null;
          })()}

          {/* Scalar Parameters Section */}
          <div className="mt-6 bg-[#2a2a2a] border border-[#444444] rounded-lg p-4">
            <h3 className="text-white mb-3 flex items-center gap-2">
              <SettingsIcon className="w-4 h-4" />
              Boost Parameters
            </h3>
            <div className="grid grid-cols-2 gap-4">
              {applicableParams.map(param => (
                <div key={param.id} className="bg-[#1a1a1a] border border-[#444444] rounded p-3">
                  <div className="flex items-start justify-between mb-2">
                    <div className="flex items-center gap-2">
                      <span>{param.icon}</span>
                      <label className="text-sm text-white">{param.name}</label>
                      {param.warningIcon && (
                        <AlertTriangle className="w-3 h-3 text-[#FF6600]" />
                      )}
                    </div>
                    <TooltipProvider>
                      <Tooltip>
                        <TooltipTrigger>
                          <Info className="w-3.5 h-3.5 text-[#888888]" />
                        </TooltipTrigger>
                        <TooltipContent className="max-w-xs bg-[#2a2a2a] border-[#555555]">
                          <p className="text-xs text-[#00AAFF] mb-1">
                            <strong>Beginner:</strong> {param.noviceExplanation}
                          </p>
                          <p className="text-xs text-[#CCCCCC]">
                            <strong>Pro:</strong> {param.professionalDetails}
                          </p>
                        </TooltipContent>
                      </Tooltip>
                    </TooltipProvider>
                  </div>

                  {param.type === 'boolean' ? (
                    <Switch 
                      checked={paramValues.get(param.id) || false}
                      onCheckedChange={(checked) => {
                        const newValues = new Map(paramValues);
                        newValues.set(param.id, checked);
                        setParamValues(newValues);
                      }}
                    />
                  ) : param.type === 'number' ? (
                    <div className="flex items-center gap-2">
                      <Input
                        type="number"
                        value={paramValues.get(param.id) || param.default}
                        onChange={(e) => {
                          const newValues = new Map(paramValues);
                          newValues.set(param.id, parseFloat(e.target.value));
                          setParamValues(newValues);
                        }}
                        min={param.min}
                        max={param.max}
                        step="0.1"
                        className="bg-[#2a2a2a] border-[#555555] text-white h-8"
                      />
                      {param.unit && <span className="text-xs text-[#888888]">{param.unit}</span>}
                    </div>
                  ) : null}
                </div>
              ))}
            </div>
          </div>
        </div>
      </ScrollArea>
    </div>
  );
}
