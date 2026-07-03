/*
 * Traffic Light Controller with Pedestrian Interrupt
 * Path: /week2/traffic_light/traffic_light.ino
 */

// Define Pins
const int redLED = 4;
const int yellowLED = 5;
const int greenLED = 6;
const int buttonPin = 7; 

// Define States for the State Machine
enum TrafficState {
  STATE_RED,
  STATE_YELLOW,
  STATE_GREEN,
  STATE_PEDESTRIAN
};

TrafficState currentState;
unsigned long previousMillis = 0; // Stores the last time a light changed

void setup() {
  Serial.begin(9600);
  
  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  
  // Assumes a 10k pull-down resistor is wired to the button
  pinMode(buttonPin, INPUT); 
  
  // Initialize the starting state
  changeState(STATE_RED);
}

void loop() {
  unsigned long currentMillis = millis();
  
  // 1. Instantly check for the pedestrian button press
  // Only trigger if we aren't already in the pedestrian sequence
  if (digitalRead(buttonPin) == HIGH && currentState != STATE_PEDESTRIAN) {
    changeState(STATE_PEDESTRIAN);
  }
  
  // 2. Handle the timed state transitions
  switch (currentState) {
    
    case STATE_RED:
      // Wait 5 seconds, then go to Yellow
      if (currentMillis - previousMillis >= 5000) {
        changeState(STATE_YELLOW);
      }
      break;
      
    case STATE_YELLOW:
      // Wait 2 seconds, then go to Green
      if (currentMillis - previousMillis >= 2000) {
        changeState(STATE_GREEN);
      }
      break;
      
    case STATE_GREEN:
      // Wait 4 seconds, then loop back to Red
      if (currentMillis - previousMillis >= 4000) {
        changeState(STATE_RED);
      }
      break;
      
    case STATE_PEDESTRIAN:
      // Hold Red for 8 seconds, then resume normal traffic flow (Green)
      if (currentMillis - previousMillis >= 8000) {
        changeState(STATE_GREEN);
      }
      break;
  }
}

// Function to handle switching lights and logging to Serial Monitor
void changeState(TrafficState newState) {
  currentState = newState;
  previousMillis = millis(); // Reset the timer
  
  // Turn all LEDs OFF first to clear previous state
  digitalWrite(redLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED, LOW);
  
  // Print timestamp
  Serial.print("[Time: ");
  Serial.print(millis());
  Serial.print(" ms] - ");
  
  // Turn ON the correct LED and log it
  if (newState == STATE_RED) {
    digitalWrite(redLED, HIGH);
    Serial.println("Light changed to: RED");
  } 
  else if (newState == STATE_YELLOW) {
    digitalWrite(yellowLED, HIGH);
    Serial.println("Light changed to: YELLOW");
  } 
  else if (newState == STATE_GREEN) {
    digitalWrite(greenLED, HIGH);
    Serial.println("Light changed to: GREEN");
  } 
  else if (newState == STATE_PEDESTRIAN) {
    digitalWrite(redLED, HIGH);
    Serial.println("PEDESTRIAN BUTTON PRESSED -> Forced RED");
  }
}
