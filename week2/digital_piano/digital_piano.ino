/*
 * Digital Piano - Chord Substitute Version (Commit 2)
 * Path: /week2/digital_piano/digital_piano.ino
 */

// Define Input & Output Pins
const int btnDo = 2;
const int btnRe = 3;
const int btnMi = 4;
const int btnFa = 5;
const int btnMode = 6; // Wired, but not used in this version
const int buzzerPin = 8;

// Define Frequencies (Hz)
const int freqDo = 262;
const int freqRe = 294;
const int freqMi_Maj = 330; // Major 3rd
const int freqFa = 349;
const int freqSol = 392;    // Chord substitute

void setup() {
  Serial.begin(9600);
  
  // Set up button pins
  pinMode(btnDo, INPUT);
  pinMode(btnRe, INPUT);
  pinMode(btnMi, INPUT);
  pinMode(btnFa, INPUT);
  pinMode(btnMode, INPUT);
  
  // Set up buzzer
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
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
    // Feature 1: Exactly one button pressed = Play corresponding note
    if (stateDo == HIGH) tone(buzzerPin, freqDo);
    else if (stateRe == HIGH) tone(buzzerPin, freqRe);
    else if (stateMi == HIGH) tone(buzzerPin, freqMi_Maj);
    else if (stateFa == HIGH) tone(buzzerPin, freqFa);
  } 
  else {
    // Feature 1: No buttons pressed = Silence
    noTone(buzzerPin);
  }
  
  delay(10); // Small loop delay for system stability
}
