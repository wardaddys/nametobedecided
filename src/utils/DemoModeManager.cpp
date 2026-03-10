/**
 * @file DemoModeManager.cpp
 * @brief Implementation of demo mode manager
 * 
 * @author Safeerullah Afridi (FA-22-151)
 * @author Muhammad Saeed Sajid (FA-22-143)
 * @supervisor Prof. Dr. Shariq Hussain
 * @date December 2025
 */

#include "DemoModeManager.h"
#include <QtMath>

DemoModeManager& DemoModeManager::instance() {
    static DemoModeManager instance;
    return instance;
}

DemoModeManager::DemoModeManager(QObject *parent)
    : QObject(parent),
      m_timer(new QTimer(this)),
      m_isActive(false),
      m_currentRPM(800),
      m_targetRPM(800),
      m_maxRPM(8000),
      m_rpmChangeRate(200),
      m_currentGear(1),
      m_currentSpeed(0),
      m_throttlePosition(0),
      m_coolantTemp(20),
      m_intakeTemp(25),
      m_engineRunTime(0),
      m_batteryVoltage(13.8),
      m_mapValue(100),
      m_random(QRandomGenerator::global())
{
    connect(m_timer, &QTimer::timeout, this, &DemoModeManager::generateData);
}

DemoModeManager::~DemoModeManager() {
    stop();
}

void DemoModeManager::start(int updateIntervalMs) {
    if (m_isActive) {
        return;
    }
    
    // Reset to idle state
    m_currentRPM = 800;
    m_targetRPM = 800;
    m_currentGear = 1;
    m_currentSpeed = 0;
    m_throttlePosition = 0;
    m_coolantTemp = 20;
    m_intakeTemp = 25;
    m_engineRunTime = 0;
    m_batteryVoltage = 13.8;
    
    m_isActive = true;
    m_timer->start(updateIntervalMs);
}

void DemoModeManager::stop() {
    if (!m_isActive) {
        return;
    }
    
    m_timer->stop();
    m_isActive = false;
}

bool DemoModeManager::isActive() const {
    return m_isActive;
}

void DemoModeManager::setSimulationParameters(int maxRPM, int rpmChangeRate) {
    m_maxRPM = maxRPM;
    m_rpmChangeRate = rpmChangeRate;
}

void DemoModeManager::generateData() {
    // Simulate realistic driving scenario
    
    // 1. Update target RPM based on random "throttle" input
    // Create a realistic driving pattern with acceleration/deceleration
    static int cycleCounter = 0;
    cycleCounter++;
    
    if (cycleCounter % 100 == 0) { // Every 5 seconds (at 20Hz)
        // Randomly decide to accelerate, cruise, or decelerate
        int action = m_random->bounded(3);
        
        if (action == 0) { // Accelerate
            m_targetRPM = m_random->bounded(2000, m_maxRPM);
            m_throttlePosition = m_random->bounded(50, 100);
        } else if (action == 1) { // Cruise
            m_targetRPM = m_random->bounded(1500, 4000);
            m_throttlePosition = m_random->bounded(20, 50);
        } else { // Decelerate
            m_targetRPM = m_random->bounded(800, 2000);
            m_throttlePosition = m_random->bounded(0, 30);
        }
    }
    
    // 2. Smooth RPM transition
    if (m_currentRPM < m_targetRPM) {
        m_currentRPM += m_rpmChangeRate;
        if (m_currentRPM > m_targetRPM) {
            m_currentRPM = m_targetRPM;
        }
    } else if (m_currentRPM > m_targetRPM) {
        m_currentRPM -= m_rpmChangeRate;
        if (m_currentRPM < m_targetRPM) {
            m_currentRPM = m_targetRPM;
        }
    }
    
    // Keep RPM in valid range
    if (m_currentRPM < 800) m_currentRPM = 800;
    if (m_currentRPM > m_maxRPM) m_currentRPM = m_maxRPM;
    
    // 3. Simulate gear changes
    if (m_currentRPM > 6500 && m_currentGear < 6) {
        m_currentGear++;
        m_currentRPM = 4000; // Drop RPM after shift
    } else if (m_currentRPM < 1500 && m_currentGear > 1) {
        m_currentGear--;
        m_currentRPM = 3000; // Raise RPM after downshift
    }
    
    // 4. Calculate speed
    m_currentSpeed = calculateSpeed(m_currentRPM, m_currentGear);
    
    // 5. Simulate temperature
    m_coolantTemp = simulateTemperature();
    m_intakeTemp = 25 + m_random->bounded(-5, 15); // Ambient ±5°C
    
    // 6. Simulate MAP
    m_mapValue = simulateMAP(m_throttlePosition);
    
    // 7. Update engine runtime
    m_engineRunTime++;
    
    // 8. Simulate battery voltage (13.5-14.5V when running)
    m_batteryVoltage = 13.8 + (m_random->bounded(-50, 70) / 100.0);
    
    // 9. Build RealTimeData structure
    RealTimeData data;
    data.secl = m_engineRunTime & 0xFF;
    data.status1 = 0x01; // Engine running
    data.engine = (m_currentRPM > 6000) ? 0x01 : 0x00; // VTEC on above 6000 RPM
    data.dwell = 30; // 3.0ms dwell (raw ×0.1ms)
    data.map = m_mapValue;
    data.iat = static_cast<uint8_t>(m_intakeTemp + 40); // Convert to offset format
    data.coolant = static_cast<uint8_t>(m_coolantTemp + 40); // Convert to offset format
    data.batCorrection = static_cast<int>((m_batteryVoltage - 6.0) * 10);
    data.battery10 = static_cast<int>(m_batteryVoltage * 10);
    data.o2 = 145 + m_random->bounded(-10, 10); // Lambda ~1.0
    data.egoCorrection = 100; // No correction
    data.iatCorrection = 100;
    data.wueCorrection = 100;
    data.rpm = m_currentRPM;
    data.aeAmount = 0;
    data.corrections = 100;
    data.ve = 80 + m_random->bounded(-5, 5);
    data.afrTarget = 147; // 14.7 AFR
    data.pw1 = static_cast<uint16_t>((m_currentRPM * m_throttlePosition) / 100); // Pulse width µs
    data.tpsDOT = 0;
    data.advance = static_cast<int8_t>(15 + (m_currentRPM / 500)); // Timing advance
    data.tps = static_cast<uint8_t>(m_throttlePosition * 2); // raw ×0.5 = %, so ×2 for raw
    data.loopsPerSecond = 200;
    data.freeRAM = 1500;
    data.boostTarget = 100;
    data.boostDuty = 0;
    data.spark = 0;
    
    // Emit the generated data
    emit dataGenerated(data);
}

int DemoModeManager::calculateSpeed(int rpm, int gear) {
    // Realistic gear ratios (approximation)
    const double gearRatios[] = {0.0, 3.8, 2.2, 1.5, 1.1, 0.9, 0.7};
    const double finalDrive = 4.1;
    const double wheelCircumference = 1.9; // meters
    
    if (gear < 1 || gear > 6) {
        return 0;
    }
    
    // Calculate wheel RPM
    double wheelRPM = rpm / (gearRatios[gear] * finalDrive);
    
    // Convert to km/h
    double speedKmh = (wheelRPM * wheelCircumference * 60) / 1000.0;
    
    return static_cast<int>(speedKmh);
}

int DemoModeManager::simulateTemperature() {
    // Simulate gradual heating from cold start to operating temperature
    const int targetTemp = 90; // °C
    const int ambientTemp = 20; // °C
    
    // Heat up slowly
    if (m_coolantTemp < targetTemp) {
        // Faster warmup at higher RPM
        int heatRate = (m_currentRPM > 3000) ? 2 : 1;
        m_coolantTemp += heatRate;
    } else if (m_coolantTemp > targetTemp) {
        // Cool down if overheating
        m_coolantTemp--;
    }
    
    // Add small random variation
    int variation = m_random->bounded(-1, 2);
    return m_coolantTemp + variation;
}

int DemoModeManager::simulateMAP(int throttle) {
    // MAP simulation:
    // - Idle/low throttle: ~30-40 kPa (vacuum)
    // - Full throttle: ~100 kPa (atmospheric)
    // - Boost: >100 kPa
    
    int baseMAP = 30 + (throttle * 70 / 100); // Linear relationship
    
    // Add small variation
    int variation = m_random->bounded(-5, 5);
    
    return qBound(20, baseMAP + variation, 250);
}
