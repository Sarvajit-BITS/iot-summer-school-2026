#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ArduinoJson.h>
#include "config.h"

// --- Pin Definitions ---
#define DHTPIN       
#define DHTTYPE      DHT11
#define MQ2_PIN      35
#define LDR_PIN      34
#define PIR_PIN      13

#define RELAY_FAN    26
#define RELAY_LIGHT  27

#define BUZZER_PIN   14
#define RED_LED      25
#define GREEN_LED    2

#define BTN_FAN      0
#define BTN_LIGHT    32

// --- Hardware Instances ---
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);
AsyncWebServer server(80);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// --- System State Variables ---
float temp = 0.0, humidity = 0.0, gasPercent = 0.0, lightPercent = 0.0;
bool pirMotion = false;

// Relay States (Active-LOW: false = OFF [HIGH], true = ON [LOW])
bool fanState = false;
bool lightState = false;

// Override States
bool fanOverrideActive = false;
unsigned long fanOverrideStartTime = 0;

bool lightOverrideActive = false;
unsigned long lightOverrideStartTime = 0;

// Motion Timer
unsigned long lastMotionTime = 0;

// Gas Emergency Machine
bool gasEmergency = false;
unsigned long gasBelowThresTime = 0;
bool gasInCoolDown = false;

// Timers for Non-Blocking Loop
unsigned long lastSensorRead = 0;
unsigned long lastOledCycle  = 0;
unsigned long lastMqttPub    = 0;
unsigned long lastBtnCheck   = 0;

int oledScreenPage = 0;

// Forward Declarations
void readSensors();
void applyAutomationRules();
void updateActuators();
void updateOLED();
void publishMQTT();
void connectWiFi();
void connectMQTT();
String getUptimeString();

// --- HTML Dashboard Template ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>IIT Jammu Smart Home Hub</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; background-color: #121212; color: #ffffff; margin: 0; padding: 20px; }
    h1 { color: #00E676; margin-bottom: 5px; }
    .status-bar { padding: 10px; border-radius: 5px; margin-bottom: 20px; font-weight: bold; }
    .status-ok { background-color: #1b5e20; color: #a5d6a7; }
    .status-alert { background-color: #b71c1c; color: #ffcdd2; animate: pulse 1s infinite; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(140px, 1fr)); gap: 15px; max-width: 600px; margin: 0 auto; }
    .card { background-color: #1e1e1e; padding: 15px; border-radius: 10px; border: 1px solid #333; }
    .card h3 { margin: 0 0 10px 0; font-size: 14px; color: #aaa; }
    .card p { margin: 0; font-size: 22px; font-weight: bold; color: #00E676; }
    .btn { display: inline-block; padding: 10px 20px; margin: 5px; border: none; border-radius: 5px; font-weight: bold; cursor: pointer; text-decoration: none; }
    .btn-on { background-color: #2e7d32; color: white; }
    .btn-off { background-color: #c62828; color: white; }
  </style>
  <script>
    setInterval(function() {
      fetch('/api/data').then(response => response.json()).then(data => {
        document.getElementById('temp').innerText = data.temp + ' °C';
        document.getElementById('hum').innerText = data.humidity + ' %';
        document.getElementById('gas').innerText = data.gas + ' %';
        document.getElementById('ldr').innerText = data.light + ' %';
        document.getElementById('pir').innerText = data.pir ? 'DETECTED' : 'CLEAR';
        document.getElementById('fan').innerText = data.fan ? 'ON' : 'OFF';
        document.getElementById('light_relay').innerText = data.light_relay ? 'ON' : 'OFF';
        document.getElementById('uptime').innerText = data.uptime;
        
        let banner = document.getElementById('status-banner');
        if(data.alert === 'GAS EMERGENCY') {
          banner.className = 'status-bar status-alert';
          banner.innerText = '⚠️ DANGER: GAS EMERGENCY DETECTED!';
        } else {
          banner.className = 'status-bar status-ok';
          banner.innerText = 'SYSTEM OPERATIONAL (OK)';
        }
      });
    }, 2000);
  </script>
</head>
<body>
  <h1>IIT JAMMU HOME HUB</h1>
  <p>Hostel Room Automation Dashboard</p>
  <div id="status-banner" class="status-bar status-ok">INITIALIZING...</div>
  
  <div class="grid">
    <div class="card"><h3>TEMPERATURE</h3><p id="temp">--</p></div>
    <div class="card"><h3>HUMIDITY</h3><p id="hum">--</p></div>
    <div class="card"><h3>GAS LEVEL</h3><p id="gas">--</p></div>
    <div class="card"><h3>LIGHT (LDR)</h3><p id="ldr">--</p></div>
    <div class="card"><h3>MOTION (PIR)</h3><p id="pir">--</p></div>
    <div class="card"><h3>UPTIME</h3><p id="uptime" style="font-size: 14px;">--</p></div>
  </div>

  <h2>Actuator Controls</h2>
  <div class="card" style="max-width:600px; margin: 15px auto;">
    <p>Fan Status: <span id="fan">--</span></p>
    <a href="/toggle/fan" class="btn btn-on">Toggle Fan Override (10m)</a>
  </div>
  <div class="card" style="max-width:600px; margin: 15px auto;">
    <p>Light Status: <span id="light_relay">--</span></p>
    <a href="/toggle/light" class="btn btn-on">Toggle Light Override (10m)</a>
  </div>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  // Pin Configurations
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_LIGHT, OUTPUT);
  digitalWrite(RELAY_FAN, HIGH);   // Active-LOW default OFF
  digitalWrite(RELAY_LIGHT, HIGH); // Active-LOW default OFF

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH); // System OK LED

  pinMode(PIR_PIN, INPUT);
  pinMode(BTN_FAN, INPUT_PULLUP);
  pinMode(BTN_LIGHT, INPUT_PULLUP);

  // Initialize Hardware
  dht.begin();
  Wire.begin(21, 22);

  if (!display.begin(0x3C, true)) {
    Serial.println(F("OLED SH1106 Init Failed"));
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(10, 20);
  display.println("Booting Home Hub...");
  display.display();

  // Connect Networking
  connectWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

  // Web Server Routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(512);
    doc["temp"]        = temp;
    doc["humidity"]    = humidity;
    doc["gas"]         = gasPercent;
    doc["light"]       = lightPercent;
    doc["pir"]         = pirMotion;
    doc["fan"]         = fanState;
    doc["light_relay"] = lightState;
    doc["alert"]       = gasEmergency ? "GAS EMERGENCY" : "OK";
    doc["uptime"]      = getUptimeString();

    String jsonResponse;
    serializeJson(doc, jsonResponse);
    request->send(200, "application/json", jsonResponse);
  });

  server.on("/toggle/fan", HTTP_GET, [](AsyncWebServerRequest *request) {
    fanOverrideActive = true;
    fanOverrideStartTime = millis();
    fanState = !fanState;
    updateActuators();
    request->redirect("/");
  });

  server.on("/toggle/light", HTTP_GET, [](AsyncWebServerRequest *request) {
    lightOverrideActive = true;
    lightOverrideStartTime = millis();
    lightState = !lightState;
    updateActuators();
    request->redirect("/");
  });

  server.begin();
  Serial.println("AsyncWebServer Started on Port 80");
}

void loop() {
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();

  // 1. Button Reading Loop (Every 50ms)
  if (millis() - lastBtnCheck > 50) {
    lastBtnCheck = millis();

    // Check Fan Button (GPIO 0)
    if (digitalRead(BTN_FAN) == LOW) {
      if (gasEmergency) {
        // Emergency Manual Silence / Reset
        gasEmergency = false;
        gasInCoolDown = false;
      } else {
        fanOverrideActive = true;
        fanOverrideStartTime = millis();
        fanState = !fanState;
        updateActuators();
      }
      delay(250); // Simple debounce
    }

    // Check Light Button (GPIO 32)
    if (digitalRead(BTN_LIGHT) == LOW) {
      lightOverrideActive = true;
      lightOverrideStartTime = millis();
      lightState = !lightState;
      updateActuators();
      delay(250); // Simple debounce
    }
  }

  // 2. Sensor Reading & Automation Rules (Every 2 seconds)
  if (millis() - lastSensorRead >= 2000) {
    lastSensorRead = millis();
    readSensors();
    applyAutomationRules();
    updateActuators();
  }

  // 3. Cycle OLED Display Page (Every 5 seconds)
  if (millis() - lastOledCycle >= 5000) {
    lastOledCycle = millis();
    oledScreenPage = (oledScreenPage + 1) % 3;
    updateOLED();
  }

  // 4. Publish MQTT Telemetry (Every 30 seconds)
  if (millis() - lastMqttPub >= 30000) {
    lastMqttPub = millis();
    publishMQTT();
  }
}

void readSensors() {
  // Read DHT11
  // Read DHT11
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println("⚠️ DHT11 Read Failed! Check wiring on Pin P4.");
  } else {
    temp = t;
    humidity = h;
  }

  // Read MQ-2 Gas (Mapped from ADC 0-4095 to 0-100%)
  int rawGas = analogRead(MQ2_PIN);
  gasPercent = map(rawGas, 0, 4095, 0, 100);

  // Read LDR Light (Mapped from ADC 0-4095 to 0-100%)
  int rawLdr = analogRead(LDR_PIN);
  lightPercent = map(rawLdr, 0, 4095, 100, 0); // Inverted mapping for dark/light %

  // Read PIR Motion
  pirMotion = digitalRead(PIR_PIN);
  if (pirMotion) {
    lastMotionTime = millis();
  }
}

void applyAutomationRules() {
  // --- RULE 3: GAS EMERGENCY (Highest Priority) ---
  if (gasPercent > GAS_ALERT_THRES) {
    gasEmergency = true;
    gasInCoolDown = false;
  } else if (gasEmergency) {
    // Gas has dropped below threshold: start 5-second delayed clear timer
    if (!gasInCoolDown) {
      gasInCoolDown = true;
      gasBelowThresTime = millis();
    } else if (millis() - gasBelowThresTime >= GAS_CLEAR_DELAY) {
      gasEmergency = false;
      gasInCoolDown = false;
    }
  }

  if (gasEmergency) {
    // Safety Shutdown: Force all relays OFF
    fanState = false;
    lightState = false;
    return;
  }

  // Check Override Expings
  if (fanOverrideActive && (millis() - fanOverrideStartTime >= OVERRIDE_TIMEOUT)) {
    fanOverrideActive = false;
  }
  if (lightOverrideActive && (millis() - lightOverrideStartTime >= OVERRIDE_TIMEOUT)) {
    lightOverrideActive = false;
  }

  // --- RULE 1: FAN AUTOMATION (Hysteresis) ---
  if (!fanOverrideActive) {
    if (!fanState && temp > TEMP_FAN_HIGH) {
      fanState = true;
    } else if (fanState && temp < TEMP_FAN_LOW) {
      fanState = false;
    }
  }

  // --- RULE 2: LIGHT AUTOMATION (LDR + PIR) ---
  if (!lightOverrideActive) {
    if (lightPercent < LDR_DARK_THRES && pirMotion) {
      lightState = true;
    } else if (lightPercent > LDR_LIGHT_THRES || (millis() - lastMotionTime >= PIR_LIGHT_TIMEOUT)) {
      lightState = false;
    }
  }
}

void updateActuators() {
  if (gasEmergency) {
    // Emergency Mode: Relays forced OFF
    digitalWrite(RELAY_FAN, HIGH);
    digitalWrite(RELAY_LIGHT, HIGH);

    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
  } else {
    // Normal Mode
    digitalWrite(RELAY_FAN, fanState ? LOW : HIGH);     // Active-LOW
    digitalWrite(RELAY_LIGHT, lightState ? LOW : HIGH); // Active-LOW

    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
  }
}

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Screen Header
  display.setCursor(0, 0);
  display.print("IIT JAMMU HUB | ");
  display.println(gasEmergency ? "ALERT" : "NORMAL");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);

  if (oledScreenPage == 0) {
    // PAGE 1: Environmental Readings
    display.setCursor(0, 18);
    display.setTextSize(2);
    display.print(temp, 1); display.println(" C");
    display.setTextSize(1);
    display.setCursor(0, 42);
    display.print("Humidity: "); display.print(humidity, 1); display.println(" %");
    display.setCursor(0, 54);
    display.print("Fan State: "); display.println(fanState ? "ON" : "OFF");

  } else if (oledScreenPage == 1) {
    // PAGE 2: Safety & Motion Sensors
    display.setCursor(0, 18);
    display.print("Gas Level: "); display.print(gasPercent, 1); display.println(" %");
    display.setCursor(0, 30);
    display.print("LDR Light: "); display.print(lightPercent, 1); display.println(" %");
    display.setCursor(0, 42);
    display.print("PIR Motion: "); display.println(pirMotion ? "DETECTED" : "CLEAR");
    display.setCursor(0, 54);
    display.print("Light Relay: "); display.println(lightState ? "ON" : "OFF");

  } else if (oledScreenPage == 2) {
    // PAGE 3: Network & System Uptime
    display.setCursor(0, 18);
    display.print("IP: "); display.println(WiFi.localIP());
    display.setCursor(0, 32);
    display.print("MQTT: "); display.println(mqttClient.connected() ? "CONNECTED" : "DISCONNECTED");
    display.setCursor(0, 48);
    display.print("Uptime: "); display.println(getUptimeString());
  }

  display.display();
}

void publishMQTT() {
  if (!mqttClient.connected()) return;

  DynamicJsonDocument doc(512);
  doc["temp"]        = temp;
  doc["humidity"]    = humidity;
  doc["gas"]         = gasPercent;
  doc["pir"]         = pirMotion ? 1 : 0;
  doc["light"]       = lightPercent;
  doc["fan"]         = fanOverrideActive ? "MANUAL" : (fanState ? "AUTO_ON" : "AUTO_OFF");
  doc["light_relay"] = lightOverrideActive ? "MANUAL" : (lightState ? "AUTO_ON" : "AUTO_OFF");
  doc["alert"]       = gasEmergency ? "GAS EMERGENCY" : "OK";

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  mqttClient.publish(MQTT_TOPIC_DATA, jsonBuffer);
  Serial.print("Published MQTT Payload to ");
  Serial.print(MQTT_TOPIC_DATA);
  Serial.print(": ");
  Serial.println(jsonBuffer);
}

void connectWiFi() {
  Serial.print("Connecting to Wi-Fi SSID: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");
  Serial.print("Local IP Address: http://");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT Broker: ");
    Serial.println(MQTT_BROKER);
    String clientId = "ESP32Hub-" + String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("MQTT Connected!");
    } else {
      Serial.print("MQTT Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

String getUptimeString() {
  unsigned long totalSec = millis() / 1000;
  int days  = totalSec / 86400;
  int hours = (totalSec % 86400) / 3600;
  int mins  = (totalSec % 3600) / 60;
  int secs  = totalSec % 60;

  char buf[25];
  sprintf(buf, "%dd %02dh %02dm %02ds", days, hours, mins, secs);
  return String(buf);
}