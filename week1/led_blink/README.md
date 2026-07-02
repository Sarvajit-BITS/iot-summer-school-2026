# Smart LED Blink with Potentiometer Speed Control

## Hardware Required
To build this project, we will need the following components:
* 1x Arduino Uno (or compatible microcontroller like ESP32)
* 1x 5mm LED (any color)
* 1x 220Ω Resistor
* 1x 10kΩ Potentiometer (Rotary)
* 1x Breadboard
* Jumper Wires

## Circuit Diagram Description
Ensure that components are wired according to the following text-based schematic:

**1. LED Circuit:**
* LED Placement: Insert the LED into the breadboard so its two legs (Anode/longer leg and Cathode/shorter leg) sit in two separate rows.
* Resistor Placement: Insert the 220Ω resistor so that one of its legs is in the exact same row as the LED's Anode, connecting them together.
* Place the resistor's other leg into a completely empty row.
* Arduino Connections:
* Use a **jumper wire** to connect the row holding the resistor's free leg to **Digital Pin 13** on the Arduino.
* Use a **jumper wire** to connect the row holding the LED's Cathode directly to a **GND** pin on the Arduino.

**2. Potentiometer Circuit:**
* Potentiometer Placement: Insert the potentiometer into the breadboard so its three pins sit in three separate, adjacent rows.
* Arduino Connections:
* Use a **jumper wire** to connect the row holding the Left Outer Pin to the **5V** pin on the Arduino.
* Use a **jumper wire** to connect the row holding the Right Outer Pin to a **GND** pin on the Arduino.
* Use a **jumper wire** to connect the row holding the Middle Wiper Pin to **Analog Pin A0** on the Arduino.

## How to Upload Code
Follow these steps to uploading the code onto your microcontroller:
1. Open the `blink.ino` file using the Arduino IDE.
2. Connect your Arduino to your computer using a USB cable.
3. In the Arduino IDE, go to **Tools > Board** and select your specific board model (e.g., "Arduino Uno").
4. Go to **Tools > Port** and select the COM port your Arduino is connected to.
5. Click the **Verify** (checkmark) button at the top left to compile the code and ensure there are no syntax errors.
6. Click the **Upload** (right arrow) button to flash the code to the microcontroller.

## Expected Output
Once the code is successfully uploaded:
* The LED connected to Pin 13 will begin to blink continuously.
* Turning the knob on the potentiometer will dynamically change the blinking speed in real-time (ranging from a 0ms to 1023ms delay between state changes).
* Opening the Serial Monitor (**Tools > Serial Monitor**) in the Arduino IDE will display a continuously updating log reading `Blink count: [X]`.

## Troubleshooting Tips
If your circuit is not behaving as expected, check the following common issues:
* **The LED is not turning on at all:** Verify the LED polarity. LEDs are directional; if the anode and cathode are swapped, current will not flow. Ensure the longer leg is connected to Pin 13.
* **The Serial Monitor displays unreadable/garbage characters:** Check your baud rate configuration. Ensure the dropdown menu at the bottom right of the Serial Monitor window is set exactly to **9600 baud** to match the `Serial.begin(9600);` command in the code.
* **The blink speed does not change when turning the potentiometer:** Double-check the middle wiper pin connection. If the middle pin is not securely connected to `A0`, `analogRead(potPin)` will return floating, random values instead of the true potentiometer resistance.
