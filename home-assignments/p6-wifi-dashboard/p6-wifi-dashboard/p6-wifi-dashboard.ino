#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- Wi-Fi Credentials ---
const char* ssid = "DESKTOP-SP";
const char* password = "DasNaDas";

// --- Pin Definitions ---
#define DHTPIN 4
#define DHTTYPE DHT11
#define LDRPIN 34
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// --- Object Initializations ---
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
AsyncWebServer server(80);

// --- Global Sensor Variables ---
float temp = 0.0;
float humidity = 0.0;
int lightLevel = 0;

// --- Timers & History (For 3 Charts) ---
unsigned long lastSensorRead = 0;
unsigned long lastWifiCheck = 0;
unsigned long lastChartUpdate = 0;

// Three arrays to store the last 24 readings
float tempHistory[24]; 
float humidityHistory[24];
int lightHistory[24];
bool historyReady = false;

// ==========================================
// HTML PAGE TEMPLATE (With 3 Charts)
// ==========================================
// ==========================================
// HTML PAGE TEMPLATE (Sanitized for ESP32)
// ==========================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Weather Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="refresh" content="10"> <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    body { 
      background-color: %BGCOLOR%; 
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
      text-align: center; color: #2c3e50; transition: background-color 0.5s; margin: 0; padding: 20px;
    }
    h1 { color: #fff; text-shadow: 1px 1px 2px rgba(0,0,0,0.2); }
    .cards { display: flex; flex-wrap: wrap; justify-content: center; gap: 20px; margin-top: 20px; }
    .card { background: white; padding: 20px; border-radius: 15px; box-shadow: 0 8px 16px rgba(0,0,0,0.15); width: 200px; }
    .card h3 { margin: 0; color: #7f8c8d; font-size: 1.2rem; }
    .card h2 { margin: 10px 0 0 0; font-size: 2.5rem; color: #2980b9; }
    
    .charts-container { display: flex; flex-wrap: wrap; justify-content: center; gap: 20px; margin-top: 40px; }
    /* Removed the 100 percent width rule that broke the parser! */
    .chart-card { background: white; padding: 20px; border-radius: 15px; box-shadow: 0 8px 16px rgba(0,0,0,0.15); width: 380px; max-width: 400px; }
  </style>
</head>
<body>
  <h1>Live Local Weather</h1>
  
  <div class="cards">
    <div class="card"><h3>Temperature</h3><h2>%TEMP% &deg;C</h2></div>
    <div class="card"><h3>Humidity</h3><h2>%HUMIDITY% &#37;</h2></div>
    <div class="card"><h3>Light Level</h3><h2>%LIGHT% &#37;</h2></div>
  </div>
  
  <div class="charts-container">
    <div class="chart-card"><canvas id="tempChart"></canvas></div>
    <div class="chart-card"><canvas id="humChart"></canvas></div>
    <div class="chart-card"><canvas id="lightChart"></canvas></div>
  </div>

  <script>
    const chartOptions = { animation: false }; 
    
    new Chart(document.getElementById('tempChart').getContext('2d'), {
      type: 'line',
      data: { labels: [%CHART_LABELS%], datasets: [{ label: 'Temperature (C)', data: [%CHART_DATA_TEMP%], borderColor: '#e74c3c', backgroundColor: 'rgba(231, 76, 60, 0.2)', fill: true, tension: 0.4 }] },
      options: chartOptions
    });

    new Chart(document.getElementById('humChart').getContext('2d'), {
      type: 'line',
      /* Changed to (Percent) to protect the ESP32 parser */
      data: { labels: [%CHART_LABELS%], datasets: [{ label: 'Humidity (Percent)', data: [%CHART_DATA_HUM%], borderColor: '#3498db', backgroundColor: 'rgba(52, 152, 219, 0.2)', fill: true, tension: 0.4 }] },
      options: chartOptions
    });

    new Chart(document.getElementById('lightChart').getContext('2d'), {
      type: 'line',
      /* Changed to (Percent) to protect the ESP32 parser */
      data: { labels: [%CHART_LABELS%], datasets: [{ label: 'Light Level (Percent)', data: [%CHART_DATA_LIG%], borderColor: '#f1c40f', backgroundColor: 'rgba(241, 196, 15, 0.2)', fill: true, tension: 0.4 }] },
      options: chartOptions
    });
  </script>
</body>
</html>
)rawliteral";

// ==========================================
// TEMPLATE PROCESSOR
// ==========================================
String processor(const String& var){
  if(var == "TEMP") return String(temp, 1);
  if(var == "HUMIDITY") return String(humidity, 1);
  if(var == "LIGHT") return String(lightLevel);
  if(var == "BGCOLOR") return (temp < 25.0) ? "#3498db" : "#e67e22"; 
  
  if(var == "CHART_DATA_TEMP") {
    String dataStr = "";
    for(int i=0; i<24; i++) { dataStr += String(tempHistory[i]); if(i < 23) dataStr += ","; }
    return dataStr;
  }
  if(var == "CHART_DATA_HUM") {
    String dataStr = "";
    for(int i=0; i<24; i++) { dataStr += String(humidityHistory[i]); if(i < 23) dataStr += ","; }
    return dataStr;
  }
  if(var == "CHART_DATA_LIG") {
    String dataStr = "";
    for(int i=0; i<24; i++) { dataStr += String(lightHistory[i]); if(i < 23) dataStr += ","; }
    return dataStr;
  }
  
  if(var == "CHART_LABELS") {
    String labelStr = "";
    for(int i=23; i>=0; i--) { labelStr += "\"-" + String(i * 5) + "m\""; if(i > 0) labelStr += ","; }
    return labelStr;
  }
  return String();
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Initialize history arrays to 0
  for(int i=0; i<24; i++) {
    tempHistory[i] = 0.0;
    humidityHistory[i] = 0.0;
    lightHistory[i] = 0;
  }

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("Starting Wi-Fi...");
    display.display();
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected.");

  // Update OLED with Connection info
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("CONNECTED!");
  display.print("SSID: "); display.println(ssid);
  display.println();
  display.println("Type this IP in Web:");
  display.setTextSize(2);
  display.println(WiFi.localIP());
  display.display();

  // Root Web Page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
    req->send_P(200, "text/html", index_html, processor);
  });

  // JSON Data Endpoint
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *req){
    String json = "{";
    json += "\"temp\":" + String(temp, 1) + ",";
    json += "\"humidity\":" + String(humidity, 1) + ",";
    json += "\"light\":" + String(lightLevel);
    json += "}";
    req->send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Read Sensors every 5 seconds
  if(currentMillis - lastSensorRead >= 5000) {
    lastSensorRead = currentMillis;
    
    float newT = dht.readTemperature();
    float newH = dht.readHumidity();
    if(!isnan(newT)) temp = newT;
    if(!isnan(newH)) humidity = newH;

    // INVERTED LDR LOGIC: Mapped 100 to 0 instead of 0 to 100!
    int rawLight = analogRead(LDRPIN);
    lightLevel = map(rawLight, 0, 4095, 100, 0); 

    // Initial fill of history arrays on startup
    if(!historyReady && temp > 0) {
      for(int i=0; i<24; i++) {
        tempHistory[i] = temp;
        humidityHistory[i] = humidity;
        lightHistory[i] = lightLevel;
      }
      historyReady = true;
    }
  }

  // 2. Update Charts History every 5 minutes (300,000 ms)
  // Note: Set to 3000 ms temporarily if you want to watch the graphs move quickly during testing!
  if(currentMillis - lastChartUpdate >= 300000) {
    lastChartUpdate = currentMillis;
    
    // Shift all old data left
    for(int i=0; i<23; i++) {
      tempHistory[i] = tempHistory[i+1];
      humidityHistory[i] = humidityHistory[i+1];
      lightHistory[i] = lightHistory[i+1];
    }
    // Add current readings to end of arrays
    tempHistory[23] = temp;
    humidityHistory[23] = humidity;
    lightHistory[23] = lightLevel;
  }

  // 3. Auto-reconnect Wi-Fi logic
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - lastWifiCheck >= 30000)) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    lastWifiCheck = currentMillis;
  }
}