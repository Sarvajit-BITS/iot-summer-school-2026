/*
 * Digital Piano - Basic 4-Key Version (Commit 1)
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

  // Play corresponding note if exactly one button is pressed
  if (buttonsPressed == 1) {
    if (stateDo == HIGH) tone(buzzerPin, freqDo);
    else if (stateRe == HIGH) tone(buzzerPin, freqRe);
    else if (stateMi == HIGH) tone(buzzerPin, freqMi_Maj);
    else if (stateFa == HIGH) tone(buzzerPin, freqFa);
  } 
  else {
    // No buttons pressed (or too many pressed) = Silence
    noTone(buzzerPin);
  }
  
  delay(10); // Small loop delay for system stability
}
