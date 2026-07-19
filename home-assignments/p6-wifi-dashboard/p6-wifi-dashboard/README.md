# Project 6: Wi-Fi Weather Dashboard 🌦️

## Project Overview
This project transforms an ESP32 into a standalone, local-network web server that monitors environmental conditions in real-time. The system provides a dual-interface experience: local status updates on an I2C OLED display and a responsive, dynamic web dashboard accessible from any browser on the same Wi-Fi network.

## Hardware Components
* **Microcontroller:** ESP32 Dev Board
* **Sensors:**
    * DHT11 (Temperature & Humidity)
    * HW-486 LDR Module (Light Intensity)
* **Display:** 0.96" I2C OLED (SSD1306)
* **Infrastructure:** Breadboard, Jumper Wires

## Wiring Architecture Matrix
| Component | Pin | ESP32 Pin | Interface |
| :--- | :--- | :--- | :--- |
| **OLED** | SDA | GPIO 21 | I2C Data |
| **OLED** | SCL | GPIO 22 | I2C Clock |
| **DHT11** | DATA | GPIO 4 | Digital I/O |
| **HW-486 LDR** | S | GPIO 34 | Analog Input |
| **All Modules**| VCC/GND | 3.3V / GND | Shared Power Rail |

## Technical Implementation Details
* **Asynchronous Web Server:** Built using `ESPAsyncWebServer`, ensuring the device serves web pages without blocking critical sensor reading loops.
* **Template Processing:** The server utilizes dynamic template processing to inject real-time sensor variables (`%TEMP%`, `%HUMIDITY%`, `%LIGHT%`) directly into the HTML source at runtime.
* **Data Visualization:** Integrated **Chart.js** via CDN, rendering three separate, color-coded line graphs to track environmental trends over the last 2 hours at 5-minute intervals.
* **Inverted LDR Logic:** The HW-486 module signal is calibrated using `map(rawLight, 0, 4095, 100, 0)`. This inversion ensures a logical 0–100% light scale where 100% represents maximum ambient light.
* **Rendering Bug Fix:** To prevent the ESP32 template processor from deleting HTML content, percentage symbols in the HTML were replaced with the HTML entity `&#37;`.

## System Features
* **Three-Card UI:** Displays Temperature, Humidity, and Light Level in distinct, responsive dashboard cards.
* **Trend Analysis:** Three dedicated, independent line graphs provide visual data history.
* **Local IP Reporting:** The device automatically displays its assigned IP address on the OLED screen at boot, facilitating easy dashboard access.
* **Adaptive Styling:** The webpage background shifts color dynamically (Blue for cool, Orange for hot) based on real-time temperature thresholds.

## How to Deploy
1. **Prerequisites:** Install the following libraries via Arduino Library Manager or Manual ZIP:
    * `DHT sensor library` (Adafruit)
    * `Adafruit SSD1306` & `Adafruit GFX`
    * `ESPAsyncWebServer` & `AsyncTCP`
2. **Configuration:** Update the `ssid` and `password` constants in the `.ino` file to match your local Wi-Fi credentials.
3. **Upload:** Compile and flash the code to your ESP32.
4. **Access:** Observe the IP address displayed on the OLED screen, then navigate to that address (e.g., `http://192.168.x.x`) in any browser on your connected device.

---
*Developed for IIT Jammu Summer School 2026 | IoT Assignment Module 6*