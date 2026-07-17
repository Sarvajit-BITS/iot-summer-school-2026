# Project 1: Smart Room Climate Monitor

## Expected Output
A local weather station utilizing an ESP32. The OLED toggles every 5 seconds between real-time data and historical min/max values. If the temperature exceeds 38°C or humidity exceeds 80%, the Green LED turns off, the Red LED turns on, and the Buzzer sounds for 1 second. 

## Library Versions
* DHTesp: v1.19.0
* Adafruit GFX Library: v1.11.9
* Adafruit SSD1306: v2.5.9

## Wiring Diagram
* **DHT11**: DATA -> GPIO4 (with 10k pull-up)
* **OLED**: SDA -> GPIO21, SCL -> GPIO22
* **Red LED**: GPIO25
* **Green LED**: GPIO26
* **Buzzer**: GPIO27