import re
import sys

with open('resources/workspaces/mapping.yaml', 'r') as f:
    content = f.read()

# TRIGGERS & SYNC
content = re.sub(r'# --- TRIGGERS & SYNC ---.*?# --- FUELING ---', '''# --- TRIGGERS & SYNC ---
- name: TrigPattern
  primary_workspace: triggers
  subsection: crank_trigger
  display_order: 10
  human_label: "Trigger Type"
  help_text: "Pattern of the trigger wheel."

- name: numTeeth
  primary_workspace: triggers
  subsection: crank_trigger
  display_order: 20
  human_label: "Trigger Teeth"
  help_text: "Total number of teeth if there were no missing teeth."

- name: missingTeeth
  primary_workspace: triggers
  subsection: crank_trigger
  display_order: 30
  human_label: "Missing Teeth"
  help_text: "Number of missing teeth on the trigger wheel."

- name: TrigAng
  primary_workspace: triggers
  subsection: crank_trigger
  display_order: 40
  human_label: "Trigger Angle"
  help_text: "Degrees BTDC at sync point."

# --- FUELING ---''', content, flags=re.DOTALL)

# FUELING
content = re.sub(r'# --- FUELING ---.*?# --- IGNITION ---', '''# --- FUELING ---
- name: injOpen
  primary_workspace: fueling
  subsection: injector_hardware
  display_order: 10
  human_label: "Injector Open Time"
  help_text: "Injector dead time at nominal battery voltage (ms)."

- name: reqFuel
  primary_workspace: fueling
  subsection: fuel_system
  display_order: 30
  human_label: "Required Fuel"
  help_text: "Base fuel pulse width (ms)."

- name: injAng
  primary_workspace: fueling
  subsection: injection_timing
  display_order: 50
  human_label: "Injection Angle"
  help_text: "End-of-injection angle in degrees."

# --- IGNITION ---''', content, flags=re.DOTALL)

# IGNITION
content = re.sub(r'# --- IGNITION ---.*?# --- COLD START ---', '''# --- IGNITION ---
- name: sparkMode
  primary_workspace: ignition
  subsection: coil_hardware
  display_order: 10
  human_label: "Spark Mode"
  help_text: "Wasted spark, sequential, or COP."

- name: IgInv
  primary_workspace: ignition
  subsection: coil_hardware
  display_order: 20
  human_label: "Ignition Polarity"
  help_text: "Whether the spark fires when the ignition signal goes high or low. Most systems use 'Going Low'."

- name: dwellrun
  primary_workspace: ignition
  subsection: coil_hardware
  display_order: 30
  human_label: "Running Dwell"
  help_text: "Nominal dwell time when the engine is running (ms)."

- name: useDwellMap
  primary_workspace: ignition
  subsection: coil_hardware
  display_order: 40
  human_label: "Use Dwell Table"
  help_text: "Enable 3D dwell table instead of a fixed dwell."

- name: dwellcrank
  primary_workspace: ignition
  subsection: coil_hardware
  display_order: 60
  human_label: "Cranking Dwell"
  help_text: "Dwell time during cranking (usually slightly higher than running dwell)."

# --- COLD START ---''', content, flags=re.DOTALL)

# COLD START
content = re.sub(r'# --- COLD START ---.*?# --- IDLE CONTROL ---', '''# --- COLD START ---
- name: crankRPM
  primary_workspace: cold_start
  subsection: cranking
  display_order: 10
  human_label: "Cranking RPM"
  help_text: "RPM threshold below which the engine is considered cranking."

- name: asePct
  primary_workspace: cold_start
  subsection: ase
  display_order: 40
  human_label: "ASE % (by CLT bin)"
  help_text: "After-start enrichment percent for each CLT bin (4-entry array)."
  secondary_workspaces: [fueling]

- name: aseTaperTime
  primary_workspace: cold_start
  subsection: ase
  display_order: 50
  human_label: "ASE Taper Time"
  help_text: "Transition time used to disable ASE (s)."

# --- IDLE CONTROL ---''', content, flags=re.DOTALL)

# IDLE CONTROL
content = re.sub(r'# --- IDLE CONTROL ---.*?# --- ENGINE PROTECTION ---', '''# --- IDLE CONTROL ---
- name: idleKP
  primary_workspace: idle
  subsection: idle_air_control
  display_order: 20
  human_label: "Idle P-Gain"
  help_text: "Proportional gain for idle PID."

- name: idleKI
  primary_workspace: idle
  subsection: idle_air_control
  display_order: 30
  human_label: "Idle I-Gain"
  help_text: "Integral gain for idle PID."

- name: idleKD
  primary_workspace: idle
  subsection: idle_air_control
  display_order: 40
  human_label: "Idle D-Gain"
  help_text: "Derivative gain for idle PID."

- name: airConIdleUpRPMAdder
  primary_workspace: idle
  subsection: compensations
  display_order: 60
  human_label: "A/C Idle Adder"
  help_text: "RPM added to the idle target when A/C is engaged."

# --- ENGINE PROTECTION ---''', content, flags=re.DOTALL)

# ENGINE PROTECTION
content = re.sub(r'# --- ENGINE PROTECTION ---.*?# --- SENSORS ---', '''# --- ENGINE PROTECTION ---
- name: hardRevLim
  primary_workspace: engine_protection
  subsection: rev_limiters
  display_order: 10
  human_label: "Hard Rev Limit"
  help_text: "Absolute maximum RPM (full cut)."

- name: SoftRevLim
  primary_workspace: engine_protection
  subsection: rev_limiters
  display_order: 20
  human_label: "Soft Rev Limit"
  help_text: "RPM threshold for soft cut (spark retard or rolling cut)."

- name: hardCutType
  primary_workspace: engine_protection
  subsection: rev_limiters
  display_order: 30
  human_label: "Cut Type"
  help_text: "Full cut or Rolling cut for rev/launch limits."

- name: boostCutEnabled
  primary_workspace: engine_protection
  subsection: boost_cut
  display_order: 40
  human_label: "Enable Boost Cut"
  help_text: "Enable overboost protection."

- name: boostLimit
  primary_workspace: engine_protection
  subsection: boost_cut
  display_order: 50
  human_label: "Boost Cut Limit"
  help_text: "Maximum allowed manifold pressure before fuel/spark cut (kPa)."

# --- SENSORS ---''', content, flags=re.DOTALL)

# SENSORS
content = re.sub(r'# --- SENSORS ---.*?# --- BOOST & ASPIRATION ---', '''# --- SENSORS ---
- name: mapMin
  primary_workspace: sensors
  subsection: map_sensor
  display_order: 10
  human_label: "MAP at 0V (kPa)"
  help_text: "Pressure value when the MAP sensor outputs 0V."

- name: mapMax
  primary_workspace: sensors
  subsection: map_sensor
  display_order: 20
  human_label: "MAP at 5V (kPa)"
  help_text: "Pressure value when the MAP sensor outputs 5V."

- name: useExtBaro
  primary_workspace: sensors
  subsection: optional_sensors
  display_order: 30
  human_label: "Use External Baro Sensor"
  help_text: "If on, the dedicated baro sensor pin is read; otherwise the initial MAP reading is used as baro."

- name: baroMin
  primary_workspace: sensors
  subsection: optional_sensors
  display_order: 40
  human_label: "Baro at 0V (kPa)"
  help_text: "Pressure when external baro sensor outputs 0V."

- name: baroMax
  primary_workspace: sensors
  subsection: optional_sensors
  display_order: 50
  human_label: "Baro at 5V (kPa)"
  help_text: "Pressure when external baro sensor outputs 5V."

# --- BOOST & ASPIRATION ---''', content, flags=re.DOTALL)

# VVT
content = re.sub(r'- name: vvtPWMFreq.*?help_text: "PWM frequency for the VVT solenoid output."', '''- name: vvtFreq
  primary_workspace: vvt
  subsection: vvt_general
  display_order: 30
  human_label: "VVT PWM Frequency"
  help_text: "PWM frequency for the VVT solenoid output (Hz)."''', content, flags=re.DOTALL)

content = re.sub(r'- name: vvtTable1.*?help_text: "Target intake-cam angle map."', '''- name: vvtTable
  primary_workspace: vvt
  subsection: vvt_cam1
  display_order: 40
  human_label: "VVT1 Target Table"
  help_text: "Target intake-cam angle map."''', content, flags=re.DOTALL)

# INPUTS & OUTPUTS
content = re.sub(r'\n- name: injAPin.*?help_text: "Output pin for ignition channel A."\n', '\n', content, flags=re.DOTALL)

# COMMUNICATIONS
content = re.sub(r'# --- COMMUNICATIONS ---.*', '''# --- COMMUNICATIONS ---
- name: CANBroadcastProt
  primary_workspace: comms
  subsection: can_bus
  display_order: 10
  human_label: "CAN Broadcast Mode"
  help_text: "Broadcast protocol (Off, BMW, VAG, Haltech). Set to Off to disable CAN output."

- name: tsCanId
  primary_workspace: comms
  subsection: can_bus
  display_order: 30
  human_label: "TS CAN ID"
  help_text: "CAN ID used for TunerStudio passthrough."

- name: enable_secondarySerial
  primary_workspace: comms
  subsection: secondary_serial
  display_order: 40
  human_label: "Secondary Serial Enabled"
  help_text: "Enable the secondary UART channel (CAN, msDroid, RealDash, TunerStudio passthrough)."
''', content, flags=re.DOTALL)

with open('resources/workspaces/mapping.yaml', 'w') as f:
    f.write(content)
