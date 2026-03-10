import { motion, AnimatePresence } from "motion/react";
import { AlertTriangle, X, AlertCircle, Info } from "lucide-react";
import { ValidationResult } from "./SensorValidation";

interface ValidationAlertProps {
  validation: ValidationResult;
  sensorName: string;
  onDismiss?: () => void;
  persistent?: boolean;
}

export function ValidationAlert({ validation, sensorName, onDismiss, persistent = false }: ValidationAlertProps) {
  if (validation.level === 'normal' && !persistent) return null;

  const getIcon = () => {
    switch (validation.level) {
      case 'critical':
        return <AlertTriangle className="w-5 h-5" />;
      case 'caution':
        return <AlertCircle className="w-5 h-5" />;
      case 'fault':
        return <AlertCircle className="w-5 h-5" />;
      default:
        return <Info className="w-5 h-5" />;
    }
  };

  const getBgColor = () => {
    switch (validation.level) {
      case 'critical':
        return 'bg-red-950/90 border-red-600';
      case 'caution':
        return 'bg-yellow-950/90 border-yellow-600';
      case 'fault':
        return 'bg-gray-800/90 border-gray-600';
      default:
        return 'bg-green-950/90 border-green-600';
    }
  };

  const getTextColor = () => {
    switch (validation.level) {
      case 'critical':
        return 'text-red-200';
      case 'caution':
        return 'text-yellow-200';
      case 'fault':
        return 'text-gray-300';
      default:
        return 'text-green-200';
    }
  };

  return (
    <AnimatePresence>
      {validation.shouldAlert && (
        <motion.div
          initial={{ opacity: 0, y: -20, scale: 0.9 }}
          animate={{ opacity: 1, y: 0, scale: 1 }}
          exit={{ opacity: 0, y: -20, scale: 0.9 }}
          className={`${getBgColor()} ${getTextColor()} border-2 rounded-lg p-4 shadow-2xl`}
        >
          <div className="flex items-start gap-3">
            <motion.div
              animate={validation.level === 'critical' ? {
                rotate: [0, 10, -10, 10, 0],
                scale: [1, 1.1, 1]
              } : {}}
              transition={{ duration: 0.5, repeat: validation.level === 'critical' ? Infinity : 0, repeatDelay: 2 }}
              style={{ color: validation.color }}
            >
              {getIcon()}
            </motion.div>
            
            <div className="flex-1">
              <div className="flex items-center gap-2 mb-1">
                <h4 className="font-data uppercase tracking-wide">
                  {validation.level === 'critical' ? 'CRITICAL ALERT' : 
                   validation.level === 'caution' ? 'WARNING' : 'NOTICE'}
                </h4>
                <span className="text-xs px-2 py-0.5 rounded bg-black/30">
                  {sensorName}
                </span>
              </div>
              <p className="text-sm">{validation.message}</p>
              
              {validation.level === 'critical' && (
                <div className="mt-2 text-xs bg-black/40 p-2 rounded border border-red-900/50">
                  <strong>Recommended Action:</strong> Reduce throttle immediately and check sensor calibration
                </div>
              )}
            </div>

            {onDismiss && (
              <button
                onClick={onDismiss}
                className="text-current hover:bg-black/30 p-1 rounded transition-colors"
              >
                <X className="w-4 h-4" />
              </button>
            )}
          </div>
        </motion.div>
      )}
    </AnimatePresence>
  );
}

// Alert stack component for multiple alerts
interface ValidationAlertStackProps {
  alerts: Array<{
    id: string;
    validation: ValidationResult;
    sensorName: string;
  }>;
  onDismiss?: (id: string) => void;
  maxVisible?: number;
}

export function ValidationAlertStack({ alerts, onDismiss, maxVisible = 3 }: ValidationAlertStackProps) {
  // Sort by severity (critical first)
  const sortedAlerts = [...alerts].sort((a, b) => {
    const severity = { critical: 3, caution: 2, fault: 1, normal: 0 };
    return (severity[b.validation.level] || 0) - (severity[a.validation.level] || 0);
  });

  const visibleAlerts = sortedAlerts.slice(0, maxVisible);
  const hiddenCount = sortedAlerts.length - maxVisible;

  return (
    <div className="fixed top-20 right-4 z-50 space-y-2 max-w-md">
      <AnimatePresence mode="popLayout">
        {visibleAlerts.map((alert, index) => (
          <motion.div
            key={alert.id}
            layout
            initial={{ opacity: 0, x: 100 }}
            animate={{ opacity: 1, x: 0 }}
            exit={{ opacity: 0, x: 100 }}
            transition={{ delay: index * 0.1 }}
          >
            <ValidationAlert
              validation={alert.validation}
              sensorName={alert.sensorName}
              onDismiss={onDismiss ? () => onDismiss(alert.id) : undefined}
            />
          </motion.div>
        ))}
      </AnimatePresence>

      {hiddenCount > 0 && (
        <motion.div
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          className="bg-[#1a1a1a] border border-[#444444] rounded p-2 text-center text-xs text-[#888888]"
        >
          +{hiddenCount} more alert{hiddenCount !== 1 ? 's' : ''}
        </motion.div>
      )}
    </div>
  );
}
