import { Toaster as Sonner } from "sonner@2.0.3";

const Toaster = ({ ...props }) => {
  return (
    <Sonner
      theme="dark"
      className="toaster group"
      toastOptions={{
        style: {
          background: "#2a2a2a",
          color: "#ffffff",
          border: "1px solid #444444",
        },
      }}
      {...props}
    />
  );
};

export { Toaster };