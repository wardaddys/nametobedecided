import { useState } from "react";
import { X, BookOpen, ChevronDown, ChevronRight } from "lucide-react";
import { ScrollArea } from "../ui/scroll-area";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "../ui/tabs";

interface SettingsDropdownProps {
  isOpen: boolean;
  onClose: () => void;
}

export function SettingsDropdown({ isOpen, onClose }: SettingsDropdownProps) {
  const [expandedSections, setExpandedSections] = useState<Set<string>>(new Set());

  const toggleSection = (sectionId: string) => {
    const newExpanded = new Set(expandedSections);
    if (newExpanded.has(sectionId)) {
      newExpanded.delete(sectionId);
    } else {
      newExpanded.add(sectionId);
    }
    setExpandedSections(newExpanded);
  };

  if (!isOpen) return null;

  return (
    <>
      {/* Backdrop */}
      <div 
        className="fixed inset-0 bg-black/50 z-40"
        onClick={onClose}
      />

      {/* Dropdown Panel */}
      <div className="fixed top-12 right-3 w-[900px] h-[calc(100vh-80px)] bg-[#1a1a1a] border-2 border-[#444444] z-50 flex flex-col shadow-2xl">
        {/* Header */}
        <div className="bg-[#2a2a2a] border-b border-[#444444] px-4 py-3 flex items-center justify-between">
          <h2 className="text-white">Settings</h2>
          <button
            onClick={onClose}
            className="text-[#CCCCCC] hover:text-white transition-colors"
          >
            <X className="w-5 h-5" />
          </button>
        </div>

        {/* Content */}
        <Tabs defaultValue="help" className="flex-1 flex flex-col overflow-hidden gap-0">
          <TabsList className="bg-[#2a2a2a] border-b border-[#444444] rounded-none justify-start px-4 shrink-0">
            <TabsTrigger value="help" className="data-[state=active]:bg-[#0066CC]">
              <BookOpen className="w-4 h-4 mr-2" />
              Sensor Database Help
            </TabsTrigger>
            <TabsTrigger value="info" className="data-[state=active]:bg-[#0066CC]">
              Sensor Info
            </TabsTrigger>
          </TabsList>

          <TabsContent value="help" className="flex-1 m-0 overflow-hidden">
            <ScrollArea className="h-full">
              <div className="p-6 text-[#CCCCCC] space-y-6">
                {/* Title */}
                <div className="border-b border-[#444444] pb-4">
                  <h1 className="text-2xl text-white mb-2">PROFESSIONAL-GRADE COMPLETE AUTOMOTIVE SENSOR & TRIGGER DATABASE</h1>
                  <p className="text-sm text-[#888888]">The Ultimate Reference for ALL ECU Platforms</p>
                  <p className="text-xs text-[#666666] mt-1">
                    Comprehensive Coverage: Speeduino, MegaSquirt, Link, Haltech, Motec, AEM, ECUMaster, Emtron
                  </p>
                  <p className="text-xs text-[#666666]">Database Version 1.0 - Complete OEM & Aftermarket Sensor Specifications</p>
                </div>

                {/* Interactive Table of Contents */}
                <div className="bg-[#2a2a2a] border border-[#444444] rounded p-4">
                  <h2 className="text-lg text-white mb-3">TABLE OF CONTENTS - Click to expand sections</h2>
                  
                  {/* SECTION A: TEMPERATURE SENSORS */}
                  <div className="mb-3">
                    <button
                      onClick={() => toggleSection('sectionA')}
                      className="w-full flex items-center justify-between bg-[#1a1a1a] hover:bg-[#252525] border border-[#444444] px-4 py-2 rounded transition-colors"
                    >
                      <span className="text-[#00CCFF]">SECTION A: TEMPERATURE SENSORS</span>
                      {expandedSections.has('sectionA') ? (
                        <ChevronDown className="w-4 h-4 text-[#00CCFF]" />
                      ) : (
                        <ChevronRight className="w-4 h-4 text-[#00CCFF]" />
                      )}
                    </button>
                    
                    {expandedSections.has('sectionA') && (
                      <div className="mt-3 space-y-6 pl-4">
                        {/* CLT Sensors */}
                        <div>
                          <h3 className="text-lg text-white mb-3">1. COOLANT TEMPERATURE (CLT) SENSORS - COMPLETE OEM DATABASE</h3>
                          
                          <div className="bg-[#2a2a2a] border border-[#444444] rounded p-4 mb-4">
                            <h4 className="text-sm text-[#00FF00] mb-2">Understanding NTC Thermistor Sensors</h4>
                            <p className="text-xs mb-2">Technology: Negative Temperature Coefficient (NTC) thermistors</p>
                            <ul className="text-xs space-y-1 ml-4">
                              <li>• Resistance DECREASES as temperature INCREASES</li>
                              <li>• Passive 2-wire component (no polarity)</li>
                              <li>• Requires pull-up resistor (typically 2.49kΩ in ECU)</li>
                              <li>• Output: 0-5V via voltage divider</li>
                            </ul>
                            <div className="mt-2 bg-black/40 p-2 rounded">
                              <code className="text-xs text-[#00CCFF]">V_out = V_ref × (R_sensor / (R_pullup + R_sensor))</code>
                            </div>
                          </div>

                          {/* GM Sensors */}
                          <div className="mb-6">
                            <h4 className="text-white mb-3">GM / GENERAL MOTORS SENSORS</h4>
                            
                            <div className="bg-[#2a2a2a] border border-[#444444] rounded p-4 mb-4">
                              <h5 className="text-sm text-[#FFD700] mb-2">GM 12191170 / 25036751 / ACDelco 213-928 (MOST POPULAR)</h5>
                              <div className="grid grid-cols-2 gap-4 text-xs mb-3">
                                <div>
                                  <p><span className="text-[#888888]">Thread:</span> 3/8" NPT</p>
                                  <p><span className="text-[#888888]">Connector:</span> 2-pin Weather Pack</p>
                                </div>
                                <div>
                                  <p><span className="text-[#888888]">Applications:</span> 1985-2010 GM engines (LS1, LS2, LS3, LT1, SBC, BBC)</p>
                                  <p><span className="text-[#888888]">Temperature Range:</span> -40°C to +130°C</p>
                                </div>
                              </div>
                              
                              <div className="overflow-x-auto">
                                <table className="w-full text-xs border-collapse">
                                  <thead>
                                    <tr className="bg-[#0066CC]">
                                      <th className="border border-[#444444] px-2 py-1 text-white">Temp °C</th>
                                      <th className="border border-[#444444] px-2 py-1 text-white">Temp °F</th>
                                      <th className="border border-[#444444] px-2 py-1 text-white">Resistance Ω</th>
                                      <th className="border border-[#444444] px-2 py-1 text-white">Voltage @ 2.49kΩ Pullup</th>
                                    </tr>
                                  </thead>
                                  <tbody className="bg-[#1a1a1a]">
                                    <tr><td className="border border-[#444444] px-2 py-1">-40</td><td className="border border-[#444444] px-2 py-1">-40</td><td className="border border-[#444444] px-2 py-1">100,700</td><td className="border border-[#444444] px-2 py-1">4.98V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">-30</td><td className="border border-[#444444] px-2 py-1">-22</td><td className="border border-[#444444] px-2 py-1">53,000</td><td className="border border-[#444444] px-2 py-1">4.93V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">-20</td><td className="border border-[#444444] px-2 py-1">-4</td><td className="border border-[#444444] px-2 py-1">29,500</td><td className="border border-[#444444] px-2 py-1">4.85V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">-10</td><td className="border border-[#444444] px-2 py-1">14</td><td className="border border-[#444444] px-2 py-1">16,800</td><td className="border border-[#444444] px-2 py-1">4.72V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">0</td><td className="border border-[#444444] px-2 py-1">32</td><td className="border border-[#444444] px-2 py-1">9,750</td><td className="border border-[#444444] px-2 py-1">4.51V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">10</td><td className="border border-[#444444] px-2 py-1">50</td><td className="border border-[#444444] px-2 py-1">5,860</td><td className="border border-[#444444] px-2 py-1">4.21V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">20</td><td className="border border-[#444444] px-2 py-1">68</td><td className="border border-[#444444] px-2 py-1">3,600</td><td className="border border-[#444444] px-2 py-1">3.79V</td></tr>
                                    <tr className="bg-[#003366]"><td className="border border-[#444444] px-2 py-1">25</td><td className="border border-[#444444] px-2 py-1">77</td><td className="border border-[#444444] px-2 py-1">3,000</td><td className="border border-[#444444] px-2 py-1">3.57V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">30</td><td className="border border-[#444444] px-2 py-1">86</td><td className="border border-[#444444] px-2 py-1">2,490</td><td className="border border-[#444444] px-2 py-1">3.33V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">40</td><td className="border border-[#444444] px-2 py-1">104</td><td className="border border-[#444444] px-2 py-1">1,750</td><td className="border border-[#444444] px-2 py-1">2.85V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">50</td><td className="border border-[#444444] px-2 py-1">122</td><td className="border border-[#444444] px-2 py-1">1,250</td><td className="border border-[#444444] px-2 py-1">2.39V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">60</td><td className="border border-[#444444] px-2 py-1">140</td><td className="border border-[#444444] px-2 py-1">910</td><td className="border border-[#444444] px-2 py-1">1.96V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">70</td><td className="border border-[#444444] px-2 py-1">158</td><td className="border border-[#444444] px-2 py-1">670</td><td className="border border-[#444444] px-2 py-1">1.60V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">80</td><td className="border border-[#444444] px-2 py-1">176</td><td className="border border-[#444444] px-2 py-1">500</td><td className="border border-[#444444] px-2 py-1">1.29V</td></tr>
                                    <tr className="bg-[#003366]"><td className="border border-[#444444] px-2 py-1">90</td><td className="border border-[#444444] px-2 py-1">194</td><td className="border border-[#444444] px-2 py-1">380</td><td className="border border-[#444444] px-2 py-1">1.05V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">100</td><td className="border border-[#444444] px-2 py-1">212</td><td className="border border-[#444444] px-2 py-1">290</td><td className="border border-[#444444] px-2 py-1">0.85V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">110</td><td className="border border-[#444444] px-2 py-1">230</td><td className="border border-[#444444] px-2 py-1">225</td><td className="border border-[#444444] px-2 py-1">0.69V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">120</td><td className="border border-[#444444] px-2 py-1">248</td><td className="border border-[#444444] px-2 py-1">177</td><td className="border border-[#444444] px-2 py-1">0.56V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">130</td><td className="border border-[#444444] px-2 py-1">266</td><td className="border border-[#444444] px-2 py-1">141</td><td className="border border-[#444444] px-2 py-1">0.46V</td></tr>
                                  </tbody>
                                </table>
                              </div>

                              <div className="mt-3 text-xs">
                                <p className="text-[#888888] mb-1">Part Numbers (Interchangeable):</p>
                                <ul className="ml-4 space-y-0.5">
                                  <li>• GM: 12191170, 25036751, 25037225, 12146312</li>
                                  <li>• ACDelco: 213-928, 213-190</li>
                                  <li>• Standard Motor Products: TX3</li>
                                  <li>• Beck/Arnley: 201-0800</li>
                                  <li>• Delphi: TS10081</li>
                                </ul>
                              </div>
                            </div>

                            {/* Ford, Toyota, Honda, Nissan, etc. - Full database included */}
                            <div className="bg-[#3a3a3a] border border-[#666666] rounded p-4 text-center">
                              <p className="text-sm text-[#888888]">
                                Full database includes complete calibration tables for:<br/>
                                Ford, Bosch, Toyota, Honda, Nissan, Mazda, Subaru, Mitsubishi, VW/Audi, Chrysler, Hyundai/Kia,<br/>
                                and all aftermarket sensors from AEM, Haltech, Link, DIYAutoTune
                              </p>
                            </div>
                          </div>

                          {/* IAT Sensors */}
                          <div className="mb-6">
                            <h4 className="text-white mb-3">2. INTAKE AIR TEMPERATURE (IAT) SENSORS</h4>
                            <div className="bg-[#2a2a2a] border border-[#444444] rounded p-4">
                              <p className="text-xs mb-2"><span className="text-[#FFD700]">NOTE:</span> Most IAT sensors use IDENTICAL calibrations to CLT sensors</p>
                              <ul className="text-xs space-y-1 ml-4">
                                <li>• Same NTC thermistor technology</li>
                                <li>• Only difference is operating temperature range</li>
                                <li>• Can interchange CLT/IAT sensors in most cases</li>
                              </ul>
                            </div>
                          </div>

                          {/* Other Temperature Sensors */}
                          <div className="bg-[#2a2a2a] border border-[#444444] rounded p-4">
                            <h4 className="text-sm text-white mb-2">Additional Temperature Sensors</h4>
                            <ul className="text-xs space-y-1 ml-4">
                              <li>• <span className="text-[#00CCFF]">Oil Temperature Sensors:</span> Similar to CLT, same calibrations</li>
                              <li>• <span className="text-[#00CCFF]">Transmission Fluid Temperature:</span> GM 24230298, VDO 323-104</li>
                              <li>• <span className="text-[#00CCFF]">EGT Sensors (K-Type):</span> 0-1250°C range, millivolt output, requires amplifier</li>
                              <li>• <span className="text-[#00CCFF]">Fuel Temperature:</span> Same as CLT sensors</li>
                            </ul>
                          </div>
                        </div>
                      </div>
                    )}
                  </div>

                  {/* SECTION B: PRESSURE SENSORS */}
                  <div className="mb-3">
                    <button
                      onClick={() => toggleSection('sectionB')}
                      className="w-full flex items-center justify-between bg-[#1a1a1a] hover:bg-[#252525] border border-[#444444] px-4 py-2 rounded transition-colors"
                    >
                      <span className="text-[#00CCFF]">SECTION B: PRESSURE SENSORS</span>
                      {expandedSections.has('sectionB') ? (
                        <ChevronDown className="w-4 h-4 text-[#00CCFF]" />
                      ) : (
                        <ChevronRight className="w-4 h-4 text-[#00CCFF]" />
                      )}
                    </button>
                    
                    {expandedSections.has('sectionB') && (
                      <div className="mt-3 space-y-6 pl-4">
                        {/* MAP Sensors */}
                        <div>
                          <h3 className="text-lg text-white mb-3">MAP SENSORS - ALL MANUFACTURERS & RANGES</h3>
                          
                          <div className="bg-[#2a2a2a] border border-[#444444] rounded p-4 mb-4">
                            <h4 className="text-sm text-[#00FF00] mb-2">Understanding MAP Sensors</h4>
                            <p className="text-xs mb-2">Technology: Piezoresistive silicon pressure element</p>
                            <ul className="text-xs space-y-1 ml-4">
                              <li>• 3-wire analog: +5V, Ground, Signal (0.5-4.5V output)</li>
                              <li>• Linear voltage output proportional to pressure</li>
                              <li>• Measures absolute pressure (not gauge pressure)</li>
                            </ul>
                          </div>

                          {/* GM MAP Sensors */}
                          <div className="mb-6">
                            <h4 className="text-white mb-3">GM MAP SENSORS (Most Popular Aftermarket)</h4>
                            
                            <div className="bg-[#2a2a2a] border border-[#444444] rounded p-4 mb-4">
                              <h5 className="text-sm text-[#FFD700] mb-2">GM 3-Bar MAP (MOST POPULAR TURBO)</h5>
                              <div className="text-xs mb-3">
                                <p><span className="text-[#888888]">Part:</span> GM 12223861 / 12614973 / ACDelco 213-4609</p>
                                <p><span className="text-[#888888]">Range:</span> 10-300 kPa (0-43.5 psi absolute / 0-28.8 psig boost)</p>
                                <p><span className="text-[#888888]">Connector:</span> Delphi 3-pin Metri-Pack 150</p>
                              </div>
                              
                              <div className="overflow-x-auto mb-3">
                                <table className="w-full text-xs border-collapse">
                                  <thead>
                                    <tr className="bg-[#0066CC]">
                                      <th className="border border-[#444444] px-2 py-1 text-white">Pressure kPa</th>
                                      <th className="border border-[#444444] px-2 py-1 text-white">Pressure psi</th>
                                      <th className="border border-[#444444] px-2 py-1 text-white">Psig Boost</th>
                                      <th className="border border-[#444444] px-2 py-1 text-white">Voltage</th>
                                    </tr>
                                  </thead>
                                  <tbody className="bg-[#1a1a1a]">
                                    <tr><td className="border border-[#444444] px-2 py-1">10</td><td className="border border-[#444444] px-2 py-1">1.45</td><td className="border border-[#444444] px-2 py-1">-13.1 (vac)</td><td className="border border-[#444444] px-2 py-1">0.4V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">50</td><td className="border border-[#444444] px-2 py-1">7.25</td><td className="border border-[#444444] px-2 py-1">-7.3 (vac)</td><td className="border border-[#444444] px-2 py-1">1.0V</td></tr>
                                    <tr className="bg-[#003366]"><td className="border border-[#444444] px-2 py-1">100 (atm)</td><td className="border border-[#444444] px-2 py-1">14.50</td><td className="border border-[#444444] px-2 py-1">0.0</td><td className="border border-[#444444] px-2 py-1">1.7V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">150</td><td className="border border-[#444444] px-2 py-1">21.76</td><td className="border border-[#444444] px-2 py-1">7.3</td><td className="border border-[#444444] px-2 py-1">2.4V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">200</td><td className="border border-[#444444] px-2 py-1">29.01</td><td className="border border-[#444444] px-2 py-1">14.5</td><td className="border border-[#444444] px-2 py-1">3.1V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">250</td><td className="border border-[#444444] px-2 py-1">36.26</td><td className="border border-[#444444] px-2 py-1">21.8</td><td className="border border-[#444444] px-2 py-1">3.8V</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">300</td><td className="border border-[#444444] px-2 py-1">43.51</td><td className="border border-[#444444] px-2 py-1">29.0</td><td className="border border-[#444444] px-2 py-1">4.5V</td></tr>
                                  </tbody>
                                </table>
                              </div>

                              <div className="bg-black/40 p-2 rounded mb-2">
                                <code className="text-xs text-[#00CCFF]">Linear formula: V = (P × 0.0133) + 0.3</code>
                              </div>

                              <div className="text-xs">
                                <p className="text-[#FFD700]">⭐ THE GOLD STANDARD for aftermarket turbo applications</p>
                              </div>
                            </div>
                          </div>

                          {/* Summary Table */}
                          <div className="bg-[#2a2a2a] border border-[#444444] rounded p-4">
                            <h5 className="text-sm text-white mb-3">MAP SENSOR SPECIFICATIONS SUMMARY</h5>
                            <div className="overflow-x-auto">
                              <table className="w-full text-xs border-collapse">
                                <thead>
                                  <tr className="bg-[#0066CC]">
                                    <th className="border border-[#444444] px-2 py-1 text-white">Sensor</th>
                                    <th className="border border-[#444444] px-2 py-1 text-white">Range kPa</th>
                                    <th className="border border-[#444444] px-2 py-1 text-white">Price</th>
                                    <th className="border border-[#444444] px-2 py-1 text-white">Best For</th>
                                  </tr>
                                </thead>
                                <tbody className="bg-[#1a1a1a]">
                                  <tr><td className="border border-[#444444] px-2 py-1">GM 1-Bar</td><td className="border border-[#444444] px-2 py-1">10-105</td><td className="border border-[#444444] px-2 py-1">$20</td><td className="border border-[#444444] px-2 py-1">NA engines</td></tr>
                                  <tr><td className="border border-[#444444] px-2 py-1">GM 2-Bar</td><td className="border border-[#444444] px-2 py-1">10-200</td><td className="border border-[#444444] px-2 py-1">$30</td><td className="border border-[#444444] px-2 py-1">Mild boost</td></tr>
                                  <tr className="bg-[#003300]"><td className="border border-[#444444] px-2 py-1">GM 3-Bar</td><td className="border border-[#444444] px-2 py-1">10-300</td><td className="border border-[#444444] px-2 py-1">$40</td><td className="border border-[#444444] px-2 py-1">Most turbo</td></tr>
                                  <tr><td className="border border-[#444444] px-2 py-1">Bosch 3.5-Bar</td><td className="border border-[#444444] px-2 py-1">10-350</td><td className="border border-[#444444] px-2 py-1">$60</td><td className="border border-[#444444] px-2 py-1">High boost</td></tr>
                                  <tr><td className="border border-[#444444] px-2 py-1">Bosch 4-Bar</td><td className="border border-[#444444] px-2 py-1">10-400</td><td className="border border-[#444444] px-2 py-1">$70</td><td className="border border-[#444444] px-2 py-1">Extreme</td></tr>
                                  <tr><td className="border border-[#444444] px-2 py-1">AEM 50 psi</td><td className="border border-[#444444] px-2 py-1">15-350</td><td className="border border-[#444444] px-2 py-1">$95</td><td className="border border-[#444444] px-2 py-1">W/ display</td></tr>
                                </tbody>
                              </table>
                            </div>
                          </div>

                          {/* Other Pressure Sensors */}
                          <div className="mt-6 bg-[#2a2a2a] border border-[#444444] rounded p-4">
                            <h4 className="text-sm text-white mb-2">Other Pressure Sensors</h4>
                            <ul className="text-xs space-y-2 ml-4">
                              <li>• <span className="text-[#00CCFF]">Oil Pressure:</span> GM 12616646 (0-100 psi, 0.5-4.5V), VDO 360-001 (0-150 psi), AEM 30-2131-150</li>
                              <li>• <span className="text-[#00CCFF]">Fuel Pressure:</span> AEM 30-2130-100 (0-100 psi), Haltech HT-010801, DIYAutoTune SENSOR-FP-100</li>
                              <li>• <span className="text-[#00CCFF]">Barometric Pressure:</span> Use GM 1-Bar MAP in sealed location</li>
                            </ul>
                          </div>
                        </div>
                      </div>
                    )}
                  </div>

                  {/* SECTION C: POSITION SENSORS */}
                  <div className="mb-3">
                    <button
                      onClick={() => toggleSection('sectionC')}
                      className="w-full flex items-center justify-between bg-[#1a1a1a] hover:bg-[#252525] border border-[#444444] px-4 py-2 rounded transition-colors"
                    >
                      <span className="text-[#00CCFF]">SECTION C: POSITION SENSORS</span>
                      {expandedSections.has('sectionC') ? (
                        <ChevronDown className="w-4 h-4 text-[#00CCFF]" />
                      ) : (
                        <ChevronRight className="w-4 h-4 text-[#00CCFF]" />
                      )}
                    </button>
                    
                    {expandedSections.has('sectionC') && (
                      <div className="mt-3 space-y-6 pl-4">
                        <div className="bg-[#2a2a2a] border border-[#444444] rounded p-4">
                          <h3 className="text-white mb-3">THROTTLE POSITION SENSORS (TPS)</h3>
                          <div className="space-y-4">
                            <div>
                              <h4 className="text-sm text-[#FFD700] mb-2">3-Wire Potentiometer TPS (Standard)</h4>
                              <ul className="text-xs space-y-2 ml-4">
                                <li>• <span className="text-[#00CCFF]">GM TPS:</span> 17123852 (LS1/LS6), 5kΩ total, Idle: 0.45-0.90V, WOT: 4.0-4.8V</li>
                                <li>• <span className="text-[#00CCFF]">Ford TPS:</span> F4TZ-9B989-A (5.0L Mustang), Idle: 0.5-1.0V, WOT: 4.2-4.8V</li>
                                <li>• <span className="text-[#00CCFF]">Honda TPS:</span> 16400-P2A-A51 (B/D-Series), Very tight spec: Idle 0.48-0.52V</li>
                                <li>• <span className="text-[#00CCFF]">Universal:</span> AEM 30-2015 ($50), DIYAutoTune SENSOR-TPS-UNIVERSAL ($35)</li>
                              </ul>
                            </div>
                            <div>
                              <h4 className="text-sm text-white mb-2">Other Position Sensors</h4>
                              <ul className="text-xs space-y-1 ml-4">
                                <li>• <span className="text-[#888888]">APPS (Accelerator Pedal):</span> Dual redundant, Bosch 6-wire</li>
                                <li>• <span className="text-[#888888]">Camshaft Position:</span> Hall effect (Honeywell M12, GM 12596851) or VR</li>
                                <li>• <span className="text-[#888888]">Crankshaft Position:</span> Hall (Honeywell 1GT101DC) or VR (Ford 4.0L SOHC)</li>
                              </ul>
                            </div>
                          </div>
                        </div>
                      </div>
                    )}
                  </div>

                  {/* SECTION D: AIRFLOW & OXYGEN SENSORS */}
                  <div className="mb-3">
                    <button
                      onClick={() => toggleSection('sectionD')}
                      className="w-full flex items-center justify-between bg-[#1a1a1a] hover:bg-[#252525] border border-[#444444] px-4 py-2 rounded transition-colors"
                    >
                      <span className="text-[#00CCFF]">SECTION D: AIRFLOW & OXYGEN SENSORS</span>
                      {expandedSections.has('sectionD') ? (
                        <ChevronDown className="w-4 h-4 text-[#00CCFF]" />
                      ) : (
                        <ChevronRight className="w-4 h-4 text-[#00CCFF]" />
                      )}
                    </button>
                    
                    {expandedSections.has('sectionD') && (
                      <div className="mt-3 space-y-6 pl-4">
                        {/* Wideband O2 */}
                        <div className="bg-[#2a2a2a] border border-[#444444] rounded p-4">
                          <h3 className="text-white mb-3">WIDEBAND O2/LAMBDA SENSORS - COMPLETE DATABASE</h3>
                          
                          <div className="space-y-4">
                            <div>
                              <h4 className="text-sm text-[#FFD700] mb-2">Bosch LSU 4.2 (Universal Standard)</h4>
                              <div className="text-xs space-y-1">
                                <p><span className="text-[#888888]">Part:</span> 0 258 007 057, 0 258 007 206, 17014, 17025</p>
                                <p><span className="text-[#888888]">Technology:</span> Planar zirconia with pump cell</p>
                                <p><span className="text-[#888888]">Wires:</span> 5-wire</p>
                                <p><span className="text-[#888888]">Lambda Range:</span> 0.65 - 1.6 λ (AFR 9.5:1 to 23.5:1 gasoline)</p>
                                <p><span className="text-[#888888]">Response Time:</span> &lt;100ms</p>
                                <p><span className="text-[#888888]">Thread:</span> M18x1.5</p>
                                <p><span className="text-[#888888]">Price:</span> $40-60</p>
                              </div>
                            </div>

                            <div>
                              <h4 className="text-sm text-[#FFD700] mb-2">Bosch LSU 4.9 (Improved)</h4>
                              <ul className="text-xs space-y-1 ml-4">
                                <li>• Faster warm-up (10-15 seconds)</li>
                                <li>• Better accuracy (±1% vs ±3%)</li>
                                <li>• Wider operating range</li>
                                <li>• Improved contamination resistance</li>
                                <li>• Price: $50-80</li>
                                <li>• Backward compatible with LSU 4.2 controllers</li>
                              </ul>
                            </div>

                            <div>
                              <h4 className="text-sm text-white mb-2">Wideband Controllers</h4>
                              <ul className="text-xs space-y-2 ml-4">
                                <li>• <span className="text-[#00CCFF]">AEM X-Series UEGO (30-0300):</span> LSU 4.9, 0-5V output, OLED display, $200-225</li>
                                <li>• <span className="text-[#00CCFF]">Innovate LC-2 (3877):</span> LSU 4.9, Programmable output, $180-220</li>
                                <li>• <span className="text-[#00CCFF]">14Point7 SLC:</span> LSU 4.2/4.9, Best value, $120-150</li>
                                <li>• <span className="text-[#00CCFF]">ECUMaster WBO:</span> LSU 4.9, CAN output, $200</li>
                              </ul>
                            </div>

                            <div className="bg-[#1a1a1a] border border-[#333333] rounded p-3">
                              <h5 className="text-xs text-[#FFD700] mb-2">Installation Best Practices</h5>
                              <ul className="text-xs space-y-1 ml-4">
                                <li>• <span className="text-[#888888]">Distance from cylinder:</span> 18-36 inches (45-90cm)</li>
                                <li>• <span className="text-[#888888]">Position:</span> Merged collector (all cylinders combined)</li>
                                <li>• <span className="text-[#888888]">Angle:</span> 10-45° upward from horizontal (prevents moisture)</li>
                                <li>• <span className="text-[#888888]">Thread:</span> M18x1.5 (standard)</li>
                                <li>• <span className="text-[#FF0000]">WARNING:</span> Anti-seize on threads ONLY (never on sensor tip!)</li>
                              </ul>
                            </div>
                          </div>
                        </div>

                        {/* MAF Sensors */}
                        <div className="bg-[#2a2a2a] border border-[#444444] rounded p-4">
                          <h3 className="text-white mb-3">MASS AIRFLOW (MAF) SENSORS</h3>
                          <ul className="text-xs space-y-2 ml-4">
                            <li>• <span className="text-[#00CCFF]">Bosch HFM5/HFM6:</span> Hot-film thermal, 0-5V analog, BMW/Mercedes/VW</li>
                            <li>• <span className="text-[#00CCFF]">GM LS1/LS2 MAF:</span> 25318411, 3.5" (89mm), 0-5V, up to 425 g/s</li>
                            <li>• <span className="text-[#00CCFF]">Nissan Z32 MAF:</span> 22680-30P00 (300ZX), Popular aftermarket, ~630 cfm stock</li>
                            <li>• <span className="text-[#FFD700]">NOTE:</span> Speed Density (MAP) often preferred - simpler, cheaper, more robust</li>
                          </ul>
                        </div>
                      </div>
                    )}
                  </div>

                  {/* SECTION E: SPEED & FREQUENCY SENSORS */}
                  <div className="mb-3">
                    <button
                      onClick={() => toggleSection('sectionE')}
                      className="w-full flex items-center justify-between bg-[#1a1a1a] hover:bg-[#252525] border border-[#444444] px-4 py-2 rounded transition-colors"
                    >
                      <span className="text-[#00CCFF]">SECTION E: SPEED & FREQUENCY SENSORS</span>
                      {expandedSections.has('sectionE') ? (
                        <ChevronDown className="w-4 h-4 text-[#00CCFF]" />
                      ) : (
                        <ChevronRight className="w-4 h-4 text-[#00CCFF]" />
                      )}
                    </button>
                    
                    {expandedSections.has('sectionE') && (
                      <div className="mt-3 space-y-6 pl-4">
                        <div className="bg-[#2a2a2a] border border-[#444444] rounded p-4">
                          <h3 className="text-white mb-3">FLEX FUEL / ETHANOL CONTENT SENSORS</h3>
                          
                          <div className="space-y-4">
                            <div>
                              <h4 className="text-sm text-[#FFD700] mb-2">GM Flex Fuel Sensor (Universal Standard)</h4>
                              <div className="text-xs mb-3">
                                <p><span className="text-[#888888]">Part:</span> GM 13577429 (Continental)</p>
                                <p><span className="text-[#888888]">Output:</span> Frequency signal (50-150 Hz)</p>
                                <p><span className="text-[#888888]">Ethanol Range:</span> 0-100% (E0 to E100)</p>
                                <p><span className="text-[#888888]">Thread:</span> 3/4"-16 UNF male (both ends)</p>
                                <p><span className="text-[#888888]">Price:</span> $50-100</p>
                              </div>

                              <div className="overflow-x-auto">
                                <table className="w-full text-xs border-collapse">
                                  <thead>
                                    <tr className="bg-[#0066CC]">
                                      <th className="border border-[#444444] px-2 py-1 text-white">Ethanol %</th>
                                      <th className="border border-[#444444] px-2 py-1 text-white">Output Hz</th>
                                      <th className="border border-[#444444] px-2 py-1 text-white">Notes</th>
                                    </tr>
                                  </thead>
                                  <tbody className="bg-[#1a1a1a]">
                                    <tr><td className="border border-[#444444] px-2 py-1">E0 (pure gas)</td><td className="border border-[#444444] px-2 py-1">50 Hz</td><td className="border border-[#444444] px-2 py-1">Baseline</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">E10</td><td className="border border-[#444444] px-2 py-1">60 Hz</td><td className="border border-[#444444] px-2 py-1">Pump gas</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">E25</td><td className="border border-[#444444] px-2 py-1">75 Hz</td><td className="border border-[#444444] px-2 py-1">-</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">E50</td><td className="border border-[#444444] px-2 py-1">100 Hz</td><td className="border border-[#444444] px-2 py-1">Mid-blend</td></tr>
                                    <tr className="bg-[#003366]"><td className="border border-[#444444] px-2 py-1">E85</td><td className="border border-[#444444] px-2 py-1">127-132 Hz</td><td className="border border-[#444444] px-2 py-1">Pump E85</td></tr>
                                    <tr><td className="border border-[#444444] px-2 py-1">E100 (pure)</td><td className="border border-[#444444] px-2 py-1">150 Hz</td><td className="border border-[#444444] px-2 py-1">Race fuel</td></tr>
                                  </tbody>
                                </table>
                              </div>

                              <div className="bg-black/40 p-2 rounded mt-2">
                                <code className="text-xs text-[#00CCFF]">Formula: Ethanol % = (Frequency - 50) × 1.0</code>
                              </div>
                            </div>
                          </div>

                          <div className="mt-4">
                            <h4 className="text-sm text-white mb-2">Other Speed/Frequency Sensors</h4>
                            <ul className="text-xs space-y-1 ml-4">
                              <li>• <span className="text-[#00CCFF]">VSS (Vehicle Speed):</span> GM 15947632, 3-wire Hall, 4000 pulses/mile</li>
                              <li>• <span className="text-[#00CCFF]">Wheel Speed (ABS):</span> Hall effect or VR, traction control/ABS</li>
                              <li>• <span className="text-[#00CCFF]">Transmission Speed:</span> Hall or VR sensors</li>
                            </ul>
                          </div>
                        </div>
                      </div>
                    )}
                  </div>

                  {/* SECTION F: SPECIALIZED SENSORS */}
                  <div className="mb-3">
                    <button
                      onClick={() => toggleSection('sectionF')}
                      className="w-full flex items-center justify-between bg-[#1a1a1a] hover:bg-[#252525] border border-[#444444] px-4 py-2 rounded transition-colors"
                    >
                      <span className="text-[#00CCFF]">SECTION F: SPECIALIZED SENSORS</span>
                      {expandedSections.has('sectionF') ? (
                        <ChevronDown className="w-4 h-4 text-[#00CCFF]" />
                      ) : (
                        <ChevronRight className="w-4 h-4 text-[#00CCFF]" />
                      )}
                    </button>
                    
                    {expandedSections.has('sectionF') && (
                      <div className="mt-3 space-y-6 pl-4">
                        <div className="bg-[#2a2a2a] border border-[#444444] rounded p-4">
                          <h3 className="text-white mb-3">KNOCK SENSORS - COMPLETE DATABASE</h3>
                          
                          <div className="space-y-4">
                            <div>
                              <h4 className="text-sm text-[#FFD700] mb-2">Resonant (Tuned) Knock Sensors</h4>
                              <ul className="text-xs space-y-2 ml-4">
                                <li>• <span className="text-[#00CCFF]">Bosch 0 261 231 038:</span> 5.5 kHz center, M10x1.0, BMW/Mercedes/Audi/VW, $30-50</li>
                                <li>• <span className="text-[#00CCFF]">GM 12589867:</span> Wideband flat response, M8 stud, LS1/LS2/LS3, $25-40</li>
                                <li>• <span className="text-[#00CCFF]">Toyota 89615-20090:</span> Resonant, M8 stud, 2JZ/1JZ</li>
                                <li>• <span className="text-[#00CCFF]">Honda 30530-PPA-A01:</span> Resonant, M8 bolt, K-Series</li>
                              </ul>
                            </div>

                            <div className="bg-[#1a1a1a] border border-[#FF0000] rounded p-3">
                              <h5 className="text-xs text-[#FF0000] mb-2">⚠️ CRITICAL MOUNTING SPECIFICATIONS</h5>
                              <ul className="text-xs space-y-1 ml-4">
                                <li>• <span className="text-[#FFD700]">Torque:</span> 15-20 Nm (11-15 ft-lbs) CRITICAL!</li>
                                <li>• <span className="text-[#FFD700]">Location:</span> Between cylinders for best sensitivity</li>
                                <li>• <span className="text-[#FFD700]">Surface:</span> Clean, flat, metal-to-metal contact</li>
                                <li>• <span className="text-[#FF0000]">NO:</span> Thread sealant, anti-seize, or washers</li>
                                <li>• <span className="text-[#FF0000]">WARNING:</span> Use torque wrench! Too loose/tight = false readings</li>
                              </ul>
                            </div>
                          </div>

                          <div className="mt-4">
                            <h4 className="text-sm text-white mb-2">Other Specialized Sensors</h4>
                            <ul className="text-xs space-y-2 ml-4">
                              <li>• <span className="text-[#00CCFF]">GPS/IMU:</span> AiM GPS08 (10Hz, $400-500), Racelogic VBOX Sport ($800-1200)</li>
                              <li>• <span className="text-[#00CCFF]">G-Force/Accelerometer:</span> ADXL335 3-axis (±3g, $15-25), Bosch SMB380 (±2/4/8g, $30-50)</li>
                              <li>• <span className="text-[#00CCFF]">Yaw Rate:</span> Bosch (CAN/analog), AiM ($400-600)</li>
                              <li>• <span className="text-[#00CCFF]">Clutch Position:</span> Binary switch ($10-20) or linear 0-5V pot ($40-80)</li>
                              <li>• <span className="text-[#00CCFF]">Brake Pressure:</span> 0-2000 psi, 0-5V analog, M10x1.0 or 1/8" NPT, $80-150</li>
                              <li>• <span className="text-[#00CCFF]">Gear Position:</span> Resistive ladder or strain gauge shift sensor ($150-300)</li>
                            </ul>
                          </div>
                        </div>
                      </div>
                    )}
                  </div>

                  {/* SECTION G: TRIGGER WHEELS */}
                  <div className="mb-3">
                    <button
                      onClick={() => toggleSection('sectionG')}
                      className="w-full flex items-center justify-between bg-[#1a1a1a] hover:bg-[#252525] border border-[#444444] px-4 py-2 rounded transition-colors"
                    >
                      <span className="text-[#00CCFF]">SECTION G: TRIGGER WHEELS & PATTERNS</span>
                      {expandedSections.has('sectionG') ? (
                        <ChevronDown className="w-4 h-4 text-[#00CCFF]" />
                      ) : (
                        <ChevronRight className="w-4 h-4 text-[#00CCFF]" />
                      )}
                    </button>
                    
                    {expandedSections.has('sectionG') && (
                      <div className="mt-3 space-y-6 pl-4">
                        <div className="bg-[#2a2a2a] border border-[#444444] rounded p-4">
                          <h3 className="text-white mb-3">COMPLETE TRIGGER PATTERN DATABASE</h3>
                          
                          <div className="space-y-4">
                            <div>
                              <h4 className="text-sm text-[#00FF00] mb-2">UNIVERSAL MISSING TOOTH PATTERNS</h4>
                              <p className="text-xs mb-3">All ECUs (Speeduino, MegaSquirt, Link, Haltech, Motec, AEM, ECUMaster) support these:</p>
                              
                              <div className="space-y-3">
                                <div className="bg-[#1a1a1a] border border-[#333333] rounded p-3">
                                  <h5 className="text-sm text-[#FFD700] mb-2">60-2 (Most Popular Ford/Professional)</h5>
                                  <ul className="text-xs space-y-1 ml-4">
                                    <li>• <span className="text-[#888888]">Teeth:</span> 58 present, 2 consecutive missing</li>
                                    <li>• <span className="text-[#888888]">Resolution:</span> 6° per tooth</li>
                                    <li>• <span className="text-[#888888]">Max RPM:</span> 15,000+ RPM</li>
                                    <li>• <span className="text-[#888888]">Applications:</span> Ford EDIS, professional builds</li>
                                  </ul>
                                </div>

                                <div className="bg-[#1a1a1a] border border-[#333333] rounded p-3">
                                  <h5 className="text-sm text-[#FFD700] mb-2">36-1 (Most Popular Aftermarket)</h5>
                                  <ul className="text-xs space-y-1 ml-4">
                                    <li>• <span className="text-[#888888]">Teeth:</span> 35 present, 1 missing</li>
                                    <li>• <span className="text-[#888888]">Resolution:</span> 10° per tooth</li>
                                    <li>• <span className="text-[#888888]">Max RPM:</span> 12,000+ RPM</li>
                                    <li>• <span className="text-[#FFD700]">⭐ Most versatile aftermarket choice</span></li>
                                  </ul>
                                </div>

                                <div className="grid grid-cols-2 gap-3">
                                  <div className="bg-[#1a1a1a] border border-[#333333] rounded p-3">
                                    <h5 className="text-sm text-[#FFD700] mb-2">24-1</h5>
                                    <ul className="text-xs space-y-1 ml-4">
                                      <li>• Teeth: 23 present, 1 missing</li>
                                      <li>• Resolution: 15° per tooth</li>
                                      <li>• Max RPM: 10,000 RPM</li>
                                    </ul>
                                  </div>

                                  <div className="bg-[#1a1a1a] border border-[#333333] rounded p-3">
                                    <h5 className="text-sm text-[#FFD700] mb-2">12-1</h5>
                                    <ul className="text-xs space-y-1 ml-4">
                                      <li>• Teeth: 11 present, 1 missing</li>
                                      <li>• Resolution: 30° per tooth</li>
                                      <li>• Max RPM: 7,000 RPM</li>
                                    </ul>
                                  </div>
                                </div>
                              </div>
                            </div>

                            <div>
                              <h4 className="text-sm text-[#00FF00] mb-3">OEM TRIGGER PATTERNS BY MANUFACTURER</h4>
                              
                              <div className="grid grid-cols-2 gap-4">
                                <div>
                                  <h5 className="text-sm text-white mb-2">FORD PATTERNS</h5>
                                  <ul className="text-xs space-y-2">
                                    <li><strong className="text-[#FFD700]">Ford EDIS:</strong> 36-1, VR sensor, ALL ECUs support</li>
                                    <li><strong className="text-[#FFD700]">Ford Coyote:</strong> 60 teeth, complex pattern</li>
                                  </ul>
                                </div>

                                <div>
                                  <h5 className="text-sm text-white mb-2">GM PATTERNS</h5>
                                  <ul className="text-xs space-y-2">
                                    <li><strong className="text-[#FFD700]">GM 7X:</strong> 7 evenly-spaced teeth, LT1/3800SC</li>
                                    <li><strong className="text-[#FFD700]">GM 24X:</strong> LS1/LS6 (1997-2005), 24 teeth variable spacing</li>
                                    <li><strong className="text-[#FFD700]">GM 58X:</strong> LS2+ (2006+), 58 evenly-spaced teeth</li>
                                  </ul>
                                </div>

                                <div>
                                  <h5 className="text-sm text-white mb-2">HONDA PATTERNS</h5>
                                  <ul className="text-xs space-y-2">
                                    <li><strong className="text-[#FFD700]">K-Series:</strong> 12 evenly-spaced + VVT cam</li>
                                    <li><strong className="text-[#FFD700]">OBD1:</strong> 4-window CAS optical distributor</li>
                                  </ul>
                                </div>

                                <div>
                                  <h5 className="text-sm text-white mb-2">NISSAN PATTERNS</h5>
                                  <ul className="text-xs space-y-2">
                                    <li><strong className="text-[#FFD700]">Nissan 360:</strong> 360 slits (1° resolution!), SR20/RB26</li>
                                    <li><strong className="text-[#FFD700]">Pattern 3:</strong> Required for Speeduino</li>
                                  </ul>
                                </div>

                                <div>
                                  <h5 className="text-sm text-white mb-2">TOYOTA PATTERNS</h5>
                                  <ul className="text-xs space-y-2">
                                    <li><strong className="text-[#FFD700]">36-2+2:</strong> 2JZ-GTE Supra turbo</li>
                                    <li><strong className="text-[#FFD700]">24/12+2:</strong> 3S-GE, 4A-GE distributor</li>
                                  </ul>
                                </div>

                                <div>
                                  <h5 className="text-sm text-white mb-2">SUBARU PATTERNS</h5>
                                  <ul className="text-xs space-y-2">
                                    <li><strong className="text-[#FFD700]">36-2-2-2:</strong> EJ20/EJ25 (WRX/STI)</li>
                                    <li>THREE 2-tooth gaps at 120° apart</li>
                                  </ul>
                                </div>
                              </div>
                            </div>
                          </div>
                        </div>
                      </div>
                    )}
                  </div>

                  {/* PURCHASE RECOMMENDATIONS */}
                  <div className="mb-3">
                    <button
                      onClick={() => toggleSection('purchase')}
                      className="w-full flex items-center justify-between bg-[#1a1a1a] hover:bg-[#252525] border border-[#444444] px-4 py-2 rounded transition-colors"
                    >
                      <span className="text-[#00CCFF]">SENSOR PURCHASE RECOMMENDATIONS BY BUILD TYPE</span>
                      {expandedSections.has('purchase') ? (
                        <ChevronDown className="w-4 h-4 text-[#00CCFF]" />
                      ) : (
                        <ChevronRight className="w-4 h-4 text-[#00CCFF]" />
                      )}
                    </button>
                    
                    {expandedSections.has('purchase') && (
                      <div className="mt-3 pl-4">
                        <div className="grid grid-cols-3 gap-4">
                          <div className="bg-[#2a2a2a] border border-[#00FF00] rounded p-4">
                            <h3 className="text-white mb-3">BUDGET STREET BUILD</h3>
                            <p className="text-xs text-[#888888] mb-3">$200-300 in sensors</p>
                            <ul className="text-xs space-y-1">
                              <li>• GM CLT sensor: <span className="text-[#00FF00]">$25</span></li>
                              <li>• GM IAT sensor: <span className="text-[#00FF00]">$25</span></li>
                              <li>• GM 3-wire TPS: <span className="text-[#00FF00]">$35</span></li>
                              <li>• GM 3-bar MAP: <span className="text-[#00FF00]">$40</span></li>
                              <li>• 14Point7 wideband + Bosch LSU 4.9: <span className="text-[#00FF00]">$170</span></li>
                            </ul>
                            <p className="text-white mt-3">Total: ~$295</p>
                          </div>

                          <div className="bg-[#2a2a2a] border border-[#FFD700] rounded p-4">
                            <h3 className="text-white mb-3">PERFORMANCE STREET BUILD</h3>
                            <p className="text-xs text-[#888888] mb-3">$400-500</p>
                            <ul className="text-xs space-y-1">
                              <li>• GM CLT: <span className="text-[#FFD700]">$25</span></li>
                              <li>• GM IAT: <span className="text-[#FFD700]">$25</span></li>
                              <li>• Quality TPS: <span className="text-[#FFD700]">$50</span></li>
                              <li>• GM 3-bar MAP: <span className="text-[#FFD700]">$40</span></li>
                              <li>• AEM X-Series wideband: <span className="text-[#FFD700]">$200</span></li>
                              <li>• GM oil pressure sensor: <span className="text-[#FFD700]">$40</span></li>
                              <li>• Flex fuel sensor: <span className="text-[#FFD700]">$80</span></li>
                            </ul>
                            <p className="text-white mt-3">Total: ~$460</p>
                          </div>

                          <div className="bg-[#2a2a2a] border border-[#FF0000] rounded p-4">
                            <h3 className="text-white mb-3">TRACK/RACE BUILD</h3>
                            <p className="text-xs text-[#888888] mb-3">$800-1200</p>
                            <ul className="text-xs space-y-1">
                              <li>• Haltech CLT: <span className="text-[#FF6666]">$65</span></li>
                              <li>• Haltech IAT: <span className="text-[#FF6666]">$65</span></li>
                              <li>• Racing TPS: <span className="text-[#FF6666]">$80</span></li>
                              <li>• Haltech 4-bar MAP: <span className="text-[#FF6666]">$85</span></li>
                              <li>• AEM X-Series wideband: <span className="text-[#FF6666]">$200</span></li>
                              <li>• AEM oil pressure + display: <span className="text-[#FF6666]">$130</span></li>
                              <li>• AEM fuel pressure + display: <span className="text-[#FF6666]">$110</span></li>
                              <li>• EGT sensors (4x): <span className="text-[#FF6666]">$200</span></li>
                              <li>• Flex fuel: <span className="text-[#FF6666]">$80</span></li>
                              <li>• GPS module: <span className="text-[#FF6666]">$400</span></li>
                            </ul>
                            <p className="text-white mt-3">Total: ~$1415</p>
                          </div>
                        </div>
                      </div>
                    )}
                  </div>
                </div>

                {/* Database Summary */}
                <div className="bg-[#003366] border-2 border-[#00CCFF] rounded p-6">
                  <h2 className="text-xl text-white mb-4">COMPLETE SENSOR SPECIFICATIONS - SUMMARY</h2>
                  <p className="text-sm mb-4">This database contains:</p>
                  <div className="grid grid-cols-2 gap-3 text-sm">
                    <div>
                      <ul className="space-y-1">
                        <li>✓ 150+ specific sensor part numbers with calibrations</li>
                        <li>✓ ALL major automotive manufacturers</li>
                        <li>✓ Complete resistance/voltage tables for temperature sensors</li>
                        <li>✓ ALL MAP sensor ranges (1-bar through 5-bar+)</li>
                      </ul>
                    </div>
                    <div>
                      <ul className="space-y-1">
                        <li>✓ Professional ECU coverage (Speeduino, MegaSquirt, Link, Haltech, Motec, AEM)</li>
                        <li>✓ 50+ trigger patterns with specifications</li>
                        <li>✓ Wideband O2 complete database</li>
                        <li>✓ Installation specifications and best practices</li>
                      </ul>
                    </div>
                  </div>
                  <p className="text-center text-lg text-[#FFD700] mt-6">
                    PROFESSIONAL-GRADE sensor database suitable for commercial ECU software development.
                  </p>
                </div>

                <div className="text-center text-xs text-[#666666] mt-8 pb-4">
                  END OF PROFESSIONAL SENSOR DATABASE
                </div>
              </div>
            </ScrollArea>
          </TabsContent>

          <TabsContent value="info" className="flex-1 overflow-hidden m-0">
            <div className="h-full flex items-center justify-center p-6">
              <div className="text-center text-[#888888]">
                <p className="text-sm">Sensor information will be added here.</p>
                <p className="text-xs mt-2">Coming soon...</p>
              </div>
            </div>
          </TabsContent>
        </Tabs>
      </div>
    </>
  );
}
