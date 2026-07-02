/*
 * Author: Sarvajit Parsaniya
 * Date: July 2, 2026
 * Description: LED blink program using analog potentiometer speed control and serial count tracking.
 */
int blinkCount = 0;
const int potPin = A0;

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(9600);
}
void loop() {
  int blinkSpeed = analogRead(potPin);
  
  digitalWrite(13, HIGH);
  delay(blinkSpeed);
  digitalWrite(13, LOW);
  delay(blinkSpeed);
  
  blinkCount++;
  Serial.print("Blink count: ");
  Serial.println(blinkCount);
}
