import { useEffect, useState } from "react";
import { Wifi, WifiOff, Cloud, CloudOff } from "lucide-react";
import { TooltipWrapper } from "./TooltipWrapper";

interface ConnectionStatusProps {
  showDatabaseSync?: boolean;
}

export function ConnectionStatus({ showDatabaseSync = true }: ConnectionStatusProps) {
  const [isOnline, setIsOnline] = useState(navigator.onLine);
  const [isDatabaseSynced, setIsDatabaseSynced] = useState(true);
  const [lastSyncTime, setLastSyncTime] = useState<Date>(new Date());

  useEffect(() => {
    const handleOnline = () => setIsOnline(true);
    const handleOffline = () => setIsOnline(false);

    window.addEventListener("online", handleOnline);
    window.addEventListener("offline", handleOffline);

    // Simulate database sync status check
    const syncInterval = setInterval(() => {
      // Check if database is accessible (this would be a real API call)
      const isDbSynced = navigator.onLine && Math.random() > 0.1; // 90% success rate
      setIsDatabaseSynced(isDbSynced);
      if (isDbSynced) {
        setLastSyncTime(new Date());
      }
    }, 30000); // Check every 30 seconds

    return () => {
      window.removeEventListener("online", handleOnline);
      window.removeEventListener("offline", handleOffline);
      clearInterval(syncInterval);
    };
  }, []);

  const formatLastSync = () => {
    const now = new Date();
    const diff = Math.floor((now.getTime() - lastSyncTime.getTime()) / 1000);
    
    if (diff < 60) return "Just now";
    if (diff < 3600) return `${Math.floor(diff / 60)}m ago`;
    if (diff < 86400) return `${Math.floor(diff / 3600)}h ago`;
    return lastSyncTime.toLocaleDateString();
  };

  return (
    <div className="flex items-center gap-3">
      {/* Internet Connection Status */}
      <TooltipWrapper
        content={
          isOnline
            ? "Internet connection active"
            : "No internet connection - working offline"
        }
      >
        <div className="flex items-center gap-1.5 px-2 py-1 rounded bg-[#1a1a1a] border border-[#333333]">
          {isOnline ? (
            <Wifi className="w-3.5 h-3.5 text-[#00FF00]" />
          ) : (
            <WifiOff className="w-3.5 h-3.5 text-[#FF0000]" />
          )}
          <span className={`text-xs ${isOnline ? "text-[#00FF00]" : "text-[#FF0000]"}`}>
            {isOnline ? "Online" : "Offline"}
          </span>
        </div>
      </TooltipWrapper>

      {/* Database Sync Status */}
      {showDatabaseSync && (
        <TooltipWrapper
          content={
            <div className="space-y-1">
              <div className="text-sm">
                {isDatabaseSynced ? "Database synchronized" : "Database sync failed"}
              </div>
              <div className="text-xs text-[#888888]">
                Last sync: {formatLastSync()}
              </div>
              {!isDatabaseSynced && (
                <div className="text-xs text-[#FFFF00] mt-2">
                  Working with cached data
                </div>
              )}
            </div>
          }
        >
          <div className="flex items-center gap-1.5 px-2 py-1 rounded bg-[#1a1a1a] border border-[#333333]">
            {isDatabaseSynced ? (
              <Cloud className="w-3.5 h-3.5 text-[#0066CC]" />
            ) : (
              <CloudOff className="w-3.5 h-3.5 text-[#FFFF00]" />
            )}
            <span className={`text-xs ${isDatabaseSynced ? "text-[#0066CC]" : "text-[#FFFF00]"}`}>
              {isDatabaseSynced ? "Synced" : "Cached"}
            </span>
          </div>
        </TooltipWrapper>
      )}
    </div>
  );
}
