#ifndef CONFIG_H
#define CONFIG_H

// --- Wi-Fi Credentials ---
const char* WIFI_SSID = "DESKTOP-SP";     // Change to your 2.4GHz Wi-Fi SSID
const char* WIFI_PASS = "DasNaDas"; // Change to your Wi-Fi Password

// --- MQTT Settings ---
const char* MQTT_BROKER = "broker.hivemq.com";
const int   MQTT_PORT   = 1883;

// Topic required by rubrics: iitjammu/[yourname]/home
const char* MQTT_TOPIC_DATA = "iitjammu/student/home";

// --- Threshold Settings ---
const float TEMP_FAN_HIGH = 32.0; // Auto Fan ON above 32°C
const float TEMP_FAN_LOW  = 31.0; // Auto Fan OFF below 28°C

const int   LDR_DARK_THRES  = 20; // Auto Light ON below 20%
const int   LDR_LIGHT_THRES = 60; // Auto Light OFF above 60%

const float GAS_ALERT_THRES = 60.0; // Gas Emergency above 60%

// Timers (in milliseconds)
const unsigned long OVERRIDE_TIMEOUT   = 600000; // 10 Minutes manual override
const unsigned long PIR_LIGHT_TIMEOUT  = 180000; // 3 Minutes light auto-off
const unsigned long GAS_CLEAR_DELAY    = 5000;   // 5 Seconds gas clear delay

#endif