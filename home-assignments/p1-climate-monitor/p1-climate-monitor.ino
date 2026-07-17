/*
 * Project 1: Smart Room Climate Monitor
 * Path: /home-assignments/p1_climate_monitor/p1_climate_monitor.ino
 * Libraries Required: DHTesp, Adafruit GFX, Adafruit SSD1306
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>

// --- Component Setup ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

DHTesp dht;
const int DHT_PIN = 4;
const int RED_LED = 25;
const int GREEN_LED = 26;
const int BUZZER = 27;

// --- Timers (Non-blocking) ---
unsigned long lastSensorRead = 0;
unsigned long lastCsvLog = 0;
unsigned long lastScreenSwap = 0;
unsigned long buzzerTurnOffTime = 0;

// --- State Variables ---
float maxTemp = -100.0; // Start impossibly low
float minTemp = 100.0;  // Start impossibly high
bool showMainScreen = true;
bool isBuzzerActive = false;

void setup() {
  Serial.begin(115200);
  
  // Initialize Pins
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BUZZER, LOW);

  // Initialize DHT
  dht.setup(DHT_PIN, DHTesp::DHT11);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for(;;); // Halt if screen fails
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  
  Serial.println("millis(),temp,humidity,status"); // CSV Header
}

void loop() {
  unsigned long currentMillis = millis();

  // --- 1. Read Sensor Data (Every 2 seconds) ---
  if (currentMillis - lastSensorRead >= 2000) {
    lastSensorRead = currentMillis;
    
    TempAndHumidity data = dht.getTempAndHumidity();
    
    if (dht.getStatus() != 0) {
      return; // Skip cycle if read failed
    }

    // Update Min/Max tracking
    if (data.temperature > maxTemp) maxTemp = data.temperature;
    if (data.temperature < minTemp) minTemp = data.temperature;

    // Determine Status & Comfort Index
    String status = "COMFORT";
    if (data.temperature > 38 || data.humidity > 80) {
      status = "DANGER";
    } else if (data.temperature > 32) {
      status = "HOT";
    } else if (data.temperature < 20) {
      status = "COOL";
    }

    // Execute Threshold Logic
    if (status == "DANGER") {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BUZZER, HIGH);
      isBuzzerActive = true;
      buzzerTurnOffTime = currentMillis + 1000; // Schedule buzzer off in 1 sec
    } else {
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
    }

    updateOLED(data.temperature, data.humidity, status);
  }

  // --- 2. Turn off Buzzer after 1 second ---
  if (isBuzzerActive && currentMillis >= buzzerTurnOffTime) {
    digitalWrite(BUZZER, LOW);
    isBuzzerActive = false;
  }

  // --- 3. Screen Cycling Logic (Every 5 seconds) ---
  if (currentMillis - lastScreenSwap >= 5000) {
    lastScreenSwap = currentMillis;
    showMainScreen = !showMainScreen; // Toggle between main and min/max screen
    
    // Force an immediate screen update with current cached values
    TempAndHumidity cachedData = dht.getTempAndHumidity();
    String currentStatus = (cachedData.temperature > 38 || cachedData.humidity > 80) ? "DANGER" : "SAFE";
    updateOLED(cachedData.temperature, cachedData.humidity, currentStatus);
  }

  // --- 4. CSV Serial Logging (Every 5 seconds) ---
  if (currentMillis - lastCsvLog >= 5000) {
    lastCsvLog = currentMillis;
    
    TempAndHumidity logData = dht.getTempAndHumidity();
    String logStatus = (logData.temperature > 38 || logData.humidity > 80) ? "DANGER" : "SAFE";
    
    Serial.print(currentMillis);
    Serial.print(",");
    Serial.print(logData.temperature, 1);
    Serial.print(",");
    Serial.print(logData.humidity, 1);
    Serial.print(",");
    Serial.println(logStatus);
  }
}

// --- Helper Function: Update OLED Display ---
void updateOLED(float temp, float hum, String status) {
  display.clearDisplay();
  
  if (showMainScreen) {
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.print("T: "); display.print(temp, 1); display.println("C");
    display.print("H: "); display.print(hum, 1); display.println("%");
    
    display.setTextSize(1);
    display.setCursor(0, 45);
    display.print("Status: "); 
    display.print(status);
  } else {
    // Bonus Challenge Screen
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("--- DAILY EXTREMES ---");
    display.println("");
    display.print("Max Temp: "); display.print(maxTemp, 1); display.println(" C");
    display.println("");
    display.print("Min Temp: "); display.print(minTemp, 1); display.println(" C");
  }
  
  display.display();
}