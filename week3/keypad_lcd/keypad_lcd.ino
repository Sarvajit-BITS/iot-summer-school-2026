/*
 * Keypad + LCD Security System
 * Path: /week3/keypad_lcd/keypad_lcd.ino
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// --- LCD Setup (I2C Address usually 0x27) ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Keypad Setup ---
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// --- Actuator Pins ---
const int greenLed = 10;
const int redLed = 11;
const int buzzer = 12;

// --- Security Variables ---
const String correctPIN = "1234";
String inputPIN = "";
int failedAttempts = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  resetScreen();
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    // If '*' is pressed, clear the current input (Back/Reset)
    if (key == '*') {
      inputPIN = "";
      resetScreen();
      return;
    }

    // Add key to input string and display a masking character '*'
    inputPIN += key;
    lcd.setCursor(inputPIN.length() - 1, 1);
    lcd.print('*');
    
    // Check PIN when 4 digits are entered
    if (inputPIN.length() == 4) {
      if (inputPIN == correctPIN) {
        grantAccess();
      } else {
        denyAccess();
      }
    }
  }
}

// --- Helper Functions ---

void resetScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ENTER PIN:");
  lcd.setCursor(0, 1);
  inputPIN = "";
  
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);
  noTone(buzzer);
}

void grantAccess() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ACCESS GRANTED");
  
  digitalWrite(greenLed, HIGH);
  failedAttempts = 0; // Reset attempts on success
  
  delay(3000); // Leave message up for 3 seconds
  resetScreen();
}

void denyAccess() {
  failedAttempts++;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ACCESS DENIED");
  
  digitalWrite(redLed, HIGH);
  tone(buzzer, 500); // Annoying buzz
  delay(2000); 
  
  noTone(buzzer);
  digitalWrite(redLed, LOW);
  
  if (failedAttempts >= 3) {
    systemLockdown();
  } else {
    resetScreen();
  }
}

void systemLockdown() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SYSTEM LOCKED!");
  lcd.setCursor(0, 1);
  lcd.print("Wait 10 seconds");
  
  // Flash red LED during lockdown
  for (int i = 0; i < 20; i++) {
    digitalWrite(redLed, HIGH);
    delay(250);
    digitalWrite(redLed, LOW);
    delay(250);
  }
  
  failedAttempts = 0; // Reset after penalty is served
  resetScreen();
}
