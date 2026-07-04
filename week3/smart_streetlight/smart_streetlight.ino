/*
 * Smart Street Light (LDR + PIR)
 * Path: /week3/smart_streetlight/smart_streetlight.ino
 */

// Pin Definitions
const int pirPin = 2;
const int ledPin = 9;  // Must be PWM
const int ldrPin = A0;

// Thresholds & Timing
const int ldrThreshold = 500; // Adjust based on lighting conditions
const unsigned long motionDuration = 30000; // 30 seconds

// PWM Values
const int brightnessFull = 255;
const int brightnessDim = 51; // ~20% of 255
const int brightnessOff = 0;

// State Tracking
enum LightState { DAY_MODE, NIGHT_DIM, NIGHT_BRIGHT };
LightState currentState = DAY_MODE;

unsigned long lastMotionTime = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  // A0 is input by default, no pinMode needed for analogRead
  
  logEvent("System Initialized. Mode: DAY_MODE");
  analogWrite(ledPin, brightnessOff);
}

void loop() {
  int ldrValue = analogRead(ldrPin);
  int pirState = digitalRead(pirPin);
  unsigned long currentMillis = millis();

  // --- 1. DAYLIGHT CONDITION ---
  if (ldrValue > ldrThreshold) {
    if (currentState != DAY_MODE) {
      analogWrite(ledPin, brightnessOff);
      currentState = DAY_MODE;
      logEvent("Daylight detected. LED turned OFF.");
    }
  } 
  
  // --- 2. NIGHT TIME CONDITIONS ---
  else {
    // A. Motion Detected
    if (pirState == HIGH) {
      lastMotionTime = currentMillis; // Reset the 30-second timer
      
      if (currentState != NIGHT_BRIGHT) {
        analogWrite(ledPin, brightnessFull);
        currentState = NIGHT_BRIGHT;
        logEvent("Motion detected at night. LED FULL BRIGHTNESS.");
      }
    } 
    // B. No Motion Detected
    else {
      // Transition from Bright to Dim after 30 seconds
      if (currentState == NIGHT_BRIGHT && (currentMillis - lastMotionTime >= motionDuration)) {
        analogWrite(ledPin, brightnessDim);
        currentState = NIGHT_DIM;
        logEvent("30s elapsed with no motion. LED DIMMED (20%).");
      }
      // Transition from Day directly to Night Dim (Sun goes down, no motion yet)
      else if (currentState == DAY_MODE) {
        analogWrite(ledPin, brightnessDim);
        currentState = NIGHT_DIM;
        logEvent("Nightfall detected. LED DIMMED (20%).");
      }
    }
  }
}

// --- HELPER FUNCTION: Event Logger ---
// Converts millis() into HH:MM:SS format and prints the required event string
void logEvent(String description) {
  unsigned long totalSeconds = millis() / 1000;
  int hours = (totalSeconds / 3600) % 24;
  int minutes = (totalSeconds / 60) % 60;
  int seconds = totalSeconds % 60;

  // Format string with leading zeros
  char timeString[11];
  sprintf(timeString, "[%02d:%02d:%02d]", hours, minutes, seconds);

  Serial.print(timeString);
  Serial.print(" EVENT: ");
  Serial.println(description);
}
