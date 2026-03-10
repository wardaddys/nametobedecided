import { useEffect, useState } from "react";
import { CheckCircle2, AlertCircle, Loader2 } from "lucide-react";
import { Progress } from "../ui/progress";

interface ProgressModalProps {
  isOpen: boolean;
  title: string;
  description?: string;
  progress: number; // 0-100
  status: "in-progress" | "success" | "error";
  logs?: string[];
  onClose?: () => void;
}

export function ProgressModal({
  isOpen,
  title,
  description,
  progress,
  status,
  logs = [],
  onClose,
}: ProgressModalProps) {
  const [displayedLogs, setDisplayedLogs] = useState<string[]>([]);

  useEffect(() => {
    if (logs.length > 0) {
      setDisplayedLogs(logs);
    }
  }, [logs]);

  if (!isOpen) return null;

  return (
    <>
      {/* Backdrop */}
      <div className="fixed inset-0 bg-black/70 z-[200] backdrop-blur-sm" />

      {/* Modal */}
      <div className="fixed top-1/2 left-1/2 transform -translate-x-1/2 -translate-y-1/2 z-[201] w-[600px] max-w-[90vw]">
        <div className="bg-[#1a1a1a] border-2 border-[#0066CC] rounded-lg shadow-2xl overflow-hidden">
          {/* Header */}
          <div className="bg-[#0f0f0f] px-6 py-4 border-b border-[#333333]">
            <div className="flex items-center gap-3">
              {status === "in-progress" && (
                <Loader2 className="w-5 h-5 text-[#0066CC] animate-spin" />
              )}
              {status === "success" && (
                <CheckCircle2 className="w-5 h-5 text-[#00FF00]" />
              )}
              {status === "error" && (
                <AlertCircle className="w-5 h-5 text-[#FF0000]" />
              )}
              <div>
                <h3 className="text-white">{title}</h3>
                {description && (
                  <p className="text-[#888888] text-sm mt-1">{description}</p>
                )}
              </div>
            </div>
          </div>

          {/* Progress Bar */}
          <div className="px-6 py-4 bg-[#0f0f0f] border-b border-[#333333]">
            <div className="space-y-2">
              <div className="flex justify-between text-sm">
                <span className="text-[#CCCCCC]">
                  {status === "in-progress"
                    ? "Processing..."
                    : status === "success"
                    ? "Completed"
                    : "Failed"}
                </span>
                <span className="text-[#0066CC] font-mono">{progress}%</span>
              </div>
              <Progress
                value={progress}
                className="h-2 bg-[#333333]"
                indicatorClassName={
                  status === "error"
                    ? "bg-[#FF0000]"
                    : status === "success"
                    ? "bg-[#00FF00]"
                    : "bg-[#0066CC]"
                }
              />
            </div>
          </div>

          {/* Log Output */}
          {displayedLogs.length > 0 && (
            <div className="px-6 py-4 max-h-[300px] overflow-y-auto bg-black">
              <div className="font-mono text-xs space-y-1">
                {displayedLogs.map((log, index) => (
                  <div
                    key={index}
                    className={`${
                      log.includes("ERROR")
                        ? "text-[#FF0000]"
                        : log.includes("SUCCESS") || log.includes("OK")
                        ? "text-[#00FF00]"
                        : log.includes("WARNING")
                        ? "text-[#FFFF00]"
                        : "text-[#00CCFF]"
                    }`}
                  >
                    {log}
                  </div>
                ))}
              </div>
            </div>
          )}

          {/* Footer */}
          {(status === "success" || status === "error") && onClose && (
            <div className="px-6 py-4 bg-[#0f0f0f] border-t border-[#333333] flex justify-end">
              <button
                onClick={onClose}
                className="px-4 py-2 bg-[#0066CC] hover:bg-[#0080FF] text-white rounded transition-colors duration-200"
              >
                Close
              </button>
            </div>
          )}
        </div>
      </div>
    </>
  );
}
