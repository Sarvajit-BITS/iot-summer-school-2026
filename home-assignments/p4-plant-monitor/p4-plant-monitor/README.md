# ESP32 Smart Plant Watering Monitor

An automated, non-blocking plant monitoring and irrigation system powered by an **ESP32**. The system tracks soil moisture levels and ambient temperature, automatically triggering a water pump via a relay while allowing manual override and live status updates on an OLED display.

---

## Features

- **Automated Irrigation** — Hysteresis-based moisture control (triggers when Dry < 30%, turns off when Wet > 40%).
- **Non-Blocking Architecture** — Built using `millis()` timing loops (zero `delay()`), ensuring continuous sensor polling, smooth debouncing, and responsive OLED updates.
- **Manual Override** — Dedicated push button to manually trigger the pump for a fixed 5-second burst.
- **Visual & Audio Feedback** — Real-time metrics shown on a 0.96" SSD1306 OLED display, status indicator LEDs, and an active buzzer for pump notifications.
- **Environmental Sensing** — Integrated DHT11 sensor for ambient temperature reporting.

---

## Hardware Requirements

| Component | Quantity | Notes / Model |
|---|---|---|
| Microcontroller | 1 | ESP32 Development Board |
| Soil Moisture Sensor | 1 | Capacitive Soil Moisture Sensor v2.0 |
| Temp Sensor | 1 | DHT11 Temperature & Humidity Sensor |
| Display | 1 | 0.96" I2C OLED (SSD1306, 128x64, Address `0x3C`) |
| Relay Module | 1 | 2-Channel Relay Module (controls 5V DC mini pump) |
| Pump | 1 | 5V DC Submersible Mini Water Pump |
| Inputs | 1 | Push Button (tactile) for manual override |
| Outputs | 1 | Active Buzzer + Status LEDs (Dry / Wet / Pump active) |

---

## Pin Mapping Table

| Component Pin | ESP32 GPIO | Description |
|---|---|---|
| Capacitive Sensor (AOUT) | GPIO 34 | Analog input (ADC1) |
| DHT11 Data Pin | GPIO 23 | Digital temperature input |
| I2C OLED SDA | GPIO 21 | I2C data line |
| I2C OLED SCL | GPIO 22 | I2C clock line |
| Pump Relay Control | GPIO 26 | Output (active LOW / HIGH depending on relay) |
| Manual Button Pin | GPIO 14 | Input with internal pull-up / external debounce |
| Active Buzzer | GPIO 27 | Audible alert on pump trigger |
| Status LEDs | GPIO 12 / 13 | Indicator LEDs for system status |

> **Note on Sensor Power:** If your Capacitive v2.0 sensor outputs 0 across dry and wet conditions, power the sensor module using the 5V / VIN rail instead of 3.3V to satisfy the onboard 555 timer regulator headroom.

---

## Dependencies & Libraries

Ensure the following libraries are installed via the Arduino IDE Library Manager:

- `Adafruit_SSD1306` (for 0.96" I2C OLED display)
- `Adafruit_GFX` (graphics core library)
- `DHT sensor library` by Adafruit (for DHT11)
- `Adafruit Unified Sensor`

---

## Calibration & Thresholds

Capacitive sensors output **lower** raw ADC values as moisture increases. Adjust these constants in your main sketch file based on your raw Serial Monitor output:

```cpp
// ADC Calibration Limits (12-bit ADC: 0 - 4095)
const int DRY_VALUE = 3200;  // Raw ADC in dry air
const int WET_VALUE = 1300;  // Raw ADC fully submerged in water

// Moisture Percentage Thresholds
const int DRY_THRESHOLD = 30; // Pump ON below this %
const int WET_THRESHOLD = 40; // Pump OFF above this %
```

---

## Troubleshooting

**Moisture stuck at 100% (Raw ADC = 0):**
- Check for short circuits along the `AOUT` pin or moisture touching the upper circuitry of the sensor.
- If the raw ADC reads 0 in dry air even after replacing jumper wires and powering via 5V, the onboard 555 IC on the Capacitive v2.0 module is defective and needs replacement.

**OLED Screen Blank / Not Allocating:**
- Verify the I2C address is set to `0x3C` in `display.begin(SSD1306_SWITCHCAPVCC, 0x3C)`.
- Ensure SDA and SCL are connected to GPIO 21 and GPIO 22 respectively.
