# Project 10: Full IoT Home Automation Hub

An advanced, asynchronous, edge-managed smart home automation hub powered by the **ESP32**. The system provides real-time multi-sensor telemetry, non-blocking local automation rules, safety trip protocols, a local web dashboard via `ESP32Async`, and live cloud telemetry streaming via MQTT.

---

## System Architecture Diagram

```text
                               +-------------------+
                               |  Power Supply     |
                               |  (5V USB / DC)    |
                               +---------+---------+
                                         |
                                         v
+---------------------------------------------------------------------------------+
|                                 ESP32 CORE                                      |
|                                                                                 |
|   +-------------------+   +-------------------+   +-------------------------+   |
|   |   Sensor Engine   |   | Automation Engine |   |  Communication Stack    |   |
|   |  (Non-blocking)   |-->|  (State Machine)  |-->|  (AsyncWeb & MQTT)     |   |
|   +-------------------+   +-------------------+   +-------------------------+   |
+---------^-------------------------------+-------------------+-------------------+
          |                               |                   |
          |                               v                   v
+---------+----------+         +----------+----------+  +-----+-------------------+
|     Sensors        |         |     Actuators       |  |     Outputs / Cloud     |
+--------------------+         +---------------------+  +-------------------------+
| • DHT11 (Temp/Hum) |         | • Fan Relay (GPIO)  |  | • 1.3" OLED (I2C 0x3C)  |
| • MQ-2 (Gas/Smoke) |         | • Light Relay (GPIO)|  | • Local Web Server (:80)|
| • LDR (Ambient Lgt)|         | • Buzzer / Red LED  |  | • HiveMQ Broker (MQTT)  |
| • PIR (Motion)     |         +---------------------+  +-------------------------+
+--------------------+

---
## Automation Rules & Threshold Matrix

| Rule ID | Priority | Trigger Condition | System Action / Response | Hysteresis / Safety Mechanism |
| :--- | :--- | :--- | :--- | :--- |
| **RULE 1** | **High** *(Climate)* | `Temperature > 30.0°C` | **Fan Relay → ON** | Turns OFF when `Temp < 28.0°C` *(2.0°C Hysteresis to prevent relay chatter)* |
| **RULE 2** | **Medium** *(Lighting)* | `Light Level < 40%` **AND** `Motion (PIR) == HIGH` | **Light Relay → ON** | Stays ON for 30 seconds after motion stops being detected |
| **RULE 3** | **CRITICAL** *(Gas Safety)* | `Gas Level > 15%` | • **Emergency Gas Trip Activated**<br>• **Red LED + Buzzer Pulse**<br>• **Safety Relay Shutdown** | Remains in Alert mode until gas levels drop below baseline threshold |

## Manual Override LogicTo ensure user control and flexibility, the system supports both Automatic Mode and Manual Override Mode accessible via the local Async Web Dashboard and MQTT control topics:State Independence:When a relay is set to AUTO, the background state machine evaluates sensor inputs and toggles relays based on Rule 1, Rule 2, and Rule 3.When a manual toggle command is issued (e.g., FAN_ON, FAN_OFF, LIGHT_ON, LIGHT_OFF), the system shifts that specific device into MANUAL mode, ignoring sensor inputs for that load.Safety Emergency Priority:Rule 3 (Gas Emergency) overrules all manual settings.If a gas leakage is detected (Gas Level > 15%), manual overrides are temporarily locked out, and all relays are safely de-energized/tripped until the gas clears.Resetting Overrides:Sending an AUTO command via the dashboard or MQTT returns device control back to the dynamic sensor fusion logic.

## Telemetry & MQTT Data FormatTelemetry is published every 2–5 seconds in JSON format to the topic iitjammu/student/home:
'''
JSON{
  "temp": 30.7,
  "humidity": 55.0,
  "gas": 5,
  "pir": 1,
  "light": 68,
  "fan": "AUTO_OFF",
  "light_relay": "AUTO_OFF",
  "alert": "OK"
}
'''

## Getting Started
**Hardware Connections:** Connect DHT11 to GPIO 4 (or GPIO 16), MQ-2 to GPIO 35, LDR to GPIO 34, PIR to GPIO 13, and I2C OLED to GPIO 21 (SDA) / 22 (SCL).
**Configuration:** Update Wi-Fi credentials (WIFI_SSID and WIFI_PASS) in config.h.
**Libraries Required:** ESP32Async (AsyncTCP & ESPAsyncWebServer)PubSubClient by Nick O'LearyAdafruit SSD1306 & Adafruit GFXDHT sensor library by Adafruit