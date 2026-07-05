/*
 * DC Motor Speed & Direction Control with L298N
 * Path: /week3/dc_motor_control/dc_motor_control.ino
 */

// Pin Definitions
const int enA = 9;   // PWM Speed Control
const int in1 = 8;   // Direction Pin 1
const int in2 = 7;   // Direction Pin 2
const int potPin = A0;
const int powerBtnPin = 2;
const int dirBtnPin = 3;

// State Variables
bool motorRunning = false;
bool motorForward = true;

// Button Debounce Tracking
bool lastPowerBtnState = HIGH;
bool lastDirBtnState = HIGH;

// Timer for Serial Output
unsigned long previousMillis = 0;
const long serialInterval = 500; // Update serial every 500ms

void setup() {
  Serial.begin(9600);
  
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  
  // Internal pull-ups so we don't need external resistors for the buttons
  pinMode(powerBtnPin, INPUT_PULLUP);
  pinMode(dirBtnPin, INPUT_PULLUP);
  
  // Ensure motor is off at startup
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  analogWrite(enA, 0);
  
  Serial.println("L298N DC Motor Controller Initialized.");
}

void loop() {
  // --- 1. READ INPUTS ---
  bool currentPowerBtnState = digitalRead(powerBtnPin);
  bool currentDirBtnState = digitalRead(dirBtnPin);
  int potValue = analogRead(potPin);
  
  // --- 2. BUTTON EDGE DETECTION (TOGGLES) ---
  
  // Check for Power Button press (HIGH to LOW transition)
  if (lastPowerBtnState == HIGH && currentPowerBtnState == LOW) {
    motorRunning = !motorRunning; 
    delay(50); // Simple debounce
  }
  lastPowerBtnState = currentPowerBtnState;
  
  // Check for Direction Button press (HIGH to LOW transition)
  if (lastDirBtnState == HIGH && currentDirBtnState == LOW) {
    motorForward = !motorForward;
    delay(50); // Simple debounce
  }
  lastDirBtnState = currentDirBtnState;

  // --- 3. MOTOR CONTROL LOGIC ---
  int pwmValue = map(potValue, 0, 1023, 0, 255);
  int speedPercent = map(potValue, 0, 1023, 0, 100);

  if (motorRunning) {
    analogWrite(enA, pwmValue);
    if (motorForward) {
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
    } else {
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
    }
  } else {
    // Stop the motor
    analogWrite(enA, 0);
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }

  // --- 4. SERIAL MONITOR LOGGING ---
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= serialInterval) {
    previousMillis = currentMillis;
    
    Serial.print("State: ");
    Serial.print(motorRunning ? "RUNNING" : "STOPPED");
    Serial.print(" | Direction: ");
    Serial.print(motorForward ? "FORWARD" : "REVERSE");
    Serial.print(" | Speed: ");
    Serial.print(speedPercent);
    Serial.println("%");
  }
}
