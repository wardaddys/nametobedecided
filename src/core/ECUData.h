/**
 * @file ECUData.h
 * @brief ECU Real-Time Data Structures — Complete 130-byte spec
 *
 * This file defines all data structures used for storing and processing
 * real-time ECU data received from the Speeduino. The RealTimeData struct
 * maps ALL 130 bytes returned by the 'A' command (LOG_ENTRY_SIZE=130).
 *
 * All multi-byte data values are little-endian.
 * Protocol framing (length, CRC) is big-endian.
 *
 * Authoritative reference: Speeduino firmware 2025.01 comms.cpp / logger.cpp
 */

#ifndef ECUDATA_H
#define ECUDATA_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <cstdint>
#include "SpeeduinoConstants.h"

/**
 * @brief Complete 130-byte real-time engine data structure
 *
 * Fields are listed in byte-offset order with their exact types and
 * conversion formulas.  parseRealTimeData() fills every field from
 * the raw 130-byte payload; accessor methods apply the physical-unit
 * conversion.
 */
struct RealTimeData {
    // =====================================================================
    //  Byte 0-3 — Timing / Status / Sync
    // =====================================================================
    uint8_t  secl;               ///< [0]  Seconds counter, wraps 255
    uint8_t  status1;            ///< [1]  INJ1-4 active, DFCO, boost cut, rev limiters
    uint8_t  engine;             ///< [2]  Running/cranking/ASE/warmup/accel/decel/MAP-accel/rev-limit
    uint8_t  syncLossCounter;    ///< [3]  Cumulative sync-loss count (never auto-resets)

    // =====================================================================
    //  Byte 4-13 — Primary sensors + corrections
    // =====================================================================
    uint16_t map;                ///< [4-5]   MAP kPa, ×1
    uint8_t  iat;                ///< [6]     IAT raw,  °C = raw − 40
    uint8_t  coolant;            ///< [7]     CLT raw,  °C = raw − 40
    uint8_t  batCorrection;      ///< [8]     Battery correction %
    uint8_t  battery10;          ///< [9]     Battery voltage, V = raw × 0.1
    uint8_t  o2;                 ///< [10]    Primary O2 AFR, AFR = raw × 0.1
    uint8_t  egoCorrection;      ///< [11]    Closed-loop correction %
    uint8_t  iatCorrection;      ///< [12]    IAT correction %
    uint8_t  wueCorrection;      ///< [13]    Warmup enrichment %

    // =====================================================================
    //  Byte 14-31 — Engine operation
    // =====================================================================
    uint16_t rpm;                ///< [14-15] RPM ×1
    uint8_t  aeAmount;           ///< [16]    Accel enrichment %, already ÷2 in FW
    uint16_t corrections;        ///< [17-18] Total gamma %, 100 = no correction
    uint8_t  ve1;                ///< [19]    Primary VE table lookup %
    uint8_t  ve2;                ///< [20]    Secondary VE table lookup %
    uint8_t  afrTarget;          ///< [21]    Target AFR, AFR = raw × 0.1
    int16_t  tpsDOT;             ///< [22-23] TPS rate-of-change %/s (signed)
    int8_t   advance;            ///< [24]    Final ignition advance °BTDC (signed)
    uint8_t  tps;                ///< [25]    TPS, % = raw × 0.5
    uint16_t loopsPerSecond;     ///< [26-27] ECU main-loop frequency Hz
    uint16_t freeRAM;            ///< [28-29] Free RAM bytes
    uint8_t  boostTarget;        ///< [30]    Boost target, kPa = raw × 2
    uint8_t  boostDuty;          ///< [31]    Boost duty, ÷100

    // =====================================================================
    //  Byte 32-41 — Status 2, rates, flex, idle, O2_2, baro
    // =====================================================================
    uint8_t  status2;            ///< [32]    Launch/idle/ethanol/flatshift/boostcut/error/sync
    int16_t  rpmDOT;             ///< [33-34] RPM rate-of-change RPM/s (signed)
    uint8_t  ethanolPct;         ///< [35]    Ethanol content % (0-100)
    uint8_t  flexCorrection;     ///< [36]    Flex fuel correction %
    int8_t   flexIgnCorrection;  ///< [37]    Flex ignition correction °(signed)
    uint8_t  idleLoad;           ///< [38]    Idle load (steps or duty%)
    uint8_t  testOutputs;        ///< [39]    Test outputs bitfield
    uint8_t  o2_2;               ///< [40]    Secondary O2 AFR, AFR = raw × 0.1
    uint8_t  baro;               ///< [41]    Barometric pressure kPa

    // =====================================================================
    //  Byte 42-75 — CAN inputs, TPS ADC, error
    // =====================================================================
    uint16_t canin[16];          ///< [42-73] 16 CAN input channels (U16LE each)
    uint8_t  tpsADC;             ///< [74]    Raw TPS ADC value
    uint8_t  errorByte;          ///< [75]    Unused (always 0)

    // =====================================================================
    //  Byte 76-83 — Injector pulse widths
    // =====================================================================
    uint16_t pw1;                ///< [76-77] Injector 1 PW µs
    uint16_t pw2;                ///< [78-79] Injector 2 PW µs
    uint16_t pw3;                ///< [80-81] Injector 3 PW µs
    uint16_t pw4;                ///< [82-83] Injector 4 PW µs

    // =====================================================================
    //  Byte 84-101 — Status 3, protect, loads, dwell, VVT1, corrections
    // =====================================================================
    uint8_t  status3;            ///< [84]    Reset-prevent / nitrous / nSquirts / halfSync
    uint8_t  engineProtectStatus;///< [85]    Protection bits (RPM/MAP/oil/AFR/coolant)
    int16_t  fuelLoad;           ///< [86-87] Fuel table load axis value (signed)
    int16_t  ignLoad;            ///< [88-89] Ign table load axis value (signed)
    uint16_t dwell;              ///< [90-91] Target dwell, ms = raw × 0.1
    uint8_t  CLIdleTarget;       ///< [92]    CL idle target, RPM = raw × 10
    int16_t  mapDOT;             ///< [93-94] MAP rate-of-change kPa/s (signed)
    int16_t  vvt1Angle;          ///< [95-96] VVT1 measured angle ° (signed)
    uint8_t  vvt1TargetAngle;    ///< [97]    VVT1 target angle °
    uint8_t  vvt1Duty;           ///< [98]    VVT1 solenoid duty %
    int16_t  flexBoostCorrection;///< [99-100] Flex boost correction kPa (signed)
    uint8_t  baroCorrection;     ///< [101]   Baro correction %

    // =====================================================================
    //  Byte 102-109 — Blended VE, ASE, VSS, gear, pressures, WMI
    // =====================================================================
    uint8_t  ve;                 ///< [102]   Final blended VE %
    uint8_t  ASEValue;           ///< [103]   After-start enrichment %
    uint16_t vss;                ///< [104-105] Vehicle speed km/h
    uint8_t  gear;               ///< [106]   Calculated gear
    uint8_t  fuelPressure;       ///< [107]   Fuel pressure PSI
    uint8_t  oilPressure;        ///< [108]   Oil pressure PSI
    uint8_t  wmiPW;              ///< [109]   WMI pulse width

    // =====================================================================
    //  Byte 110-129 — Status 4, VVT2, outputs, temps, advance, EMAP, fan,
    //                  A/C, actual dwell, status5, knock
    // =====================================================================
    uint8_t  status4;            ///< [110]   WMI empty/VVT err/fan/burn-pending/SD-active
    int16_t  vvt2Angle;          ///< [111-112] VVT2 measured angle ° (signed)
    uint8_t  vvt2TargetAngle;    ///< [113]   VVT2 target angle °
    uint8_t  vvt2Duty;           ///< [114]   VVT2 solenoid duty %
    uint8_t  outputsStatus;      ///< [115]   Programmable output states
    uint8_t  fuelTemp;           ///< [116]   Fuel temp raw, °C = raw − 40
    uint8_t  fuelTempCorrection; ///< [117]   Fuel temp correction %
    int8_t   advance1;           ///< [118]   Primary ign table timing °
    int8_t   advance2;           ///< [119]   Secondary ign table timing °
    uint8_t  TS_SD_Status;       ///< [120]   SD card status code
    int16_t  EMAP;               ///< [121-122] Exhaust MAP kPa (signed)
    uint8_t  fanDuty;            ///< [123]   Fan duty %
    uint8_t  airConStatus;       ///< [124]   A/C status bits
    uint16_t actualDwell;        ///< [125-126] Measured actual dwell µs
    uint8_t  status5;            ///< [127]   Status 5 bits
    uint8_t  knockCount;         ///< [128]   Knock event count
    uint8_t  knockRetard;        ///< [129]   Knock retard degrees

    // =====================================================================
    //  Non-protocol fields (kept for backward compatibility / dashboard use)
    // =====================================================================
    int8_t   advance3;           ///< (not in 130-byte stream, kept for UI compat)
    int8_t   advance4;           ///< (not in 130-byte stream, kept for UI compat)
    uint8_t  decoderState;       ///< (not in 'A' cmd, populated from other sources)
    uint8_t  toothCurrentCount;  ///< (not in 'A' cmd)
    uint16_t triggerToothAngle;  ///< (not in 'A' cmd)
    uint8_t  spark;              ///< (legacy compat field)

    // Timestamp of when this data was received
    QDateTime timestamp;

    /**
     * @brief Default constructor — initializes all values to safe defaults
     */
    RealTimeData()
        : secl(0), status1(0), engine(0), syncLossCounter(0),
          map(0), iat(40), coolant(40),                      // 40 = 0°C after -40
          batCorrection(100), battery10(0),
          o2(0), egoCorrection(100), iatCorrection(100),
          wueCorrection(100), afrTarget(147),                // 14.7 AFR
          rpm(0), aeAmount(0), corrections(100), ve1(0), ve2(0),
          tpsDOT(0), advance(0), tps(0),
          loopsPerSecond(0), freeRAM(0),
          boostTarget(0), boostDuty(0),
          status2(0), rpmDOT(0),
          ethanolPct(0), flexCorrection(100), flexIgnCorrection(0),
          idleLoad(0), testOutputs(0),
          o2_2(0), baro(101),
          tpsADC(0), errorByte(0),
          pw1(0), pw2(0), pw3(0), pw4(0),
          status3(0), engineProtectStatus(0),
          fuelLoad(0), ignLoad(0),
          dwell(0), CLIdleTarget(0), mapDOT(0),
          vvt1Angle(0), vvt1TargetAngle(0), vvt1Duty(0),
          flexBoostCorrection(0), baroCorrection(100),
          ve(0), ASEValue(0),
          vss(0), gear(0), fuelPressure(0), oilPressure(0), wmiPW(0),
          status4(0),
          vvt2Angle(0), vvt2TargetAngle(0), vvt2Duty(0),
          outputsStatus(0),
          fuelTemp(40), fuelTempCorrection(100),
          advance1(0), advance2(0),
          TS_SD_Status(0), EMAP(0), fanDuty(0), airConStatus(0),
          actualDwell(0), status5(0), knockCount(0), knockRetard(0),
          advance3(0), advance4(0),
          decoderState(0), toothCurrentCount(0), triggerToothAngle(0),
          spark(0),
          timestamp(QDateTime::currentDateTime()) {}


    // =========================================================================
    // VALUE ACCESSORS — Apply physical-unit conversions
    // =========================================================================

    /// RPM (direct)
    int getRPM() const { return static_cast<int>(rpm); }

    /// TPS percentage (raw × 0.5)
    double getTPS() const { return static_cast<double>(tps) * 0.5; }

    /// MAP in kPa (raw direct)
    double getMAP() const { return static_cast<double>(map); }

    /// IAT in °C (raw − 40)
    double getIAT() const { return static_cast<double>(iat) - 40.0; }

    /// CLT in °C (raw − 40)
    double getCoolant() const { return static_cast<double>(coolant) - 40.0; }

    /// Primary AFR (raw × 0.1)
    double getAFR() const { return static_cast<double>(o2) * 0.1; }

    /// Secondary AFR (raw × 0.1)
    double getAFR2() const { return static_cast<double>(o2_2) * 0.1; }

    /// Battery voltage (raw × 0.1)
    double getBatteryVoltage() const { return static_cast<double>(battery10) * 0.1; }

    /// Final ignition advance °BTDC (signed, direct)
    int getAdvance() const { return static_cast<int>(advance); }

    /// Injector 1 pulse width in ms (raw µs ÷ 1000)
    double getPulseWidth() const { return static_cast<double>(pw1) / 1000.0; }

    /// Ethanol content %
    int getFlexPercent() const { return static_cast<int>(ethanolPct); }

    /// Boost target kPa (raw × 2)
    double getBoostTarget() const { return static_cast<double>(boostTarget) * 2.0; }

    /// Dwell time in ms (raw × 0.1)
    double getDwell() const { return static_cast<double>(dwell) * 0.1; }

    /// Actual dwell in µs (direct)
    double getActualDwell() const { return static_cast<double>(actualDwell); }

    /// Vehicle speed km/h (direct)
    int getVSS() const { return static_cast<int>(vss); }

    /// Fuel temp °C (raw − 40)
    double getFuelTemp() const { return static_cast<double>(fuelTemp) - 40.0; }

    /// Exhaust MAP kPa (signed, direct)
    int getEMAP() const { return static_cast<int>(EMAP); }

    /// CL idle target RPM (raw × 10)
    int getIdleTargetRPM() const { return static_cast<int>(CLIdleTarget) * 10; }

    /// Blended VE %
    int getVE() const { return static_cast<int>(ve); }

    /// Target AFR (raw × 0.1)
    double getTargetAFR() const { return static_cast<double>(afrTarget) * 0.1; }

    // Legacy compat (old code had getMAP2 which referenced map2 — now returns EMAP)
    double getMAP2() const { return static_cast<double>(EMAP); }

    // =========================================================================
    // ENGINE STATUS HELPERS (based on SpeeduinoStatus bits)
    // =========================================================================

    bool isEngineRunning() const {
        return SPEEDUINO_BIT_CHECK(engine, SpeeduinoStatus::BIT_ENGINE_RUN);
    }

    bool isCranking() const {
        return SPEEDUINO_BIT_CHECK(engine, SpeeduinoStatus::BIT_ENGINE_CRANK);
    }

    bool isWarmingUp() const {
        return SPEEDUINO_BIT_CHECK(engine, SpeeduinoStatus::BIT_ENGINE_WARMUP);
    }

    bool isAccelerating() const {
        return SPEEDUINO_BIT_CHECK(engine, SpeeduinoStatus::BIT_ENGINE_ACC);
    }

    bool isDFCOActive() const {
        return SPEEDUINO_BIT_CHECK(status1, SpeeduinoStatus::BIT_STATUS1_DFCO);
    }

    bool isHardLimitActive() const {
        return SPEEDUINO_BIT_CHECK(engine, 7); // BIT_ENGINE_REV
    }

    bool isSoftLimitActive() const {
        // Soft limit is not a direct bit — check via rev limiter in engine byte
        return false; // Requires config-level check
    }

    bool isBoostCutActive() const {
        return SPEEDUINO_BIT_CHECK(status1, SpeeduinoStatus::BIT_STATUS1_BOOSTCUT);
    }

    bool hasTriggerSync() const {
        // Full sync = status2 bit 7
        return SPEEDUINO_BIT_CHECK(status2, 7);
    }

    bool isSequential() const {
        // Sequential requires full sync (status2 bit 7) and no half-sync only
        return hasTriggerSync() && !hasHalfSync();
    }

    bool hasHalfSync() const {
        return SPEEDUINO_BIT_CHECK(status3, 4); // BIT_STATUS3_HALFSYNC
    }

    bool hasSyncError() const {
        return syncLossCounter > 0;
    }

    bool isLaunchActive() const {
        return SPEEDUINO_BIT_CHECK(status2, SpeeduinoStatus::BIT_STATUS2_HLAUNCH) ||
               SPEEDUINO_BIT_CHECK(status2, SpeeduinoStatus::BIT_STATUS2_SLAUNCH);
    }

    bool isFlatShiftActive() const {
        return SPEEDUINO_BIT_CHECK(status2, SpeeduinoStatus::BIT_STATUS2_FLATSHIFT);
    }

    bool isFanOn() const {
        return SPEEDUINO_BIT_CHECK(status4, SpeeduinoStatus::BIT_STATUS4_FAN);
    }

    bool isBurnPending() const {
        return SPEEDUINO_BIT_CHECK(status4, SpeeduinoStatus::BIT_STATUS4_BURNPENDING);
    }

    bool isIdleActive() const {
        return SPEEDUINO_BIT_CHECK(status2, 1); // BIT_STATUS2_IDLE
    }

    bool isASEActive() const {
        return SPEEDUINO_BIT_CHECK(engine, SpeeduinoStatus::BIT_ENGINE_ASE);
    }

    /// Engine protection bitmask helpers
    bool isRPMProtectionActive() const { return engineProtectStatus & 0x01; }
    bool isBoostProtectionActive() const { return engineProtectStatus & 0x02; }
    bool isOilProtectionActive() const { return engineProtectStatus & 0x04; }
    bool isAFRProtectionActive() const { return engineProtectStatus & 0x08; }
    bool isCoolantProtectionActive() const { return engineProtectStatus & 0x10; }
    bool isAnyProtectionActive() const { return engineProtectStatus != 0; }

    /// nSquirts extraction from status3 bits 5-7
    uint8_t getNSquirts() const { return (status3 >> 5) & 0x07; }

    /// Injector duty cycle calculation (for 4-stroke)
    double getInjectorDuty(bool fourStroke = true) const {
        if (rpm == 0) return 0.0;
        double cycleTime_us = 60000000.0 / rpm;
        double duty = (static_cast<double>(pw1) * getNSquirts() / cycleTime_us) * 100.0;
        if (fourStroke) duty /= 2.0;
        return duty;
    }

    /// Disconnected sensor detection
    bool isCLTSensorFault() const { return coolant == 0 || coolant == 255; }
    bool isIATSensorFault() const { return iat == 0 || iat == 255; }
    bool isMAPSensorFault() const { return map == 0 || map == 255; }
    bool isBatteryFault() const { return battery10 < 60 || battery10 > 200; }
};

/**
 * @brief ECU identification and signature information
 */
struct ECUSignature {
    QString firmwareVersion;
    QString boardType;
    uint8_t protocolVersion;
    uint8_t pageCount;
    uint16_t blockingFactor;      ///< Max bytes per read/write chunk
    uint16_t tableBlockingFactor; ///< Max bytes per table read/write chunk

    ECUSignature() : protocolVersion(0), pageCount(0),
                     blockingFactor(121), tableBlockingFactor(64) {}

    bool isValid() const { return !firmwareVersion.isEmpty(); }

    QString toString() const {
        return QString("Speeduino %1 | Board: %2 | Protocol: v%3 | BlockFact: %4/%5")
               .arg(firmwareVersion)
               .arg(boardType)
               .arg(protocolVersion)
               .arg(blockingFactor)
               .arg(tableBlockingFactor);
    }
};

/**
 * @brief Connection status enumeration
 */
enum class ConnectionStatus {
    Disconnected,
    Connecting,
    Connected,
    Error
};

inline QString connectionStatusToString(ConnectionStatus status) {
    switch (status) {
        case ConnectionStatus::Disconnected: return "Disconnected";
        case ConnectionStatus::Connecting:   return "Connecting...";
        case ConnectionStatus::Connected:    return "Connected";
        case ConnectionStatus::Error:        return "Error";
        default:                            return "Unknown";
    }
}

#endif // ECUDATA_H
