import { useState } from "react";
import { X, Download, FileText, Image as ImageIcon, Table } from "lucide-react";
import { Button } from "../ui/button";
import { Checkbox } from "../ui/checkbox";
import { toast } from "sonner@2.0.3";

interface ExportDialogProps {
  isOpen: boolean;
  onClose: () => void;
  dataType: "maps" | "logs" | "calibration" | "graphs";
  data?: any;
}

export function ExportDialog({
  isOpen,
  onClose,
  dataType,
  data,
}: ExportDialogProps) {
  const [exportFormat, setExportFormat] = useState<"csv" | "json" | "png">("csv");
  const [includeMetadata, setIncludeMetadata] = useState(true);
  const [includeTimestamp, setIncludeTimestamp] = useState(true);

  if (!isOpen) return null;

  const handleExport = () => {
    const timestamp = new Date().toISOString().replace(/[:.]/g, "-");
    let filename = "";
    let content = "";

    switch (dataType) {
      case "maps":
        filename = `fuel_ignition_maps_${timestamp}.${exportFormat}`;
        if (exportFormat === "csv") {
          content = generateCSV(data);
        } else if (exportFormat === "json") {
          content = JSON.stringify(data, null, 2);
        }
        break;

      case "logs":
        filename = `ecu_logs_${timestamp}.${exportFormat}`;
        if (exportFormat === "csv") {
          content = generateLogCSV(data);
        } else if (exportFormat === "json") {
          content = JSON.stringify(data, null, 2);
        }
        break;

      case "calibration":
        filename = `calibration_${timestamp}.cal`;
        content = generateCalibrationFile(data);
        break;

      case "graphs":
        filename = `graph_export_${timestamp}.png`;
        // For PNG export, we'd use canvas toDataURL
        exportGraphAsPNG();
        return;
    }

    // Create and download file
    const blob = new Blob([content], { type: "text/plain" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    a.download = filename;
    a.click();
    URL.revokeObjectURL(url);

    toast.success("Export successful", {
      description: `Saved as ${filename}`,
      duration: 3000,
    });

    onClose();
  };

  const generateCSV = (mapData: any) => {
    // Placeholder CSV generation
    let csv = includeMetadata ? `# ECU Tuner Pro Export\n# Date: ${new Date().toISOString()}\n# Type: Fuel/Ignition Maps\n\n` : "";
    csv += "RPM,Load,VE Value,Ignition Timing\n";
    // Add actual data rows here
    csv += "1000,20,85.5,12.0\n";
    csv += "2000,40,90.2,15.5\n";
    return csv;
  };

  const generateLogCSV = (logData: any) => {
    let csv = includeMetadata ? `# ECU Log Export\n# Date: ${new Date().toISOString()}\n\n` : "";
    csv += "Timestamp,RPM,MAP,AFR,ECT,Speed\n";
    // Add actual log data
    return csv;
  };

  const generateCalibrationFile = (calData: any) => {
    return JSON.stringify({
      metadata: includeMetadata ? {
        version: "2.0",
        exported: new Date().toISOString(),
        vehicle: "K20A2 Turbo",
      } : {},
      calibration: calData || {},
    }, null, 2);
  };

  const exportGraphAsPNG = () => {
    // Find canvas element and export
    const canvas = document.querySelector("canvas");
    if (canvas) {
      canvas.toBlob((blob) => {
        if (blob) {
          const url = URL.createObjectURL(blob);
          const a = document.createElement("a");
          a.href = url;
          a.download = `graph_${new Date().toISOString().replace(/[:.]/g, "-")}.png`;
          a.click();
          URL.revokeObjectURL(url);
          toast.success("Graph exported", { duration: 3000 });
        }
      });
    }
    onClose();
  };

  return (
    <>
      {/* Backdrop */}
      <div
        className="fixed inset-0 bg-black/70 z-[150] backdrop-blur-sm"
        onClick={onClose}
      />

      {/* Dialog */}
      <div className="fixed top-1/2 left-1/2 transform -translate-x-1/2 -translate-y-1/2 z-[151] w-[500px] max-w-[90vw]">
        <div className="bg-[#1a1a1a] border-2 border-[#0066CC] rounded-lg shadow-2xl overflow-hidden">
          {/* Header */}
          <div className="bg-[#0f0f0f] px-6 py-4 flex items-center justify-between border-b border-[#333333]">
            <div className="flex items-center gap-3">
              <Download className="w-5 h-5 text-[#0066CC]" />
              <h3 className="text-white">Export Data</h3>
            </div>
            <button
              onClick={onClose}
              className="text-[#888888] hover:text-white transition-colors"
            >
              <X className="w-5 h-5" />
            </button>
          </div>

          {/* Content */}
          <div className="p-6 space-y-6">
            {/* Format Selection */}
            <div>
              <label className="text-sm text-[#CCCCCC] mb-3 block">
                Export Format
              </label>
              <div className="grid grid-cols-3 gap-3">
                <button
                  onClick={() => setExportFormat("csv")}
                  className={`p-4 rounded border-2 transition-all ${
                    exportFormat === "csv"
                      ? "border-[#0066CC] bg-[#0066CC]/10"
                      : "border-[#333333] hover:border-[#555555]"
                  }`}
                >
                  <Table className="w-6 h-6 mx-auto mb-2 text-[#0066CC]" />
                  <div className="text-xs text-white">CSV</div>
                </button>

                <button
                  onClick={() => setExportFormat("json")}
                  className={`p-4 rounded border-2 transition-all ${
                    exportFormat === "json"
                      ? "border-[#0066CC] bg-[#0066CC]/10"
                      : "border-[#333333] hover:border-[#555555]"
                  }`}
                >
                  <FileText className="w-6 h-6 mx-auto mb-2 text-[#0066CC]" />
                  <div className="text-xs text-white">JSON</div>
                </button>

                <button
                  onClick={() => setExportFormat("png")}
                  disabled={dataType !== "graphs"}
                  className={`p-4 rounded border-2 transition-all ${
                    exportFormat === "png"
                      ? "border-[#0066CC] bg-[#0066CC]/10"
                      : "border-[#333333] hover:border-[#555555]"
                  } disabled:opacity-30 disabled:cursor-not-allowed`}
                >
                  <ImageIcon className="w-6 h-6 mx-auto mb-2 text-[#0066CC]" />
                  <div className="text-xs text-white">PNG</div>
                </button>
              </div>
            </div>

            {/* Options */}
            <div className="space-y-3">
              <label className="text-sm text-[#CCCCCC] block">Options</label>
              
              <div className="flex items-center space-x-3">
                <Checkbox
                  id="metadata"
                  checked={includeMetadata}
                  onCheckedChange={(checked) => setIncludeMetadata(checked as boolean)}
                />
                <label
                  htmlFor="metadata"
                  className="text-sm text-[#CCCCCC] cursor-pointer"
                >
                  Include metadata (vehicle info, version)
                </label>
              </div>

              <div className="flex items-center space-x-3">
                <Checkbox
                  id="timestamp"
                  checked={includeTimestamp}
                  onCheckedChange={(checked) => setIncludeTimestamp(checked as boolean)}
                />
                <label
                  htmlFor="timestamp"
                  className="text-sm text-[#CCCCCC] cursor-pointer"
                >
                  Add timestamp to filename
                </label>
              </div>
            </div>

            {/* Info */}
            <div className="bg-[#0f0f0f] border border-[#333333] rounded p-3">
              <p className="text-xs text-[#888888]">
                {dataType === "maps" && "Export includes VE tables, ignition timing, and axis configurations."}
                {dataType === "logs" && "Export includes all logged sensor data with timestamps."}
                {dataType === "calibration" && "Export complete calibration file compatible with ECU Tuner Pro."}
                {dataType === "graphs" && "Export current graph view as high-resolution PNG image."}
              </p>
            </div>
          </div>

          {/* Footer */}
          <div className="bg-[#0f0f0f] px-6 py-4 border-t border-[#333333] flex justify-end gap-3">
            <Button
              onClick={onClose}
              variant="outline"
              className="bg-transparent border-[#333333] text-white hover:bg-[#333333]"
            >
              Cancel
            </Button>
            <Button
              onClick={handleExport}
              className="bg-[#0066CC] hover:bg-[#0080FF] text-white"
            >
              <Download className="w-4 h-4 mr-2" />
              Export
            </Button>
          </div>
        </div>
      </div>
    </>
  );
}
