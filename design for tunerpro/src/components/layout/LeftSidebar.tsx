import {
  LayoutDashboard,
  Table2,
  LineChart,
  Wand2,
  Gauge,
  Settings,
  ChevronLeft,
  ChevronRight,
} from "lucide-react";
import { Button } from "../ui/button";
import {
  Tooltip,
  TooltipContent,
  TooltipProvider,
  TooltipTrigger,
} from "../ui/tooltip";

interface LeftSidebarProps {
  activeScreen: string;
  onScreenChange: (screen: string) => void;
  collapsed: boolean;
  onToggleCollapse: () => void;
}

const navItems = [
  { id: "dashboard", label: "Dashboard", icon: LayoutDashboard },
  { id: "maps", label: "Maps & Tables", icon: Table2 },
  { id: "logs", label: "Graphing & Logging", icon: LineChart },
  { id: "autotune", label: "AutoTune", icon: Wand2 },
  { id: "sensors", label: "Sensors", icon: Gauge },
  { id: "settings", label: "Settings", icon: Settings },
];

export function LeftSidebar({
  activeScreen,
  onScreenChange,
  collapsed,
  onToggleCollapse,
}: LeftSidebarProps) {
  return (
    <div
      className={`h-full bg-[#111419] border-r border-[rgba(175,198,210,0.1)] flex flex-col transition-all duration-300 ${
        collapsed ? "w-16" : "w-60"
      }`}
    >
      {/* Navigation Items */}
      <nav className="flex-1 py-4">
        <TooltipProvider>
          {navItems.map((item) => {
            const Icon = item.icon;
            const isActive = activeScreen === item.id;

            return (
              <Tooltip key={item.id} delayDuration={0}>
                <TooltipTrigger asChild>
                  <button
                    onClick={() => onScreenChange(item.id)}
                    className={`w-full flex items-center gap-3 px-4 py-3 transition-colors ${
                      isActive
                        ? "bg-[#1FB6FF] text-[#0B0F12]"
                        : "text-[#AFC6D2] hover:bg-[rgba(31,182,255,0.1)] hover:text-[#1FB6FF]"
                    }`}
                  >
                    <Icon className="w-5 h-5 flex-shrink-0" />
                    {!collapsed && (
                      <span className="text-sm font-medium">{item.label}</span>
                    )}
                  </button>
                </TooltipTrigger>
                {collapsed && (
                  <TooltipContent side="right">
                    <p>{item.label}</p>
                  </TooltipContent>
                )}
              </Tooltip>
            );
          })}
        </TooltipProvider>
      </nav>

      {/* Collapse Toggle */}
      <div className="border-t border-[rgba(175,198,210,0.1)] p-2">
        <Button
          variant="ghost"
          size="sm"
          onClick={onToggleCollapse}
          className="w-full justify-center"
        >
          {collapsed ? (
            <ChevronRight className="w-4 h-4" />
          ) : (
            <>
              <ChevronLeft className="w-4 h-4 mr-2" />
              Collapse
            </>
          )}
        </Button>
      </div>
    </div>
  );
}
