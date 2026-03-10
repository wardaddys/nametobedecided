import { useState } from "react";
import { X, Settings as SettingsIcon, Info, ChevronDown, ChevronRight, Search } from "lucide-react";
import { ScrollArea } from "../ui/scroll-area";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "../ui/tabs";
import { 
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "../ui/select";
import { Input } from "../ui/input";
import { Badge } from "../ui/badge";
import { Separator } from "../ui/separator";
import { useEngineConfig } from "../context/EngineConfigContext";
import { 
  getTechnicalTerm, 
  getTermsByCategory, 
  searchTerms,
  getAllCategories 
} from "../database/TechnicalDatabase";
import { InjectorSettings } from "./InjectorSettings";
import { DeadTimeTable } from "./DeadTimeTable";
import { interpolateDeadTime } from "../database/InjectorDatabase";

interface EnhancedSettingsDropdownProps {
  isOpen: boolean;
  onClose: () => void;
}

export function EnhancedSettingsDropdown({ isOpen, onClose }: EnhancedSettingsDropdownProps) {
  const { 
    config, 
    setEngineType,
    setForcedInductionType,
    setValveTrainType, 
    setFuelSystemType, 
    setControlMode,
    setPrimaryInjector,
    setSecondaryInjector,
    updatePrimaryInjectorConfig,
    updateSecondaryInjectorConfig,
  } = useEngineConfig();
  const [searchQuery, setSearchQuery] = useState("");
  const [expandedSections, setExpandedSections] = useState<Set<string>>(new Set());
  const [selectedTerm, setSelectedTerm] = useState<string | null>(null);

  const toggleSection = (sectionId: string) => {
    const newExpanded = new Set(expandedSections);
    if (newExpanded.has(sectionId)) {
      newExpanded.delete(sectionId);
    } else {
      newExpanded.add(sectionId);
    }
    setExpandedSections(newExpanded);
  };

  const searchResults = searchQuery.length > 2 ? searchTerms(searchQuery) : [];

  if (!isOpen) return null;

  return (
    <>
      {/* Backdrop */}
      <div 
        className="fixed inset-0 bg-black/50 z-40"
        onClick={onClose}
      />

      {/* Dropdown Panel */}
      <div className="fixed top-12 right-3 w-[1000px] h-[calc(100vh-80px)] bg-[#1a1a1a] border-2 border-[#444444] z-50 flex flex-col shadow-2xl">
        {/* Header */}
        <div className="bg-[#2a2a2a] border-b border-[#444444] px-4 py-3 flex items-center justify-between">
          <div className="flex items-center gap-3">
            <SettingsIcon className="w-5 h-5 text-[#00CCFF]" />
            <h2 className="text-white">Engine Configuration & Technical Database</h2>
          </div>
          <button
            onClick={onClose}
            className="text-[#CCCCCC] hover:text-white transition-colors"
          >
            <X className="w-5 h-5" />
          </button>
        </div>

        {/* Content */}
        <Tabs defaultValue="config" className="flex-1 flex flex-col overflow-hidden gap-0">
          <TabsList className="bg-[#2a2a2a] border-b border-[#444444] rounded-none justify-start px-4 shrink-0">
            <TabsTrigger value="config" className="data-[state=active]:bg-[#0066CC]">
              Engine Configuration
            </TabsTrigger>
            <TabsTrigger value="fuel" className="data-[state=active]:bg-[#0066CC]">
              Fuel & Injectors
            </TabsTrigger>
            <TabsTrigger value="database" className="data-[state=active]:bg-[#0066CC]">
              Technical Database
            </TabsTrigger>
            <TabsTrigger value="control" className="data-[state=active]:bg-[#0066CC]">
              Control Strategies
            </TabsTrigger>
          </TabsList>

          {/* Configuration Tab */}
          <TabsContent value="config" className="flex-1 m-0 overflow-hidden">
            <ScrollArea className="h-full w-full">
              <div className="p-6 space-y-6 pb-12">
                {/* Engine Type Selection */}
                <div className="bg-[#2a2a2a] border border-[#444444] rounded-lg p-4">
                  <div className="flex items-center justify-between mb-4">
                    <h3 className="text-white flex items-center gap-2">
                      ⚙️ Engine Type
                      <button
                        onClick={() => setSelectedTerm('ENG-001-' + config.engineType.toUpperCase())}
                        className="text-[#00CCFF] hover:text-white"
                      >
                        <Info className="w-4 h-4" />
                      </button>
                    </h3>
                    {config.features.hasBoostControl && (
                      <Badge className="bg-[#00FF00] text-black">Boost Control Active</Badge>
                    )}
                  </div>
                  
                  <Select 
                    value={config.engineType} 
                    onValueChange={(value: any) => setEngineType(value)}
                  >
                    <SelectTrigger className="w-full bg-[#1a1a1a] border-[#444444]">
                      <SelectValue />
                    </SelectTrigger>
                    <SelectContent>
                      <SelectItem value="na">Naturally Aspirated (NA)</SelectItem>
                      <SelectItem value="turbocharged">Turbocharged</SelectItem>
                      <SelectItem value="supercharged">Supercharged</SelectItem>
                      <SelectItem value="twin-turbo">Twin-Turbo</SelectItem>
                      <SelectItem value="sequential-turbo">Sequential Turbo</SelectItem>
                    </SelectContent>
                  </Select>

                  <div className="mt-3 text-xs text-[#888888]">
                    {config.engineType === 'na' && (
                      <p>✓ VE Tables only • No boost control required</p>
                    )}
                    {config.engineType !== 'na' && (
                      <p>✓ Boost Control enabled • Additional tables required</p>
                    )}
                  </div>
                </div>

                {/* Forced Induction Type - Only shown if boosted */}
                {config.engineType !== 'na' && (
                  <div className="bg-[#2a2a2a] border border-[#444444] rounded-lg p-4">
                    <div className="flex items-center justify-between mb-4">
                      <h3 className="text-white flex items-center gap-2">
                        🚀 Forced Induction Configuration
                      </h3>
                      <Badge className="bg-[#00CCFF] text-black">Advanced Setup</Badge>
                    </div>
                    
                    <Select 
                      value={config.forcedInductionType || 'single-turbo'} 
                      onValueChange={(value: any) => setForcedInductionType(value)}
                    >
                      <SelectTrigger className="w-full bg-[#1a1a1a] border-[#444444]">
                        <SelectValue />
                      </SelectTrigger>
                      <SelectContent>
                        <SelectItem value="single-turbo">Single Turbo</SelectItem>
                        <SelectItem value="parallel-twin">Parallel Twin Turbo</SelectItem>
                        <SelectItem value="sequential-twin">Sequential Twin Turbo</SelectItem>
                        <SelectItem value="supercharger">Supercharger</SelectItem>
                        <SelectItem value="twin-charger">Twin-Charger (Compound)</SelectItem>
                        <SelectItem value="staged-boost">Staged Boost</SelectItem>
                      </SelectContent>
                    </Select>

                    <div className="mt-3 p-3 bg-[#1a1a1a] border border-[#00CCFF] rounded">
                      <p className="text-xs text-[#00CCFF] mb-1">
                        {config.forcedInductionType === 'single-turbo' && '✓ One big fan pushing air—like a single strong push'}
                        {config.forcedInductionType === 'parallel-twin' && '✓ Two fans side-by-side—like teamwork for quick start'}
                        {config.forcedInductionType === 'sequential-twin' && '⚠️ Small fan for quick start, big for power—like gears shifting'}
                        {config.forcedInductionType === 'supercharger' && '✓ Belt-driven fan—no exhaust wait, instant push'}
                        {config.forcedInductionType === 'twin-charger' && '⚠️ Super for low-end kick, turbo for high—expert setup'}
                        {config.forcedInductionType === 'staged-boost' && '⚠️ Multi-level push—per-stage controls'}
                      </p>
                      <p className="text-xs text-[#888888]">
                        Visit the Boost tab to configure detailed boost control parameters and tables.
                      </p>
                    </div>
                  </div>
                )}

                {/* Valve Train Configuration */}
                <div className="bg-[#2a2a2a] border border-[#444444] rounded-lg p-4">
                  <div className="flex items-center justify-between mb-4">
                    <h3 className="text-white flex items-center gap-2">
                      🔧 Valve Train System
                      <button
                        onClick={() => setSelectedTerm('VLV-001-' + config.valveTrainType.toUpperCase())}
                        className="text-[#00CCFF] hover:text-white"
                      >
                        <Info className="w-4 h-4" />
                      </button>
                    </h3>
                    {config.features.requiresDualMaps && (
                      <Badge className="bg-[#FF00FF] text-white">Dual Maps Required</Badge>
                    )}
                  </div>
                  
                  <Select 
                    value={config.valveTrainType} 
                    onValueChange={(value: any) => setValveTrainType(value)}
                  >
                    <SelectTrigger className="w-full bg-[#1a1a1a] border-[#444444]">
                      <SelectValue />
                    </SelectTrigger>
                    <SelectContent>
                      <SelectItem value="standard">Standard (Single Profile)</SelectItem>
                      <SelectItem value="vtec">VTEC (Dual Profile) - Honda</SelectItem>
                      <SelectItem value="vvt">VVT (Variable Timing)</SelectItem>
                      <SelectItem value="vvti">VVT-i (Intelligent VVT) - Toyota</SelectItem>
                      <SelectItem value="vanos">VANOS - BMW</SelectItem>
                      <SelectItem value="variocam">VarioCam - Porsche</SelectItem>
                    </SelectContent>
                  </Select>

                  {config.features.requiresDualMaps && (
                    <div className="mt-3 p-3 bg-[#1a1a1a] border border-[#FF00FF] rounded">
                      <p className="text-xs text-[#FF00FF] font-semibold mb-2">⚠️ VTEC Configuration Active</p>
                      <p className="text-xs text-[#CCCCCC]">
                        You must configure separate tables for Low-Speed and High-Speed cam profiles.
                        All fuel and ignition tables will be duplicated.
                      </p>
                    </div>
                  )}
                </div>

                {/* Fuel System Configuration */}
                <div className="bg-[#2a2a2a] border border-[#444444] rounded-lg p-4">
                  <div className="flex items-center justify-between mb-4">
                    <h3 className="text-white flex items-center gap-2">
                      ⛽ Fuel System
                      <button
                        onClick={() => setSelectedTerm('FUEL-001-PORT-' + (config.fuelSystemType === 'port-staged' ? 'STAGED' : 'SINGLE'))}
                        className="text-[#00CCFF] hover:text-white"
                      >
                        <Info className="w-4 h-4" />
                      </button>
                    </h3>
                    {config.features.hasStagedInjection && (
                      <Badge className="bg-[#FFD700] text-black">Staged Injection</Badge>
                    )}
                  </div>
                  
                  <Select 
                    value={config.fuelSystemType} 
                    onValueChange={(value: any) => setFuelSystemType(value)}
                  >
                    <SelectTrigger className="w-full bg-[#1a1a1a] border-[#444444]">
                      <SelectValue />
                    </SelectTrigger>
                    <SelectContent>
                      <SelectItem value="port-single">Port Injection (Single Stage)</SelectItem>
                      <SelectItem value="port-staged">Port Injection (Staged)</SelectItem>
                      <SelectItem value="direct-injection">Direct Injection (DI)</SelectItem>
                      <SelectItem value="dual-injection">Dual Injection (Port + DI)</SelectItem>
                    </SelectContent>
                  </Select>
                </div>

                {/* Control Mode */}
                <div className="bg-[#2a2a2a] border border-[#444444] rounded-lg p-4">
                  <div className="flex items-center justify-between mb-4">
                    <h3 className="text-white flex items-center gap-2">
                      🎛️ Fuel Control Mode
                      <button
                        onClick={() => setSelectedTerm('CTL-001-' + config.controlMode.toUpperCase().replace('-', ''))}
                        className="text-[#00CCFF] hover:text-white"
                      >
                        <Info className="w-4 h-4" />
                      </button>
                    </h3>
                  </div>
                  
                  <Select 
                    value={config.controlMode} 
                    onValueChange={(value: any) => setControlMode(value)}
                  >
                    <SelectTrigger className="w-full bg-[#1a1a1a] border-[#444444]">
                      <SelectValue />
                    </SelectTrigger>
                    <SelectContent>
                      <SelectItem value="closed-loop">Closed Loop (O2 Feedback)</SelectItem>
                      <SelectItem value="open-loop">Open Loop (Table Only)</SelectItem>
                    </SelectContent>
                  </Select>

                  <div className="mt-3 text-xs text-[#888888]">
                    {config.controlMode === 'closed-loop' ? (
                      <p>✓ Automatic AFR correction using wideband O2 sensor</p>
                    ) : (
                      <p>✓ No feedback correction - requires precise VE table tuning</p>
                    )}
                  </div>
                </div>

                {/* Feature Summary */}
                <div className="bg-[#1a1a1a] border-2 border-[#00CCFF] rounded-lg p-4">
                  <h3 className="text-[#00CCFF] mb-3">Active Features Summary</h3>
                  <div className="grid grid-cols-2 gap-2 text-xs">
                    <div className="flex items-center gap-2">
                      <div className={`w-2 h-2 rounded-full ${config.features.hasBoostControl ? 'bg-[#00FF00]' : 'bg-[#666666]'}`} />
                      <span className="text-[#CCCCCC]">Boost Control</span>
                    </div>
                    <div className="flex items-center gap-2">
                      <div className={`w-2 h-2 rounded-full ${config.features.hasVTECControl ? 'bg-[#00FF00]' : 'bg-[#666666]'}`} />
                      <span className="text-[#CCCCCC]">VTEC Control</span>
                    </div>
                    <div className="flex items-center gap-2">
                      <div className={`w-2 h-2 rounded-full ${config.features.hasStagedInjection ? 'bg-[#00FF00]' : 'bg-[#666666]'}`} />
                      <span className="text-[#CCCCCC]">Staged Injection</span>
                    </div>
                    <div className="flex items-center gap-2">
                      <div className={`w-2 h-2 rounded-full ${config.features.hasVVTControl ? 'bg-[#00FF00]' : 'bg-[#666666]'}`} />
                      <span className="text-[#CCCCCC]">VVT Control</span>
                    </div>
                    <div className="flex items-center gap-2">
                      <div className={`w-2 h-2 rounded-full ${config.features.requiresDualMaps ? 'bg-[#00FF00]' : 'bg-[#666666]'}`} />
                      <span className="text-[#CCCCCC]">Dual Map Mode</span>
                    </div>
                    <div className="flex items-center gap-2">
                      <div className={`w-2 h-2 rounded-full ${config.controlMode === 'closed-loop' ? 'bg-[#00FF00]' : 'bg-[#666666]'}`} />
                      <span className="text-[#CCCCCC]">Closed Loop</span>
                    </div>
                  </div>
                </div>
              </div>
            </ScrollArea>
          </TabsContent>

          {/* Fuel & Injectors Tab */}
          <TabsContent value="fuel" className="flex-1 m-0 overflow-hidden">
            <ScrollArea className="h-full w-full">
              <div className="p-6 space-y-8 pb-12">
                {/* Primary Injector Section */}
                <div className="space-y-4">
                  <div className="flex items-center gap-2 mb-4">
                    <h2 className="text-[#00CCFF] text-lg">Primary Injector Configuration</h2>
                  </div>
                  
                  <InjectorSettings
                    isPrimary={true}
                    selectedInjector={config.primaryInjector.injector}
                    onInjectorChange={setPrimaryInjector}
                    customFlowRate={config.primaryInjector.customFlowRate}
                    customPressure={config.primaryInjector.customPressure}
                    onCustomFlowChange={(flow) => updatePrimaryInjectorConfig({ customFlowRate: flow })}
                    onCustomPressureChange={(pressure) => updatePrimaryInjectorConfig({ customPressure: pressure })}
                  />

                  <Separator className="bg-[#444444] my-4" />

                  <div className="bg-[#2a2a2a] border border-[#0066CC] rounded-lg p-4">
                    <h3 className="text-[#00CCFF] mb-4">Primary Injector Dead Time Calibration</h3>
                    <DeadTimeTable
                      data={config.primaryInjector.deadTimeData}
                      onChange={(data) => updatePrimaryInjectorConfig({ deadTimeData: data })}
                      injector={config.primaryInjector.injector || undefined}
                      onLoadFromInjector={() => {
                        if (config.primaryInjector.injector) {
                          const inj = config.primaryInjector.injector;
                          const newData = {
                            ...config.primaryInjector.deadTimeData,
                            latencies: config.primaryInjector.deadTimeData.voltages.map((voltage) =>
                              config.primaryInjector.deadTimeData.pulseWidths.map(() =>
                                interpolateDeadTime(inj, voltage)
                              )
                            ),
                          };
                          updatePrimaryInjectorConfig({ deadTimeData: newData });
                        }
                      }}
                    />
                  </div>
                </div>

                {/* Secondary Injector Section (Staged Only) */}
                {config.features.hasStagedInjection && config.secondaryInjector && (
                  <>
                    <Separator className="bg-[#666666] my-8" />
                    
                    <div className="space-y-4">
                      <div className="flex items-center gap-2 mb-4">
                        <h2 className="text-[#FF00FF] text-lg">Secondary Injector Configuration</h2>
                        <Badge className="bg-[#9D00FF] text-white">Staged Injection</Badge>
                      </div>
                      
                      <InjectorSettings
                        isPrimary={false}
                        selectedInjector={config.secondaryInjector.injector}
                        onInjectorChange={setSecondaryInjector}
                        customFlowRate={config.secondaryInjector.customFlowRate}
                        customPressure={config.secondaryInjector.customPressure}
                        onCustomFlowChange={(flow) => updateSecondaryInjectorConfig({ customFlowRate: flow })}
                        onCustomPressureChange={(pressure) => updateSecondaryInjectorConfig({ customPressure: pressure })}
                      />

                      <Separator className="bg-[#444444] my-4" />

                      <div className="bg-[#2a2a2a] border border-[#9D00FF] rounded-lg p-4">
                        <h3 className="text-[#FF00FF] mb-4">Secondary Injector Dead Time Calibration</h3>
                        <DeadTimeTable
                          data={config.secondaryInjector.deadTimeData}
                          onChange={(data) => updateSecondaryInjectorConfig({ deadTimeData: data })}
                          injector={config.secondaryInjector.injector || undefined}
                          onLoadFromInjector={() => {
                            if (config.secondaryInjector?.injector) {
                              const inj = config.secondaryInjector.injector;
                              const newData = {
                                ...config.secondaryInjector.deadTimeData,
                                latencies: config.secondaryInjector.deadTimeData.voltages.map((voltage) =>
                                  config.secondaryInjector.deadTimeData.pulseWidths.map(() =>
                                    interpolateDeadTime(inj, voltage)
                                  )
                                ),
                              };
                              updateSecondaryInjectorConfig({ deadTimeData: newData });
                            }
                          }}
                        />
                      </div>
                    </div>
                  </>
                )}
              </div>
            </ScrollArea>
          </TabsContent>

          {/* Technical Database Tab */}
          <TabsContent value="database" className="flex-1 m-0 overflow-hidden">
            <div className="h-full flex flex-col">
              {/* Search Bar */}
              <div className="p-4 bg-[#2a2a2a] border-b border-[#444444]">
                <div className="relative">
                  <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 w-4 h-4 text-[#888888]" />
                  <Input
                    type="text"
                    placeholder="Search technical terms..."
                    value={searchQuery}
                    onChange={(e) => setSearchQuery(e.target.value)}
                    className="pl-10 bg-[#1a1a1a] border-[#444444]"
                  />
                </div>
                {searchResults.length > 0 && (
                  <div className="mt-2 text-xs text-[#00CCFF]">
                    Found {searchResults.length} results
                  </div>
                )}
              </div>

              <ScrollArea className="flex-1 w-full">
                <div className="p-6 space-y-4 pb-12">
                  {searchQuery.length > 2 ? (
                    // Show search results
                    searchResults.map(term => (
                      <div
                        key={term.id}
                        className="bg-[#2a2a2a] border border-[#444444] rounded-lg p-4 cursor-pointer hover:border-[#00CCFF] transition-colors"
                        onClick={() => setSelectedTerm(term.id)}
                      >
                        <div className="flex items-start justify-between mb-2">
                          <h4 className="text-white">{term.title}</h4>
                          <Badge className="bg-[#0066CC]">{term.category}</Badge>
                        </div>
                        <p className="text-sm text-[#CCCCCC]">{term.shortDescription}</p>
                      </div>
                    ))
                  ) : (
                    // Show categories
                    getAllCategories().map(category => {
                      const terms = getTermsByCategory(category);
                      return (
                        <div key={category} className="mb-4">
                          <button
                            onClick={() => toggleSection(category)}
                            className="w-full flex items-center justify-between bg-[#2a2a2a] hover:bg-[#333333] border border-[#444444] px-4 py-3 rounded-lg transition-colors"
                          >
                            <span className="text-[#00CCFF]">{category}</span>
                            <div className="flex items-center gap-2">
                              <Badge className="bg-[#666666]">{terms.length}</Badge>
                              {expandedSections.has(category) ? (
                                <ChevronDown className="w-4 h-4 text-[#00CCFF]" />
                              ) : (
                                <ChevronRight className="w-4 h-4 text-[#00CCFF]" />
                              )}
                            </div>
                          </button>

                          {expandedSections.has(category) && (
                            <div className="mt-2 ml-4 space-y-2">
                              {terms.map(term => (
                                <div
                                  key={term.id}
                                  className="bg-[#1a1a1a] border border-[#444444] rounded p-3 cursor-pointer hover:border-[#00CCFF] transition-colors"
                                  onClick={() => setSelectedTerm(term.id)}
                                >
                                  <div className="flex items-center justify-between mb-1">
                                    <span className="text-sm text-white">{term.title}</span>
                                    <Info className="w-3 h-3 text-[#00CCFF]" />
                                  </div>
                                  <p className="text-xs text-[#888888]">{term.shortDescription}</p>
                                </div>
                              ))}
                            </div>
                          )}
                        </div>
                      );
                    })
                  )}
                </div>
              </ScrollArea>
            </div>
          </TabsContent>

          {/* Control Strategies Tab */}
          <TabsContent value="control" className="flex-1 m-0 overflow-hidden">
            <ScrollArea className="h-full w-full">
              <div className="p-6 space-y-6 pb-12">
                {/* Closed Loop vs Open Loop */}
                <div className="grid grid-cols-2 gap-4">
                  <div className="bg-[#2a2a2a] border-2 border-[#00FF00] rounded-lg p-4">
                    <h3 className="text-[#00FF00] mb-3 flex items-center justify-between">
                      Closed Loop Control
                      <button onClick={() => setSelectedTerm('CTL-001-CLOSED')}>
                        <Info className="w-4 h-4" />
                      </button>
                    </h3>
                    <p className="text-xs text-[#CCCCCC] mb-3">
                      Feedback control using O2 sensor for automatic AFR correction
                    </p>
                    <div className="space-y-2 text-xs">
                      <div>
                        <span className="text-[#888888]">When Active:</span>
                        <ul className="ml-4 mt-1 text-[#CCCCCC] space-y-1">
                          <li>• Engine at operating temp</li>
                          <li>• TPS {'<'} 70%</li>
                          <li>• RPM 800-4000</li>
                          <li>• Steady-state operation</li>
                        </ul>
                      </div>
                      <Separator className="bg-[#444444]" />
                      <div>
                        <span className="text-[#888888]">Benefits:</span>
                        <ul className="ml-4 mt-1 text-[#00FF00] space-y-1">
                          <li>✓ Self-correcting</li>
                          <li>✓ Adapts to conditions</li>
                          <li>✓ Precise AFR control</li>
                        </ul>
                      </div>
                    </div>
                  </div>

                  <div className="bg-[#2a2a2a] border-2 border-[#FFD700] rounded-lg p-4">
                    <h3 className="text-[#FFD700] mb-3 flex items-center justify-between">
                      Open Loop Control
                      <button onClick={() => setSelectedTerm('CTL-001-OPEN')}>
                        <Info className="w-4 h-4" />
                      </button>
                    </h3>
                    <p className="text-xs text-[#CCCCCC] mb-3">
                      Table-based control without feedback - requires precise tuning
                    </p>
                    <div className="space-y-2 text-xs">
                      <div>
                        <span className="text-[#888888]">When Active:</span>
                        <ul className="ml-4 mt-1 text-[#CCCCCC] space-y-1">
                          <li>• Engine cold</li>
                          <li>• TPS {'>'}70% (acceleration)</li>
                          <li>• WOT conditions</li>
                          <li>• Rapid transients</li>
                        </ul>
                      </div>
                      <Separator className="bg-[#444444]" />
                      <div>
                        <span className="text-[#888888]">Benefits:</span>
                        <ul className="ml-4 mt-1 text-[#FFD700] space-y-1">
                          <li>✓ Immediate response</li>
                          <li>✓ No sensor delay</li>
                          <li>✓ Predictable behavior</li>
                        </ul>
                      </div>
                    </div>
                  </div>
                </div>

                {/* PID Control Explanation */}
                <div className="bg-[#2a2a2a] border border-[#444444] rounded-lg p-4">
                  <h3 className="text-white mb-3">PID Control Algorithm</h3>
                  <div className="space-y-3 text-xs text-[#CCCCCC]">
                    <div className="bg-[#1a1a1a] p-3 rounded">
                      <span className="text-[#00CCFF]">P (Proportional):</span> Immediate correction proportional to error size
                      <div className="mt-1 text-[#888888]">Typical: 0.3-0.5</div>
                    </div>
                    <div className="bg-[#1a1a1a] p-3 rounded">
                      <span className="text-[#00CCFF]">I (Integral):</span> Accumulated error over time, eliminates steady-state error
                      <div className="mt-1 text-[#888888]">Typical: 0.05-0.1</div>
                    </div>
                    <div className="bg-[#1a1a1a] p-3 rounded">
                      <span className="text-[#00CCFF]">D (Derivative):</span> Rate of error change, dampens oscillation
                      <div className="mt-1 text-[#888888]">Typical: 0.01-0.05</div>
                    </div>
                  </div>
                </div>
              </div>
            </ScrollArea>
          </TabsContent>
        </Tabs>

        {/* Term Detail Modal */}
        {selectedTerm && (
          <div className="absolute inset-0 bg-black/80 flex items-center justify-center p-8 z-50">
            <div className="bg-[#1a1a1a] border-2 border-[#00CCFF] rounded-lg max-w-3xl max-h-full overflow-hidden flex flex-col">
              <div className="bg-[#2a2a2a] border-b border-[#444444] px-6 py-4 flex items-center justify-between">
                <h2 className="text-[#00CCFF]">{getTechnicalTerm(selectedTerm)?.title}</h2>
                <button onClick={() => setSelectedTerm(null)} className="text-[#CCCCCC] hover:text-white">
                  <X className="w-5 h-5" />
                </button>
              </div>
              <ScrollArea className="flex-1 p-6">
                <TermDetail termId={selectedTerm} />
              </ScrollArea>
            </div>
          </div>
        )}
      </div>
    </>
  );
}

// Term Detail Component
function TermDetail({ termId }: { termId: string }) {
  const term = getTechnicalTerm(termId);
  if (!term) return <div>Term not found</div>;

  return (
    <div className="space-y-4 text-[#CCCCCC]">
      <div>
        <Badge className="bg-[#0066CC] mb-2">{term.category}</Badge>
        <p className="text-sm">{term.fullDescription}</p>
      </div>

      {term.characteristics && term.characteristics.length > 0 && (
        <div>
          <h4 className="text-white mb-2">Characteristics:</h4>
          <ul className="space-y-1 ml-4 text-sm">
            {term.characteristics.map((char, i) => (
              <li key={i}>• {char}</li>
            ))}
          </ul>
        </div>
      )}

      {term.tuningFocus && term.tuningFocus.length > 0 && (
        <div>
          <h4 className="text-[#00FF00] mb-2">Tuning Focus:</h4>
          <ul className="space-y-1 ml-4 text-sm">
            {term.tuningFocus.map((focus, i) => (
              <li key={i} className="text-[#00FF00]">✓ {focus}</li>
            ))}
          </ul>
        </div>
      )}

      {term.tablesRequired && term.tablesRequired.length > 0 && (
        <div>
          <h4 className="text-[#FFD700] mb-2">Tables Required:</h4>
          <ul className="space-y-1 ml-4 text-sm">
            {term.tablesRequired.map((table, i) => (
              <li key={i} className="text-[#FFD700]">▶ {table}</li>
            ))}
          </ul>
        </div>
      )}

      {term.advantages && term.advantages.length > 0 && (
        <div>
          <h4 className="text-[#00FF00] mb-2">Advantages:</h4>
          <ul className="space-y-1 ml-4 text-sm">
            {term.advantages.map((adv, i) => (
              <li key={i} className="text-[#00FF00]">+ {adv}</li>
            ))}
          </ul>
        </div>
      )}

      {term.disadvantages && term.disadvantages.length > 0 && (
        <div>
          <h4 className="text-[#FF3B30] mb-2">Disadvantages:</h4>
          <ul className="space-y-1 ml-4 text-sm">
            {term.disadvantages.map((dis, i) => (
              <li key={i} className="text-[#FF3B30]">- {dis}</li>
            ))}
          </ul>
        </div>
      )}

      {term.typicalValues && Object.keys(term.typicalValues).length > 0 && (
        <div>
          <h4 className="text-white mb-2">Typical Values:</h4>
          <div className="bg-[#2a2a2a] border border-[#444444] rounded p-3 space-y-1 text-sm">
            {Object.entries(term.typicalValues).map(([key, value]) => (
              <div key={key} className="flex justify-between">
                <span className="text-[#888888]">{key}:</span>
                <span className="text-[#00CCFF]">{value}</span>
              </div>
            ))}
          </div>
        </div>
      )}
    </div>
  );
}
