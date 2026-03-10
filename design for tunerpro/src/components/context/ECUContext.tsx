import { createContext, useContext, useState, ReactNode } from 'react';
import { ECUConfig, getECUById } from './ECUDatabase';

interface ECUContextType {
  selectedECU: ECUConfig | null;
  selectECU: (ecuId: string) => void;
  clearECU: () => void;
}

const ECUContext = createContext<ECUContextType | undefined>(undefined);

export function ECUProvider({ children }: { children: ReactNode }) {
  const [selectedECU, setSelectedECU] = useState<ECUConfig | null>(null);

  const selectECU = (ecuId: string) => {
    const ecu = getECUById(ecuId);
    if (ecu) {
      setSelectedECU(ecu);
      // You could add toast notification here
      console.log(`ECU selected: ${ecu.name}`);
    }
  };

  const clearECU = () => {
    setSelectedECU(null);
  };

  return (
    <ECUContext.Provider value={{ selectedECU, selectECU, clearECU }}>
      {children}
    </ECUContext.Provider>
  );
}

export function useECU() {
  const context = useContext(ECUContext);
  if (context === undefined) {
    throw new Error('useECU must be used within an ECUProvider');
  }
  return context;
}
