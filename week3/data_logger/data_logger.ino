/*
 * Multi-Sensor Data Logger
 * Path: /week3/data_logger/data_logger.ino
 */

#include "DHT.h"

// Sensor Pins
#define DHTPIN 2
#define DHTTYPE DHT22 // Wokwi default. Change to DHT11 for physical hardware.
const int trigPin = 9;
const int echoPin = 10;
const int ldrPin = A0;

// Initialize DHT
DHT dht(DHTPIN, DHTTYPE);

// Timer variables
unsigned long previousMillis = 0;
const long interval = 5000; // 5 seconds

void setup() {
  Serial.begin(9600);
  
  dht.begin();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  Serial.println("Multi-Sensor Logger Initialized...");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // --- 1. Read DHT Sensor ---
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    // Fallback if DHT fails
    if (isnan(h) || isnan(t)) {
      t = 0.0;
      h = 0.0;
    }

    // --- 2. Read LDR Sensor ---
    int ldrRaw = analogRead(ldrPin);
    // Convert 0-1023 to 0-100%
    int lightPercent = map(ldrRaw, 0, 1023, 0, 100); 
    
    // Determine light string description
    String lightDesc = "Dark";
    if (lightPercent > 70) lightDesc = "Bright";
    else if (lightPercent > 30) lightDesc = "Moderate";

    // --- 3. Read HC-SR04 Sensor ---
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    long duration = pulseIn(echoPin, HIGH);
    float distance = (duration * 0.034) / 2;

    // --- 4. Formatted Output ---
    Serial.println("=== SENSOR LOG ===");
    
    Serial.print("Time      : ");
    Serial.print(currentMillis);
    Serial.println(" ms");
    
    Serial.print("Temp      : ");
    Serial.print(t, 1); // 1 decimal place
    Serial.print(" C | Humidity: ");
    Serial.print(h, 0); // 0 decimal places
    Serial.println("%");
    
    Serial.print("Light     : ");
    Serial.print(lightPercent);
    Serial.print("% (");
    Serial.print(lightDesc);
    Serial.println(")");
    
    Serial.print("Distance  : ");
    Serial.print(distance, 0);
    Serial.println(" cm");
    
    Serial.println("==================");
  }
}
