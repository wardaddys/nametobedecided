import { Card } from "../../ui/card";

interface EngineStateSettingsProps {
  onSettingsChange: () => void;
}

export function EngineStateSettings({ onSettingsChange }: EngineStateSettingsProps) {
  return (
    <Card className="glass-panel-strong bg-gradient-to-br from-[#1a1a2e] to-[#16213e] border-2 border-[#0088FF]/40 p-6">
      <h3 className="text-white mb-4 flex items-center gap-2 text-xl">
        <span className="text-2xl">🌡️</span>
        Engine State Settings
      </h3>
      <p className="text-[#888888]">Cranking detection, warm-up conditions, and after-start enrichment settings.</p>
    </Card>
  );
}
