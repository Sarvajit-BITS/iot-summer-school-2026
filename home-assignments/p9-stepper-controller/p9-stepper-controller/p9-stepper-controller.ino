#include <Stepper.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// --- OLED Setup (GME12864-81 / SH1106) ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- Stepper Setup ---
const int STEPS_PER_REV = 2048; 
// Wiring sequence for ULN2003: Pins 8, 10, 9, 11 (IN2 and IN3 swapped for library sync)
Stepper myStepper(STEPS_PER_REV, 8, 10, 9, 11); 

// --- Pin Definitions ---
const int POT_PIN = A0;
const int BTN_CW = 2;
const int BTN_CCW = 3;
const int BTN_HOME = 4;

// --- System Variables ---
long currentStepPosition = 0; 
float currentAngle = 0.0;
float targetAngle = 0.0;
String lastDirection = "IDLE";
int moveCounter = 0; // Tracks moves for your GitHub deliverable log

// Debounce & Refresh Timers
unsigned long lastDebounceTime = 0;
const long debounceDelay = 250;
unsigned long lastOledUpdate = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(BTN_CW, INPUT_PULLUP);
  pinMode(BTN_CCW, INPUT_PULLUP);
  pinMode(BTN_HOME, INPUT_PULLUP);

  // Initialize SH1106 OLED
  if(!display.begin(0x3C, true)) {
    Serial.println(F("SH1106 OLED allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 10);
  display.println("Precision Positioner");
  display.display();
  delay(1000);
  
  // Set safe operating speed (8 RPM prevents step skipping/drift)
  myStepper.setSpeed(8); 

  Serial.println("==========================================");
  Serial.println("PROJECT 9: STEPPER MOTOR POSITION CONTROLLER");
  Serial.println("==========================================");
}

void loop() {
  // 1. Read Potentiometer to set Target Angle (0 - 360 degrees)
  // (Using average filter to eliminate noise)
  long rawPotSum = 0;
  for(int i = 0; i < 15; i++) {
    rawPotSum += analogRead(POT_PIN);
    delayMicroseconds(50);
  }
  int potValue = rawPotSum / 15;
  targetAngle = map(potValue, 0, 1023, 0, 360);

  // 2. Read Manual Buttons (LOW means pressed due to INPUT_PULLUP)
  
  // CW Button: Rotate motor Clockwise by the target angle set by potentiometer
  if (digitalRead(BTN_CW) == LOW && (millis() - lastDebounceTime > debounceDelay)) {
    lastDebounceTime = millis();
    
    // Calculate steps required for the target angle
    long stepsToMove = (targetAngle / 360.0) * STEPS_PER_REV;
    
    moveMotor(stepsToMove, "CW");
  }
  
  // CCW Button: Rotate motor Counter-Clockwise by the target angle set by potentiometer
  if (digitalRead(BTN_CCW) == LOW && (millis() - lastDebounceTime > debounceDelay)) {
    lastDebounceTime = millis();
    
    // Calculate steps required for the target angle (negative for CCW)
    long stepsToMove = -((targetAngle / 360.0) * STEPS_PER_REV);
    
    moveMotor(stepsToMove, "CCW");
  }

  // HOME Button: Reset current position reference to 0 degrees
  if (digitalRead(BTN_HOME) == LOW && (millis() - lastDebounceTime > debounceDelay)) {
    lastDebounceTime = millis();
    currentStepPosition = 0; 
    lastDirection = "HOME";
    logMove("HOME-RESET");
  }

  // 3. Math: Convert absolute steps to a normalized 0-360 Degree reading for display
  long normalizedSteps = currentStepPosition % STEPS_PER_REV;
  if (normalizedSteps < 0) normalizedSteps += STEPS_PER_REV; 
  currentAngle = (normalizedSteps * 360.0) / (float)STEPS_PER_REV;

  // 4. Smooth OLED Refresh (Every 150ms)
  if (millis() - lastOledUpdate > 150) {
    updateOLED();
    lastOledUpdate = millis();
  }
}

void moveMotor(long steps, String dir) {
  lastDirection = dir;
  myStepper.step(steps);
  currentStepPosition += steps;
  logMove(dir);
}

void logMove(String actionType) {
  moveCounter++;
  
  long normSteps = currentStepPosition % STEPS_PER_REV;
  if (normSteps < 0) normSteps += STEPS_PER_REV; 
  float calcAngle = (normSteps * 360.0) / (float)STEPS_PER_REV;

  Serial.print("[Move #");
  if (moveCounter < 10) Serial.print("0");
  Serial.print(moveCounter);
  Serial.print("] Action: ");
  Serial.print(actionType);
  Serial.print("\t| Target Set: ");
  Serial.print((int)targetAngle);
  Serial.print(" deg\t| Current Angle: ");
  Serial.print((int)calcAngle);
  Serial.print(" deg\t| Abs Steps: ");
  Serial.println(currentStepPosition);
}

void updateOLED() {
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("STATUS: ");
  display.print(lastDirection);
  
  display.setCursor(0, 18);
  display.print("Target:  ");
  display.print((int)targetAngle);
  display.print(" deg");
  
  display.setCursor(0, 32);
  display.print("Current: ");
  display.print((int)currentAngle);
  display.print(" deg");
  
  display.setCursor(0, 48);
  display.print("Moves:   ");
  display.print(moveCounter);

  display.display();
}