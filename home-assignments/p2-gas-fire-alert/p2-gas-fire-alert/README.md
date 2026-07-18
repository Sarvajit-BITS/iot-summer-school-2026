# Project 2: Gas & Fire Safety Alert System

## Project Overview
A responsive embedded safety alert system built using an Arduino Uno R3. The system parses structural analog input streams from an MQ-2 Gas sensor alongside digital outputs from an optical infrared Flame Sensor to protect homes against unexpected fire hazards and gas leakage occurrences.

## Hardware Configurations
- **MQ-2 Sensor:** Powered securely via 5V rail (3.3V power rails supply insufficient operating voltage for the internal heating element to consistently alter core surface resistances).
- **Flame Sensor:** Configured to monitor Digital Output Pin 7.
- **Buzzer:** Assigned to Pin 9 (PWM capability enabled).
- **Status Layout Indicators:** Green (Pin 10), Yellow (Pin 11), Red (Pin 12).
- **Silence Switch:** Implemented on Pin 2 utilizing native internal pull-up architectures.

## Architecture Deep Dives

### Why is the Flame Sensor Active-Low?
The flame sensor circuit relies on an onboard comparator IC (like the LM393). In normal environmental operating conditions, the infrared photodiode registers high resistance, causing the comparator circuit output line to remain pulled high (`HIGH`). 

When open fire fields emit infrared wavelengths, the sensor's photodiode conductance surges sharply, dragging the signal line path straight down to ground. This makes the output read `LOW` when a fire is actively detected.

### MQ-2 Warm-Up Phase
The active gas sensor contains an internal micro-heating layer that needs roughly 1 to 2 minutes of constant power-on operations to burn off baseline atmospheric moisture accumulations. Sensor readings will drift abnormally until it reaches thermal equilibrium.

## Functional States Matrix
1. **SAFE (< 30% Gas / No Flame):** Steady green illumination. Silent buzzer.
2. **WARNING (30% - 60% Gas):** Steady yellow illumination. Pulsed audio rhythm (1Hz frequency).
3. **DANGER (> 60% Gas OR Flame Detected):** Solid red safety visual. Continuous alarm patterns match the exact active triggers present.