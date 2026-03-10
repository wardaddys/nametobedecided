/**
 * @file DemoModeManager.h
 * @brief Demo mode manager for simulating ECU data
 * 
 * Provides realistic simulation of Speeduino ECU data for testing
 * and demonstration purposes without requiring physical hardware.
 * 
 * @author Safeerullah Afridi (FA-22-151)
 * @author Muhammad Saeed Sajid (FA-22-143)
 * @supervisor Prof. Dr. Shariq Hussain
 * @date December 2025
 */

#ifndef DEMOMODEMANAGER_H
#define DEMOMODEMANAGER_H

#include <QObject>
#include <QTimer>
#include <QRandomGenerator>
#include "core/ECUData.h"

/**
 * @class DemoModeManager
 * @brief Singleton class for generating simulated ECU data
 * 
 * Features:
 * - Realistic RPM simulation with smooth transitions
 * - Speed calculation based on simulated gear ratios
 * - Temperature gradients (coolant, intake air)
 * - Manifold pressure simulation
 * - Battery voltage variations
 * - Fuel and ignition parameters
 */
class DemoModeManager : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief Get singleton instance
     * @return Reference to the singleton instance
     */
    static DemoModeManager& instance();
    
    /**
     * @brief Start demo mode simulation
     * @param updateIntervalMs Update interval in milliseconds (default: 50ms = 20Hz)
     */
    void start(int updateIntervalMs = 50);
    
    /**
     * @brief Stop demo mode simulation
     */
    void stop();
    
    /**
     * @brief Check if demo mode is active
     * @return true if simulation is running
     */
    bool isActive() const;
    
    /**
     * @brief Set simulation parameters
     * @param maxRPM Maximum RPM for simulation (default: 8000)
     * @param rpmChangeRate RPM change rate (default: 200 RPM/update)
     */
    void setSimulationParameters(int maxRPM = 8000, int rpmChangeRate = 200);
    
signals:
    /**
     * @brief Emitted when new simulated data is available
     * @param data Simulated ECU data
     */
    void dataGenerated(const RealTimeData &data);
    
private:
    // Singleton pattern - private constructor
    explicit DemoModeManager(QObject *parent = nullptr);
    ~DemoModeManager();
    
    // Prevent copying
    DemoModeManager(const DemoModeManager&) = delete;
    DemoModeManager& operator=(const DemoModeManager&) = delete;
    
    /**
     * @brief Generate next frame of simulated data
     */
    void generateData();
    
    /**
     * @brief Calculate realistic speed based on RPM and gear
     * @param rpm Current RPM
     * @param gear Current gear (1-6)
     * @return Speed in km/h
     */
    int calculateSpeed(int rpm, int gear);
    
    /**
     * @brief Simulate engine temperature heating/cooling
     * @return Coolant temperature in °C
     */
    int simulateTemperature();
    
    /**
     * @brief Simulate manifold pressure based on throttle
     * @param throttle Throttle position (0-100%)
     * @return MAP in kPa
     */
    int simulateMAP(int throttle);
    
    // Simulation state
    QTimer *m_timer;
    bool m_isActive;
    
    // Engine parameters
    int m_currentRPM;
    int m_targetRPM;
    int m_maxRPM;
    int m_rpmChangeRate;
    
    // Driving simulation
    int m_currentGear;
    int m_currentSpeed;
    int m_throttlePosition;
    
    // Temperature simulation
    int m_coolantTemp;
    int m_intakeTemp;
    int m_engineRunTime; // seconds
    
    // Other parameters
    double m_batteryVoltage;
    int m_mapValue;
    
    // Random generator
    QRandomGenerator *m_random;
};

#endif // DEMOMODEMANAGER_H
