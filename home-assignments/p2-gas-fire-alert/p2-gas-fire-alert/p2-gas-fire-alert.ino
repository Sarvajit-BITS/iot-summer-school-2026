/**
 * Project 2: Gas & Fire Safety Alert System
 * Board: Arduino Uno R3
 * Target Folder: /home-assignments/p2-gas-fire-alert/
 */

// Pin Definitions
const int MQ2_PIN = A0;
const int FLAME_DIGITAL_PIN = 7;
const int FLAME_ANALOG_PIN = A1;
const int GREEN_LED = 10;
const int YELLOW_LED = 11;
const int RED_LED = 12;
const int BUZZER_PIN = 9;
const int SILENCE_BUTTON_PIN = 2;

// Timing Constants
const unsigned long WARMUP_DURATION = 120000; // 2 minutes warm-up time
const unsigned long LOG_INTERVAL = 1000;      // 1-second serial logging interval
const unsigned long SILENCE_DURATION = 30000;  // 30-second buzzer mute interval

// Global Variables
unsigned long systemStartTime = 0;
unsigned long lastLogTime = 0;
unsigned long lastBuzzerToggleTime = 0;
unsigned long silenceStartTime = 0;

bool isBuzzerSilenced = false;
bool buzzerState = false;

void setup() {
  Serial.begin(9600);

  // Pin Modes
  pinMode(FLAME_DIGITAL_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Using internal pull-up for silence button (Active-Low setup)
  pinMode(SILENCE_BUTTON_PIN, INPUT_PULLUP);

  systemStartTime = millis();
  Serial.println(F("System Initialization Completed."));
  Serial.println(F("Notice: MQ-2 sensor requires a 2-minute stabilization sequence."));
}

void loop() {
  unsigned long currentMillis = millis();
  bool isWarmingUp = (currentMillis - systemStartTime < WARMUP_DURATION);

  // 1. Read and Map Sensors
  int gasRaw = analogRead(MQ2_PIN);
  int gasPercent = map(gasRaw, 0, 1023, 0, 100);
  
  // Flame sensor is active-low: LOW means fire detected
  bool flameDetected = (digitalRead(FLAME_DIGITAL_PIN) == LOW);

  // 2. Handle Silence Button
  // Button reads LOW when actively pressed
  if (digitalRead(SILENCE_BUTTON_PIN) == LOW) {
    if (!isBuzzerSilenced) {
      isBuzzerSilenced = true;
      silenceStartTime = currentMillis;
      Serial.println(F("--> ALARM ALERT: Buzzer manually muted for 30 seconds."));
    }
  }

  // Check if the 30-second silence window has expired
  if (isBuzzerSilenced && (currentMillis - silenceStartTime >= SILENCE_DURATION)) {
    isBuzzerSilenced = false;
    Serial.println(F("--> ALARM ALERT: Mute window expired. Buzzer armed."));
  }

  // 3. Evaluate Hazard States & Output Logic
  String currentStatus = "SAFE";

  // Check for Danger state first (Gas > 60% OR Fire detected)
  if (gasPercent > 60 || flameDetected) {
    currentStatus = "DANGER";
    
    // Light matching LED
    digitalWrite(RED_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);

    // If danger persists, auto-reactivate the alarm if it was muted
    if (gasPercent > 60 && flameDetected) {
      isBuzzerSilenced = false; // Combined danger overrides manual silence
    }

    // Alarm Tone Routing Block
    if (!isBuzzerSilenced) {
      if (gasPercent > 60 && flameDetected) {
        // Combined Danger Pattern: High-intensity fast strobe tone
        executeNonBlockingBuzzer(currentMillis, 100); 
      } else if (flameDetected) {
        // Flame Danger Pattern: Continuous clear tone
        digitalWrite(BUZZER_PIN, HIGH);
      } else {
        // Gas Danger Pattern: Intense long bursts
        executeNonBlockingBuzzer(currentMillis, 300);
      }
    } else {
      digitalWrite(BUZZER_PIN, LOW); // Hold silent if button was pushed
    }
  } 
  // Check Warning state (Gas 30% - 60% and no fire)
  else if (gasPercent > 30) {
    currentStatus = "WARNING";
    
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);

    // Warning alert standard layout: 1 slow beep per second
    if (!isBuzzerSilenced) {
      executeNonBlockingBuzzer(currentMillis, 500);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }
  } 
  // Safe State (Gas < 30% and no fire)
  else {
    currentStatus = "SAFE";
    isBuzzerSilenced = false; // Reset the silence latch once clean state is reached
    
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
  }

  // 4. Print Formatted Status Log to Serial Monitor every second
  if (currentMillis - lastLogTime >= LOG_INTERVAL) {
    lastLogTime = currentMillis;

    Serial.print(F("GAS: "));
    Serial.print(gasPercent);
    Serial.print(F("% | FLAME: "));
    Serial.print(flameDetected ? F("DETECTED") : F("NONE"));
    Serial.print(F(" | STATUS: "));
    Serial.print(currentStatus);
    
    if (isWarmingUp) {
      Serial.print(F(" [SENSOR WARMING UP - READINGS UNSTABLE]"));
    }
    if (isBuzzerSilenced) {
      Serial.print(F(" (MUTED)"));
    }
    Serial.println();
  }
}

/**
 * Toggles the buzzer state at a specified frequency interval without halting execution loops
 */
void executeNonBlockingBuzzer(unsigned long currentMillis, unsigned long toggleRate) {
  if (currentMillis - lastBuzzerToggleTime >= toggleRate) {
    lastBuzzerToggleTime = currentMillis;
    buzzerState = !buzzerState;
    digitalWrite(BUZZER_PIN, buzzerState);
  }
}