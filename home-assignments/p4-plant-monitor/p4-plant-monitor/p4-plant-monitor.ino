#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>  // Standard 0.96" SSD1306 OLED Driver
#include <DHT.h>                // DHT Sensor Library

// --- Pin Definitions ---
#define SOIL_PIN        34      // ADC Pin for Capacitive Soil Moisture
#define DHT_PIN         4       // GPIO Pin for DHT11
#define BUTTON_PIN      0       // Manual Override Button (INPUT_PULLUP)
#define RELAY_PUMP      26      // Relay Channel 1 (Active-LOW)
#define BUZZER_PIN      13      // Active Buzzer
#define LED_GREEN       18      // Status LED: Optimal/Wet
#define LED_RED         19      // Status LED: Dry / Warning

// --- Sensor Settings ---
#define DHTTYPE         DHT11
DHT dht(DHT_PIN, DHTTYPE);

// --- OLED Settings (0.96" SSD1306 Display) ---
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Calibration Values ---
const int DRY_VALUE = 3200;   // Your dry air reading
const int WET_VALUE = 1400;   // Your water cup reading

// --- State Variables ---
bool pumpOn = false;
bool isManualOverride = false;
unsigned long overrideStartTime = 0;
const unsigned long OVERRIDE_DURATION = 5000; // 5 seconds override

int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 50;

void drawMoistureBar(int percent) {
  int barWidth = map(percent, 0, 100, 0, 120);
  display.drawRect(4, 52, 120, 10, SSD1306_WHITE);
  display.fillRect(4, 52, barWidth, 10, SSD1306_WHITE);
}

void setup() {
  Serial.begin(115200);

  // Pin Configuration
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RELAY_PUMP, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // Relay default OFF (Active-LOW: HIGH = OFF)
  digitalWrite(RELAY_PUMP, HIGH);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);

  // Initialize Sensors
  dht.begin();

  // Initialize 0.96" SSD1306 OLED Display (Address 0x3C)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 OLED allocation failed!"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 25);
  display.println(F("Initializing System..."));
  display.display();
  delay(1500);
}

void loop() {
  // 1. Read Manual Override Button
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading == LOW && !isManualOverride) {
      isManualOverride = true;
      overrideStartTime = millis();
      pumpOn = true;
      digitalWrite(RELAY_PUMP, LOW); // Active-LOW ON
      Serial.println(F("[OVERRIDE] Manual watering triggered for 5s"));
    }
  }
  lastButtonState = reading;

  // 2. Handle Manual Override Expiration
  if (isManualOverride) {
    if (millis() - overrideStartTime >= OVERRIDE_DURATION) {
      isManualOverride = false;
      pumpOn = false;
      digitalWrite(RELAY_PUMP, HIGH); // Turn OFF
      Serial.println(F("[OVERRIDE] Resuming Auto Mode."));
    }
  }

  // 3. Read Sensors
  int rawMoisture = analogRead(SOIL_PIN);
  int moisturePercent = map(rawMoisture, DRY_VALUE, WET_VALUE, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  float tempC = dht.readTemperature();
  if (isnan(tempC)) tempC = 0.0;

  // 4. Automatic Hysteresis & State Logic
  String statusStr = "";
  if (!isManualOverride) {
    if (!pumpOn && moisturePercent < 30) {
      pumpOn = true;
      digitalWrite(RELAY_PUMP, LOW); // Turn Pump ON
      digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW);
    } else if (pumpOn && moisturePercent > 40) {
      pumpOn = false;
      digitalWrite(RELAY_PUMP, HIGH); // Turn Pump OFF
    }
  }

  // Status Indicators
  if (moisturePercent < 30) {
    statusStr = "DRY";
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
  } else if (moisturePercent <= 70) {
    statusStr = "OPTIMAL";
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
  } else {
    statusStr = "WET";
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
  }

  // 5. Serial Monitor Output
  Serial.print(F("Raw ADC: ")); Serial.print(rawMoisture);
  Serial.print(F(" | Moisture: ")); Serial.print(moisturePercent); Serial.print(F("%"));
  Serial.print(F(" | Temp: ")); Serial.print(tempC); Serial.print(F("C"));
  Serial.print(F(" | Status: ")); Serial.print(statusStr);
  Serial.print(F(" | Pump: ")); Serial.println(pumpOn ? "ON" : "OFF");

  // 6. OLED Screen Update
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("PLANT MONITOR "));
  if (isManualOverride) display.print(F("[MANUAL]"));
  else display.print(F("[AUTO]"));
  
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  display.setCursor(0, 16);
  display.print(F("Moisture: ")); display.print(moisturePercent); display.println(F("%"));
  
  display.setCursor(0, 28);
  display.print(F("Temp:     ")); display.print(tempC, 1); display.println(F(" C"));

  display.setCursor(0, 40);
  display.print(F("Status:   ")); display.println(statusStr);

  drawMoistureBar(moisturePercent);
  display.display();

  delay(200);
}