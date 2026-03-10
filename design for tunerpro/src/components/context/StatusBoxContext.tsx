import { createContext, useContext, useState, ReactNode } from "react";

export interface StatusBoxConfig {
  id: string;
  label: string;
  position: "top" | "bottom";
  enabled: boolean;
  type: "ecu" | "fuel" | "ignition" | "boost" | "vtec" | "launch" | "traction" | "revlimit";
}

interface StatusBoxContextType {
  statusBoxes: StatusBoxConfig[];
  updateStatusBox: (id: string, updates: Partial<StatusBoxConfig>) => void;
  toggleStatusBox: (id: string) => void;
}

const defaultStatusBoxes: StatusBoxConfig[] = [
  { id: "ecu", label: "ECU COMM", position: "top", enabled: true, type: "ecu" },
  { id: "fuel", label: "FUEL SYSTEM", position: "top", enabled: true, type: "fuel" },
  { id: "ignition", label: "IGNITION", position: "top", enabled: true, type: "ignition" },
  { id: "boost", label: "BOOST", position: "top", enabled: true, type: "boost" },
  { id: "vtec", label: "VTEC", position: "bottom", enabled: true, type: "vtec" },
  { id: "launch", label: "LAUNCH", position: "bottom", enabled: true, type: "launch" },
  { id: "traction", label: "TRACTION", position: "bottom", enabled: true, type: "traction" },
  { id: "revlimit", label: "REV LIMIT", position: "bottom", enabled: true, type: "revlimit" },
];

const StatusBoxContext = createContext<StatusBoxContextType | undefined>(undefined);

export function StatusBoxProvider({ children }: { children: ReactNode }) {
  const [statusBoxes, setStatusBoxes] = useState<StatusBoxConfig[]>(defaultStatusBoxes);

  const updateStatusBox = (id: string, updates: Partial<StatusBoxConfig>) => {
    setStatusBoxes((prev) =>
      prev.map((box) => (box.id === id ? { ...box, ...updates } : box))
    );
  };

  const toggleStatusBox = (id: string) => {
    setStatusBoxes((prev) =>
      prev.map((box) => (box.id === id ? { ...box, enabled: !box.enabled } : box))
    );
  };

  return (
    <StatusBoxContext.Provider value={{ statusBoxes, updateStatusBox, toggleStatusBox }}>
      {children}
    </StatusBoxContext.Provider>
  );
}

export function useStatusBoxes() {
  const context = useContext(StatusBoxContext);
  if (context === undefined) {
    throw new Error("useStatusBoxes must be used within a StatusBoxProvider");
  }
  return context;
}
