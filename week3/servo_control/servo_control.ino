/*
 * Servo Motor Sweep Control
 * Path: /week3/servo_control/servo_control.ino
 */

#include <Servo.h>

// Create a servo object
Servo myServo;

// Pin definitions
const int potPin = A0;
const int buttonPin = 2;
const int servoPin = 9;

void setup() {
  Serial.begin(9600);
  
  // Attach the servo to the PWM pin
  myServo.attach(servoPin);
  
  // Use INPUT_PULLUP to activate the internal resistor. 
  // This means the button reads HIGH when unpressed, and LOW when pressed.
  pinMode(buttonPin, INPUT_PULLUP);
  
  Serial.println("Servo Control System Initialized...");
}

void loop() {
  // Read the button state
  bool isButtonPressed = (digitalRead(buttonPin) == LOW);

  // --- OVERRIDE MODE: Button Sweep ---
  if (isButtonPressed) {
    Serial.println("Button Pressed! Executing 0 -> 180 -> 0 Sweep");
    
    // Sweep forward to 180
    for (int angle = 0; angle <= 180; angle += 2) {
      myServo.write(angle);
      delay(15); 
    }
    
    // Sweep backward to 0
    for (int angle = 180; angle >= 0; angle -= 2) {
      myServo.write(angle);
      delay(15);
    }
    
    Serial.println("Sweep Complete. Returning to Potentiometer control.");
    delay(200); // Small debounce delay to prevent rapid re-triggering
  } 
  
  // --- NORMAL MODE: Potentiometer Control ---
  else {
    int potValue = analogRead(potPin);
    
    // Map the 0-1023 analog reading to a 0-180 degree angle
    int targetAngle = map(potValue, 0, 1023, 0, 180);
    
    // Command the servo to move to that angle
    myServo.write(targetAngle);
    
    // Log data to Serial Monitor
    Serial.print("Potentiometer: ");
    Serial.print(potValue);
    Serial.print(" | Servo Angle: ");
    Serial.print(targetAngle);
    Serial.println("°");
    
    delay(100); // Small delay to keep the Serial Monitor readable
  }
}
