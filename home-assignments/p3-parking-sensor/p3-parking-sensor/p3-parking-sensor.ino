#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Hardware Pins (Renesas Uno R4 Safe)
const int EXT_TRIG_PIN   = 9;
const int EXT_ECHO_PIN   = 10;
const int BUZZER_PIN     = 6;
const int EXT_LED_GREEN  = 3;
const int EXT_LED_YELLOW = 4;
const int EXT_LED_RED    = 5;

// System Variables
float currentDistance = 0.0;
String currentZone    = "SAFE";
int beepInterval      = 0;     // 0 = Off, -1 = Continuous, >0 = pulse ms
bool buzzerState      = false;

// Timing Variables
unsigned long currentMillis    = 0;
unsigned long lastBeepTime     = 0;
unsigned long lastTelemetryTime = 0;

void setup() {
  Serial.begin(9600);
  delay(1000); 

  pinMode(EXT_TRIG_PIN, OUTPUT);
  pinMode(EXT_ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(EXT_LED_GREEN, OUTPUT);
  pinMode(EXT_LED_YELLOW, OUTPUT);
  pinMode(EXT_LED_RED, OUTPUT);

  // Turn LEDs off initially
  digitalWrite(EXT_LED_GREEN, LOW);
  digitalWrite(EXT_LED_YELLOW, LOW);
  digitalWrite(EXT_LED_RED, LOW);

  // Initialize OLED Safely using the exact address we just found!
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED failed to connect, check I2C wiring."));
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 20);
    display.println("System Ready!");
    display.display();
  }
  delay(1000);
}

void loop() {
  currentMillis = millis();

  // 1. Get Filtered Distance (Average of 3 quick samples for stability)
  float total = 0;
  for(int i=0; i<3; i++) {
    digitalWrite(EXT_TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(EXT_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(EXT_TRIG_PIN, LOW);
    long duration = pulseIn(EXT_ECHO_PIN, HIGH);
    total += (duration * 0.0343) / 2.0;
    delay(5); // tiny delay between pings
  }
  currentDistance = total / 3.0;

  // 2. Zone Logic Engine
  if (currentDistance > 60.0) {
    currentZone = "SAFE";
    beepInterval = 0;
    digitalWrite(EXT_LED_GREEN, HIGH);
    digitalWrite(EXT_LED_YELLOW, LOW);
    digitalWrite(EXT_LED_RED, LOW);
  } 
  else if (currentDistance > 30.0 && currentDistance <= 60.0) {
    currentZone = "CAUTION";
    beepInterval = 800;
    digitalWrite(EXT_LED_GREEN, LOW);
    digitalWrite(EXT_LED_YELLOW, HIGH);
    digitalWrite(EXT_LED_RED, LOW);
  } 
  else if (currentDistance > 15.0 && currentDistance <= 30.0) {
    currentZone = "CLOSE";
    beepInterval = 300;
    digitalWrite(EXT_LED_GREEN, LOW);
    digitalWrite(EXT_LED_YELLOW, HIGH);
    digitalWrite(EXT_LED_RED, HIGH);
  } 
  else { 
    currentZone = "DANGER";
    beepInterval = -1; 
    digitalWrite(EXT_LED_GREEN, LOW);
    digitalWrite(EXT_LED_YELLOW, LOW);
    digitalWrite(EXT_LED_RED, HIGH);
  }

  // 3. Asynchronous Buzzer Controller
  if (beepInterval == 0) {
    noTone(BUZZER_PIN);
    buzzerState = false;
  } 
  else if (beepInterval == -1) {
    tone(BUZZER_PIN, 1000); // Continuous
  } 
  else {
    if (currentMillis - lastBeepTime >= (unsigned long)beepInterval) {
      lastBeepTime = currentMillis;
      buzzerState = !buzzerState;
      if (buzzerState) tone(BUZZER_PIN, 1000, 70); 
      else noTone(BUZZER_PIN);
    }
  }

  // 4. Update Screen & Serial every 500ms
  if (currentMillis - lastTelemetryTime >= 500) {
    lastTelemetryTime = currentMillis;
    
    // Serial Output
    Serial.print("Distance: ");
    Serial.print(currentDistance, 1);
    Serial.print(" cm | Zone: ");
    Serial.println(currentZone);

    // OLED Rendering (Clean, Text-Only Layout)
    display.clearDisplay();
    
    // Top Header: Zone Indicator
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("STATUS: ");
    display.print(currentZone);

    // Main Body: Massive Distance Readout
    display.setTextSize(3);
    display.setCursor(10, 25);
    
    if(currentDistance > 999) {
      display.print("MAX");
    } else {
      display.print((int)currentDistance);
      display.setTextSize(2); 
      display.print("cm");
    }

    display.display();
  }
}