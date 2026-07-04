/*
 * Environmental Monitoring Station (DHT11/22)
 * Path: /week3/environmental_monitor/environmental_monitor.ino
 */

#include "DHT.h"

#define DHTPIN 2        // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // Using DHT22 (Wokwi default). Change to DHT11 for physical hardware.

DHT dht(DHTPIN, DHTTYPE);

const int redLed = 3;
const int greenLed = 4;

unsigned long previousMillis = 0;
const long interval = 2000; // Read every 2 seconds

void setup() {
  Serial.begin(9600);
  
  // Initialize Sensor and Pins
  dht.begin();
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  
  // Print CSV Header to Serial Monitor
  Serial.println("timestamp,temp_C,temp_F,humidity");
}

void loop() {
  unsigned long currentMillis = millis();

  // Non-blocking 2-second timer
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Read humidity and temperature
    float h = dht.readHumidity();
    float tC = dht.readTemperature();       // Celsius
    float tF = dht.readTemperature(true);   // Fahrenheit

    // Check if any reads failed and exit early (to try again)
    if (isnan(h) || isnan(tC) || isnan(tF)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Print to Serial Monitor in CSV format
    Serial.print(currentMillis / 1000); // Timestamp in seconds
    Serial.print(",");
    Serial.print(tC);
    Serial.print(",");
    Serial.print(tF);
    Serial.print(",");
    Serial.println(h);

    // Alert Logic: Red LED if Temp > 35C OR Humidity > 80%
    if (tC > 35.0 || h > 80.0) {
      digitalWrite(redLed, HIGH);
      digitalWrite(greenLed, LOW);
    } else {
      digitalWrite(redLed, LOW);
      digitalWrite(greenLed, HIGH);
    }
  }
}
