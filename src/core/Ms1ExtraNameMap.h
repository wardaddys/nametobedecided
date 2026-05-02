/**
 * @file Ms1ExtraNameMap.h
 * @brief Static translation table: MS1/Extra MSQ names → internal table/constant names
 *
 * MS1/Extra firmware (format 029y3 / 029y4) stores tuning tables in MSQ files
 * under different names than those used internally by this application.
 * This mapping layer allows injectMsqData() to recognise and inject MS1 table
 * data into the correct internal slots without modifying the core parser.
 *
 * To extend support for more MS1/Extra tables, add entries to the static maps below.
 *
 * References:
 *   - MS1/Extra documentation: http://www.msextra.com/doc/ms1extra/
 *   - MSQ format 029y3 / 029y4 field names from CurrentTune.msq
 */
#pragma once

#include <QMap>
#include <QString>

class Ms1ExtraNameMap {
public:
    // -----------------------------------------------------------------------
    // Table name mapping
    //   Key:   MS1/Extra MSQ <constant name="..."> attribute
    //   Value: Internal table name used by ECUDefinition::getDefaultSpeeduinoTables()
    // -----------------------------------------------------------------------
    static QString toInternalTableName(const QString &ms1Name) {
        // Keys  : MS1/Extra MSQ <constant name="..."> attribute (from CurrentTune.msq)
        // Values: Exact table title string from [TableEditor] in mainController.ini
        //         These must match character-for-character what ECUDefinition::parseTableEditor()
        //         stores in m_tableDefinitions.
        static const QMap<QString, QString> kTableMap = {
            // VE (Volumetric Efficiency) tables
            { "veBins1",    "VE Table 1" },
            { "veBins2",    "VE Table 2 (DT)" },
            { "veBins3",    "VE Table 3. See Fuel Table Selection" },

            // Ignition advance tables
            { "advTable1",  "Ignition Advance Main Table" },
            { "advTable2",  "Ignition Advance Table 2. See Spark Table selection" },
            { "ignBins1",   "Ignition Advance Main Table" },  // alternate MSQ name
            { "sparkTable1","Ignition Advance Main Table" },

            // AFR / lambda target tables
            { "afrBins1",   "AFR Targets for VE Table 1 (AFR)" },
            { "afrBins2",   "AFR Targets for VE Table 3 (AFR)" },

            // Boost tables
            { "bcBins1",    "Boost kpa target 1" },
            { "bcBins2",    "Boost duty target" },
            { "bcBins3",    "Boost kpa target 2" },

            // Rotary split
            { "splitTable", "Rotary split Table - see settings" },

            // Axis bins — no 2D table equivalent; return empty to skip
            { "rpmBins1",   "" },
            { "mapBins1",   "" },
            { "rpmBins2",   "" },
            { "mapBins2",   "" },
        };
        return kTableMap.value(ms1Name, QString());
    }

    // -----------------------------------------------------------------------
    // Scalar constant name mapping
    //   Key:   MS1/Extra MSQ <constant name="..."> attribute
    //   Value: Internal constant name from ECUDefinition::getDefaultSpeeduinoConstants()
    // -----------------------------------------------------------------------
    static QString toInternalConstantName(const QString &ms1Name) {
        static const QMap<QString, QString> kConstMap = {
            // EGO / O2 sensor settings
            { "egoTemp1",          "egoMinClt"      },
            { "egoCount1",         "egoUpdateRate"  },
            { "egoRPM1",           "egoMinRpm"      },

            // Cranking & startup
            { "crankingRPM",       "crankingRpm"    },
            { "primePulseWidth",   "reqFuel"        },

            // Limiters
            { "rpmhigh",           "hardRevLim"     },
            { "rpmhighCrank",      "crankingRpm"    },
        };
        return kConstMap.value(ms1Name, QString());
    }

    // -----------------------------------------------------------------------
    // Detect MS1/Extra signature strings
    // -----------------------------------------------------------------------
    static bool isMs1ExtraSignature(const QString &sig) {
        return sig.contains("MS1/Extra", Qt::CaseInsensitive)
            || sig.contains("ms1extra",  Qt::CaseInsensitive)
            || sig.contains("ms1_extra", Qt::CaseInsensitive);
    }
};
