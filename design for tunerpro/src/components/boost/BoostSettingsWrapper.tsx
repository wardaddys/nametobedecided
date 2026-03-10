import { useEngineConfig } from '../context/EngineConfigContext';
import { ComprehensiveBoostSettings } from './ComprehensiveBoostSettings';

interface BoostSettingsWrapperProps {
  liveTuning: boolean;
}

export function BoostSettingsWrapper({ liveTuning }: BoostSettingsWrapperProps) {
  const { config, setForcedInductionType } = useEngineConfig();
  
  // Default to single-turbo if no forced induction type is set
  const forcedInductionType = config.forcedInductionType || 'single-turbo';

  return (
    <ComprehensiveBoostSettings 
      liveTuning={liveTuning}
      forcedInductionType={forcedInductionType}
      onConfigChange={setForcedInductionType}
    />
  );
}
