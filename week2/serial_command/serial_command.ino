/*
 * Serial Command Interface
 * Path: /week2/serial_command/serial_command.ino
 */

const int ledPin = LED_BUILTIN; // Uses the onboard LED (usually Pin 13)
int totalBlinks = 0;            // Tracks the blink counter
bool isLedOn = false;           // Tracks the current state for STATUS

void setup() {
  // Initialize serial communication at 9600 bits per second
  Serial.begin(9600);
  
  // Configure the built-in LED pin as an output
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  Serial.println("System Ready. Waiting for commands...");
}

void loop() {
  // Check if data is available to read
  if (Serial.available() > 0) {
    
    // Read the incoming string until a newline character is received
    String command = Serial.readStringUntil('\n');
    
    // Trim any invisible whitespace, carriage returns (\r), or newlines (\n)
    command.trim(); 
    
    // Ignore empty commands (e.g., if the user just presses Enter)
    if (command.length() == 0) return;

    // --- COMMAND ROUTING ---
    
    if (command == "LED_ON") {
      digitalWrite(ledPin, HIGH);
      isLedOn = true;
      Serial.println("OK: LED turned ON");
    } 
    else if (command == "LED_OFF") {
      digitalWrite(ledPin, LOW);
      isLedOn = false;
      Serial.println("OK: LED turned OFF");
    } 
    else if (command.startsWith("BLINK_")) {
      // Extract the character(s) after "BLINK_" (which starts at index 6)
      String numPart = command.substring(6);
      int x = numPart.toInt();
      
      // Validate that X is between 1 and 9
      if (x >= 1 && x <= 9) {
        Serial.print("OK: Blinking ");
        Serial.print(x);
        Serial.println(" times");
        
        for (int i = 0; i < x; i++) {
          digitalWrite(ledPin, HIGH);
          delay(250);
          digitalWrite(ledPin, LOW);
          delay(250);
          totalBlinks++; // Increment the blink counter
        }
        
        // Restore the LED to whatever state it was in before blinking
        if (isLedOn) {
          digitalWrite(ledPin, HIGH);
        }
      } 
      else {
        Serial.println("ERROR: X must be a number between 1 and 9");
      }
    } 
    else if (command == "STATUS") {
      Serial.println("--- SYSTEM STATUS ---");
      Serial.print("Built-in LED: ");
      Serial.println(isLedOn ? "ON" : "OFF");
      Serial.print("Total Blinks: ");
      Serial.println(totalBlinks);
      Serial.println("---------------------");
    } 
    else if (command == "RESET") {
      totalBlinks = 0;
      Serial.println("OK: Blink counter reset to 0");
    } 
    else {
      // Input Validation Failure
      Serial.println("ERROR: Unknown command");
    }
  }
}
