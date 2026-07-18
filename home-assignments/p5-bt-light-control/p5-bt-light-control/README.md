# Project 5: Bluetooth Home Light Controller

This project uses an ESP32 to control two different lights via Bluetooth from an Android phone. It also features manual button overrides and a 30-minute safety auto-timeout to save electricity.

## Connection Guide & How to Pair
1. Download the **Serial Bluetooth Terminal** app from the Google Play Store on your Android phone.
2. Turn on your phone's Bluetooth and search for new devices.
3. Look for the name **`IIT_IoT_HomeCtrl`** and pair with it.
4. Open the Serial Bluetooth Terminal app, open the side menu, click **Devices**, and click on `IIT_IoT_HomeCtrl` to connect.
5. Once you are successfully connected, the **onboard blue LED (GPIO 2)** will turn on automatically to indicate that the connection is active.

## Supported Commands Index
You can type and send these single-character commands inside the Bluetooth terminal app:

* `1` : Turns Light 1 ON
* `2` : Turns Light 1 OFF
* `3` : Turns Light 2 ON
* `4` : Turns Light 2 OFF
* `5` : Turns both Light 1 and Light 2 ON at the same time
* `6` : Turns both Light 1 and Light 2 OFF at the same time
* `?` : Requests the current state of both lights

Whenever you send a valid command, the system will give a short **50ms confirmation beep** on the buzzer and return the current light status formatted like this: `L1:ON L2:OFF`.

## Hardware Features Included
* **Active-LOW Relays:** The relay module turns on when the pins are written LOW and turns off when they are written HIGH.
* **Manual Overrides:** Pressing the onboard BOOT button toggles Light 1. Pressing the external push button on your breadboard (wired to GPIO 35 with a 10k resistor) toggles Light 2.
* **Auto-Timeout Feature:** If you leave the lights on and do not send any commands or press any buttons for 30 minutes straight, the ESP32 will automatically shut off both lights to save power.