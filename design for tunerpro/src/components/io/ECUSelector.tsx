import { useECU } from "../context/ECUContext";
import { getAllECUNames, getECUsByManufacturer } from "../context/ECUDatabase";
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "../ui/select";
import { Badge } from "../ui/badge";
import { toast } from "sonner@2.0.3";

export function ECUSelector() {
  const { selectedECU, selectECU, clearECU } = useECU();
  const allECUs = getAllECUNames();
  const speeduinoECUs = getECUsByManufacturer('Speeduino');
  const megasquirtECUs = getECUsByManufacturer('MegaSquirt');

  const handleECUChange = (value: string) => {
    if (value === "none") {
      clearECU();
      toast.info("ECU configuration cleared");
    } else {
      selectECU(value);
      const ecu = allECUs.find(e => e.id === value);
      if (ecu) {
        toast.success(`ECU selected: ${ecu.name}`, {
          description: "I/O configuration has been automatically set up",
        });
      }
    }
  };

  return (
    <div className="bg-[#1a1a1a] border border-[#333333] p-4">
      <div className="flex items-center justify-between mb-4">
        <div>
          <h2 className="text-white mb-1">ECU Configuration</h2>
          <p className="text-xs text-[#888888]">
            Select your ECU to automatically configure inputs and outputs
          </p>
        </div>
        {selectedECU && (
          <Badge variant="outline" className="bg-[#00FF00] text-black border-[#00FF00]">
            Auto-Configured
          </Badge>
        )}
      </div>

      <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
        <div>
          <label className="block text-[#CCCCCC] mb-2 text-xs">ECU Model</label>
          <Select value={selectedECU?.id || "none"} onValueChange={handleECUChange}>
            <SelectTrigger className="w-full bg-black border-[#444444] text-white">
              <SelectValue placeholder="Select ECU model..." />
            </SelectTrigger>
            <SelectContent className="bg-[#1a1a1a] border-[#444444]">
              <SelectItem value="none" className="text-white hover:bg-[#2a2a2a]">
                Manual Configuration (No Preset)
              </SelectItem>
              
              {/* Speeduino ECUs */}
              <div className="px-2 py-1.5 text-xs text-[#888888] mt-2">
                SPEEDUINO ECUs
              </div>
              {speeduinoECUs.map((ecu) => (
                <SelectItem 
                  key={ecu.id} 
                  value={ecu.id}
                  className="text-white hover:bg-[#2a2a2a] pl-6"
                >
                  {ecu.name}
                </SelectItem>
              ))}

              {/* MegaSquirt ECUs */}
              <div className="px-2 py-1.5 text-xs text-[#888888] mt-2">
                MEGASQUIRT ECUs
              </div>
              {megasquirtECUs.map((ecu) => (
                <SelectItem 
                  key={ecu.id} 
                  value={ecu.id}
                  className="text-white hover:bg-[#2a2a2a] pl-6"
                >
                  {ecu.name}
                </SelectItem>
              ))}
            </SelectContent>
          </Select>
        </div>

        {selectedECU && (
          <div className="space-y-2">
            <div className="text-xs">
              <span className="text-[#888888]">Platform: </span>
              <span className="text-[#CCCCCC]">{selectedECU.platform}</span>
            </div>
            <div className="text-xs">
              <span className="text-[#888888]">Connector: </span>
              <span className="text-[#CCCCCC]">{selectedECU.connectorType}</span>
            </div>
            <div className="text-xs">
              <span className="text-[#888888]">Map Size: </span>
              <span className="text-[#CCCCCC]">{selectedECU.mapSize}</span>
            </div>
            <div className="text-xs">
              <span className="text-[#888888]">Outputs: </span>
              <span className="text-[#00FF00]">
                {selectedECU.outputs.filter(o => o.enabled).length} configured
              </span>
            </div>
            <div className="text-xs">
              <span className="text-[#888888]">Inputs: </span>
              <span className="text-[#00FF00]">
                {selectedECU.inputs.filter(i => i.enabled).length} configured
              </span>
            </div>
          </div>
        )}
      </div>

      {selectedECU && (
        <div className="mt-4 p-3 bg-[#0a0a0a] border border-[#444444] rounded">
          <div className="text-xs text-[#888888] mb-2">Recommended Use:</div>
          <div className="text-xs text-[#CCCCCC]">{selectedECU.recommendedUse}</div>
          
          <div className="text-xs text-[#888888] mt-3 mb-2">Engine Support:</div>
          <ul className="text-xs text-[#CCCCCC] space-y-1">
            {selectedECU.engineSupport.map((support, i) => (
              <li key={i} className="flex items-start gap-2">
                <span className="text-[#00FF00]">•</span>
                <span>{support}</span>
              </li>
            ))}
          </ul>

          {selectedECU.features.length > 0 && (
            <>
              <div className="text-xs text-[#888888] mt-3 mb-2">Key Features:</div>
              <ul className="text-xs text-[#CCCCCC] space-y-1">
                {selectedECU.features.slice(0, 3).map((feature, i) => (
                  <li key={i} className="flex items-start gap-2">
                    <span className="text-[#00FF00]">✓</span>
                    <span>{feature}</span>
                  </li>
                ))}
              </ul>
            </>
          )}
        </div>
      )}
    </div>
  );
}
