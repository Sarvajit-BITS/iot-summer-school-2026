/*
 * Digital Piano with Chord Substitute and Mode Toggle
 * Path: /week2/digital_piano/digital_piano.ino
 */

// Define Input & Output Pins
const int btnDo = 2;
const int btnRe = 3;
const int btnMi = 4;
const int btnFa = 5;
const int btnMode = 6;
const int buzzerPin = 8;

// Define Frequencies (Hz)
const int freqDo = 262;
const int freqRe = 294;
const int freqMi_Maj = 330; // Major 3rd
const int freqMi_Min = 311; // Minor 3rd (Flat)
const int freqFa = 349;
const int freqSol = 392;    // Chord substitute

// State Variables
bool isMinor = false;
int lastModeState = LOW;

void setup() {
  Serial.begin(9600);
  
  // Set up button pins (Assuming external 10k pull-down resistors are used)
  pinMode(btnDo, INPUT);
  pinMode(btnRe, INPUT);
  pinMode(btnMi, INPUT);
  pinMode(btnFa, INPUT);
  pinMode(btnMode, INPUT);
  
  // Set up buzzer
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  // --- FEATURE 3: MODE TOGGLE (Major/Minor) ---
  int currentModeState = digitalRead(btnMode);
  // Detect only the moment the button is pressed down (Edge Detection)
  if (currentModeState == HIGH && lastModeState == LOW) {
    isMinor = !isMinor; // Toggle the state
    if(isMinor) {
        Serial.println("Scale changed to: MINOR");
    } else {
        Serial.println("Scale changed to: MAJOR");
    }
    delay(50); // Simple debounce to prevent multiple triggers
  }
  lastModeState = currentModeState;

  // --- FEATURE 1: BASIC PIANO NOTES ---
  int stateDo = digitalRead(btnDo);
  int stateRe = digitalRead(btnRe);
  int stateMi = digitalRead(btnMi);
  int stateFa = digitalRead(btnFa);

  // Calculate how many note buttons are currently pressed
  int buttonsPressed = stateDo + stateRe + stateMi + stateFa;

  // --- FEATURE 2 & 1 COMBINED: PLAY TONES ---
  if (buttonsPressed >= 2) {
    // Feature 2: Two or more buttons pressed = Play Sol
    tone(buzzerPin, freqSol);
  } 
  else if (buttonsPressed == 1) {
    // Feature 1 & 3: Exactly one button pressed = Play corresponding note
    if (stateDo == HIGH) tone(buzzerPin, freqDo);
    else if (stateRe == HIGH) tone(buzzerPin, freqRe);
    else if (stateMi == HIGH) {
      // Feature 3: Check which scale we are in for the 'Mi' note
      if (isMinor) {
        tone(buzzerPin, freqMi_Min);
      } else {
        tone(buzzerPin, freqMi_Maj);
      }
    }
    else if (stateFa == HIGH) tone(buzzerPin, freqFa);
  } 
  else {
    // Feature 1: No buttons pressed = Silence
    noTone(buzzerPin);
  }
  
  delay(10); // Small loop delay for system stability
}
