#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// Wi-Fi credentials - ΑΛΛΑΞΤΕ ΑΥΤΑ!
const char* ssid = "Vodafone-E79546683";              // Βάλτε το όνομα του Wi-Fi σας εδώ
const char* password = "RLg2s6b73EfarXRx";       // Βάλτε τον κωδικό του Wi-Fi σας εδώ

// Create objects
Adafruit_BMP280 bmp;
WebServer server(80);
Preferences preferences;

// ESP32-S3 I2C pins (CONFIRMED WORKING!)
#define SDA_PIN 8   // Confirmed working
#define SCL_PIN 9   // Confirmed working

bool sensorInitialized = false;
float currentTemperature = 0.0;
float currentPressure = 0.0;
unsigned long lastReading = 0;
unsigned long lastWiFiCheck = 0;
bool wifiWasConnected = true;
unsigned long wifiReconnectAttempts = 0;
unsigned long lastReconnectAttempt = 0;
bool serverStarted = false;

// Statistics variables
float minTemperature = 999.0;
float maxTemperature = -999.0;
float totalTemperature = 0.0;
int readingCount = 0;
unsigned long totalUptime = 0;
unsigned long statsStartTime = 0;

// Alert thresholds
const float HIGH_TEMP_THRESHOLD = 30.0;  // °C
const float LOW_TEMP_THRESHOLD = 15.0;   // °C
String lastAlert = "";
unsigned long lastAlertTime = 0;

// Daily statistics management
const unsigned long DAILY_RESET_INTERVAL = 86400000; // 24 hours in milliseconds
unsigned long dailyStartTime = 0;
float dailyMinTemp = 999.0;
float dailyMaxTemp = -999.0;
float dailyTotalTemp = 0.0;
int dailyReadingCount = 0;
String dailyDate = "";

// Daily history (keep last few days for comparison)
struct DailyRecord {
  String date;
  float minTemp;
  float maxTemp;
  float avgTemp;
  int readingCount;
  unsigned long timestamp;
};
const int MAX_DAILY_RECORDS = 7; // Keep 7 days of history
DailyRecord dailyHistory[MAX_DAILY_RECORDS];

// Watchdog and stability
unsigned long lastPrintTime = 0;
bool systemStable = true;
unsigned long lastSaveTime = 0;
const unsigned long SAVE_INTERVAL = 30000; // Save every 30 seconds

// Forward declarations
void saveDailyStatistics();

// Functions for persistent storage
String getCurrentDate() {
  // Simple date string format (you could enhance this with RTC if needed)
  unsigned long days = millis() / 86400000; // Days since boot
  return "Day_" + String(days + 1);
}

void resetDailyStatistics() {
  // Archive current day before reset
  if (dailyReadingCount > 0) {
    // Shift history array
    for (int i = MAX_DAILY_RECORDS - 1; i > 0; i--) {
      dailyHistory[i] = dailyHistory[i-1];
    }
    
    // Add current day to history
    dailyHistory[0].date = dailyDate;
    dailyHistory[0].minTemp = dailyMinTemp;
    dailyHistory[0].maxTemp = dailyMaxTemp;
    dailyHistory[0].avgTemp = dailyTotalTemp / dailyReadingCount;
    dailyHistory[0].readingCount = dailyReadingCount;
    dailyHistory[0].timestamp = millis();
    
    Serial.println("📊 Ημερήσια στατιστικά αρχειοθετήθηκαν:");
    Serial.printf("   📅 Ημερομηνία: %s\n", dailyDate.c_str());
    Serial.printf("   🔽 Min: %.1f°C\n", dailyMinTemp);
    Serial.printf("   🔺 Max: %.1f°C\n", dailyMaxTemp);
    Serial.printf("   📊 Μέσος: %.1f°C\n", dailyTotalTemp / dailyReadingCount);
    Serial.printf("   🔢 Μετρήσεις: %d\n", dailyReadingCount);
  }
  
  // Reset daily counters
  dailyStartTime = millis();
  dailyDate = getCurrentDate();
  dailyMinTemp = 999.0;
  dailyMaxTemp = -999.0;
  dailyTotalTemp = 0.0;
  dailyReadingCount = 0;
  
  Serial.println("🗓️ Νέα ημέρα ξεκίνησε! Ημερήσια στατιστικά επαναφέρθηκαν.");
  Serial.printf("📅 Τρέχουσα ημερομηνία: %s\n", dailyDate.c_str());
}

void checkDailyReset() {
  // Check if 24 hours have passed
  if (millis() - dailyStartTime >= DAILY_RESET_INTERVAL) {
    resetDailyStatistics();
    saveDailyStatistics(); // Save the reset
  }
}

void saveDailyStatistics() {
  preferences.begin("daily_stats", false);
  
  // Save current daily stats
  preferences.putFloat("dailyMinTemp", dailyMinTemp);
  preferences.putFloat("dailyMaxTemp", dailyMaxTemp);
  preferences.putFloat("dailyTotalTemp", dailyTotalTemp);
  preferences.putInt("dailyReadCount", dailyReadingCount);
  preferences.putULong("dailyStartTime", dailyStartTime);
  preferences.putString("dailyDate", dailyDate);
  
  // Save daily history
  for (int i = 0; i < MAX_DAILY_RECORDS; i++) {
    String prefix = "hist" + String(i) + "_";
    preferences.putString((prefix + "date").c_str(), dailyHistory[i].date);
    preferences.putFloat((prefix + "minTemp").c_str(), dailyHistory[i].minTemp);
    preferences.putFloat((prefix + "maxTemp").c_str(), dailyHistory[i].maxTemp);
    preferences.putFloat((prefix + "avgTemp").c_str(), dailyHistory[i].avgTemp);
    preferences.putInt((prefix + "readCount").c_str(), dailyHistory[i].readingCount);
    preferences.putULong((prefix + "timestamp").c_str(), dailyHistory[i].timestamp);
  }
  
  preferences.end();
  Serial.println("💾 Ημερήσια στατιστικά αποθηκεύτηκαν!");
}

void loadDailyStatistics() {
  preferences.begin("daily_stats", true);
  
  // Load current daily stats
  dailyMinTemp = preferences.getFloat("dailyMinTemp", 999.0);
  dailyMaxTemp = preferences.getFloat("dailyMaxTemp", -999.0);
  dailyTotalTemp = preferences.getFloat("dailyTotalTemp", 0.0);
  dailyReadingCount = preferences.getInt("dailyReadCount", 0);
  dailyStartTime = preferences.getULong("dailyStartTime", millis());
  dailyDate = preferences.getString("dailyDate", getCurrentDate());
  
  // Load daily history
  for (int i = 0; i < MAX_DAILY_RECORDS; i++) {
    String prefix = "hist" + String(i) + "_";
    dailyHistory[i].date = preferences.getString((prefix + "date").c_str(), "");
    dailyHistory[i].minTemp = preferences.getFloat((prefix + "minTemp").c_str(), 0.0);
    dailyHistory[i].maxTemp = preferences.getFloat((prefix + "maxTemp").c_str(), 0.0);
    dailyHistory[i].avgTemp = preferences.getFloat((prefix + "avgTemp").c_str(), 0.0);
    dailyHistory[i].readingCount = preferences.getInt((prefix + "readCount").c_str(), 0);
    dailyHistory[i].timestamp = preferences.getULong((prefix + "timestamp").c_str(), 0);
  }
  
  preferences.end();
  
  // Check if we need to reset (24 hours passed)
  if (millis() - dailyStartTime >= DAILY_RESET_INTERVAL) {
    Serial.println("⏰ Πέρασαν 24 ώρες - αυτόματη επαναφορά ημερήσιων στατιστικών");
    resetDailyStatistics();
  }
  
  if (dailyReadingCount > 0) {
    Serial.println("📂 Ημερήσια στατιστικά φορτώθηκαν:");
    Serial.printf("   📅 Τρέχουσα ημέρα: %s\n", dailyDate.c_str());
    Serial.printf("   🔢 Μετρήσεις: %d\n", dailyReadingCount);
    Serial.printf("   🔽 Min: %.1f°C\n", dailyMinTemp);
    Serial.printf("   🔺 Max: %.1f°C\n", dailyMaxTemp);
    if (dailyReadingCount > 0) {
      Serial.printf("   📊 Μέσος: %.1f°C\n", dailyTotalTemp / dailyReadingCount);
    }
    
    // Show how much time is left in current day
    unsigned long timeLeft = DAILY_RESET_INTERVAL - (millis() - dailyStartTime);
    unsigned long hoursLeft = timeLeft / 3600000;
    unsigned long minutesLeft = (timeLeft % 3600000) / 60000;
    Serial.printf("   ⏰ Απομένουν: %luω %luλ μέχρι επαναφορά\n", hoursLeft, minutesLeft);
  } else {
    Serial.println("📂 Νέα ημερήσια περίοδος ξεκίνησε");
  }
}
void saveStatistics() {
  preferences.begin("thermometer", false);
  preferences.putFloat("minTemp", minTemperature);
  preferences.putFloat("maxTemp", maxTemperature);
  preferences.putFloat("totalTemp", totalTemperature);
  preferences.putInt("readCount", readingCount);
  preferences.putULong("startTime", statsStartTime);
  preferences.putULong("wifiAttempts", wifiReconnectAttempts);
  preferences.end();
  Serial.println("💾 Στατιστικά αποθηκεύτηκαν στη μνήμη!");
}

void loadStatistics() {
  preferences.begin("thermometer", true);
  minTemperature = preferences.getFloat("minTemp", 999.0);
  maxTemperature = preferences.getFloat("maxTemp", -999.0);
  totalTemperature = preferences.getFloat("totalTemp", 0.0);
  readingCount = preferences.getInt("readCount", 0);
  statsStartTime = preferences.getULong("startTime", millis());
  wifiReconnectAttempts = preferences.getULong("wifiAttempts", 0);
  preferences.end();
  
  if (readingCount > 0) {
    Serial.println("📂 Στατιστικά φορτώθηκαν από τη μνήμη!");
    Serial.printf("   📊 Μετρήσεις: %d\n", readingCount);
    Serial.printf("   🔽 Min: %.1f°C\n", minTemperature);
    Serial.printf("   🔺 Max: %.1f°C\n", maxTemperature);
    Serial.printf("   📊 Μέσος: %.1f°C\n", totalTemperature/readingCount);
    Serial.printf("   🔄 Wi-Fi προσπάθειες: %lu\n", wifiReconnectAttempts);
  } else {
    Serial.println("📂 Νέα εκκίνηση - δεν βρέθηκαν αποθηκευμένα στατιστικά");
  }
}

void resetStatistics() {
  preferences.begin("thermometer", false);
  preferences.clear();
  preferences.end();
  
  preferences.begin("daily_stats", false);
  preferences.clear();
  preferences.end();
  
  minTemperature = 999.0;
  maxTemperature = -999.0;
  totalTemperature = 0.0;
  readingCount = 0;
  statsStartTime = millis();
  wifiReconnectAttempts = 0;
  
  // Reset daily stats too
  resetDailyStatistics();
  
  Serial.println("🗑️ Όλα τα στατιστικά επαναφέρθηκαν (συνολικά και ημερήσια)!");
}

// CORS headers for web access
void handleCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// Handle JSON API endpoint with crash protection
void handleAPI() {
  // Safety check
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  
  handleCORS();
  
  // Create JSON response with error handling
  JsonDocument doc;
  
  // Basic sensor data
  doc["temperature"] = currentTemperature;
  doc["pressure"] = currentPressure;
  doc["timestamp"] = millis();
  doc["sensor_status"] = sensorInitialized ? "OK" : "ERROR";
  doc["uptime"] = millis() / 1000;
  doc["device"] = "ESP32-S3";
  doc["sensor"] = "BMP280";
  
  // Wi-Fi status with detailed info
  bool wifiConnected = WiFi.status() == WL_CONNECTED;
  doc["wifi_connected"] = wifiConnected;
  if (wifiConnected) {
    doc["wifi_status"] = "CONNECTED";
    doc["wifi_ip"] = WiFi.localIP().toString();
    doc["wifi_rssi"] = WiFi.RSSI();
  } else {
    doc["wifi_status"] = "DISCONNECTED - Επιστροφή το συντομότερο δυνατό!";
    doc["reconnect_attempts"] = wifiReconnectAttempts;
  }
  
  // Statistics
  if (readingCount > 0) {
    doc["min_temp"] = minTemperature;
    doc["max_temp"] = maxTemperature;
    doc["avg_temp"] = totalTemperature / readingCount;
    doc["reading_count"] = readingCount;
    
    // Temperature analysis
    float avgTemp = totalTemperature / readingCount;
    if (avgTemp >= HIGH_TEMP_THRESHOLD) {
      doc["temp_analysis"] = "Έχει πολύ υψηλές θερμοκρασίες! ⚠️";
    } else if (avgTemp <= LOW_TEMP_THRESHOLD) {
      doc["temp_analysis"] = "Έχει πολύ χαμηλές θερμοκρασίες! ❄️";
    } else {
      doc["temp_analysis"] = "Θερμοκρασίες σε κανονικά επίπεδα ✅";
    }
  } else {
    doc["min_temp"] = 0;
    doc["max_temp"] = 0;
    doc["avg_temp"] = 0;
    doc["reading_count"] = 0;
    doc["temp_analysis"] = "Δεν υπάρχουν ακόμα δεδομένα";
  }
  
  // Daily statistics
  if (dailyReadingCount > 0) {
    doc["daily_min_temp"] = dailyMinTemp;
    doc["daily_max_temp"] = dailyMaxTemp;
    doc["daily_avg_temp"] = dailyTotalTemp / dailyReadingCount;
    doc["daily_reading_count"] = dailyReadingCount;
    doc["daily_date"] = dailyDate;
    
    // Time remaining in current day
    unsigned long timeLeft = DAILY_RESET_INTERVAL - (millis() - dailyStartTime);
    doc["daily_time_left_hours"] = timeLeft / 3600000;
    doc["daily_time_left_minutes"] = (timeLeft % 3600000) / 60000;
  } else {
    doc["daily_min_temp"] = 0;
    doc["daily_max_temp"] = 0;
    doc["daily_avg_temp"] = 0;
    doc["daily_reading_count"] = 0;
    doc["daily_date"] = dailyDate;
    doc["daily_time_left_hours"] = 24;
    doc["daily_time_left_minutes"] = 0;
  }
  
  // Temperature alerts
  doc["last_alert"] = lastAlert;
  doc["alert_time"] = lastAlertTime;
  
  // System stability
  doc["system_stable"] = systemStable;
  doc["free_heap"] = ESP.getFreeHeap();
  
  // Persistent data info
  doc["data_persistent"] = true;
  doc["uptime_total"] = (millis() - statsStartTime) / 1000;
  doc["last_save"] = (millis() - lastSaveTime) / 1000;
  
  String jsonString;
  if (serializeJson(doc, jsonString) == 0) {
    server.send(500, "application/json", "{\"error\":\"JSON serialization failed\"}");
    return;
  }
  
  server.send(200, "application/json", jsonString);
}

// Handle root web page with safety checks
void handleRoot() {
  // Safety check
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  
  handleCORS();
  
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32-S3 Θερμόμετρο</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
            margin: 0; 
            padding: 20px; 
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
        }
        .container { 
            max-width: 600px; 
            margin: 0 auto; 
            background: white; 
            padding: 30px; 
            border-radius: 15px; 
            box-shadow: 0 4px 20px rgba(0,0,0,0.15);
        }
        h1 {
            text-align: center;
            color: #333;
            margin-bottom: 30px;
            font-size: 28px;
        }
        .reading { 
            background: linear-gradient(45deg, #f1f8ff, #e8f4fd); 
            padding: 20px; 
            margin: 15px 0; 
            border-radius: 12px; 
            border-left: 5px solid #2196F3;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
        }
        .temp { 
            font-size: 32px; 
            color: #ff6b35; 
            font-weight: bold;
            text-align: center;
        }
        .pressure { 
            font-size: 24px; 
            color: #4caf50; 
            font-weight: bold;
            text-align: center;
        }
        .status { 
            padding: 15px; 
            border-radius: 8px; 
            margin: 15px 0;
            text-align: center;
            font-weight: bold;
        }
        .status.ok { 
            background: #d4edda; 
            color: #155724; 
            border: 2px solid #c3e6cb;
        }
        .status.connecting {
            background: #fff3cd;
            color: #856404;
            border: 2px solid #ffeaa7;
        }
        .stats-section {
            margin-top: 25px;
            padding: 20px;
            background: #f8f9fa;
            border-radius: 12px;
            border: 1px solid #dee2e6;
        }
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 15px;
            margin-top: 15px;
        }
        .stat-item {
            background: white;
            padding: 15px;
            border-radius: 8px;
            text-align: center;
            border: 1px solid #dee2e6;
            box-shadow: 0 1px 3px rgba(0,0,0,0.1);
        }
        .stat-value {
            font-size: 20px;
            font-weight: bold;
            color: #2196F3;
        }
        .stat-label {
            font-size: 12px;
            color: #666;
            margin-top: 5px;
        }
        .alert-section {
            margin-top: 20px;
            padding: 15px;
            border-radius: 8px;
            display: none;
        }
        .alert-high {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        .alert-low {
            background: #d1ecf1;
            color: #0c5460;
            border: 1px solid #bee5eb;
        }
        .wifi-status {
            font-size: 14px;
            margin-top: 10px;
            padding: 8px;
            border-radius: 5px;
            text-align: center;
        }
        .wifi-connected {
            background: #d4edda;
            color: #155724;
        }
        .wifi-disconnected {
            background: #f8d7da;
            color: #721c24;
        }
        .footer { 
            text-align: center; 
            margin-top: 30px; 
            color: #666;
            font-size: 14px;
        }
        .info-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
            margin-top: 20px;
        }
        .info-item {
            background: #f8f9fa;
            padding: 15px;
            border-radius: 8px;
            text-align: center;
            border: 1px solid #dee2e6;
        }
        .loading {
            /* Removed loading animation to prevent screen flashing */
        }
        .auto-refresh-info {
            margin-top: 15px;
            padding: 8px;
            background: #e8f4fd;
            border: 1px solid #bee5eb;
            border-radius: 8px;
            text-align: center;
            font-size: 12px;
            color: #0c5460;
        }
        .disconnected-notice {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
            padding: 15px;
            margin: 15px 0;
            border-radius: 8px;
            text-align: center;
            font-weight: bold;
            display: none;
        }
        .reset-button {
            margin-top: 20px;
            padding: 10px 20px;
            background: #dc3545;
            color: white;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            font-size: 14px;
            transition: background 0.3s;
        }
        .reset-button:hover {
            background: #c82333;
        }
        .persistent-info {
            margin-top: 15px;
            padding: 10px;
            background: #d4edda;
            border: 1px solid #c3e6cb;
            border-radius: 8px;
            color: #155724;
            text-align: center;
            font-size: 12px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌡️ ESP32-S3 Θερμόμετρο BMP280</h1>
        
        <div class="reading">
            <div class="temp">🌡️ <span id="temp">--</span> °C</div>
        </div>
        
        <div class="reading">
            <div class="pressure">🌬️ <span id="pressure">--</span> hPa</div>
        </div>
        
        <div class="disconnected-notice" id="disconnected-notice">
            ⚠️ Η συσκευή είναι αποσυνδεδεμένη ή εκτός ρεύματος
        </div>
        
        <div id="status" class="status">
            📡 Κατάσταση: <span id="sensor-status">Φόρτωση...</span>
        </div>
        
        <div class="info-grid">
            <div class="info-item">
                <strong>⏱️ Τελευταία ενημέρωση</strong><br>
                <span id="lastUpdate">--</span>
            </div>
            <div class="info-item">
                <strong>🚀 Χρόνος λειτουργίας</strong><br>
                <span id="uptime">--</span>
            </div>
        </div>
        
        <div class="wifi-status" id="wifi-status">
            📶 Wi-Fi: <span id="wifi-text">Έλεγχος...</span>
        </div>
        
        <div class="alert-section" id="alert-section">
            <strong>⚠️ Ειδοποίηση:</strong> <span id="alert-text"></span>
        </div>
        
        <div class="stats-section">
            <h3 style="margin: 0 0 15px 0; color: #333;">📊 Στατιστικά Μετρήσεων</h3>
            
            <!-- Daily Statistics -->
            <div style="margin-bottom: 20px; padding: 15px; background: #fff3cd; border-radius: 8px; border: 1px solid #ffeaa7;">
                <h4 style="margin: 0 0 10px 0; color: #856404;">🗓️ Ημερήσια Στατιστικά (<span id="daily-date">--</span>)</h4>
                <div class="stats-grid">
                    <div class="stat-item" style="background: #fff8e1;">
                        <div class="stat-value" id="daily-min-temp" style="color: #f57c00;">--</div>
                        <div class="stat-label">🔽 Ημερήσια Min °C</div>
                    </div>
                    <div class="stat-item" style="background: #fff8e1;">
                        <div class="stat-value" id="daily-max-temp" style="color: #f57c00;">--</div>
                        <div class="stat-label">🔺 Ημερήσια Max °C</div>
                    </div>
                    <div class="stat-item" style="background: #fff8e1;">
                        <div class="stat-value" id="daily-avg-temp" style="color: #f57c00;">--</div>
                        <div class="stat-label">📊 Ημερήσιος Μέσος °C</div>
                    </div>
                    <div class="stat-item" style="background: #fff8e1;">
                        <div class="stat-value" id="daily-reading-count" style="color: #f57c00;">--</div>
                        <div class="stat-label">🔢 Ημερήσιες Μετρήσεις</div>
                    </div>
                </div>
                <div style="margin-top: 10px; text-align: center; font-size: 12px; color: #856404;">
                    ⏰ Απομένουν: <span id="daily-time-left">--</span> μέχρι επαναφορά (24ωρο)
                </div>
            </div>
            
            <!-- Total Statistics -->
            <div style="margin-bottom: 15px; padding: 10px; background: #e8f4fd; border-radius: 8px;">
                <h4 style="margin: 0 0 10px 0; color: #2196F3;">📈 Συνολικά Στατιστικά</h4>
                <div class="stats-grid">
                    <div class="stat-item">
                        <div class="stat-value" id="min-temp">--</div>
                        <div class="stat-label">🔽 Συνολική Min °C</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-value" id="max-temp">--</div>
                        <div class="stat-label">🔺 Συνολική Max °C</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-value" id="avg-temp">--</div>
                        <div class="stat-label">📊 Συνολικός Μέσος °C</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-value" id="reading-count">--</div>
                        <div class="stat-label">🔢 Συνολικές Μετρήσεις</div>
                    </div>
                </div>
            </div>
            <div id="temp-analysis" style="margin-top: 15px; padding: 10px; background: #e8f4fd; border-radius: 8px; text-align: center; font-weight: bold; display: none;">
            </div>
            <div class="persistent-info">
                💾 Τα συνολικά στατιστικά διατηρούνται μόνιμα - τα ημερήσια επαναφέρονται κάθε 24 ώρες αυτόματα!
            </div>
            <div class="auto-refresh-info">
                🔄 Γίνεται ανανέωση κάθε 2 δευτερόλεπτα
            </div>
            <button class="reset-button" onclick="resetStats()">Reset Statistics</button>
        </div>
        
        <div class="footer">
            <p><strong>ESP32-S3 N16R8</strong> | <strong>BMP280 Sensor</strong></p>
            <p>Real-time temperature and pressure monitoring</p>
        </div>
    </div>

    <script>
        let isOnline = true;
        let connectionAttempts = 0;
        const maxReconnectAttempts = 5;
        
        function updateData() {
            // Removed loading animation to prevent screen flashing
            
            fetch('/api')
                .then(response => {
                    if (!response.ok) throw new Error('Network response was not ok');
                    return response.json();
                })
                .then(data => {
                    connectionAttempts = 0;
                    
                    // Hide disconnected notice when connected
                    document.getElementById('disconnected-notice').style.display = 'none';
                    
                    // Update temperature
                    document.getElementById('temp').textContent = data.temperature.toFixed(2);
                    
                    // Update pressure
                    document.getElementById('pressure').textContent = data.pressure.toFixed(2);
                    
                    // Update uptime
                    const hours = Math.floor(data.uptime / 3600);
                    const minutes = Math.floor((data.uptime % 3600) / 60);
                    const seconds = data.uptime % 60;
                    document.getElementById('uptime').textContent = 
                        hours + "h " + minutes + "m " + seconds + "s";
                    
                    // Update last update time
                    document.getElementById('lastUpdate').textContent = 
                        new Date().toLocaleTimeString('el-GR');
                    
                    // Update sensor status
                    const statusDiv = document.getElementById('status');
                    const statusSpan = document.getElementById('sensor-status');
                    if (data.sensor_status === 'OK') {
                        statusDiv.className = 'status ok';
                        statusSpan.textContent = 'Connected';
                    } else {
                        statusDiv.className = 'status error';
                        statusSpan.textContent = 'Sensor Error';
                    }
                    
                    // Update Wi-Fi status
                    const wifiDiv = document.getElementById('wifi-status');
                    const wifiText = document.getElementById('wifi-text');
                    if (data.wifi_connected) {
                        wifiDiv.className = 'wifi-status wifi-connected';
                        wifiText.textContent = 'Connected (RSSI: ' + (data.wifi_rssi || 'N/A') + ' dBm)';
                    } else {
                        wifiDiv.className = 'wifi-status wifi-disconnected';
                        wifiText.textContent = 'DISCONNECTED - Returning soon! (Attempts: ' + (data.reconnect_attempts || 0) + ')';
                    }
                    
                    // Update statistics
                    if (data.reading_count > 0) {
                        document.getElementById('min-temp').textContent = data.min_temp.toFixed(2);
                        document.getElementById('max-temp').textContent = data.max_temp.toFixed(2);
                        document.getElementById('avg-temp').textContent = data.avg_temp.toFixed(2);
                        document.getElementById('reading-count').textContent = data.reading_count;
                        
                        // Show temperature analysis
                        if (data.temp_analysis) {
                            const analysisDiv = document.getElementById('temp-analysis');
                            if (analysisDiv) {
                                analysisDiv.textContent = data.temp_analysis;
                                analysisDiv.style.display = 'block';
                            }
                        }
                    }
                    
                    // Update daily statistics
                    if (data.daily_reading_count > 0) {
                        document.getElementById('daily-min-temp').textContent = data.daily_min_temp.toFixed(2);
                        document.getElementById('daily-max-temp').textContent = data.daily_max_temp.toFixed(2);
                        document.getElementById('daily-avg-temp').textContent = data.daily_avg_temp.toFixed(2);
                        document.getElementById('daily-reading-count').textContent = data.daily_reading_count;
                        document.getElementById('daily-date').textContent = data.daily_date || '--';
                        
                        // Update time left
                        const hoursLeft = data.daily_time_left_hours || 0;
                        const minutesLeft = data.daily_time_left_minutes || 0;
                        document.getElementById('daily-time-left').textContent = hoursLeft + "h " + minutesLeft + "m";
                    } else {
                        document.getElementById('daily-date').textContent = data.daily_date || '--';
                        document.getElementById('daily-time-left').textContent = '24h 0m';
                    }
                    
                    // Update alerts
                    const alertDiv = document.getElementById('alert-section');
                    const alertText = document.getElementById('alert-text');
                    if (data.last_alert && data.last_alert !== "") {
                        alertDiv.style.display = 'block';
                        alertText.textContent = data.last_alert;
                        
                        if (data.last_alert.includes('υψηλή')) {
                            alertDiv.className = 'alert-section alert-high';
                        } else if (data.last_alert.includes('χαμηλή')) {
                            alertDiv.className = 'alert-section alert-low';
                        }
                    } else {
                        alertDiv.style.display = 'none';
                    }
                    
                    isOnline = true;
                })
                .catch(error => {
                    console.error('Error:', error);
                    connectionAttempts++;
                    
                    // Show disconnected notice
                    document.getElementById('disconnected-notice').style.display = 'block';
                    
                    // Set all values to dashes when disconnected
                    document.getElementById('temp').textContent = '--';
                    document.getElementById('pressure').textContent = '--';
                    document.getElementById('uptime').textContent = '--';
                    document.getElementById('lastUpdate').textContent = '--';
                    
                    // Update status to show disconnected
                    const statusDiv = document.getElementById('status');
                    const statusSpan = document.getElementById('sensor-status');
                    statusDiv.className = 'status error';
                    statusSpan.textContent = 'Disconnected';
                    
                    // Update Wi-Fi status
                    const wifiDiv = document.getElementById('wifi-status');
                    const wifiText = document.getElementById('wifi-text');
                    wifiDiv.className = 'wifi-status wifi-disconnected';
                    
                    if (isOnline || connectionAttempts === 1) {
                        wifiText.textContent = "Disconnected - Reconnecting... (" + connectionAttempts + "/" + maxReconnectAttempts + ")";
                        isOnline = false;
                    }
                    
                    if (connectionAttempts >= maxReconnectAttempts) {
                        wifiText.textContent = 'Device offline or without power';
                        
                        // Set statistics to dashes when completely disconnected
                        document.getElementById('min-temp').textContent = '--';
                        document.getElementById('max-temp').textContent = '--';
                        document.getElementById('avg-temp').textContent = '--';
                        document.getElementById('reading-count').textContent = '--';
                        
                        document.getElementById('daily-min-temp').textContent = '--';
                        document.getElementById('daily-max-temp').textContent = '--';
                        document.getElementById('daily-avg-temp').textContent = '--';
                        document.getElementById('daily-reading-count').textContent = '--';
                        document.getElementById('daily-date').textContent = '--';
                        document.getElementById('daily-time-left').textContent = '--';
                        
                        // Hide alerts when disconnected
                        document.getElementById('alert-section').style.display = 'none';
                        
                        // Hide temperature analysis
                        const analysisDiv = document.getElementById('temp-analysis');
                        if (analysisDiv) {
                            analysisDiv.style.display = 'none';
                        }
                    }
                })
                .finally(() => {
                    // Removed loading animation removal to prevent screen flashing
                });
        }

        // Update every 2 seconds
        setInterval(updateData, 2000);
        updateData(); // Initial load
        
        // Reset statistics function
        function resetStats() {
            if (confirm('Are you sure you want to reset all statistics?')) {
                fetch('/reset')
                    .then(response => response.json())
                    .then(data => {
                        alert('Statistics reset successfully!');
                        updateData(); // Refresh display
                    })
                    .catch(error => {
                        alert('Error resetting statistics');
                    });
            }
        }
        
        // Show connection status in title
        document.addEventListener('visibilitychange', function() {
            if (!document.hidden) {
                updateData();
            }
        });
    </script>
</body>
</html>)rawliteral";
  
  server.send(200, "text/html", html);
}

// Handle OPTIONS requests for CORS
void handleOptions() {
  handleCORS();
  server.send(200, "text/plain", "");
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("🌡️ ESP32-S3 BMP280 Web Θερμόμετρο");
  Serial.println("===================================");
  Serial.println("🌐 Με Wi-Fi και Web Server");
  Serial.println();
  Serial.println("⚠️  ΣΗΜΑΝΤΙΚΟ: Αλλάξτε το Wi-Fi όνομα και κωδικό στον κώδικα!");
  Serial.println();
  
  // Initialize I2C with custom pins
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.printf("I2C αρχικοποιήθηκε: SDA=%d, SCL=%d\n", SDA_PIN, SCL_PIN);
  
  // Try to initialize BMP280
  if (bmp.begin(0x76) || bmp.begin(0x77)) {
    Serial.println("✅ BMP280 αισθητήρας βρέθηκε και αρχικοποιήθηκε!");
    sensorInitialized = true;
    
    // Configure sensor settings for better accuracy
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Operating Mode
                    Adafruit_BMP280::SAMPLING_X2,     // Temp. oversampling
                    Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                    Adafruit_BMP280::FILTER_X16,      // Filtering
                    Adafruit_BMP280::STANDBY_MS_500); // Standby time
    
    Serial.println("🎯 Ρυθμίσεις αισθητήρα ολοκληρώθηκαν.");
  } else {
    Serial.println("❌ Δεν βρέθηκε έγκυρος αισθητήρας BMP280!");
    Serial.println("Ελέγξτε τις συνδέσεις και τη διεύθυνση I2C.");
    sensorInitialized = false;
  }
  
  // Connect to Wi-Fi
  Serial.println();
  Serial.println("📶 Σύνδεση στο Wi-Fi...");
  Serial.printf("SSID: %s\n", ssid);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("✅ Wi-Fi συνδέθηκε επιτυχώς!");
    Serial.print("🌐 IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    Serial.println("🔗 Διευθύνσεις:");
    Serial.print("   📱 Web Interface: http://");
    Serial.println(WiFi.localIP());
    Serial.print("   📊 JSON API: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/api");
    Serial.println();
    Serial.println("💡 Συμβουλή: Αντιγράψτε το IP address στον browser σας!");
    
  // Setup web server routes only if WiFi connected
  if (WiFi.status() == WL_CONNECTED) {
    server.on("/", handleRoot);
    server.on("/api", handleAPI);
    server.on("/status", handleAPI);  // Alternative endpoint
    
    // Handle CORS for all endpoints
    server.onNotFound([]() {
      if (WiFi.status() == WL_CONNECTED) {
        handleCORS();
        server.send(404, "application/json", "{\"error\":\"Not Found\"}");
      }
    });
    
    // Add reset endpoint for statistics
    server.on("/reset", []() {
      if (WiFi.status() == WL_CONNECTED) {
        handleCORS();
        resetStatistics();
        server.send(200, "application/json", "{\"message\":\"Statistics reset successfully\"}");
      }
    });
    
    // Start server
    server.begin();
    serverStarted = true;
    Serial.println("🚀 Web Server ξεκίνησε!");
    Serial.println("═══════════════════════════════════════");
  } else {
    serverStarted = false;
  }
  } else {
    Serial.println();
    Serial.println("❌ Αποτυχία σύνδεσης Wi-Fi!");
    Serial.println("Ελέγξτε:");
    Serial.println("1. Το όνομα Wi-Fi (ssid) είναι σωστό");
    Serial.println("2. Ο κωδικός Wi-Fi (password) είναι σωστός");
    Serial.println("3. Το Wi-Fi είναι ενεργοποιημένο");
  }
  
  Serial.println("🚀 Έναρξη μετρήσεων...");
  Serial.println();
  
  // Load saved statistics from flash memory
  loadStatistics();
  loadDailyStatistics();
  
  // Initialize statistics if this is first run
  if (readingCount == 0) {
    statsStartTime = millis();
    minTemperature = 999.0;
    maxTemperature = -999.0;
    totalTemperature = 0.0;
    Serial.println("📊 Στατιστικά αρχικοποιήθηκαν για πρώτη φορά!");
  }
  
  // Initialize daily statistics if needed
  if (dailyReadingCount == 0 && dailyDate == "") {
    resetDailyStatistics();
  }
  
  lastSaveTime = millis();
}

void loop() {
  // Handle web server requests only if connected and server started
  if (WiFi.status() == WL_CONNECTED && serverStarted) {
    server.handleClient();
  }
  
  // Check Wi-Fi status every 5 seconds with enhanced recovery
  if (millis() - lastWiFiCheck > 5000) {
    bool currentlyConnected = WiFi.status() == WL_CONNECTED;
    
    if (!currentlyConnected && wifiWasConnected) {
      Serial.println("⚠️  Wi-Fi ΑΠΟΣΥΝΔΕΘΗΚΕ! Κατάσταση: ΑΠΟΣΥΝΔΕΣΗΣ");
      Serial.println("🔄 Θα επιστρέψει το γρηγορότερο δυνατό!");
      wifiWasConnected = false;
      serverStarted = false;
      wifiReconnectAttempts++;
      lastReconnectAttempt = millis();
      
      // Force reconnection
      WiFi.disconnect();
      delay(100);
      WiFi.begin(ssid, password);
      
    } else if (currentlyConnected && !wifiWasConnected) {
      Serial.println("✅ Wi-Fi ΕΠΑΝΑΣΥΝΔΕΘΗΚΕ!");
      Serial.printf("🌐 IP Address: %s\n", WiFi.localIP().toString().c_str());
      Serial.printf("📊 Προσπάθειες επανασύνδεσης: %lu\n", wifiReconnectAttempts);
      wifiWasConnected = true;
      
      // Restart web server
      if (!serverStarted) {
        server.on("/", handleRoot);
        server.on("/api", handleAPI);
        server.on("/status", handleAPI);
        server.begin();
        serverStarted = true;
        Serial.println("🚀 Web Server επανεκκινήθηκε!");
      }
    }
    
    // Force reconnection if disconnected for too long
    if (!currentlyConnected && (millis() - lastReconnectAttempt > 30000)) {
      Serial.println("🔄 Εξαναγκασμένη επανασύνδεση Wi-Fi...");
      WiFi.disconnect();
      delay(100);
      WiFi.begin(ssid, password);
      lastReconnectAttempt = millis();
      wifiReconnectAttempts++;
    }
    
    lastWiFiCheck = millis();
  }
  
  // Read sensor data every 2 seconds
  if (millis() - lastReading > 2000) {
    if (sensorInitialized) {
      // Read sensor values
      float temperature = bmp.readTemperature();
      float pressure = bmp.readPressure() / 100.0; // Convert Pa to hPa
      
      // Validate temperature reading (BMP280 range: -40°C to +85°C)
      if (temperature >= -40.0 && temperature <= 85.0) {
        currentTemperature = temperature;
        currentPressure = pressure;
        
        // Update statistics
        readingCount++;
        totalTemperature += temperature;
        
        // Update daily statistics
        dailyReadingCount++;
        dailyTotalTemp += temperature;
        
        if (temperature < minTemperature) {
          minTemperature = temperature;
        }
        if (temperature > maxTemperature) {
          maxTemperature = temperature;
        }
        
        if (temperature < dailyMinTemp) {
          dailyMinTemp = temperature;
        }
        if (temperature > dailyMaxTemp) {
          dailyMaxTemp = temperature;
        }
        
        // Check for temperature alerts
        String currentAlert = "";
        if (temperature >= HIGH_TEMP_THRESHOLD) {
          currentAlert = "🔥 Υψηλή θερμοκρασία! (" + String(temperature, 1) + "°C ≥ " + String(HIGH_TEMP_THRESHOLD, 1) + "°C)";
        } else if (temperature <= LOW_TEMP_THRESHOLD) {
          currentAlert = "❄️ Χαμηλή θερμοκρασία! (" + String(temperature, 1) + "°C ≤ " + String(LOW_TEMP_THRESHOLD, 1) + "°C)";
        }
        
        if (currentAlert != lastAlert) {
          lastAlert = currentAlert;
          lastAlertTime = millis() / 1000;
          if (currentAlert != "") {
            Serial.println("� ΕΙΔΟΠΟΙΗΣΗ: " + currentAlert);
          }
        }
        
        Serial.println("�📊 === ΜΕΤΡΗΣΕΙΣ BMP280 ===");
        Serial.printf("🌡️  Θερμοκρασία: %.2f °C\n", temperature);
        Serial.printf("🌬️  Πίεση: %.2f hPa\n", pressure);
        Serial.printf("📈 Στατιστικά: Min=%.1f°C, Max=%.1f°C, Μέσος=%.1f°C (%d μετρήσεις)\n", 
                     minTemperature, maxTemperature, totalTemperature/readingCount, readingCount);
        Serial.printf("📅 Ημερήσια: Min=%.1f°C, Max=%.1f°C, Μέσος=%.1f°C (%d μετρήσεις)\n", 
                     dailyMinTemp, dailyMaxTemp, dailyTotalTemp/dailyReadingCount, dailyReadingCount);
        
        if (WiFi.status() == WL_CONNECTED) {
          Serial.printf("🌐 Web: http://%s\n", WiFi.localIP().toString().c_str());
        } else {
          Serial.println("📶 Wi-Fi: Αποσυνδεδεμένο - Προσπάθεια επανασύνδεσης...");
        }
        
        if (lastAlert != "") {
          Serial.println("🚨 Ενεργή ειδοποίηση: " + lastAlert);
        }
        
        Serial.println("─────────────────────────────");
        Serial.println();
      } else {
        Serial.println("⚠️  ΑΝΙΧΝΕΥΣΗ ΛΑΘΟΣ ΜΕΤΡΗΣΗΣ!");
        Serial.printf("Θερμοκρασία: %.2f °C (Εκτός ορίων!)\n", temperature);
        Serial.println("Αυτό υποδηλώνει σφάλμα επικοινωνίας I2C.");
        Serial.println("Ελέγξτε τις συνδέσεις σας!");
        Serial.println("─────────────────────────────");
        Serial.println();
      }
    } else {
      Serial.println("❌ Αισθητήρας δεν αρχικοποιήθηκε. Ελέγξτε συνδέσεις.");
    }
    
    lastReading = millis();
  }
  
  // Check if we need to reset daily statistics
  checkDailyReset();
  
  // Save statistics periodically to prevent data loss
  if (millis() - lastSaveTime > SAVE_INTERVAL) {
    saveStatistics();
    saveDailyStatistics();
    lastSaveTime = millis();
  }
  
  // Small delay to prevent watchdog issues
  delay(10);
}
