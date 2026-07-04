/*
 * Vending Machine State Machine
 * Path: /week2/vending_machine/vending_machine.ino
 *
 * STATE TRANSITION DIAGRAM:
 * 
 * [IDLE] (All LEDs OFF)
 *   |-- (Insert Coin Button) --------> [COIN_INSERTED]
 * 
 * [COIN_INSERTED] (Yellow LED ON)
 *   |-- (Select Item Button) --------> [ITEM_SELECTED]
 *   |-- (Cancel Button) -------------> [IDLE] (Refund)
 * 
 * [ITEM_SELECTED] (Blue LED ON)
 *   |-- (Cancel Button) -------------> [IDLE] (Refund)
 *   |-- (Auto after 1.5 seconds) ----> [DISPENSING]
 * 
 * [DISPENSING] (Green LED ON)
 *   |-- (Auto after 2 seconds) ------> [IDLE]
 */

// Define Pins
const int btnCoin = 2;
const int btnSelect = 3;
const int btnCancel = 4;

const int ledCoin = 5;      // Represents COIN_INSERTED state
const int ledSelect = 6;    // Represents ITEM_SELECTED state
const int ledDispense = 7;  // Represents DISPENSING state

// Define State Machine
enum VendingState {
  IDLE,
  COIN_INSERTED,
  ITEM_SELECTED,
  DISPENSING
};

VendingState currentState = IDLE;

// Edge Detection Variables
int lastBtnCoin = LOW;
int lastBtnSelect = LOW;
int lastBtnCancel = LOW;

// Timer for automatic transitions
unsigned long stateTimer = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(btnCoin, INPUT);
  pinMode(btnSelect, INPUT);
  pinMode(btnCancel, INPUT);
  
  pinMode(ledCoin, OUTPUT);
  pinMode(ledSelect, OUTPUT);
  pinMode(ledDispense, OUTPUT);
  
  Serial.println("Vending Machine Initialized. State: IDLE");
  updateSystem(); // Set initial LEDs
}

void loop() {
  unsigned long currentMillis = millis();

  // Read button states
  int readCoin = digitalRead(btnCoin);
  int readSelect = digitalRead(btnSelect);
  int readCancel = digitalRead(btnCancel);

  // Detect exact moments buttons are pressed (Edge Detection)
  bool coinPressed = (readCoin == HIGH && lastBtnCoin == LOW);
  bool selectPressed = (readSelect == HIGH && lastBtnSelect == LOW);
  bool cancelPressed = (readCancel == HIGH && lastBtnCancel == LOW);

  // Update previous button states
  lastBtnCoin = readCoin;
  lastBtnSelect = readSelect;
  lastBtnCancel = readCancel;

  // --- STATE MACHINE LOGIC ---
  switch (currentState) {
    
    case IDLE:
      if (coinPressed) {
        currentState = COIN_INSERTED;
        updateSystem();
      }
      // Select and Cancel buttons do nothing in IDLE state
      break;
      
    case COIN_INSERTED:
      if (selectPressed) {
        currentState = ITEM_SELECTED;
        stateTimer = currentMillis; // Start timer for auto-dispense
        updateSystem();
      } 
      else if (cancelPressed) {
        Serial.println("Action: Coin Refunded.");
        currentState = IDLE;
        updateSystem();
      }
      break;
      
    case ITEM_SELECTED:
      if (cancelPressed) {
        Serial.println("Action: Transaction Cancelled. Coin Refunded.");
        currentState = IDLE;
        updateSystem();
      }
      // Wait 1.5 seconds to simulate processing, then dispense
      else if (currentMillis - stateTimer >= 1500) {
        currentState = DISPENSING;
        stateTimer = currentMillis; // Start timer for dispense duration
        updateSystem();
      }
      break;
      
    case DISPENSING:
      // Ignore all buttons while dispensing. Wait 2 seconds, then reset.
      if (currentMillis - stateTimer >= 2000) {
        Serial.println("Action: Item Dropped. Thank you!");
        currentState = IDLE;
        updateSystem();
      }
      break;
  }
}

// --- HELPER FUNCTION ---
// By taking no arguments, we completely bypass the Tinkercad compiler bug!
void updateSystem() {
  // Print Transition to Serial Monitor
  Serial.print("State Changed to: ");
  if (currentState == IDLE) Serial.println("IDLE");
  else if (currentState == COIN_INSERTED) Serial.println("COIN_INSERTED");
  else if (currentState == ITEM_SELECTED) Serial.println("ITEM_SELECTED");
  else if (currentState == DISPENSING) Serial.println("DISPENSING");

  // Update LEDs based on current state
  digitalWrite(ledCoin, (currentState == COIN_INSERTED) ? HIGH : LOW);
  digitalWrite(ledSelect, (currentState == ITEM_SELECTED) ? HIGH : LOW);
  digitalWrite(ledDispense, (currentState == DISPENSING) ? HIGH : LOW);
}
