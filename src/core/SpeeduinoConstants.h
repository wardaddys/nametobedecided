/**
 * @file SpeeduinoConstants.h
 * @brief Speeduino Firmware Constants from Official Doxygen Documentation
 *
 * This file contains all constants extracted from the official Speeduino
 * Doxygen documentation at https://speeduino.github.io/speeduino-doxygen/
 *
 * Reference: docsbyme/Speeduino Doxygen Source Files/
 *
 * @author Generated from Speeduino Doxygen - TunerPro Integration
 * @date January 2026
 */

#ifndef SPEEDUINOCONSTANTS_H
#define SPEEDUINOCONSTANTS_H

#include <cstdint>

// ============================================================================
// SERIAL COMMANDS (from comms.h)
// Reference: docsbyme/Speeduino Doxygen Source Files/01 - Communications Protocol.md
// ============================================================================

// SpeeduinoCommands namespace removed to prevent redefinitions with SpeeduinoProtocol.h

// ============================================================================
// DECODER TYPES (from decoders.h)
// Reference: docsbyme/Speeduino Doxygen Source Files/02 - Trigger Decoders.md
// ============================================================================

namespace SpeeduinoDecoders {

/// @brief Trigger decoder types - all 25+ patterns from Speeduino firmware
enum class DecoderType : uint8_t {
    MissingTooth = 0,        ///< Generic missing tooth (36-1, 60-2, etc.)
    BasicDistributor = 1,    ///< Simple distributor pickup
    DualWheel = 2,           ///< Crank + Cam wheels
    GM7X = 3,                ///< GM 7-tooth crank pattern
    Mitsubishi4G63 = 4,      ///< Mitsubishi CAS pattern
    GM24X = 5,               ///< 24-tooth wheel
    Jeep2000 = 6,            ///< Jeep 4.0L pattern
    Audi135 = 7,             ///< Audi 5-cylinder pattern
    HondaD17 = 8,            ///< Honda D-series VTEC
    Miata9905 = 9,           ///< Mazda Miata NB (99-05)
    MazdaAU = 10,            ///< Australian Mazda pattern
    Nissan360 = 11,          ///< Nissan CAS pattern
    Subaru67 = 12,           ///< Subaru EJ pattern
    DaihatsuPlus1 = 13,      ///< Daihatsu pattern
    Harley = 14,             ///< Harley-Davidson
    Tooth36_2_2_2 = 15,      ///< Triple missing tooth (36-2-2-2)
    Tooth36_2_1 = 16,        ///< Alternative 36-tooth pattern
    Chrysler420A = 17,       ///< Chrysler/Mitsubishi 2.0L
    Weber = 18,              ///< Weber-Alpha pattern
    FordST170 = 19,          ///< Ford Duratec ST170
    SuzukiDRZ400 = 20,       ///< Suzuki single-cylinder
    ChryslerNGC = 21,        ///< Chrysler Next-Gen Controller
    YamahaVMAX = 22,         ///< Yamaha V4
    Renix = 23,              ///< Jeep Renix pattern
    RoverMEMS = 24,          ///< Rover/MG pattern
    SuzukiK6A = 25,          ///< Suzuki Kei car
    FordTFI = 26,            ///< Ford Thick Film Ignition
    HondaJ32 = 27,           ///< Honda V6
    Non360 = 28              ///< Non-360 degree patterns
};

/// @brief Get human-readable name for decoder type
inline const char* getDecoderName(DecoderType type) {
    switch (type) {
        case DecoderType::MissingTooth:      return "Missing Tooth";
        case DecoderType::BasicDistributor:  return "Basic Distributor";
        case DecoderType::DualWheel:         return "Dual Wheel";
        case DecoderType::GM7X:              return "GM 7X";
        case DecoderType::Mitsubishi4G63:    return "Mitsubishi 4G63";
        case DecoderType::GM24X:             return "GM 24X";
        case DecoderType::Jeep2000:          return "Jeep 2000";
        case DecoderType::Audi135:           return "Audi 135";
        case DecoderType::HondaD17:          return "Honda D17";
        case DecoderType::Miata9905:         return "Miata 99-05";
        case DecoderType::MazdaAU:           return "Mazda AU";
        case DecoderType::Nissan360:         return "Nissan 360";
        case DecoderType::Subaru67:          return "Subaru 6/7";
        case DecoderType::DaihatsuPlus1:     return "Daihatsu +1";
        case DecoderType::Harley:            return "Harley";
        case DecoderType::Tooth36_2_2_2:     return "36-2-2-2";
        case DecoderType::Tooth36_2_1:       return "36-2-1";
        case DecoderType::Chrysler420A:      return "Chrysler 420A";
        case DecoderType::Weber:             return "Weber";
        case DecoderType::FordST170:         return "Ford ST170";
        case DecoderType::SuzukiDRZ400:      return "Suzuki DRZ400";
        case DecoderType::ChryslerNGC:       return "Chrysler NGC";
        case DecoderType::YamahaVMAX:        return "Yamaha VMAX";
        case DecoderType::Renix:             return "Renix";
        case DecoderType::RoverMEMS:         return "Rover MEMS";
        case DecoderType::SuzukiK6A:         return "Suzuki K6A";
        case DecoderType::FordTFI:           return "Ford TFI";
        case DecoderType::HondaJ32:          return "Honda J32";
        case DecoderType::Non360:            return "Non-360";
        default:                             return "Unknown";
    }
}

// === Decoder State Bits (from decoders.h) ===
constexpr uint8_t BIT_DECODER_VALID_TRIGGER = 0;     ///< Last trigger was valid
constexpr uint8_t BIT_DECODER_TOOTH_ANG_CORRECT = 1; ///< Tooth angle is correct
constexpr uint8_t BIT_DECODER_IS_SEQUENTIAL = 2;     ///< Running in sequential mode
constexpr uint8_t BIT_DECODER_HAS_SECONDARY = 3;     ///< Has secondary input
constexpr uint8_t BIT_DECODER_HAS_FIXED_CRANKING = 4;///< Has fixed cranking timing
constexpr uint8_t BIT_DECODER_2ND_DERIV = 5;         ///< Using 2nd derivative filter

// === Trigger Filter Settings ===
constexpr uint8_t TRIGGER_FILTER_OFF = 0;
constexpr uint8_t TRIGGER_FILTER_LITE = 1;
constexpr uint8_t TRIGGER_FILTER_MEDIUM = 2;
constexpr uint8_t TRIGGER_FILTER_AGGRESSIVE = 3;

// === Speed Constants ===
constexpr uint8_t CRANK_SPEED = 1;
constexpr uint8_t CAM_SPEED = 2;

} // namespace SpeeduinoDecoders

// ============================================================================
// ENGINE STATUS BITS (from globals.h / statuses.h)
// Reference: docsbyme/Speeduino Doxygen Source Files/05 - Globals and Status.md
// ============================================================================

namespace SpeeduinoStatus {

// === Status1 Bits (Injection/Ignition command status) ===
constexpr uint8_t BIT_STATUS1_INJ1 = 0;  ///< Injector 1 commanded
constexpr uint8_t BIT_STATUS1_INJ2 = 1;  ///< Injector 2 commanded
constexpr uint8_t BIT_STATUS1_INJ3 = 2;  ///< Injector 3 commanded
constexpr uint8_t BIT_STATUS1_INJ4 = 3;  ///< Injector 4 commanded
constexpr uint8_t BIT_STATUS1_DFCO = 4;  ///< Decel fuel cut off active
constexpr uint8_t BIT_STATUS1_BOOSTCUT = 5; ///< Boost cut active
constexpr uint8_t BIT_STATUS1_HARDLIM = 6;  ///< Hard limiter active
constexpr uint8_t BIT_STATUS1_SOFTLIM = 7;  ///< Soft limiter active

// === Status2 Bits ===
constexpr uint8_t BIT_STATUS2_HLAUNCH = 0;  ///< Hard launch active
constexpr uint8_t BIT_STATUS2_SLAUNCH = 1;  ///< Soft launch active
constexpr uint8_t BIT_STATUS2_FLATSHIFT = 2;///< Flat shift active
constexpr uint8_t BIT_STATUS2_SPARK2ACTIVE = 3; ///< Secondary spark table active
constexpr uint8_t BIT_STATUS2_FUEL2ACTIVE = 4;  ///< Secondary fuel table active
constexpr uint8_t BIT_STATUS2_WCMI = 5;         ///< Water/Meth injection active
constexpr uint8_t BIT_STATUS2_VVT1ERR = 6;      ///< VVT1 error
constexpr uint8_t BIT_STATUS2_VVT2ERR = 7;      ///< VVT2 error

// === Status3 Bits (Engine state) ===
constexpr uint8_t BIT_STATUS3_RESET_PREVENT = 0; ///< Reset prevention active
constexpr uint8_t BIT_STATUS3_NITROUS = 1;       ///< Nitrous active
constexpr uint8_t BIT_STATUS3_VSS_REFRESH = 2;   ///< VSS refresh pending
constexpr uint8_t BIT_STATUS3_HALFSYNC = 3;      ///< Half sync achieved
constexpr uint8_t BIT_STATUS3_NSQUIRTS = 4;      ///< N-squirts mode bit 0
constexpr uint8_t BIT_STATUS3_UNUSED6 = 5;
constexpr uint8_t BIT_STATUS3_UNUSED7 = 6;
constexpr uint8_t BIT_STATUS3_UNUSED8 = 7;

// === Status4 Bits ===
constexpr uint8_t BIT_STATUS4_WMI_EMPTY = 0;     ///< WMI tank empty
constexpr uint8_t BIT_STATUS4_VVT1_ERROR = 1;    ///< VVT1 target error
constexpr uint8_t BIT_STATUS4_VVT2_ERROR = 2;    ///< VVT2 target error
constexpr uint8_t BIT_STATUS4_FAN = 3;           ///< Fan output active
constexpr uint8_t BIT_STATUS4_BURNPENDING = 4;   ///< EEPROM burn pending
constexpr uint8_t BIT_STATUS4_STAGING_ACTIVE = 5;///< Staged injection active
constexpr uint8_t BIT_STATUS4_SPARK_ERROR = 6;   ///< Spark output error
constexpr uint8_t BIT_STATUS4_SYNC_ERROR = 7;    ///< Trigger sync error

// === Engine Status Bits (engine variable) ===
constexpr uint8_t BIT_ENGINE_RUN = 0;      ///< Engine is running
constexpr uint8_t BIT_ENGINE_CRANK = 1;    ///< Engine is cranking
constexpr uint8_t BIT_ENGINE_ASE = 2;      ///< After-start enrichment active
constexpr uint8_t BIT_ENGINE_WARMUP = 3;   ///< Warmup enrichment active
constexpr uint8_t BIT_ENGINE_ACC = 4;      ///< Acceleration enrichment active
constexpr uint8_t BIT_ENGINE_DCC = 5;      ///< Deceleration mode
constexpr uint8_t BIT_ENGINE_MAPACC = 6;   ///< MAP-based accel active
constexpr uint8_t BIT_ENGINE_TPSACC = 7;   ///< TPS-based accel active

} // namespace SpeeduinoStatus

// ============================================================================
// TIMER FLAGS (from globals.h)
// Reference: docsbyme/Speeduino Doxygen Source Files/05 - Globals and Status.md
// ============================================================================

namespace SpeeduinoTimers {

constexpr uint8_t BIT_TIMER_1HZ = 0;    ///< 1 Hz timer flag
constexpr uint8_t BIT_TIMER_4HZ = 1;    ///< 4 Hz timer flag
constexpr uint8_t BIT_TIMER_10HZ = 2;   ///< 10 Hz timer flag
constexpr uint8_t BIT_TIMER_15HZ = 3;   ///< 15 Hz timer flag
constexpr uint8_t BIT_TIMER_30HZ = 4;   ///< 30 Hz timer flag
constexpr uint8_t BIT_TIMER_50HZ = 5;   ///< 50 Hz timer flag
constexpr uint8_t BIT_TIMER_200HZ = 6;  ///< 200 Hz timer flag
constexpr uint8_t BIT_TIMER_1KHZ = 7;   ///< 1 kHz timer flag

} // namespace SpeeduinoTimers

// ============================================================================
// CONFIGURATION PAGE NUMBERS (from config_pages.h)
// Reference: docsbyme/Speeduino Doxygen Source Files/09 - Configuration Pages.md
// ============================================================================

namespace SpeeduinoPages {

constexpr uint8_t PAGE_VE_TABLE = 1;          ///< Primary fuel VE map
constexpr uint8_t PAGE_CONFIG_FUEL = 2;       ///< Fuel settings (configPage2)
constexpr uint8_t PAGE_IGNITION_TABLE = 3;    ///< Primary ignition map
constexpr uint8_t PAGE_CONFIG_IGNITION = 4;   ///< Ignition settings (configPage4)
constexpr uint8_t PAGE_AFR_TABLE = 5;         ///< Target AFR map
constexpr uint8_t PAGE_CONFIG_AUX = 6;        ///< Aux settings (configPage6)
constexpr uint8_t PAGE_BOOST_TABLE = 7;       ///< Boost target map
constexpr uint8_t PAGE_VVT_TABLE = 8;         ///< VVT target map
constexpr uint8_t PAGE_CONFIG_CALIBRATION = 9;///< Calibration (configPage9)
constexpr uint8_t PAGE_CONFIG_BOOST_VVT = 10; ///< Boost/VVT settings (configPage10)
constexpr uint8_t PAGE_FUEL2_TABLE = 11;      ///< Secondary VE
constexpr uint8_t PAGE_WMI_TABLE = 12;        ///< Water Meth Injection
constexpr uint8_t PAGE_CONFIG_CAN = 13;       ///< CAN settings (configPage13)
constexpr uint8_t PAGE_IGNITION2_TABLE = 14;  ///< Secondary ignition
constexpr uint8_t PAGE_CONFIG_EXTENDED = 15;  ///< Extended settings (configPage15)

// Calibration table pages
constexpr uint8_t CALIBRATION_CLT = 0;        ///< Coolant temp calibration
constexpr uint8_t CALIBRATION_IAT = 1;        ///< Intake temp calibration
constexpr uint8_t CALIBRATION_O2 = 2;         ///< O2 sensor calibration

constexpr uint16_t CALIBRATION_TABLE_SIZE = 512; ///< Size of calibration tables

} // namespace SpeeduinoPages

// ============================================================================
// LOGGER SETTINGS (from globals.h)
// Reference: docsbyme/Speeduino Doxygen Source Files/10 - Utilities and Math.md
// ============================================================================

namespace SpeeduinoLogger {

constexpr uint8_t LOGGER_DISABLED = 0;
constexpr uint8_t LOGGER_CSV = 1;
constexpr uint8_t LOGGER_BINARY = 2;

constexpr uint8_t LOGGER_RATE_1HZ = 0;
constexpr uint8_t LOGGER_RATE_4HZ = 1;
constexpr uint8_t LOGGER_RATE_10HZ = 2;
constexpr uint8_t LOGGER_RATE_30HZ = 3;

// CSV separators
constexpr uint8_t LOGGER_CSV_SEPARATOR_COMMA = 0;
constexpr uint8_t LOGGER_CSV_SEPARATOR_SEMICOLON = 1;
constexpr uint8_t LOGGER_CSV_SEPARATOR_TAB = 2;
constexpr uint8_t LOGGER_CSV_SEPARATOR_SPACE = 3;

// File naming modes
constexpr uint8_t LOGGER_FILENAMING_OVERWRITE = 0;
constexpr uint8_t LOGGER_FILENAMING_DATETIME = 1;
constexpr uint8_t LOGGER_FILENAMING_SEQUENTIAL = 2;

} // namespace SpeeduinoLogger

// ============================================================================
// OUTPUT CONTROL METHODS (from globals.h)
// Reference: docsbyme/Speeduino Doxygen Source Files/08 - Board Support.md
// ============================================================================

namespace SpeeduinoOutput {

constexpr uint8_t OUTPUT_CONTROL_DIRECT = 0;   ///< Direct GPIO control
constexpr uint8_t OUTPUT_CONTROL_MC33810 = 1;  ///< Via MC33810 driver IC

} // namespace SpeeduinoOutput

// ============================================================================
// SERIAL RETURN CODES (from comms.h)
// ============================================================================

namespace SpeeduinoRC {

constexpr uint8_t RC_OK        = 0x00; ///< Success
constexpr uint8_t RC_BURN_OK   = 0x04; ///< EEPROM write succeeded
constexpr uint8_t RC_TIMEOUT   = 0x80; ///< ECU timeout
constexpr uint8_t RC_CRC_ERR   = 0x82; ///< CRC mismatch
constexpr uint8_t RC_UKWN_ERR  = 0x83; ///< Unknown command
constexpr uint8_t RC_RANGE_ERR = 0x84; ///< Offset+length exceeds page size
constexpr uint8_t RC_BUSY_ERR  = 0x85; ///< Deferred write pending

} // namespace SpeeduinoRC

// ============================================================================
// PACKET SIZES
// ============================================================================

namespace SpeeduinoPackets {

constexpr int REALTIME_DATA_SIZE = 130;    ///< Full 130-byte real-time data packet (LOG_ENTRY_SIZE)
constexpr int SIGNATURE_SIZE = 20;         ///< Signature response size (variable, min expected)
constexpr int MAX_PAGE_SIZE = 288;         ///< Maximum configuration page size
constexpr int TOOTH_LOG_SIZE = 508;        ///< Tooth log: 127 × uint32_t BE
constexpr int CRC32_SIZE = 4;              ///< CRC32 response size
constexpr int NEW_PROTO_HEADER = 2;        ///< Length header (2B BE)
constexpr int NEW_PROTO_CRC = 4;           ///< CRC32 trailer (4B BE)
constexpr int NEW_PROTO_OVERHEAD = 6;      ///< Total framing overhead (header + CRC)

} // namespace SpeeduinoPackets

// ============================================================================
// HELPER MACROS
// ============================================================================

/// Check if a bit is set in a status byte
#define SPEEDUINO_BIT_CHECK(var, bit) (((var) >> (bit)) & 1)

/// Set a bit in a status byte
#define SPEEDUINO_BIT_SET(var, bit) ((var) |= (1 << (bit)))

/// Clear a bit in a status byte
#define SPEEDUINO_BIT_CLEAR(var, bit) ((var) &= ~(1 << (bit)))

#endif // SPEEDUINOCONSTANTS_H
