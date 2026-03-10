// This file creates all remaining stub panel components

import { Card } from "../../ui/card";

interface BaseSettingsProps {
  onSettingsChange: () => void;
}

export function TachoOutputSettings({ onSettingsChange }: BaseSettingsProps) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">📊</span>
        Tacho Output Settings
      </h3>
      <p className="text-[#888888]">Configure tachometer output pulses per revolution and pin assignment.</p>
    </Card>
  );
}

export function FanControlSettings({ onSettingsChange }: BaseSettingsProps) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">🌀</span>
        Fan Control Settings
      </h3>
      <p className="text-[#888888]">Configure fan enable/disable temperatures and hysteresis.</p>
    </Card>
  );
}

export function TorqueConvertorSettings({ onSettingsChange }: BaseSettingsProps) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">🔒</span>
        Torque Convertor Lockup
      </h3>
      <p className="text-[#888888]">Configure automatic transmission torque converter lockup conditions.</p>
    </Card>
  );
}

export function AlternatorControlSettings({ onSettingsChange }: BaseSettingsProps) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">⚡</span>
        Alternator Control Settings
      </h3>
      <p className="text-[#888888]">Configure target voltage and PWM frequency for alternator control.</p>
    </Card>
  );
}

export function BarometricCorrectionSettings({ onSettingsChange }: BaseSettingsProps) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">🌤️</span>
        Barometric Correction
      </h3>
      <p className="text-[#888888]">Configure automatic altitude/barometric pressure compensation.</p>
    </Card>
  );
}

export function IATMAPSampleSettings({ onSettingsChange }: BaseSettingsProps) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">📈</span>
        IAT/MAP Sample Settings
      </h3>
      <p className="text-[#888888]">Configure sample rate and filtering for IAT and MAP sensors.</p>
    </Card>
  );
}

export function ITBLoadSettings({ onSettingsChange }: BaseSettingsProps) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">🎛️</span>
        ITB Load Settings
      </h3>
      <p className="text-[#888888]">Configure Individual Throttle Body load calculation and TPS synchronization.</p>
    </Card>
  );
}

export function MAFSettings({ onSettingsChange, activeSubPanel }: BaseSettingsProps & { activeSubPanel?: string }) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">📊</span>
        MAF Settings
      </h3>
      <p className="text-[#888888]">Configure Mass Air Flow sensor calibration and correction tables.</p>
    </Card>
  );
}

export function MATAirDensitySettings({ onSettingsChange }: BaseSettingsProps) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">🌡️</span>
        MAT Air Density Table
      </h3>
      <p className="text-[#888888]">Configure manifold air temperature vs air density compensation.</p>
    </Card>
  );
}

export function MATCLTCorrectionSettings({ onSettingsChange }: BaseSettingsProps) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">📉</span>
        MAT/CLT Correction
      </h3>
      <p className="text-[#888888]">Configure temperature-based fuel correction curves.</p>
    </Card>
  );
}

export function RealtimeDisplaySettings({ onSettingsChange }: BaseSettingsProps) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">📺</span>
        Realtime Display Settings
      </h3>
      <p className="text-[#888888]">Configure dashboard gauge layout and alert thresholds.</p>
    </Card>
  );
}

export function IOPinsFeatureList({ onSettingsChange }: BaseSettingsProps) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">📍</span>
        Feature List Showing IO pins
      </h3>
      <p className="text-[#888888]">Visual pin assignment diagram with status indicators.</p>
    </Card>
  );
}

export function IOPinsUsageSettings({ onSettingsChange, listNumber }: BaseSettingsProps & { listNumber?: number }) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">🔌</span>
        IO pins List/{listNumber} Usage and Status
      </h3>
      <p className="text-[#888888]">Real-time pin state monitoring and assignment management.</p>
    </Card>
  );
}

export function CANLoopSettings({ onSettingsChange }: BaseSettingsProps) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">🔗</span>
        CANloop Usage & Status
      </h3>
      <p className="text-[#888888]">Configure CAN bus devices and monitor message traffic.</p>
    </Card>
  );
}

export function ExpansionIOSettings({ onSettingsChange }: BaseSettingsProps) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">➕</span>
        Expansion IO pins
      </h3>
      <p className="text-[#888888]">Configure external expansion modules and additional I/O pins.</p>
    </Card>
  );
}

export function GaugeSettingsLimits({ onSettingsChange }: BaseSettingsProps) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">⚠️</span>
        Gauge and Settings Limits
      </h3>
      <p className="text-[#888888]">Configure minimum/maximum values and warning thresholds for all parameters.</p>
    </Card>
  );
}
