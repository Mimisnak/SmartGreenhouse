#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <HTTPClient.h>

// Wi-Fi credentials - ΑΛΛΑΞΤΕ ΑΥΤΑ!
const char* ssid = "Vodafone-E79546683";              
const char* password = "RLg2s6b73EfarXRx";       

// 🔥 Firebase configuration - ΕΝΗΜΕΡΩΜΕΝΟ ΜΕ ΤΟ ΠΡΑΓΜΑΤΙΚΟ URL!
const char* FIREBASE_HOST = "https://smartgreenhouse-fb494-default-rtdb.firebaseio.com";
const char* FIREBASE_PATH = "/greenhouse/current.json";

// Create objects
Adafruit_BMP280 bmp;
WebServer server(80);
Preferences preferences;
HTTPClient http;

// ESP32-S3 I2C pins
#define SDA_PIN 8   
#define SCL_PIN 9   

bool sensorInitialized = false;
float currentTemperature = 0.0;
float currentPressure = 0.0;
unsigned long lastReading = 0;
unsigned long lastFirebaseUpdate = 0;
unsigned long lastWiFiCheck = 0;  // Missing variable for WiFi check timing
const unsigned long FIREBASE_UPDATE_INTERVAL = 10000; // Upload to Firebase every 10 seconds

// Statistics variables
float minTemperature = 999.0;
float maxTemperature = -999.0;
float totalTemperature = 0.0;
int readingCount = 0;
unsigned long totalUptime = 0;
unsigned long statsStartTime = 0;

// Daily statistics
const unsigned long DAILY_RESET_INTERVAL = 86400000; // 24 hours
unsigned long dailyStartTime = 0;
float dailyMinTemp = 999.0;
float dailyMaxTemp = -999.0;
float dailyTotalTemp = 0.0;
int dailyReadingCount = 0;

bool initializeSensor() {
  Wire.begin(SDA_PIN, SCL_PIN);
  
  if (!bmp.begin(0x76)) {
    Serial.println("❌ BMP280 not found at 0x76, trying 0x77...");
    if (!bmp.begin(0x77)) {
      Serial.println("❌ BMP280 sensor not found!");
      return false;
    }
  }
  
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
                  
  Serial.println("✅ BMP280 sensor initialized successfully!");
  return true;
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("🔗 Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("✅ WiFi connected successfully!");
    Serial.print("📍 IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("📶 Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("");
    Serial.println("❌ WiFi connection failed!");
  }
}

// 🔥 Firebase upload function
void uploadToFirebase() {
  if (WiFi.status() != WL_CONNECTED) return;
  
  String url = String(FIREBASE_HOST) + FIREBASE_PATH;
  
  // Create JSON payload
  DynamicJsonDocument doc(1024);
  doc["temperature"] = currentTemperature;
  doc["pressure"] = currentPressure;
  doc["timestamp"] = millis();
  doc["uptime"] = millis() / 1000;
  doc["device"] = "ESP32-S3";
  doc["sensor"] = "BMP280";
  doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  doc["wifi_ip"] = WiFi.localIP().toString();
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["sensor_status"] = sensorInitialized ? "OK" : "ERROR";
  
  // Statistics
  if (readingCount > 0) {
    doc["min_temp"] = minTemperature;
    doc["max_temp"] = maxTemperature;
    doc["avg_temp"] = totalTemperature / readingCount;
    doc["reading_count"] = readingCount;
  }
  
  // Daily statistics
  if (dailyReadingCount > 0) {
    doc["daily_min_temp"] = dailyMinTemp;
    doc["daily_max_temp"] = dailyMaxTemp;
    doc["daily_avg_temp"] = dailyTotalTemp / dailyReadingCount;
    doc["daily_reading_count"] = dailyReadingCount;
    
    // Calculate time left until daily reset
    unsigned long elapsed = millis() - dailyStartTime;
    unsigned long timeLeft = DAILY_RESET_INTERVAL - elapsed;
    doc["daily_time_left_hours"] = (timeLeft / 3600000);
    doc["daily_time_left_minutes"] = ((timeLeft % 3600000) / 60000);
    
    // Current date
    doc["daily_date"] = "Day " + String((millis() - dailyStartTime) / 86400000 + 1);
  }
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode = http.PUT(jsonString);
  
  if (httpResponseCode > 0) {
    Serial.println("🔥 Data uploaded to Firebase successfully!");
    Serial.print("Response: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("❌ Firebase upload failed: ");
    Serial.println(httpResponseCode);
  }
  
  http.end();
}

void handleRoot() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>🌱 ESP32-S3 Local Monitor</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f0f8ff; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        .reading { background: #e8f4fd; padding: 15px; margin: 10px 0; border-radius: 8px; text-align: center; }
        .temp { font-size: 24px; color: #ff6b35; font-weight: bold; }
        .pressure { font-size: 20px; color: #4caf50; font-weight: bold; }
        .status { background: #d4edda; color: #155724; padding: 10px; border-radius: 5px; margin: 10px 0; text-align: center; }
        .firebase-notice { background: #fff3cd; color: #856404; padding: 15px; border-radius: 8px; margin: 15px 0; text-align: center; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌱 ESP32-S3 Smart Greenhouse</h1>
        <div class="firebase-notice">
            🔥 <strong>Cloud Mode Active!</strong><br>
            Data is being uploaded to Firebase every 10 seconds<br>
            Access from anywhere: <a href="https://mimisnak.github.io/SmartGreenhouse/" target="_blank">GitHub Pages</a>
        </div>
        <div class="reading">
            <div class="temp">🌡️ )" + String(currentTemperature, 2) + R"( °C</div>
        </div>
        <div class="reading">
            <div class="pressure">🌬️ )" + String(currentPressure, 2) + R"( hPa</div>
        </div>
        <div class="status">
            📡 Sensor: )" + (sensorInitialized ? "✅ OK" : "❌ Error") + R"( | 
            📶 WiFi: )" + (WiFi.status() == WL_CONNECTED ? "✅ Connected" : "❌ Disconnected") + R"( |
            🔥 Firebase: Active
        </div>
        <div style="text-align: center; margin-top: 20px; color: #666;">
            <p>📈 Total Readings: )" + String(readingCount) + R"(</p>
            <p>⏱️ Uptime: )" + String(millis() / 1000) + R"( seconds</p>
        </div>
    </div>
</body>
</html>
  )";
  
  server.send(200, "text/html", html);
}

void handleAPI() {
  DynamicJsonDocument doc(1024);
  doc["temperature"] = currentTemperature;
  doc["pressure"] = currentPressure;
  doc["timestamp"] = millis();
  doc["uptime"] = millis() / 1000;
  doc["device"] = "ESP32-S3";
  doc["sensor"] = "BMP280";
  doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  doc["wifi_ip"] = WiFi.localIP().toString();
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["sensor_status"] = sensorInitialized ? "OK" : "ERROR";
  
  // Statistics
  if (readingCount > 0) {
    doc["min_temp"] = minTemperature;
    doc["max_temp"] = maxTemperature;
    doc["avg_temp"] = totalTemperature / readingCount;
    doc["reading_count"] = readingCount;
  }
  
  // Daily statistics
  if (dailyReadingCount > 0) {
    doc["daily_min_temp"] = dailyMinTemp;
    doc["daily_max_temp"] = dailyMaxTemp;
    doc["daily_avg_temp"] = dailyTotalTemp / dailyReadingCount;
    doc["daily_reading_count"] = dailyReadingCount;
    
    unsigned long elapsed = millis() - dailyStartTime;
    unsigned long timeLeft = DAILY_RESET_INTERVAL - elapsed;
    doc["daily_time_left_hours"] = (timeLeft / 3600000);
    doc["daily_time_left_minutes"] = ((timeLeft % 3600000) / 60000);
    doc["daily_date"] = "Day " + String((millis() - dailyStartTime) / 86400000 + 1);
  }
  
  String output;
  serializeJson(doc, output);
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(200, "application/json", output);
}

void readSensor() {
  if (!sensorInitialized) return;
  
  currentTemperature = bmp.readTemperature();
  currentPressure = bmp.readPressure() / 100.0F; // Convert to hPa
  
  // Update statistics
  if (readingCount == 0) {
    minTemperature = currentTemperature;
    maxTemperature = currentTemperature;
    totalTemperature = currentTemperature;
    dailyMinTemp = currentTemperature;
    dailyMaxTemp = currentTemperature;
    dailyTotalTemp = currentTemperature;
    statsStartTime = millis();
    dailyStartTime = millis();
  } else {
    if (currentTemperature < minTemperature) minTemperature = currentTemperature;
    if (currentTemperature > maxTemperature) maxTemperature = currentTemperature;
    totalTemperature += currentTemperature;
    
    if (currentTemperature < dailyMinTemp) dailyMinTemp = currentTemperature;
    if (currentTemperature > dailyMaxTemp) dailyMaxTemp = currentTemperature;
    dailyTotalTemp += currentTemperature;
  }
  
  readingCount++;
  dailyReadingCount++;
  
  // Check for daily reset
  if (millis() - dailyStartTime >= DAILY_RESET_INTERVAL) {
    dailyStartTime = millis();
    dailyMinTemp = currentTemperature;
    dailyMaxTemp = currentTemperature;
    dailyTotalTemp = currentTemperature;
    dailyReadingCount = 1;
  }
  
  Serial.printf("🌡️ Temperature: %.2f°C | 🌬️ Pressure: %.2f hPa\n", 
                currentTemperature, currentPressure);
}

void setup() {
  Serial.begin(115200);
  Serial.println("🚀 ESP32-S3 Smart Greenhouse with Firebase");
  Serial.println("================================================");
  
  // Initialize sensor
  sensorInitialized = initializeSensor();
  
  // Initialize WiFi
  initWiFi();
  
  // Initialize web server
  server.on("/", handleRoot);
  server.on("/api", HTTP_GET, handleAPI);
  server.enableCORS(true);
  server.begin();
  
  Serial.println("🌐 Web server started");
  Serial.printf("🔗 Local access: http://%s\n", WiFi.localIP().toString().c_str());
  Serial.println("🔥 Firebase uploads every 10 seconds");
  Serial.println("🌍 Remote access via: https://mimisnak.github.io/SmartGreenhouse/");
  
  lastReading = millis();
  lastFirebaseUpdate = millis();
}

void loop() {
  server.handleClient();
  
  // Read sensor every 3 seconds
  if (millis() - lastReading >= 3000) {
    readSensor();
    lastReading = millis();
  }
  
  // Upload to Firebase every 10 seconds
  if (millis() - lastFirebaseUpdate >= FIREBASE_UPDATE_INTERVAL) {
    uploadToFirebase();
    lastFirebaseUpdate = millis();
  }
  
  // Check WiFi connection every 30 seconds
  if (millis() - lastWiFiCheck >= 30000) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("🔄 WiFi disconnected, reconnecting...");
      initWiFi();
    }
    lastWiFiCheck = millis();
  }
  
  delay(100);
}
