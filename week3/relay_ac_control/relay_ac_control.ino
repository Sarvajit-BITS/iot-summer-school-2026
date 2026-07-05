/*
 * Relay-Controlled AC Device Simulation (Hysteresis & Override)
 * Path: /week3/relay_ac_control/relay_ac_control.ino
 */

#include "DHT.h"

// Pin Definitions
#define DHTPIN 2
#define DHTTYPE DHT22 // Using Wokwi's default DHT22
const int buttonPin = 3;
const int relayPin = 4;

// Initialize DHT
DHT dht(DHTPIN, DHTTYPE);

// State Tracking
bool relayState = false;
bool manualOverride = false;
bool lastButtonState = HIGH;

// Non-blocking timer for DHT (Requires ~2 seconds between reads)
unsigned long previousMillis = 0;
const long dhtInterval = 2000; 

void setup() {
  Serial.begin(9600);
  dht.begin();
  
  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  
  digitalWrite(relayPin, LOW); // Ensure AC is off at startup
  
  Serial.println("AC Thermostat Controller Initialized.");
  Serial.println("Mode: AUTO");
}

void loop() {
  // --- 1. OVERRIDE BUTTON LOGIC (Edge Detection) ---
  bool currentButtonState = digitalRead(buttonPin);
  
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    manualOverride = !manualOverride; // Toggle between AUTO and OVERRIDE
    
    if (manualOverride) {
      relayState = !relayState; // Flip the relay to the opposite state
      Serial.print("EVENT: Manual Override ENGAGED. AC manually turned ");
      Serial.println(relayState ? "ON" : "OFF");
    } else {
      Serial.println("EVENT: Manual Override DISABLED. Returning to AUTO.");
    }
    
    digitalWrite(relayPin, relayState ? HIGH : LOW);
    delay(50); // Button Debounce
  }
  lastButtonState = currentButtonState;

  // --- 2. TEMPERATURE SENSING & HYSTERESIS ---
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= dhtInterval) {
    previousMillis = currentMillis;
    
    float currentTemp = dht.readTemperature();
    
    if (isnan(currentTemp)) {
      Serial.println("Error reading DHT sensor!");
      return;
    }

    // Only apply temperature logic if override is disabled
    if (!manualOverride) {
      // Turn ON if > 32C
      if (currentTemp > 32.0 && relayState == false) {
        relayState = true;
        digitalWrite(relayPin, HIGH);
        logEvent("AC turned ON (Temp exceeded 32 C)", currentTemp);
      } 
      // Turn OFF if < 28C
      else if (currentTemp < 28.0 && relayState == true) {
        relayState = false;
        digitalWrite(relayPin, LOW);
        logEvent("AC turned OFF (Temp dropped below 28 C)", currentTemp);
      }
    }
  }
}

// Helper function to keep Serial output clean
void logEvent(String reason, float temp) {
  Serial.print("EVENT: ");
  Serial.print(reason);
  Serial.print(" | Current Temp: ");
  Serial.print(temp, 1);
  Serial.println(" C");
}
