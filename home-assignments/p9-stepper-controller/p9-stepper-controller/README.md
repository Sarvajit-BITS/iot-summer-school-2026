# Project 9: Stepper Motor Precision Positioner

**IIT Jammu Summer School 2026 | IoT Assignment**
**Level:** Advanced | **Estimated Time:** 2–3 Hours | **Marks:** 15

## Project Overview
This project demonstrates a closed-loop, precision position controller using a stepper motor, establishing the foundational logic used in CNC machines, 3D printers, and robotic arms. 

Unlike a standard continuous-rotation motor, this system separates **Target Selection** from **Execution**:
1. A **Potentiometer** acts as an analog setpoint dial, allowing the user to select a precise target angle (0°–360°).
2. The **CW / CCW Push Buttons** execute the movement, reading the analog setpoint and driving the motor exactly that many degrees in the chosen direction.
3. A **1.3" I2C OLED Display** serves as a real-time digital Human-Machine Interface (HMI), tracking the Target Angle, Current Angle, and Absolute Step Count.

## Hardware Architecture & Wiring Matrix

* **Microcontroller:** Arduino Uno R3
* **Actuator:** 28BYJ-48 Stepper Motor + ULN2003 Driver Board
* **Display:** 1.3" I2C OLED (GME12864-81 / SH1106 Driver)
* **Inputs:** 10kΩ Potentiometer, 3x Tactile Push Buttons

| Component | Pin / Terminal | Arduino Pin | Notes |
| :--- | :--- | :--- | :--- |
| **ULN2003 Driver** 
| | `IN1` | Pin 8 | |
| | `IN2` | Pin 9 | *Swapped with IN3 physically to match `Stepper.h` coil sequence.* |
| | `IN3` | Pin 10 | *Swapped with IN2 physically to match `Stepper.h` coil sequence.* |
| | `IN4` | Pin 11 | |
| | `+` / `-` | 5V / GND | Direct to breadboard power rail. |
| **Potentiometer** | Middle Wiper | A0 | Analog input (0-1023) mapped to 0°-360°. |
| **Buttons** | CW Button | Pin 2 | Uses internal `INPUT_PULLUP`. Connect to GND. |
| | CCW Button | Pin 3 | Uses internal `INPUT_PULLUP`. Connect to GND. |
| | Home Button | Pin 4 | Uses internal `INPUT_PULLUP`. Connect to GND. |
| **OLED (SH1106)** | SDA | A4 (SDA) | I2C Data. |
| | SCL | A5 (SCL) | I2C Clock. |

## Technical Highlights & Optimizations
* **ADC Noise Filtering (Jitter Prevention):** Standard analog pins pick up microscopic electrical ripples from USB power, causing the setpoint angle to jitter. The code implements a 15-sample **Moving Average Filter** to smooth the raw `analogRead()` data, ensuring rock-solid target selection.
* **SH1106 Display Calibration:** Upgraded from the standard SSD1306 library to the `Adafruit_SH110X` library to correctly map the memory matrix of the 1.3" GME12864-81 display, eliminating screen static and pixel offset.
* **Absolute Position Tracking:** Instead of using error-prone single-step loops, the system tracks absolute position mathematically (`currentStepPosition += steps`) and utilizes modulo operations to normalize the display angle between 0° and 360° without causing motor snap-back.
* **Speed Capping:** The motor speed is strictly capped at **8 RPM** in the code to ensure the magnetic coils have enough torque to lock into place without slipping or missing steps under load.

---

## Engineering Theory: The 28BYJ-48 Stepper & 4096 Steps/Rev
*(Rubric Requirement Deliverable)*

To achieve precision, we must understand the mechanical anatomy of the **28BYJ-48** motor.

1. **Internal Coils (The Stator):** The internal motor has a "stride angle" of 5.625°. This means the internal rotor requires **64 steps** to complete one full revolution (360 / 5.625 = 64).
2. **The Gearbox:** However, the internal rotor is connected to a reduction gear train with a ratio of roughly **1:64**. 
3. **Calculating Total Resolution:** * `64 internal steps` × `64 gear ratio` = **4096 Steps per Revolution**.

**Why does our code use 2048?**
The 4096 specification assumes the motor is being driven in **"Half-Step" mode** (an 8-step sequence firing 1-2 coils at a time). Our code utilizes the standard Arduino `Stepper.h` library, which drives the motor in **"Full-Step" mode** (a 4-step sequence firing 2 coils at a time). Full-stepping cuts the resolution in half but increases magnetic torque. Therefore, in our code:
`4096 half-steps / 2 = 2048 full-steps per revolution.`

## Dependencies
To compile this code, ensure the following libraries are installed via the Arduino Library Manager:
* `Stepper` (Built-in)
* `Wire` (Built-in)
* `Adafruit GFX Library`
* `Adafruit SH110X` (Specific for 1.3" displays)