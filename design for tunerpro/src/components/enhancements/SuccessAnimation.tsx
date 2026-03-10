import { useEffect, useState, useRef } from "react";

interface SuccessAnimationProps {
  trigger: boolean;
  message: string;
  onComplete?: () => void;
}

/**
 * Success Animation with progress bar
 * Shows completion status for ECU operations (flash, sync, etc.)
 */
export function SuccessAnimation({ trigger, message, onComplete }: SuccessAnimationProps) {
  const [isVisible, setIsVisible] = useState(false);
  const [progress, setProgress] = useState(0);
  const timersRef = useRef<{
    progressInterval?: NodeJS.Timeout;
    hideTimeout?: NodeJS.Timeout;
  }>({});

  useEffect(() => {
    // Clear any existing timers
    const clearTimers = () => {
      if (timersRef.current.progressInterval) {
        clearInterval(timersRef.current.progressInterval);
        timersRef.current.progressInterval = undefined;
      }
      if (timersRef.current.hideTimeout) {
        clearTimeout(timersRef.current.hideTimeout);
        timersRef.current.hideTimeout = undefined;
      }
    };

    // Only trigger when true
    if (trigger) {
      // Clear any existing timers first
      clearTimers();
      
      // Show and reset progress
      setIsVisible(true);
      setProgress(0);

      // Animate progress bar from 0 to 100% over 1 second
      timersRef.current.progressInterval = setInterval(() => {
        setProgress((prev) => {
          if (prev >= 100) {
            return 100;
          }
          return prev + 2; // 50 steps * 20ms = 1000ms (1 second)
        });
      }, 20);

      // Auto-hide after 1 second and call onComplete
      timersRef.current.hideTimeout = setTimeout(() => {
        setIsVisible(false);
        setProgress(0);
        clearTimers();
        onComplete?.();
      }, 1000);
    }

    // Cleanup on unmount or when trigger changes
    return clearTimers;
  }, [trigger, onComplete]);

  if (!isVisible) return null;

  return (
    <div className="fixed top-8 left-1/2 transform -translate-x-1/2 z-[400] animate-in slide-in-from-top duration-200">
      <div className="bg-[#1a1a1a] border-2 border-[#00FF00] rounded-lg px-6 py-4 shadow-2xl min-w-[300px]">
        <div className="flex flex-col gap-3">
          {/* Message */}
          <div className="text-center">
            <p className="text-[#00FF00] text-lg" style={{ textShadow: "0 0 10px rgba(0, 255, 0, 0.6)" }}>
              {message}
            </p>
          </div>
          
          {/* Progress Bar */}
          <div className="w-full h-2 bg-[#0a0a0a] rounded-full overflow-hidden">
            <div
              className="h-full bg-gradient-to-r from-[#00FF00] to-[#00CC00] transition-all duration-75 ease-linear"
              style={{
                width: `${progress}%`,
                boxShadow: "0 0 10px rgba(0, 255, 0, 0.8)",
              }}
            />
          </div>
        </div>
      </div>
    </div>
  );
}
