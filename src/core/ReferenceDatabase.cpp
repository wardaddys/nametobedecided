#include "ReferenceDatabase.h"

ReferenceDatabase& ReferenceDatabase::instance() {
    static ReferenceDatabase inst;
    return inst;
}

ReferenceDatabase::ReferenceDatabase() {
    initializeTechnicalData();
    initializeInjectorData();
}

QList<TechnicalTerm> ReferenceDatabase::getTermsByCategory(const QString &category) const {
    QList<TechnicalTerm> result;
    for (const auto &term : m_technicalTerms) {
        if (term.category == category) result.append(term);
    }
    return result;
}

TechnicalTerm ReferenceDatabase::getTermById(const QString &id) const {
    return m_technicalTerms.value(id);
}

QStringList ReferenceDatabase::getCategories() const {
    QStringList result;
    for (const auto &term : m_technicalTerms) {
        if (!result.contains(term.category)) result.append(term.category);
    }
    result.sort();
    return result;
}

QList<InjectorSpec> ReferenceDatabase::getAllInjectors() const {
    return m_injectors;
}

QList<InjectorSpec> ReferenceDatabase::getInjectorsByManufacturer(const QString &manufacturer) const {
    QList<InjectorSpec> result;
    for (const auto &inj : m_injectors) {
        if (inj.manufacturer == manufacturer) result.append(inj);
    }
    return result;
}

QStringList ReferenceDatabase::getManufacturers() const {
    QStringList result;
    for (const auto &inj : m_injectors) {
        if (!result.contains(inj.manufacturer)) result.append(inj.manufacturer);
    }
    result.sort();
    return result;
}

void ReferenceDatabase::initializeTechnicalData() {
    // Ported from TechnicalDatabase.ts
    
    TechnicalTerm na;
    na.id = "ENG-001-NA";
    na.category = "Engine Configuration";
    na.title = "Naturally Aspirated (NA)";
    na.shortDescription = "Engine relying solely on atmospheric pressure for air intake";
    na.fullDescription = "Engine that relies solely on atmospheric pressure for air intake without forced induction. Features linear power delivery and naturally breathing characteristics without boost pressure.";
    na.characteristics = {"Linear power delivery", "Naturally breathing", "No boost pressure", "Altitude sensitive"};
    na.tuningFocus = {"Volumetric efficiency optimization", "Cam timing", "VE tables only", "Throttle response"};
    na.tablesRequired = {"VE Table (Primary/Secondary)", "Ignition Timing", "AFR Target"};
    na.advantages = {"Simplicity and reliability", "Predictable behavior", "Lower heat generation", "Minimal complexity"};
    na.disadvantages = {"Lower power density", "Altitude sensitivity", "Limited tuning range"};
    m_technicalTerms[na.id] = na;

    TechnicalTerm turbo;
    turbo.id = "ENG-001-TURBO";
    turbo.category = "Engine Configuration";
    turbo.title = "Turbocharged";
    turbo.shortDescription = "Forced induction using exhaust gas energy";
    turbo.fullDescription = "Forced induction system using exhaust gas energy to compress intake air. Features exponential power delivery, boost pressure dependent performance, and turbo lag characteristics.";
    turbo.characteristics = {"Exponential power delivery", "Boost pressure dependent", "Turbo lag present", "High thermal loads"};
    turbo.tuningFocus = {"Boost control", "Fueling enrichment", "Timing retard under boost", "Compressor efficiency", "Wastegate duty cycle"};
    turbo.tablesRequired = {"All VE tables", "Boost Target", "Boost Compensation", "Wastegate Duty Cycle", "Overboost Protection"};
    turbo.advantages = {"High power density", "Efficiency at cruise", "Adjustable power levels", "Excellent power-to-weight"};
    turbo.disadvantages = {"Turbo lag", "Heat management complexity", "Complex control systems"};
    turbo.typicalValues["Target Boost (Street)"] = "7-15 PSI";
    turbo.typicalValues["Target Boost (Race)"] = "20-35 PSI";
    turbo.typicalValues["Wastegate Base Duty"] = "30-40%";
    m_technicalTerms[turbo.id] = turbo;

    TechnicalTerm super;
    super.id = "ENG-001-SUPER";
    super.category = "Engine Configuration";
    super.title = "Supercharged";
    super.shortDescription = "Mechanically driven forced induction";
    super.fullDescription = "Forced induction mechanically driven by engine crankshaft via belt. Features instant boost response, linear power delivery, and parasitic power consumption.";
    super.characteristics = {"Instant boost response", "Linear power delivery", "Parasitic drag", "No lag"};
    super.advantages = {"No lag", "Linear response", "Simple control", "Predictable behavior"};
    super.disadvantages = {"Parasitic power loss", "Fixed boost curve", "Heat generation", "Limited boost range"};
    super.typicalValues["Typical Boost"] = "6-12 PSI";
    m_technicalTerms[super.id] = super;

    TechnicalTerm vtec;
    vtec.id = "VLV-001-VTEC";
    vtec.category = "Valve Train";
    vtec.title = "VTEC (Variable Valve Timing and Lift Electronic Control)";
    vtec.shortDescription = "Honda discrete cam profile switching system";
    vtec.fullDescription = "Hydraulic pin engagement system that switches between two cam profiles for optimized performance across RPM range. Low-speed profile provides economy and emissions, high-speed profile maximizes power.";
    vtec.characteristics = {"Discrete profile switching", "Hydraulic pin actuation", "Dual cam lobes", "RPM-based engagement"};
    vtec.tuningFocus = {"Engagement point optimization", "Dual map tuning (if supported)", "Transition smoothness", "Oil pressure monitoring"};
    vtec.tablesRequired = {"Primary VE Table (Low-Speed)", "Secondary VE Table (High-Speed)", "Low-Speed Ignition Map", "High-Speed Ignition Map"};
    vtec.advantages = {"Best of both worlds", "Excellent fuel economy at cruise", "High power at high RPM"};
    vtec.typicalValues["Engagement RPM"] = "4500-6000 RPM";
    vtec.typicalValues["Hysteresis"] = "200-400 RPM";
    m_technicalTerms[vtec.id] = vtec;

    TechnicalTerm vvt;
    vvt.id = "VLV-001-VVT";
    vvt.category = "Valve Train";
    vvt.title = "VVT (Variable Valve Timing)";
    vvt.shortDescription = "Continuous cam phaser system";
    vvt.fullDescription = "Oil pressure actuated vane-type phaser that continuously adjusts cam timing. Provides optimized timing across entire RPM range for improved torque curve and emissions.";
    vvt.characteristics = {"Continuous adjustment", "Oil pressure actuated", "Vane-type phaser", "Smooth operation"};
    vvt.tuningFocus = {"Target advance table tuning", "PID control gains", "Slew rate limits"};
    m_technicalTerms[vvt.id] = vvt;

    TechnicalTerm cl;
    cl.id = "CTL-001-CLOSED";
    cl.category = "Control Strategy";
    cl.title = "Closed Loop Fuel Control";
    cl.shortDescription = "Feedback-based fuel control using O2 sensor";
    cl.fullDescription = "Feedback control system where ECU continuously adjusts fuel delivery based on measured AFR from wideband O2 sensor to achieve target AFR. Uses PID control algorithm for precise correction.";
    cl.characteristics = {"Continuous feedback", "O2 sensor based", "PID control", "Self-correcting"};
    cl.tuningFocus = {"PID gain tuning", "Authority limits", "Enable conditions", "Target AFR table"};
    cl.advantages = {"Self-correcting", "Compensates for drift", "Adapts to conditions"};
    cl.typicalValues["P Gain"] = "0.3-0.5";
    cl.typicalValues["I Gain"] = "0.05-0.1";
    cl.typicalValues["Authority Limit"] = "±15-25%";
    m_technicalTerms[cl.id] = cl;

    // SENSORS
    TechnicalTerm map;
    map.id = "SEN-001-MAP";
    map.category = "Sensors";
    map.title = "MAP (Manifold Absolute Pressure) Sensor";
    map.shortDescription = "Measures absolute air pressure in intake manifold";
    map.fullDescription = "Piezoelectric or capacitive pressure transducer that measures absolute air pressure in intake manifold. Primary load axis for speed-density tuning strategy.";
    map.typicalValues["NA Range"] = "10-105 kPa";
    map.typicalValues["Turbo Range (3-Bar)"] = "10-300 kPa";
    m_technicalTerms[map.id] = map;

    TechnicalTerm boost;
    boost.id = "BST-001-PID";
    boost.category = "Boost Control";
    boost.title = "PID Boost Control";
    boost.shortDescription = "Closed-loop boost control target tracking";
    boost.fullDescription = "Advanced boost control strategy that uses a Proportional-Integral-Derivative algorithm to reach and maintain a target boost pressure. Automatically compensates for changes in ambient temperature and gear position.";
    boost.characteristics = {"Target tracking", "Self-correcting", "Gear/RPM dependent", "Overboost protection"};
    boost.tuningFocus = {"PID gain tuning", "Slew rate limits", "Duty cycle range", "Target offset"};
    boost.typicalValues["Typical P Gain"] = "0.2 - 0.6";
    boost.typicalValues["Typical I Gain"] = "0.01 - 0.05";
    m_technicalTerms[boost.id] = boost;

    TechnicalTerm staged;
    staged.id = "FUEL-003-STAGED";
    staged.category = "Fuel Configuration";
    staged.title = "Staged Injection";
    staged.shortDescription = "Primary and secondary injector sets";
    staged.fullDescription = "Uses two sets of injectors per cylinder. Primary injectors handle idle and low load for better atomization, while secondary injectors kick in at high load to provide the necessary fuel volume for high power.";
    staged.characteristics = {"Dual injector sets", "Load-based transition", "Improved low-end atomization", "High-power capability"};
    staged.typicalValues["Transition RPM"] = "3500-5000 RPM";
    staged.typicalValues["Transition MAP"] = "80-120 kPa";
    m_technicalTerms[staged.id] = staged;
}

void ReferenceDatabase::initializeInjectorData() {
    // Ported from InjectorDatabase.ts
    
    InjectorSpec id1000;
    id1000.id = "ID-1000";
    id1000.brand = "Injector Dynamics";
    id1000.model = "ID1000";
    id1000.flowRateCCMin = 1045;
    id1000.flowRateLbHr = 99.5;
    id1000.impedance = 15;
    id1000.deadTime14V = 0.70;
    id1000.deadTime12V = 0.88;
    id1000.deadTime10V = 1.18;
    id1000.pressure = 3.0;
    id1000.category = "race";
    id1000.manufacturer = "Injector Dynamics";
    id1000.notes = "High HP turbo, 600+ hp capable";
    m_injectors.append(id1000);

    InjectorSpec id1300;
    id1300.id = "ID-1300X";
    id1300.brand = "Injector Dynamics";
    id1300.model = "ID1300X";
    id1300.flowRateCCMin = 1340;
    id1300.flowRateLbHr = 127.6;
    id1300.impedance = 14;
    id1300.deadTime14V = 0.68;
    id1300.deadTime12V = 0.85;
    id1300.deadTime10V = 1.15;
    id1300.pressure = 3.0;
    id1300.category = "race";
    id1300.manufacturer = "Injector Dynamics";
    id1300.notes = "E85 compatible, 800+ hp";
    m_injectors.append(id1300);

    InjectorSpec denso;
    denso.id = "DENSO-23250-66030";
    denso.brand = "Denso";
    denso.model = "23250-66030";
    denso.flowRateCCMin = 310;
    denso.flowRateLbHr = 29.5;
    denso.impedance = 12;
    denso.deadTime14V = 0.75;
    denso.deadTime12V = 0.92;
    denso.deadTime10V = 1.22;
    denso.pressure = 3.0;
    denso.category = "street";
    denso.manufacturer = "Denso";
    denso.notes = "Toyota Supra 2JZ-GTE OEM";
    m_injectors.append(denso);
    
    InjectorSpec bosch;
    bosch.id = "BOSCH-0445110247";
    bosch.brand = "Bosch";
    bosch.model = "1200cc";
    bosch.flowRateCCMin = 1200;
    bosch.flowRateLbHr = 114.3;
    bosch.impedance = 14;
    bosch.deadTime14V = 0.68;
    bosch.deadTime12V = 0.85;
    bosch.deadTime10V = 1.15;
    bosch.pressure = 3.0;
    bosch.category = "race";
    bosch.manufacturer = "Bosch";
    m_injectors.append(bosch);

    InjectorSpec deka80;
    deka80.id = "DEKA-80";
    deka80.brand = "Siemens Deka";
    deka80.model = "80 lb/hr";
    deka80.flowRateCCMin = 840;
    deka80.flowRateLbHr = 80.0;
    deka80.impedance = 12;
    deka80.deadTime14V = 0.88;
    deka80.deadTime12V = 1.10;
    deka80.deadTime10V = 1.42;
    deka80.pressure = 3.0;
    deka80.category = "race";
    deka80.manufacturer = "Siemens Deka";
    deka80.notes = "Boosted LS applications popular choice";
    m_injectors.append(deka80);

    InjectorSpec fic2150;
    fic2150.id = "FIC-2150";
    fic2150.brand = "Fuel Injector Clinic";
    fic2150.model = "FIC2150";
    fic2150.flowRateCCMin = 2265;
    fic2150.flowRateLbHr = 215.7;
    fic2150.impedance = 13;
    fic2150.deadTime14V = 0.60;
    fic2150.deadTime12V = 0.78;
    fic2150.deadTime10V = 1.05;
    fic2150.pressure = 3.0;
    fic2150.category = "extreme";
    fic2150.manufacturer = "Fuel Injector Clinic";
    fic2150.notes = "Drag racing, 1200+ hp capable";
    m_injectors.append(fic2150);
}
