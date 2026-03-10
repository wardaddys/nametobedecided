import { createContext, useContext, useState, ReactNode } from 'react';

interface VTECContextType {
  vtecEnabled: boolean;
  setVtecEnabled: (enabled: boolean) => void;
  currentMap: 'low' | 'high';
  setCurrentMap: (map: 'low' | 'high') => void;
}

const VTECContext = createContext<VTECContextType | undefined>(undefined);

export function VTECProvider({ children }: { children: ReactNode }) {
  const [vtecEnabled, setVtecEnabled] = useState(false);
  const [currentMap, setCurrentMap] = useState<'low' | 'high'>('low');

  return (
    <VTECContext.Provider value={{ vtecEnabled, setVtecEnabled, currentMap, setCurrentMap }}>
      {children}
    </VTECContext.Provider>
  );
}

export function useVTEC() {
  const context = useContext(VTECContext);
  if (context === undefined) {
    throw new Error('useVTEC must be used within a VTECProvider');
  }
  return context;
}
