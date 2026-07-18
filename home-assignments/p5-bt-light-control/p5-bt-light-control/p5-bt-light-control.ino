#include "BluetoothSerial.h"

// Pin configuration
const int RELAY_1 = 26;
const int RELAY_2 = 27;
const int BUZZER_PIN = 25;
const int BT_LED = 2; // Onboard LED
const int BUTTON_1 = 0;   // Onboard BOOT button
const int BUTTON_2 = 35;  // External button

BluetoothSerial SerialBT;

// Keeping track of lights (false means OFF, true means ON)
bool light1On = false;
bool light2On = false;

// Variables for the 30-minute auto timeout
unsigned long lastActionTime = 0;
const unsigned long TIMEOUT_MS = 1800000; // 30 minutes in milliseconds

// Variables for manual button debouncing
int lastBtn1State = HIGH;
int lastBtn2State = LOW;
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
const unsigned long DEBOUNCE_DELAY = 50; 

void setup() {
  Serial.begin(115200);
  
  // Set up pins
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BT_LED, OUTPUT);
  
  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);

  // Relays are Active-LOW, so HIGH means turned OFF initially
  digitalWrite(RELAY_1, HIGH);
  digitalWrite(RELAY_2, HIGH);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(BT_LED, LOW);

  // Start Bluetooth Classic
  SerialBT.begin("IIT_IoT_HomeCtrl");
  Serial.println("Bluetooth Started! Connect your phone to 'IIT_IoT_HomeCtrl'.");
  
  lastActionTime = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Check if Bluetooth is connected to light up the indicator LED
  if (SerialBT.hasClient()) {
    digitalWrite(BT_LED, HIGH);
  } else {
    digitalWrite(BT_LED, LOW);
  }

  // 2. Handle Incoming Bluetooth Commands
  if (SerialBT.available()) {
    char command = SerialBT.read();
    bool isValid = true;
    
    // Reset the activity timer since we got a command
    lastActionTime = currentMillis;

    if (command == '1') {
      light1On = true;
    } else if (command == '2') {
      light1On = false;
    } else if (command == '3') {
      light2On = true;
    } else if (command == '4') {
      light2On = false;
    } else if (command == '5') {
      light1On = true;
      light2On = true;
    } else if (command == '6') {
      light1On = false;
      light2On = false;
    } else if (command == '?') {
      // Just sending status back
    } else {
      isValid = false; // Ignore any random characters
    }

    if (isValid) {
      updateHardware();
      beepConfirmation();
      sendBluetoothStatus();
    }
  }

  // 3. Handle Physical Buttons with Simple Debounce Logic
  int read1 = digitalRead(BUTTON_1);
  int read2 = digitalRead(BUTTON_2);

  // Button 1 logic (Active-LOW BOOT Button)
  if (read1 != lastBtn1State) {
    lastDebounceTime1 = currentMillis;
  }
  if ((currentMillis - lastDebounceTime1) > DEBOUNCE_DELAY) {
    if (read1 == LOW) { // Button pressed down
      light1On = !light1On;
      updateHardware();
      beepConfirmation();
      sendBluetoothStatus();
      lastActionTime = currentMillis;
      delay(200); // Prevent double triggers
    }
  }
  lastBtn1State = read1;

  // Button 2 logic (Active-HIGH External Button)
  if (read2 != lastBtn2State) {
    lastDebounceTime2 = currentMillis;
  }
  if ((currentMillis - lastDebounceTime2) > DEBOUNCE_DELAY) {
    if (read2 == HIGH) { // Button pressed down
      light2On = !light2On;
      updateHardware();
      beepConfirmation();
      sendBluetoothStatus();
      lastActionTime = currentMillis;
      delay(200); // Prevent double triggers
    }
  }
  lastBtn2State = read2;

  // 4. Energy Saving Timeout (30 minutes)
  if ((light1On || light2On) && (currentMillis - lastActionTime >= TIMEOUT_MS)) {
    light1On = false;
    light2On = false;
    updateHardware();
    beepConfirmation();
    if (SerialBT.hasClient()) {
      SerialBT.println("Auto-timeout active: Turned off all lights!");
    }
    Serial.println("30 mins idle reached. Turning all lights off.");
    lastActionTime = currentMillis;
  }
}

// Function to sync our boolean variables with physical relay states
void updateHardware() {
  // Active-LOW logic: LOW turns relay ON, HIGH turns relay OFF
  digitalWrite(RELAY_1, light1On ? LOW : HIGH);
  digitalWrite(RELAY_2, light2On ? LOW : HIGH);
}

// Function to make a simple 50ms confirmation beep
void beepConfirmation() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(50);
  digitalWrite(BUZZER_PIN, LOW);
}

// Function to format and send the status update string
void sendBluetoothStatus() {
  String statusStr = "L1:";
  if (light1On) statusStr += "ON";
  else statusStr += "OFF";
  
  statusStr += " L2:";
  if (light2On) statusStr += "ON";
  else statusStr += "OFF";
  
  SerialBT.println(statusStr);
  Serial.println("Sent status: " + statusStr);
}