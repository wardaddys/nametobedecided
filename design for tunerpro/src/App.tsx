import { useState, useEffect, useCallback } from "react";
import { Toaster } from "./components/ui/sonner";
import { TopBar } from "./components/layout/TopBar";
import { StatusBar } from "./components/layout/StatusBar";
import { TabBar } from "./components/layout/TabBar";
import { ParticleBackground } from "./components/layout/ParticleBackground";
import { Dashboard } from "./components/dashboard/Dashboard";
import { MapsEditor } from "./components/maps/MapsEditor";
import { IdleControl } from "./components/idle/IdleControl";
import { ColdStart } from "./components/coldstart/ColdStart";
import { AccelEnrichment } from "./components/accel/AccelEnrichment";
import { TrimsEditor } from "./components/trims/TrimsEditor";
import { VTECControl } from "./components/vtec/VTECControl";
import { BoostControl } from "./components/boost/BoostControl";
import { BoostSettingsWrapper } from "./components/boost/BoostSettingsWrapper";
import { KnockControl } from "./components/knock/KnockControl";
import { Limiters } from "./components/limiters/Limiters";
import { AdvancedFeatures } from "./components/advanced/AdvancedFeatures";
import { IOConfiguration } from "./components/io/IOConfiguration";
import { O2VETuning } from "./components/o2ve/O2VETuning";
import { LogViewer } from "./components/logs/LogViewer";
import { AdvancedLogViewer } from "./components/logs/AdvancedLogViewer";
import { SensorsCalibration } from "./components/sensors/SensorsCalibration";
import { EngineParameters } from "./components/settings/EngineParameters";
import { SensorProvider } from "./components/context/SensorContext";
import { ThemeProvider } from "./components/context/ThemeContext";
import { StatusBoxProvider } from "./components/context/StatusBoxContext";
import { ECUProvider } from "./components/context/ECUContext";
import { VTECProvider } from "./components/context/VTECContext";
import { EngineConfigProvider } from "./components/context/EngineConfigContext";
import { toast } from "sonner@2.0.3";
// Enhancement components - ADDED ONLY, NO MODIFICATIONS TO EXISTING CODE
import { QuickGuideOverlay } from "./components/enhancements/QuickGuideOverlay";
import { KeyboardShortcuts } from "./components/enhancements/KeyboardShortcuts";
import { HotkeyOverlay } from "./components/enhancements/HotkeyOverlay";
import { SuccessAnimation } from "./components/enhancements/SuccessAnimation";
import { AllTables } from "./components/tables/AllTables";
import { SettingsTab } from "./components/settings/SettingsTab";

function AppContent() {
  const [activeScreen, setActiveScreen] = useState("dashboard");
  const [liveTuning, setLiveTuning] = useState(false);
  const [connectionStatus, setConnectionStatus] = useState<
    "connected" | "disconnected" | "error"
  >("connected");
  // Enhancement state - ADDED ONLY
  const [showQuickGuide, setShowQuickGuide] = useState(false);
  const [showSuccessAnimation, setShowSuccessAnimation] = useState(false);
  const [successMessage, setSuccessMessage] = useState("");
  
  // Live status bar data - merged with gauge data
  const [statusData, setStatusData] = useState({
    rpm: 5943,
    map: 98.5,
    afr: 14.7,
    ect: 88,
    boost: 5,
    speed: 0,
    gear: 5,
    oilTemp: 102,
    oilPressure: 85,
    fuelPressure: 95,
    commHealth: "good" as "good" | "warning" | "error",
  });

  // I/O status for status bar
  const [ioStatus, setIoStatus] = useState<string>("");

  // Shared gauge data for dashboard
  const [gaugeData, setGaugeData] = useState({
    boost: 5,
    speed: 0,
    gear: 5,
    rpm: 5943,
    trip: 1532,
    manifoldPressure: 45,
    fuelPressure: 95,
    oilPressure: 85,
    oilTemp: 102,
  });

  // Simulate live data updates - unified for both dashboard and status bar
  useEffect(() => {
    if (!liveTuning) {
      const defaultData = {
        boost: 5,
        speed: 0,
        gear: 5,
        rpm: 5943,
        trip: 1532,
        manifoldPressure: 45,
        fuelPressure: 95,
        oilPressure: 85,
        oilTemp: 102,
      };
      setGaugeData(defaultData);
      setStatusData({
        rpm: defaultData.rpm,
        map: 98.5,
        afr: 14.7,
        ect: 88,
        boost: defaultData.boost,
        speed: defaultData.speed,
        gear: defaultData.gear,
        oilTemp: defaultData.oilTemp,
        oilPressure: defaultData.oilPressure,
        fuelPressure: defaultData.fuelPressure,
        commHealth: "good",
      });
      return;
    }

    const interval = setInterval(() => {
      setGaugeData((prev) => {
        const newRpm = Math.max(1000, Math.min(9000, prev.rpm + (Math.random() - 0.5) * 200));
        const newBoost = Math.max(0, Math.min(25, prev.boost + (Math.random() - 0.5) * 0.5));
        const newSpeed = Math.max(0, Math.min(200, prev.speed + (Math.random() - 0.5) * 10));
        const newGear = Math.random() > 0.98 ? Math.max(1, Math.min(6, prev.gear + (Math.random() > 0.5 ? 1 : -1))) : prev.gear;
        const newOilTemp = Math.max(80, Math.min(120, prev.oilTemp + (Math.random() - 0.5) * 2));
        const newOilPressure = Math.max(40, Math.min(120, prev.oilPressure + (Math.random() - 0.5) * 3));
        const newFuelPressure = Math.max(80, Math.min(120, prev.fuelPressure + (Math.random() - 0.5) * 3));
        
        // Update status bar with same data
        setStatusData((prevStatus) => ({
          ...prevStatus,
          rpm: newRpm,
          map: Math.max(20, Math.min(200, prevStatus.map + (Math.random() - 0.5) * 10)),
          afr: Math.max(10, Math.min(16, prevStatus.afr + (Math.random() - 0.5) * 0.2)),
          ect: Math.max(80, Math.min(105, prevStatus.ect + (Math.random() - 0.5) * 1)),
          boost: newBoost,
          speed: newSpeed,
          gear: newGear,
          oilTemp: newOilTemp,
          oilPressure: newOilPressure,
          fuelPressure: newFuelPressure,
        }));

        return {
          boost: newBoost,
          speed: newSpeed,
          gear: newGear,
          rpm: newRpm,
          trip: prev.trip,
          manifoldPressure: Math.max(0, Math.min(120, prev.manifoldPressure + (Math.random() - 0.5) * 5)),
          fuelPressure: newFuelPressure,
          oilPressure: newOilPressure,
          oilTemp: newOilTemp,
        };
      });
    }, 100);

    return () => clearInterval(interval);
  }, [liveTuning]);

  const handleLiveTuningChange = (value: boolean) => {
    setLiveTuning(value);
    if (value) {
      toast.success("Live Tuning enabled", {
        description: "ECU modifications are now active",
        duration: 3000,
      });
    } else {
      toast.info("Live Tuning disabled", {
        description: "ECU is now read-only",
        duration: 3000,
      });
    }
  };

  const handleSave = () => {
    toast.success("Calibration saved", {
      description: "K20A2_Stage2.cal",
      duration: 3000,
    });
  };

  const handleUpload = () => {
    if (!liveTuning) {
      toast.error("Upload failed", {
        description: "Live Tuning must be enabled",
        duration: 1000,
      });
      return;
    }
    toast.success("Uploading to ECU...", {
      description: "This may take a few moments",
      duration: 1000,
    });
    
    // Enhancement: Trigger success animation on completion
    // Reset animation state first
    setShowSuccessAnimation(false);
    setSuccessMessage("");
    
    // Show success message after simulated upload delay
    setTimeout(() => {
      setSuccessMessage("ECU Flash Complete!");
      // Use a small delay to ensure state update
      setTimeout(() => {
        setShowSuccessAnimation(true);
      }, 10);
    }, 2000);
  };

  const handleDownload = () => {
    toast.success("Downloading from ECU...", {
      description: "Reading current calibration",
      duration: 3000,
    });
  };

  // Handler for success animation completion
  const handleAnimationComplete = useCallback(() => {
    setShowSuccessAnimation(false);
    setSuccessMessage("");
  }, []);

  // Handler for I/O configuration changes
  const handleIOStatusChange = useCallback((status: string) => {
    setIoStatus(status);
    // Auto-clear after 5 seconds
    setTimeout(() => setIoStatus(""), 5000);
  }, []);

  const renderScreen = () => {
    switch (activeScreen) {
      case "dashboard":
        return null; // Dashboard is always shown at top
      case "tables":
        return <AllTables liveTuning={liveTuning} />;
      case "idle":
        return <IdleControl liveTuning={liveTuning} />;
      case "coldstart":
        return <ColdStart liveTuning={liveTuning} />;
      case "accel":
        return <AccelEnrichment liveTuning={liveTuning} />;
      case "trims":
        return <TrimsEditor liveTuning={liveTuning} />;
      case "vtec":
        return <VTECControl liveTuning={liveTuning} />;
      case "boost":
        return <BoostSettingsWrapper liveTuning={liveTuning} />;
      case "knock":
        return <KnockControl liveTuning={liveTuning} />;
      case "limiters":
        return <Limiters liveTuning={liveTuning} />;
      case "advanced":
        return <AdvancedFeatures liveTuning={liveTuning} />;
      case "io":
        return <IOConfiguration liveTuning={liveTuning} onStatusChange={handleIOStatusChange} />;
      case "o2ve":
        return <O2VETuning liveTuning={liveTuning} />;
      case "logs":
        return <AdvancedLogViewer />;
      case "sensors":
        return <SensorsCalibration liveTuning={liveTuning} />;
      case "settings":
        return <SettingsTab liveTuning={liveTuning} />;
      default:
        return null;
    }
  };

  const isFullDashboard = activeScreen === "dashboard";

  return (
    <div className="h-screen w-screen flex flex-col overflow-hidden scanline" style={{ background: "var(--app-background)" }}>
      {/* Animated Particle Background */}
      <ParticleBackground />
      
      {/* Animated Circuit Pattern Overlay */}
      <div className="fixed inset-0 circuit-pattern pointer-events-none" style={{ zIndex: 1 }} />
      
      {/* Enhancement: Quick Guide Overlay - ADDED ONLY */}
      {showQuickGuide && <QuickGuideOverlay onClose={() => setShowQuickGuide(false)} />}
      
      {/* Enhancement: Global Keyboard Shortcuts - ADDED ONLY */}
      <KeyboardShortcuts
        onScreenChange={setActiveScreen}
        onSave={handleSave}
        onUpload={handleUpload}
        onDownload={handleDownload}
        liveTuning={liveTuning}
        onToggleLiveTuning={() => handleLiveTuningChange(!liveTuning)}
      />
      
      {/* Enhancement: Hotkey Overlay (F1 for help) - ADDED ONLY */}
      <HotkeyOverlay />
      
      {/* Enhancement: Success Animation (confetti on flash) - ADDED ONLY */}
      <SuccessAnimation
        trigger={showSuccessAnimation}
        message={successMessage}
        onComplete={handleAnimationComplete}
      />
      
      {/* Main Content */}
      <div className="relative z-10 h-full w-full flex flex-col">
        {/* Top Bar */}
        <TopBar
          liveTuning={liveTuning}
          onLiveTuningChange={handleLiveTuningChange}
          connectionStatus={connectionStatus}
          onSave={handleSave}
          onUpload={handleUpload}
          onDownload={handleDownload}
        />

        {/* Tab Navigation Bar */}
        <TabBar activeScreen={activeScreen} onScreenChange={setActiveScreen} />

        {/* Dashboard - ONLY shown when dashboard tab is active (NO mini dash) */}
        {isFullDashboard && (
          <div className="flex-1">
            <Dashboard 
              liveTuning={liveTuning} 
              isMinimized={false}
              gaugeData={gaugeData}
            />
          </div>
        )}

        {/* Main Content Area - Shown when not on dashboard (MORE SPACE NOW) */}
        {!isFullDashboard && (
          <main className="flex-1 overflow-hidden" style={{ background: "var(--app-background)" }}>{renderScreen()}</main>
        )}

        {/* Status Bar at Bottom - Now includes all mini dash data */}
        <StatusBar
          rpm={Math.round(statusData.rpm)}
          map={statusData.map}
          afr={statusData.afr}
          ect={Math.round(statusData.ect)}
          boost={statusData.boost}
          speed={statusData.speed}
          gear={statusData.gear}
          oilTemp={statusData.oilTemp}
          oilPressure={statusData.oilPressure}
          fuelPressure={statusData.fuelPressure}
          lastBackup="2024-10-17 14:23:45"
          commHealth={statusData.commHealth}
          liveTuning={liveTuning}
          ioStatus={ioStatus}
        />
      </div>

      {/* Toast Notifications */}
      <Toaster position="top-right" />
    </div>
  );
}

export default function App() {
  return (
    <ThemeProvider>
      <EngineConfigProvider>
        <StatusBoxProvider>
          <SensorProvider>
            <ECUProvider>
              <VTECProvider>
                <AppContent />
              </VTECProvider>
            </ECUProvider>
          </SensorProvider>
        </StatusBoxProvider>
      </EngineConfigProvider>
    </ThemeProvider>
  );
}