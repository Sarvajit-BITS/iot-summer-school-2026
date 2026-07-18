# Project 3: Ultrasonic Parking Assistant

## Project Overview
This project replicates a real-world automotive parking sensor system. It utilizes an HC-SR04 ultrasonic transducer to calculate the proximity of physical obstacles and provides real-time, dynamic feedback through an active buzzer, a tiered LED alert system (Green, Yellow, Red), and a digital OLED dashboard readout.

## Hardware Components
* Arduino Uno R4 WiFi
* HC-SR04 Ultrasonic Sensor
* Active Buzzer
* 3x LEDs (Green, Yellow, Red) + 3x 220Ω Resistors
* 0.96" I2C OLED Display
* Breadboard & Jumper Wires

## Wiring Architecture Matrix
| Component Interface | Arduino Target Pin | Electrical Notes |
| :--- | :--- | :--- |
| **HC-SR04 VCC** | 5V | Primary power rail. |
| **HC-SR04 GND** | GND | Common ground loop. |
| **HC-SR04 TRIG** | Pin 9 | Digital output triggering the ultrasonic pulse. |
| **HC-SR04 ECHO** | Pin 10 | Input capture channel reading the bounce duration. |
| **Active Buzzer (+)** | Pin 6 | PWM-capable pin for dynamic `tone()` generation. |
| **Green LED** | Pin 3 | Routed through inline 220Ω resistor to GND. |
| **Yellow LED** | Pin 4 | Routed through inline 220Ω resistor to GND. |
| **Red LED** | Pin 5 | Routed through inline 220Ω resistor to GND. |
| **OLED SDA** | SDA (or A4) | I2C Data line. |
| **OLED SCL** | SCL (or A5) | I2C Clock line. |

## Physics Calculation & Signal Filtering

The system measures distance using the **Time-of-Flight (ToF)** of high-frequency acoustic waves. 

### The Formula
Sound travels through air at approximately 343 meters per second (or 0.0343 centimeters per microsecond). When the sensor fires, we measure the time it takes for the echo to return. Because the sound wave has to travel to the object *and* back, we divide the total time by 2.
* **Equation:** `Distance (cm) = (Pulse Duration in µs * 0.0343) / 2`

### Noise Reduction
To eliminate ghost echoes and ambient acoustic interference, the firmware utilizes a moving average filter. It rapidly fires 3 sequential ultrasonic pulses, adds them together, and computes a stabilized mathematical mean before shifting the system's alert state.

## Software Mechanics & State Machine
* **Asynchronous Multitasking:** The system avoids `delay()` commands entirely in the main loop. By using `millis()` delta-tracking, the visual OLED refresh cycles (500ms) and the dynamic buzzer pacing run completely concurrently without blocking the sensor from taking rapid measurements.
* **Zone Logic Engine:** * **SAFE (>60cm):** Green LED active, Buzzer completely silent.
  * **CAUTION (30–60cm):** Yellow LED active, Slow Beep (800ms interval).
  * **CLOSE (15–30cm):** Yellow and Red LEDs active concurrently, Fast Beep (300ms interval).
  * **DANGER (<15cm):** Red LED active, Continuous emergency tone.