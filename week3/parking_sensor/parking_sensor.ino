/*
 * Ultrasonic Parking Sensor (Non-Blocking)
 * Path: /week3/parking_sensor/parking_sensor.ino
 * Bonus: Fully implemented using millis() instead of delay()
 */

// Pin Definitions
const int trigPin = 9;
const int echoPin = 10;
const int yellowLed = 5;
const int redLed = 6;
const int buzzerPin = 7;

// Variables for calculation
long duration;
float distance;

// Non-blocking timer variables
unsigned long previousPingMillis = 0;
unsigned long previousBuzzerMillis = 0;
unsigned long previousLedMillis = 0;

// State toggles for flashing and beeping
bool buzzerState = false;
bool ledState = false;

void setup() {
  Serial.begin(9600);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  pinMode(yellowLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  Serial.println("Parking Sensor Initialized...");
}

void loop() {
  unsigned long currentMillis = millis();

  // --- 1. SENSOR READING (Every 100ms to avoid spamming) ---
  if (currentMillis - previousPingMillis >= 100) {
    previousPingMillis = currentMillis;

    // Send the ultrasonic pulse
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2); 
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Read the pulse duration and apply the required formula
    duration = pulseIn(echoPin, HIGH);
    distance = (duration * 0.034) / 2;

    // Print to Serial Monitor
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" cm - Status: ");
    
    // Handle the 0.00cm timeout glitch properly
    if (distance == 0 || distance > 50) Serial.println("SAFE / OUT OF RANGE");
    else if (distance > 20 && distance <= 50) Serial.println("APPROACHING (20-50cm)");
    else if (distance > 10 && distance <= 20) Serial.println("WARNING (10-20cm)");
    else Serial.println("STOP! (<10cm)");
  }

  // --- 2. ALERT LOGIC (Fully Non-Blocking) ---
  
  // Zone 1: SAFE (> 50cm)
  if (distance > 50 || distance == 0) { 
    digitalWrite(yellowLed, LOW);
    digitalWrite(redLed, LOW);
    noTone(buzzerPin);
  }
  
  // Zone 2: APPROACHING (20cm - 50cm)
  else if (distance > 20 && distance <= 50) {
    digitalWrite(yellowLed, HIGH);
    digitalWrite(redLed, LOW);
    
    // Buzzer beeps every 500ms
    if (currentMillis - previousBuzzerMillis >= 500) {
      previousBuzzerMillis = currentMillis;
      buzzerState = !buzzerState;
      if (buzzerState) tone(buzzerPin, 1000); 
      else noTone(buzzerPin);
    }
  }
  
  // Zone 3: WARNING (10cm - 20cm)
  else if (distance > 10 && distance <= 20) {
    digitalWrite(yellowLed, LOW);
    digitalWrite(redLed, HIGH);
    
    // Buzzer beeps every 200ms
    if (currentMillis - previousBuzzerMillis >= 200) {
      previousBuzzerMillis = currentMillis;
      buzzerState = !buzzerState;
      if (buzzerState) tone(buzzerPin, 1000);
      else noTone(buzzerPin);
    }
  }
  
  // Zone 4: STOP (< 10cm)
  else if (distance > 0 && distance <= 10) {
    // Continuous tone
    tone(buzzerPin, 1000); 
    
    // Rapidly flash both LEDs every 100ms
    if (currentMillis - previousLedMillis >= 100) {
      previousLedMillis = currentMillis;
      ledState = !ledState;
      digitalWrite(yellowLed, ledState);
      digitalWrite(redLed, ledState);
    }
  }
}
