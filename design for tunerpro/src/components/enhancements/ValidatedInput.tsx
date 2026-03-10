import { useState, useEffect, InputHTMLAttributes } from "react";
import { AlertCircle, CheckCircle2, AlertTriangle } from "lucide-react";
import { TooltipWrapper } from "./TooltipWrapper";

interface ValidatedInputProps extends Omit<InputHTMLAttributes<HTMLInputElement>, "onChange"> {
  value: number | string;
  onChange: (value: number | string) => void;
  min?: number;
  max?: number;
  step?: number;
  unit?: string;
  warningThreshold?: number;
  dangerThreshold?: number;
  label?: string;
  helpText?: string;
  validateOnChange?: boolean;
}

type ValidationState = "valid" | "warning" | "error" | "neutral";

export function ValidatedInput({
  value,
  onChange,
  min,
  max,
  step = 0.1,
  unit,
  warningThreshold,
  dangerThreshold,
  label,
  helpText,
  validateOnChange = true,
  className = "",
  ...props
}: ValidatedInputProps) {
  const [validationState, setValidationState] = useState<ValidationState>("neutral");
  const [validationMessage, setValidationMessage] = useState("");
  const [isFocused, setIsFocused] = useState(false);

  useEffect(() => {
    if (!validateOnChange && !isFocused) return;

    const numValue = typeof value === "string" ? parseFloat(value) : value;

    if (isNaN(numValue)) {
      setValidationState("error");
      setValidationMessage("Invalid number");
      return;
    }

    // Check min/max bounds
    if (min !== undefined && numValue < min) {
      setValidationState("error");
      setValidationMessage(`Value must be at least ${min}${unit || ""}`);
      return;
    }

    if (max !== undefined && numValue > max) {
      setValidationState("error");
      setValidationMessage(`Value must not exceed ${max}${unit || ""}`);
      return;
    }

    // Check danger threshold
    if (dangerThreshold !== undefined && numValue >= dangerThreshold) {
      setValidationState("error");
      setValidationMessage(`⚠ Danger zone! Maximum safe value: ${dangerThreshold}${unit || ""}`);
      return;
    }

    // Check warning threshold
    if (warningThreshold !== undefined && numValue >= warningThreshold) {
      setValidationState("warning");
      setValidationMessage(`Approaching limit. Recommended max: ${warningThreshold}${unit || ""}`);
      return;
    }

    setValidationState("valid");
    setValidationMessage("Value OK");
  }, [value, min, max, warningThreshold, dangerThreshold, unit, validateOnChange, isFocused]);

  const handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const newValue = e.target.value;
    onChange(props.type === "number" ? parseFloat(newValue) : newValue);
  };

  const handleIncrement = () => {
    const numValue = typeof value === "string" ? parseFloat(value) : value;
    const newValue = numValue + (step || 1);
    if (max === undefined || newValue <= max) {
      onChange(newValue);
    }
  };

  const handleDecrement = () => {
    const numValue = typeof value === "string" ? parseFloat(value) : value;
    const newValue = numValue - (step || 1);
    if (min === undefined || newValue >= min) {
      onChange(newValue);
    }
  };

  const getInputClassName = () => {
    const baseClass = "w-full px-3 py-2 bg-[#000000] border rounded text-white font-mono transition-all duration-200";
    
    switch (validationState) {
      case "valid":
        return `${baseClass} input-valid border-[#00FF00]`;
      case "warning":
        return `${baseClass} input-warning border-[#FFFF00]`;
      case "error":
        return `${baseClass} input-invalid border-[#FF0000]`;
      default:
        return `${baseClass} border-[#333333] focus:border-[#0066CC]`;
    }
  };

  const ValidationIcon = () => {
    switch (validationState) {
      case "valid":
        return <CheckCircle2 className="w-4 h-4 text-[#00FF00]" />;
      case "warning":
        return <AlertTriangle className="w-4 h-4 text-[#FFFF00]" />;
      case "error":
        return <AlertCircle className="w-4 h-4 text-[#FF0000]" />;
      default:
        return null;
    }
  };

  return (
    <div className="space-y-1">
      {label && (
        <label className="flex items-center gap-2 text-sm text-[#CCCCCC]">
          {label}
          {helpText && (
            <TooltipWrapper content={helpText}>
              <div className="w-4 h-4 rounded-full bg-[#333333] flex items-center justify-center cursor-help text-xs">
                ?
              </div>
            </TooltipWrapper>
          )}
        </label>
      )}

      <div className="relative">
        <input
          {...props}
          value={value}
          onChange={handleChange}
          onFocus={() => setIsFocused(true)}
          onBlur={() => setIsFocused(false)}
          className={`${getInputClassName()} ${className} pr-20`}
          aria-label={label}
          aria-invalid={validationState === "error"}
          aria-describedby={validationMessage ? `validation-${label}` : undefined}
        />

        {/* Unit Label */}
        {unit && (
          <div className="absolute right-10 top-1/2 -translate-y-1/2 text-xs text-[#888888] font-mono pointer-events-none">
            {unit}
          </div>
        )}

        {/* Validation Icon */}
        {validationState !== "neutral" && (
          <TooltipWrapper content={validationMessage}>
            <div className="absolute right-2 top-1/2 -translate-y-1/2">
              <ValidationIcon />
            </div>
          </TooltipWrapper>
        )}

        {/* Spinner Controls (for number inputs) */}
        {props.type === "number" && (
          <div className="absolute right-12 top-1/2 -translate-y-1/2 flex flex-col">
            <button
              type="button"
              onClick={handleIncrement}
              className="px-1 py-0 text-[#888888] hover:text-white transition-colors text-xs"
              aria-label="Increment"
            >
              ▲
            </button>
            <button
              type="button"
              onClick={handleDecrement}
              className="px-1 py-0 text-[#888888] hover:text-white transition-colors text-xs"
              aria-label="Decrement"
            >
              ▼
            </button>
          </div>
        )}
      </div>

      {/* Validation Message */}
      {validationState !== "neutral" && validationState !== "valid" && (
        <div
          id={`validation-${label}`}
          className={`text-xs mt-1 ${
            validationState === "warning" ? "text-[#FFFF00]" : "text-[#FF0000]"
          }`}
          role="alert"
        >
          {validationMessage}
        </div>
      )}

      {/* Min/Max Range Indicator */}
      {min !== undefined && max !== undefined && (
        <div className="flex items-center gap-2 text-xs text-[#666666] mt-1">
          <span>Range:</span>
          <span className="font-mono">{min}</span>
          <div className="flex-1 h-1 bg-[#333333] rounded relative overflow-hidden">
            <div
              className={`absolute h-full ${
                validationState === "error"
                  ? "bg-[#FF0000]"
                  : validationState === "warning"
                  ? "bg-[#FFFF00]"
                  : "bg-[#00FF00]"
              }`}
              style={{
                width: `${((typeof value === "number" ? value : parseFloat(value)) - min) / (max - min) * 100}%`,
              }}
            />
          </div>
          <span className="font-mono">{max}</span>
        </div>
      )}
    </div>
  );
}
