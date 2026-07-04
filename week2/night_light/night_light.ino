/*
 * PWM Fading Night Light with SOS Mode
 * Path: /week2/night_light/night_light.ino
 */

const int ledPin = 9;   // MUST be a PWM pin (has a ~ symbol)
const int buttonPin = 2;

// State Tracking Variables
int currentMode = 0;          // 0: Slow Breathe, 1: Fast Pulse, 2: SOS
int lastButtonState = LOW;

// SOS State Machine Variables
int sosIndex = 0;
unsigned long previousSosMillis = 0;

// SOS Pattern Array (Even indexes = ON duration, Odd indexes = OFF duration)
// Pattern: S (...) O (---) S (...)
const int sosPattern[] = {
  250, 250, 250, 250, 250, 750, // S: 3 short flashes (250ms), 750ms gap
  750, 250, 750, 250, 750, 750, // O: 3 long flashes (750ms), 750ms gap
  250, 250, 250, 250, 250, 1500 // S: 3 short flashes (250ms), 1500ms word gap
};
const int sosLength = 18; // Total items in the array

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  
  Serial.println("System Ready. Mode 1: Slow Breathing (3s)");
}

void loop() {
  unsigned long currentMillis = millis();

  // --- 1. BUTTON HANDLING (Corrected Edge Detection) ---
  int reading = digitalRead(buttonPin);
  
  // Detect the exact moment the button changes from unpressed (LOW) to pressed (HIGH)
  if (reading == HIGH && lastButtonState == LOW) {
    
    // Change modes
    currentMode++;
    if (currentMode > 2) {
      currentMode = 0; // Loop back to the first mode
    }
    
    // Reset variables to ensure clean transitions
    sosIndex = 0; 
    previousSosMillis = currentMillis;
    
    // Print the new mode to Serial Monitor
    Serial.print("Mode Switched: ");
    if (currentMode == 0) Serial.println("1 - Slow Breathing (3s cycle)");
    else if (currentMode == 1) Serial.println("2 - Fast Pulse (0.5s cycle)");
    else if (currentMode == 2) Serial.println("3 - SOS Pattern");
  }
  
  // Save the current state for the next time the loop runs
  lastButtonState = reading;

  // --- 2. LED MODE EXECUTION ---
  if (currentMode == 0) {
    runBreathing(3000, currentMillis); // 3000ms = 3 seconds
  } 
  else if (currentMode == 1) {
    runBreathing(500, currentMillis);  // 500ms = 0.5 seconds
  } 
  else if (currentMode == 2) {
    runSOS(currentMillis);
  }
}

// --- HELPER FUNCTIONS ---

// Function to handle the PWM Fading mathematically without delay()
void runBreathing(int cycleDuration, unsigned long currentMillis) {
  // Use modulo to find our exact position inside the current cycle loop
  unsigned long timeInCycle = currentMillis % cycleDuration;
  int halfCycle = cycleDuration / 2;
  int brightness = 0;

  if (timeInCycle < halfCycle) {
    // First half of the cycle: Fade UP (0 to 255)
    brightness = map(timeInCycle, 0, halfCycle, 0, 255);
  } else {
    // Second half of the cycle: Fade DOWN (255 to 0)
    brightness = map(timeInCycle, halfCycle, cycleDuration, 255, 0);
  }
  
  analogWrite(ledPin, brightness);
}

// Function to handle the SOS pattern without delay()
void runSOS(unsigned long currentMillis) {
  // Check if it is time to move to the next ON/OFF step in the pattern array
  if (currentMillis - previousSosMillis >= sosPattern[sosIndex]) {
    previousSosMillis = currentMillis;
    sosIndex++;
    
    // If we reached the end of the pattern, loop back to the beginning
    if (sosIndex >= sosLength) {
      sosIndex = 0;
    }
  }

  // Even indexes in our array are ON states, Odd indexes are OFF states
  if (sosIndex % 2 == 0) {
    analogWrite(ledPin, 255); // Full brightness
  } else {
    analogWrite(ledPin, 0);   // Off
  }
}
