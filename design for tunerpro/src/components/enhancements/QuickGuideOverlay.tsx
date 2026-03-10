import { useState, useEffect } from "react";
import { X, ChevronRight, ChevronLeft } from "lucide-react";
import { Button } from "../ui/button";

interface QuickGuideOverlayProps {
  onClose: () => void;
}

const guideSteps = [
  {
    title: "Welcome to ECU Tuner Pro",
    description: "Let's walk through the key features to get you started with professional-grade ECU tuning.",
    highlight: null,
  },
  {
    title: "Select Your Vehicle",
    description: "Start by selecting your vehicle from the ECU selector in the top bar. This auto-configures I/O pins and sensor calibrations.",
    highlight: "top-bar",
  },
  {
    title: "Edit Fuel & Ignition Maps",
    description: "Navigate to 'Maps' to edit VE tables, ignition timing, and view 3D visualizations. Linear and cubic interpolation available.",
    highlight: "maps-tab",
  },
  {
    title: "Sync with ECU",
    description: "Use the Upload/Download buttons to sync changes. Enable 'Live Tuning' for real-time modifications (ensure ECU is in bench mode).",
    highlight: "sync-buttons",
  },
  {
    title: "Monitor Real-Time Data",
    description: "The dashboard shows live RPM, AFR, boost, and temperatures. Status bar displays communication health.",
    highlight: "dashboard",
  },
];

export function QuickGuideOverlay({ onClose }: QuickGuideOverlayProps) {
  const [currentStep, setCurrentStep] = useState(0);
  const [isVisible, setIsVisible] = useState(true);

  useEffect(() => {
    // Check if user has seen guide before
    const hasSeenGuide = localStorage.getItem("hasSeenQuickGuide");
    if (hasSeenGuide) {
      setIsVisible(false);
      onClose();
    }
  }, [onClose]);

  const handleNext = () => {
    if (currentStep < guideSteps.length - 1) {
      setCurrentStep(currentStep + 1);
    } else {
      handleComplete();
    }
  };

  const handlePrevious = () => {
    if (currentStep > 0) {
      setCurrentStep(currentStep - 1);
    }
  };

  const handleComplete = () => {
    localStorage.setItem("hasSeenQuickGuide", "true");
    setIsVisible(false);
    onClose();
  };

  const handleSkip = () => {
    localStorage.setItem("hasSeenQuickGuide", "true");
    setIsVisible(false);
    onClose();
  };

  if (!isVisible) return null;

  const step = guideSteps[currentStep];

  return (
    <>
      {/* Backdrop */}
      <div className="fixed inset-0 bg-black/80 z-[100] backdrop-blur-sm transition-opacity duration-300" />

      {/* Guide Card */}
      <div className="fixed top-1/2 left-1/2 transform -translate-x-1/2 -translate-y-1/2 z-[101] w-[600px] max-w-[90vw]">
        <div className="bg-[#1a1a1a] border-2 border-[#0066CC] rounded-lg shadow-2xl overflow-hidden animate-in fade-in zoom-in duration-300">
          {/* Header */}
          <div className="bg-gradient-to-r from-[#0066CC] to-[#0099FF] px-6 py-4 flex items-center justify-between">
            <div className="flex items-center gap-3">
              <div className="w-10 h-10 bg-white/20 rounded-full flex items-center justify-center backdrop-blur">
                <span className="text-white">{currentStep + 1}</span>
              </div>
              <h2 className="text-white">Quick Start Guide</h2>
            </div>
            <button
              onClick={handleSkip}
              className="text-white/80 hover:text-white transition-colors"
              aria-label="Close guide"
            >
              <X className="w-5 h-5" />
            </button>
          </div>

          {/* Content */}
          <div className="p-8">
            <h3 className="text-white text-xl mb-3">{step.title}</h3>
            <p className="text-[#CCCCCC] leading-relaxed">{step.description}</p>

            {/* Progress Indicators */}
            <div className="flex gap-2 mt-6">
              {guideSteps.map((_, index) => (
                <div
                  key={index}
                  className={`h-1 flex-1 rounded-full transition-all duration-300 ${
                    index === currentStep
                      ? "bg-[#0066CC]"
                      : index < currentStep
                      ? "bg-[#00FF00]"
                      : "bg-[#333333]"
                  }`}
                />
              ))}
            </div>
          </div>

          {/* Footer */}
          <div className="bg-[#0f0f0f] px-6 py-4 flex items-center justify-between border-t border-[#333333]">
            <button
              onClick={handleSkip}
              className="text-[#888888] hover:text-white transition-colors text-sm"
            >
              Skip Tutorial
            </button>

            <div className="flex items-center gap-3">
              <Button
                onClick={handlePrevious}
                disabled={currentStep === 0}
                variant="outline"
                className="bg-transparent border-[#333333] text-white hover:bg-[#333333] disabled:opacity-30"
              >
                <ChevronLeft className="w-4 h-4 mr-1" />
                Previous
              </Button>

              <Button
                onClick={handleNext}
                className="bg-[#0066CC] hover:bg-[#0080FF] text-white transition-colors duration-200"
              >
                {currentStep === guideSteps.length - 1 ? (
                  "Get Started"
                ) : (
                  <>
                    Next
                    <ChevronRight className="w-4 h-4 ml-1" />
                  </>
                )}
              </Button>
            </div>
          </div>
        </div>
      </div>
    </>
  );
}
