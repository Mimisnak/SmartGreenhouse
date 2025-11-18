/*
 * ================================================================================
 * 🌱 SMART GREENHOUSE ESP32-S3 - ΠΛΉΡΩΣ ΣΧΟΛΙΑΣΜΈΝΟΣ ΚΏΔΙΚΑΣ
 * ================================================================================
 * 
 * Περιγραφή: Σύστημα παρακολούθησης περιβάλλοντος με 4 sensors
 * Hardware: ESP32-S3, BMP280, BH1750, Analog Soil Sensor
 * Δημιουργός: SmartGreenhouse Project
 * Ημερομηνία: August 2025
 * 
 * ================================================================================
 */

// ================================================================================
// 📚 ΒΙΒΛΙΟΘΉΚΕΣ - Εισαγωγή όλων των απαραίτητων libraries
// ================================================================================

#include <WiFi.h>                 // WiFi connectivity για ESP32
#include <ESPAsyncWebServer.h>    // Async web server για HTTP endpoints  
#include <Adafruit_BMP280.h>      // Driver για BMP280 temperature/pressure sensor
#include <ArduinoJson.h>          // JSON serialization/deserialization
#include <BH1750.h>               // Driver για BH1750 light sensor
#include <Wire.h>                 // I2C communication library

// ================================================================================
// ⚙️ CONFIGURATION - Ρυθμίσεις συστήματος και sensors
// ================================================================================

// --- 💧 Soil Moisture Sensor Configuration ---
// Ο soil sensor συνδέεται στο analog pin και διαβάζει ADC values
// Τα raw values μετατρέπονται σε ποσοστό υγρασίας με mapping
#define SOIL_PIN 1               // GPIO pin για soil sensor (ADC capable)
#define SOIL_DRY_VALUE 50        // Raw ADC value για 0% υγρασία (αέρας)
#define SOIL_WET_VALUE 200       // Raw ADC value για 100% υγρασία (νερό)

// --- 🌐 WiFi Network Credentials ---
const char* ssid = "Vodafone-E79546683";      // Όνομα WiFi δικτύου
const char* password = "RLg2s6b73EfarXRx";    // Password WiFi δικτύου

// --- 🔧 I2C Pin Configuration ---
// Προσαρμοσμένα pins για I2C communication (SDA/SCL)
#define SDA_PIN 16               // Serial Data line για I2C
#define SCL_PIN 17               // Serial Clock line για I2C

// ================================================================================
// 🎛️ HARDWARE OBJECTS - Δημιουργία instances για hardware components
// ================================================================================

Adafruit_BMP280 bmp;            // Object για BMP280 sensor (temp/pressure)
BH1750 lightMeter;              // Object για BH1750 light sensor
AsyncWebServer server(80);       // Web server που ακούει στο port 80

// ================================================================================
// 📊 GLOBAL VARIABLES - Καθολικές μεταβλητές για sensor data
// ================================================================================

float temperature = 0.0;        // Τρέχουσα θερμοκρασία σε Celsius
float pressure = 0.0;           // Τρέχουσα πίεση σε hPa
float lightLevel = 0.0;         // Τρέχον επίπεδο φωτός σε lux
float soilMoisture = -1.0;      // Υγρασία εδάφους σε % (-1 = μη αρχικοποιημένο)
int soilRaw = -1;               // Raw ADC reading από soil sensor

// ================================================================================
// 🔍 DEBUG & FEATURE FLAGS - Ενεργοποίηση/απενεργοποίηση features
// ================================================================================

#define ENABLE_SOIL_DEBUG 1        // 1=Εμφάνιση raw soil values στο Serial
#define ENABLE_REQUEST_LOG 1       // 1=Logging όλων των HTTP requests
#define ENABLE_CALIBRATION_MODE 1  // 1=Εμφάνιση calibration info
#define ENABLE_ALERTS 1            // 1=Ενεργοποίηση alert system

// ================================================================================
// ⚠️ ALERT THRESHOLDS - Όρια για ενεργοποίηση ειδοποιήσεων
// ================================================================================

#define TEMP_HIGH_ALERT 30.0       // Alert αν θερμοκρασία > 30°C
#define TEMP_LOW_ALERT 10.0        // Alert αν θερμοκρασία < 10°C  
#define SOIL_LOW_ALERT 20.0        // Alert αν υγρασία εδάφους < 20%

// ================================================================================
// 🛠️ UTILITY FUNCTIONS - Βοηθητικές συναρτήσεις
// ================================================================================

/**
 * Μετατρέπει HTTP method enum σε string για logging
 * @param r Pointer στο HTTP request object
 * @return String με το όνομα του HTTP method
 */
static const char* httpMethodName(AsyncWebServerRequest *r){
  switch(r->method()){                    // Έλεγχος του τύπου HTTP request
    case HTTP_GET: return "GET";          // GET request (ανάκτηση δεδομένων)
    case HTTP_POST: return "POST";        // POST request (αποστολή δεδομένων)
    case HTTP_PUT: return "PUT";          // PUT request (ενημέρωση δεδομένων)
    case HTTP_PATCH: return "PATCH";      // PATCH request (μερική ενημέρωση)
    case HTTP_DELETE: return "DELETE";    // DELETE request (διαγραφή)
    case HTTP_HEAD: return "HEAD";        // HEAD request (headers μόνο)
    case HTTP_OPTIONS: return "OPTIONS";  // OPTIONS request (available methods)
    default: return "UNKNOWN";            // Άγνωστος τύπος request
  }
}

/**
 * Καταγράφει HTTP requests στο Serial Monitor για debugging
 * @param request Pointer στο HTTP request
 * @param statusCode HTTP status code της απάντησης
 */
void logRequest(AsyncWebServerRequest *request, int statusCode){
#if ENABLE_REQUEST_LOG                    // Μόνο αν το logging είναι ενεργό
  String clientIP = request->client()->remoteIP().toString();  // IP του client
  String method = httpMethodName(request);                     // HTTP method
  String url = request->url();                                 // Requested URL
  
  // Εκτύπωση formatted log message στο Serial
  Serial.printf("REQ %s %s FROM %s -> %d\\n", 
                method.c_str(), url.c_str(), clientIP.c_str(), statusCode);
#endif
}

// ================================================================================
// 🔧 SENSOR INITIALIZATION - Αρχικοποίηση hardware sensors
// ================================================================================

/**
 * Αρχικοποιεί το BMP280 sensor για temperature/pressure readings
 * @return true αν η αρχικοποίηση ήταν επιτυχής, false αλλιώς
 */
bool initBMP280() {
  Wire.begin(SDA_PIN, SCL_PIN);           // Αρχικοποίηση I2C με custom pins
  
  if (!bmp.begin()) {                     // Προσπάθεια σύνδεσης στο BMP280
    Serial.println("Error: BMP280 sensor not found!");  // Error message
    return false;                         // Επιστροφή false σε περίπτωση αποτυχίας
  }
  
  // Ρύθμιση παραμέτρων BMP280 για βέλτιστη απόδοση
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Continuous mode
                  Adafruit_BMP280::SAMPLING_X2,     // Temperature oversampling
                  Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling  
                  Adafruit_BMP280::FILTER_X16,      // IIR filter coefficient
                  Adafruit_BMP280::STANDBY_MS_500); // Standby time
  
  Serial.println("BMP280 initialized successfully!");  // Success message
  return true;                            // Επιστροφή true σε περίπτωση επιτυχίας
}

/**
 * Αρχικοποιεί το BH1750 light sensor
 * @return true αν η αρχικοποίηση ήταν επιτυχής, false αλλιώς
 */
bool initBH1750() {
  if (!lightMeter.begin()) {              // Προσπάθεια σύνδεσης στο BH1750
    Serial.println("Error: BH1750 sensor not found!");  // Error message
    return false;                         // Επιστροφή false σε περίπτωση αποτυχίας
  }
  Serial.println("BH1750 initialized successfully!");   // Success message
  return true;                            // Επιστροφή true σε περίπτωση επιτυχίας
}

// ================================================================================
// 💧 SOIL MOISTURE SENSOR - Ανάγνωση και επεξεργασία soil sensor
// ================================================================================

/**
 * Διαβάζει το soil sensor και επιστρέφει ποσοστό υγρασίας με anti-bounce filtering
 * Χρησιμοποιεί multiple readings και smoothing για σταθερότητα
 * @return Ποσοστό υγρασίας (0-100%) ή -1 σε περίπτωση σφάλματος
 */
float readSoilMoisturePercent() {
  // --- Πολλαπλές αναγνώσεις για σταθερότητα ---
  int readings[5];                        // Array για αποθήκευση 5 measurements
  int validReadings = 0;                  // Μετρητής έγκυρων αναγνώσεων
  
  // Λήψη 5 διαδοχικών αναγνώσεων για averaging
  for (int i = 0; i < 5; i++) {
    int reading = analogRead(SOIL_PIN);   // Ανάγνωση ADC value από soil sensor
    if (reading > 0) {                    // Έλεγχος για έγκυρη ανάγνωση
      readings[validReadings] = reading;  // Αποθήκευση έγκυρης ανάγνωσης
      validReadings++;                    // Αύξηση μετρητή έγκυρων αναγνώσεων
    }
    delay(10);                            // Μικρή καθυστέρηση μεταξύ αναγνώσεων
  }
  
  if (validReadings == 0) return -1;      // Καμία έγκυρη ανάγνωση - επιστροφή error
  
  // --- Υπολογισμός μέσου όρου των αναγνώσεων ---
  long sum = 0;                           // Άθροισμα όλων των αναγνώσεων
  for (int i = 0; i < validReadings; i++) {
    sum += readings[i];                   // Προσθήκη κάθε ανάγνωσης στο άθροισμα
  }
  soilRaw = sum / validReadings;          // Μέσος όρος = άθροισμα / πλήθος

  // --- Anti-bounce filtering για αποφυγή erratic jumps ---
  static int lastRaw = -1;               // Προηγούμενη raw ανάγνωση (static = διατηρείται)
  static float lastPercent = -1;         // Προηγούμενο ποσοστό (static = διατηρείται)
  
  if (lastRaw != -1) {                   // Αν υπάρχει προηγούμενη ανάγνωση
    int diff = abs(soilRaw - lastRaw);   // Υπολογισμός διαφοράς με προηγούμενη
    if (diff < 20) {                     // Αν η αλλαγή είναι μικρή
      soilRaw = (soilRaw + lastRaw) / 2; // Smoothing: μέσος όρος με προηγούμενη
    } else if (diff > 100) {             // Αν η αλλαγή είναι πολύ μεγάλη
      soilRaw = lastRaw;                 // Αγνόηση - χρήση προηγούμενης τιμής
    }
  }
  lastRaw = soilRaw;                     // Αποθήκευση current τιμής για επόμενη φορά
  
  // --- Μετατροπή raw value σε ποσοστό υγρασίας ---
  int pct = map(soilRaw, SOIL_DRY_VALUE, SOIL_WET_VALUE, 0, 100);
  if (pct < 0) pct = 0;                  // Clamping: ελάχιστο 0%
  if (pct > 100) pct = 100;              // Clamping: μέγιστο 100%
  
  float currentPercent = (float)pct;     // Μετατροπή σε float
  
  // --- Percentage-level smoothing για επιπλέον σταθερότητα ---
  if (lastPercent != -1) {               // Αν υπάρχει προηγούμενο ποσοστό
    float diff = abs(currentPercent - lastPercent);  // Διαφορά ποσοστών
    if (diff > 50) {                     // Αν το jump είναι > 50%
      currentPercent = (currentPercent + lastPercent) / 2;  // Smoothing
    }
  }
  lastPercent = currentPercent;          // Αποθήκευση για επόμενη φορά
  
  return currentPercent;                 // Επιστροφή τελικού ποσοστού υγρασίας
}

// ================================================================================
// 📊 CALIBRATION SYSTEM - Σύστημα calibration για soil sensor
// ================================================================================

/**
 * Παρακολουθεί τις raw αναγνώσεις του soil sensor και προτείνει calibration values
 * Εμφανίζει στο Serial τα min/max values και προτάσεις για SOIL_DRY/WET_VALUE
 */
void calibrateSoilSensor() {
#if ENABLE_CALIBRATION_MODE               // Μόνο αν το calibration mode είναι ενεργό
  static int minRaw = 4095;               // Ελάχιστη raw τιμή (αρχικά max ADC value)
  static int maxRaw = 0;                  // Μέγιστη raw τιμή (αρχικά 0)
  static unsigned long lastDebug = 0;     // Timestamp τελευταίου debug output
  
  if (soilRaw > 0) {                      // Αν υπάρχει έγκυρη ανάγνωση
    if (soilRaw < minRaw) minRaw = soilRaw;  // Ενημέρωση ελάχιστης τιμής
    if (soilRaw > maxRaw) maxRaw = soilRaw;  // Ενημέρωση μέγιστης τιμής
    
    // Εκτύπωση debug info κάθε 5 δευτερόλεπτα
    if (millis() - lastDebug > 5000) {    // 5000ms = 5 δευτερόλεπτα
      Serial.printf("📊 CALIBRATION: Range %d-%d | Current: %d | Suggest: DRY=%d, WET=%d\\n", 
                    minRaw, maxRaw, soilRaw, minRaw, maxRaw);
      lastDebug = millis();               // Ενημέρωση timestamp
    }
  }
#endif
}

// ================================================================================
// ⚠️ ALERT SYSTEM - Σύστημα ειδοποιήσεων για ακραίες τιμές
// ================================================================================

/**
 * Ελέγχει τις sensor readings και εμφανίζει alerts για ακραίες τιμές
 * Περιλαμβάνει rate limiting για αποφυγή spam alerts
 */
void checkAlerts() {
#if ENABLE_ALERTS                         // Μόνο αν το alert system είναι ενεργό
  static unsigned long lastAlert = 0;     // Timestamp τελευταίου alert
  unsigned long now = millis();           // Τρέχων χρόνος
  
  // Rate limiting: alerts μόνο κάθε 30 δευτερόλεπτα
  if (now - lastAlert < 30000) return;    // 30000ms = 30 δευτερόλεπτα
  
  bool alertShown = false;                // Flag για tracking αν εμφανίστηκε alert
  
  // --- Temperature Alerts ---
  if (temperature > TEMP_HIGH_ALERT) {    // Υψηλή θερμοκρασία
    Serial.printf("🌡️ ALERT: High temperature %.1f°C (threshold: %.1f°C)\\n", 
                  temperature, TEMP_HIGH_ALERT);
    alertShown = true;                    // Σημείωση ότι εμφανίστηκε alert
  } else if (temperature < TEMP_LOW_ALERT) {  // Χαμηλή θερμοκρασία
    Serial.printf("❄️ ALERT: Low temperature %.1f°C (threshold: %.1f°C)\\n", 
                  temperature, TEMP_LOW_ALERT);
    alertShown = true;                    // Σημείωση ότι εμφανίστηκε alert
  }
  
  // --- Soil Moisture Alerts ---
  if (soilMoisture >= 0 && soilMoisture < SOIL_LOW_ALERT) {  // Χαμηλή υγρασία εδάφους
    Serial.printf("🌵 ALERT: Low soil moisture %.0f%% (threshold: %.0f%%)\\n", 
                  soilMoisture, SOIL_LOW_ALERT);
    alertShown = true;                    // Σημείωση ότι εμφανίστηκε alert
  }
  
  // Ενημέρωση timestamp αν εμφανίστηκε κάποιο alert
  if (alertShown) {
    lastAlert = now;                      // Reset του timer για rate limiting
  }
#endif
}

// ================================================================================
// 🌐 WEB SERVER SETUP - Ρύθμιση HTTP endpoints και web interface
// ================================================================================

/**
 * Ρυθμίζει όλα τα HTTP endpoints του web server
 * Περιλαμβάνει data APIs, web interface, και utility endpoints
 */
void setupWebServer() {
  
  // --- 📄 Root Endpoint: Main Web Interface ---
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    // Δημιουργία HTML response με embedded CSS και JavaScript
    String response = "<!DOCTYPE html><html><head>";
    response += "<meta charset='utf-8'>";                     // UTF-8 encoding
    response += "<meta name='viewport' content='width=device-width,initial-scale=1'>";  // Mobile responsive
    response += "<title>Smart Greenhouse</title>";            // Page title
    response += "<style>";                                    // CSS styles
    response += "body{font-family:Arial,sans-serif;margin:20px;background:#f0f0f0;}";
    response += ".container{max-width:800px;margin:0 auto;background:white;padding:20px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
    response += "h1{color:#2e7d32;text-align:center;margin-bottom:30px;}";
    response += ".sensor{background:#e8f5e8;padding:15px;margin:10px 0;border-radius:8px;border-left:4px solid #4caf50;}";
    response += ".value{font-size:24px;font-weight:bold;color:#1b5e20;}";
    response += ".unit{color:#666;font-size:14px;}";
    response += ".timestamp{text-align:center;color:#888;margin-top:20px;}";
    response += "</style></head><body>";
    response += "<div class='container'>";
    response += "<h1>🌱 Smart Greenhouse Monitor</h1>";
    
    // Sensor data display containers (populated by JavaScript)
    response += "<div class='sensor'>🌡️ Temperature: <span class='value' id='temp'>--</span> <span class='unit'>°C</span></div>";
    response += "<div class='sensor'>🌪️ Pressure: <span class='value' id='press'>--</span> <span class='unit'>hPa</span></div>";
    response += "<div class='sensor'>💡 Light: <span class='value' id='light'>--</span> <span class='unit'>lux</span></div>";
    response += "<div class='sensor'>💧 Soil Moisture: <span class='value' id='soil'>--</span> <span class='unit'>%</span></div>";
    response += "<div class='timestamp'>Last Update: <span id='time'>--</span></div>";
    response += "</div>";
    
    // JavaScript για automatic data refresh
    response += "<script>";
    response += "function updateData(){";                     // Function για data update
    response += "fetch('/api').then(r=>r.json()).then(d=>{"; // API call
    response += "document.getElementById('temp').textContent=d.temperature.toFixed(1);";   // Temperature update
    response += "document.getElementById('press').textContent=d.pressure.toFixed(1);";     // Pressure update
    response += "document.getElementById('light').textContent=d.light.toFixed(0);";        // Light update
    response += "document.getElementById('soil').textContent=d.soil>=0?d.soil.toFixed(0):'N/A';";  // Soil update
    response += "document.getElementById('time').textContent=new Date().toLocaleTimeString();";    // Timestamp
    response += "});}";
    response += "updateData();";                              // Initial data load
    response += "setInterval(updateData,2000);";              // Auto-refresh κάθε 2 δευτερόλεπτα
    response += "</script></body></html>";
    
    logRequest(request, 200);                                 // Log του request
    request->send(200, "text/html", response);                // Αποστολή HTML response
  });

  // --- 📱 Simple Endpoint: Lightweight Mobile Interface ---
  server.on("/simple", HTTP_GET, [](AsyncWebServerRequest *request){
    // Minimalistic HTML για mobile devices ή slow connections
    String p = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Greenhouse Simple</title>";
    p += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
    p += "<style>body{font-family:Arial;margin:10px;}table{border-collapse:collapse;}";
    p += "td,th{border:1px solid #888;padding:6px;}code{background:#eee;padding:2px 4px;border-radius:4px;}</style></head>";
    p += "<body><h2>Smart Greenhouse - Simple</h2><div id='ip'></div>";
    p += "<table><thead><tr><th>Metric</th><th>Value</th></tr></thead><tbody>";
    p += "<tr><td>Temperature (°C)</td><td id='t'>--</td></tr>";
    p += "<tr><td>Pressure (hPa)</td><td id='p'>--</td></tr>";
    p += "<tr><td>Light (lux)</td><td id='l'>--</td></tr>";
    p += "<tr><td>Soil (%)</td><td id='s'>--</td></tr>";
    p += "<tr><td>Uptime (s)</td><td id='u'>--</td></tr>";
    p += "</tbody></table>";
    p += "<p>API: <code>/api</code>, Health: <code>/health</code>, Metrics: <code>/metrics</code></p>";
    
    // Compact JavaScript για data updates
    p += "<script>";
    p += "function g(id){return document.getElementById(id);}";  // Helper function
    p += "function upd(){";                                      // Update function
    p += "fetch('/api').then(r=>r.json()).then(d=>{";           // API fetch
    p += "g('t').textContent=d.temperature.toFixed(1);";        // Update temperature
    p += "g('p').textContent=d.pressure.toFixed(2);";           // Update pressure  
    p += "g('l').textContent=d.light>0?d.light.toFixed(0):'N/A';";  // Update light
    p += "g('s').textContent=d.soil>=0?d.soil.toFixed(0):'N/A';";   // Update soil
    p += "g('u').textContent=(d.timestamp/1000).toFixed(0);";   // Update uptime
    p += "});}";
    p += "upd();setInterval(upd,2000);";                        // Initial + auto-refresh
    p += "</script></body></html>";
    
    logRequest(request, 200);                                   // Log του request
    request->send(200, "text/html", p);                         // Αποστολή response
  });

  // --- 📊 API Endpoint: JSON Data για Applications ---
  server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument doc;                                           // JSON document creation
    doc["temperature"] = temperature;                           // Προσθήκη temperature data
    doc["pressure"] = pressure;                                 // Προσθήκη pressure data
    doc["light"] = (lightLevel != -1) ? lightLevel : 0;        // Light data (0 αν N/A)
    doc["soil"] = (soilMoisture >= 0) ? soilMoisture : -1;      // Soil data (-1 αν N/A)
    doc["timestamp"] = millis();                                // System uptime
    
    String res;                                                 // String για JSON output
    serializeJson(doc, res);                                    // JSON serialization
    request->send(200, "application/json", res);                // Αποστολή JSON response
    logRequest(request, 200);                                   // Log του request
  });

  // --- 🔍 Health Endpoint: System Status Information ---
  server.on("/health", HTTP_GET, [](AsyncWebServerRequest *request){
    String body = "OK\\n";                                      // Status indicator
    body += "uptime_ms=" + String(millis()) + "\\n";           // System uptime
    body += "free_heap=" + String(ESP.getFreeHeap()) + "\\n";  // Available RAM
    body += "wifi_strength=" + String(WiFi.RSSI()) + "\\n";    // WiFi signal strength
    
    logRequest(request, 200);                                   // Log του request
    request->send(200, "text/plain", body);                     // Αποστολή plain text
  });

  // --- 📈 Metrics Endpoint: Prometheus-Compatible Metrics ---
  server.on("/metrics", HTTP_GET, [](AsyncWebServerRequest *request){
    String m;                                                   // Metrics string
    
    // Temperature metric
    m += "# HELP greenhouse_temperature_c Current temperature in Celsius\\n";
    m += "# TYPE greenhouse_temperature_c gauge\\n";
    m += String("greenhouse_temperature_c ") + String(temperature, 2) + "\\n";
    
    // Pressure metric
    m += "# HELP greenhouse_pressure_hpa Atmospheric pressure in hPa\\n";
    m += "# TYPE greenhouse_pressure_hpa gauge\\n";
    m += String("greenhouse_pressure_hpa ") + String(pressure, 2) + "\\n";
    
    // Light metric
    m += "# HELP greenhouse_light_lux Light level in lux\\n";
    m += "# TYPE greenhouse_light_lux gauge\\n";
    m += String("greenhouse_light_lux ") + String(lightLevel != -1 ? lightLevel : 0, 2) + "\\n";
    
    // Soil moisture metric
    m += "# HELP greenhouse_soil_percent Soil moisture percent\\n";
    m += "# TYPE greenhouse_soil_percent gauge\\n";
    m += String("greenhouse_soil_percent ") + String(soilMoisture >= 0 ? soilMoisture : 0, 2) + "\\n";
    
    // System metrics
    m += "# HELP greenhouse_uptime_ms Uptime in milliseconds\\n";
    m += "# TYPE greenhouse_uptime_ms counter\\n";
    m += String("greenhouse_uptime_ms ") + String(millis()) + "\\n";
    
    m += "# HELP greenhouse_free_heap_bytes Free heap bytes\\n";
    m += "# TYPE greenhouse_free_heap_bytes gauge\\n";
    m += String("greenhouse_free_heap_bytes ") + String(ESP.getFreeHeap()) + "\\n";
    
    logRequest(request, 200);                                   // Log του request
    request->send(200, "text/plain; version=0.0.4", m);        // Prometheus format
  });

  // --- 🔧 Calibrate Endpoint: Soil Sensor Calibration Helper ---
  server.on("/calibrate", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Soil Calibration</title>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
    html += "<style>body{font-family:Arial;margin:20px;background:#f0f0f0;} ";
    html += ".container{max-width:600px;margin:0 auto;background:white;padding:20px;border-radius:10px;} ";
    html += ".raw{font-size:2em;text-align:center;margin:20px 0;padding:20px;background:#e3f2fd;border-radius:8px;} ";
    html += ".step{background:#f5f5f5;padding:15px;margin:10px 0;border-radius:5px;} ";
    html += ".code{background:#333;color:#0f0;padding:10px;border-radius:5px;font-family:monospace;}</style></head><body>";
    html += "<div class='container'><h1>🌱 Soil Sensor Calibration</h1>";
    
    // Real-time raw value display
    html += "<div class='raw'>Current Raw: <span id='raw'>" + String(soilRaw) + "</span></div>";
    
    // Calibration instructions
    html += "<div class='step'><h3>Step 1: Dry Measurement</h3>";
    html += "<p>Remove sensor from soil and measure in air.</p>";
    html += "<div class='code'>Current: " + String(soilRaw) + "</div></div>";
    
    html += "<div class='step'><h3>Step 2: Wet Measurement</h3>";
    html += "<p>Dip sensor in water and measure.</p></div>";
    
    html += "<div class='step'><h3>Step 3: Update Code</h3>";
    html += "<div class='code'>#define SOIL_DRY_VALUE " + String(soilRaw) + "<br>";
    html += "#define SOIL_WET_VALUE [wet_value]</div></div></div>";
    
    // Auto-refresh script για real-time updates
    html += "<script>setInterval(()=>fetch('/api').then(r=>r.json()).then(d=>";
    html += "document.getElementById('raw').textContent=d.soil_raw),2000);</script>";
    html += "</body></html>";
    
    logRequest(request, 200);                                   // Log του request
    request->send(200, "text/html", html);                      // Αποστολή calibration page
  });

  server.begin();                                               // Εκκίνηση του web server
  Serial.println("Web server started!");                       // Confirmation message
}

// ================================================================================
// 🚀 ARDUINO SETUP - Αρχικοποίηση συστήματος (εκτελείται μία φορά)
// ================================================================================

void setup() {
  // --- 📺 Serial Communication Setup ---
  Serial.begin(115200);                                        // Serial για debugging (115200 baud)
  delay(1000);                                                 // Περίμενε 1 δευτερόλεπτο για stabilization
  Serial.println("\\n🌱 Smart Greenhouse Starting...");        // Startup message

  // --- 🔧 Sensor Initialization ---
  Serial.println("Initializing sensors...");
  
  // BMP280 initialization
  if (!initBMP280()) {                                         // Προσπάθεια αρχικοποίησης BMP280
    Serial.println("Failed to initialize BMP280!");           // Error αν αποτύχει
    // Συνέχεια χωρίς BMP280 (graceful degradation)
  }
  
  // BH1750 initialization  
  if (!initBH1750()) {                                         // Προσπάθεια αρχικοποίησης BH1750
    Serial.println("Failed to initialize BH1750!");           // Error αν αποτύχει
    lightLevel = -1;                                           // Σημείωση ότι το light sensor δεν είναι διαθέσιμο
  }

  // --- 🌐 WiFi Connection Setup ---
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);                                         // Station mode (client)
  WiFi.begin(ssid, password);                                  // Σύνδεση στο WiFi network
  
  // Περίμενε για WiFi connection με timeout
  int attempts = 0;                                            // Μετρητής προσπαθειών
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {     // Max 20 προσπάθειες
    delay(500);                                                // Περίμενε 500ms
    Serial.print(".");                                         // Progress indicator
    attempts++;                                                // Αύξηση μετρητή
  }
  
  if (WiFi.status() == WL_CONNECTED) {                         // Αν η σύνδεση ήταν επιτυχής
    Serial.println("\\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());                           // Εμφάνιση IP address
    Serial.print("Signal strength: ");
    Serial.print(WiFi.RSSI());                                // Εμφάνιση signal strength
    Serial.println(" dBm");
  } else {                                                     // Αν η σύνδεση απέτυχε
    Serial.println("\\nWiFi connection failed!");
    Serial.println("Continuing in offline mode...");          // Συνέχεια χωρίς WiFi
  }

  // --- 🌐 Web Server Setup ---
  setupWebServer();                                            // Ρύθμιση όλων των HTTP endpoints

  // --- ✅ Startup Complete ---
  Serial.println("\\n✅ Smart Greenhouse ready!");
  Serial.println("Available endpoints:");
  Serial.println("  http://" + WiFi.localIP().toString() + "/         - Main interface");
  Serial.println("  http://" + WiFi.localIP().toString() + "/simple   - Simple view");
  Serial.println("  http://" + WiFi.localIP().toString() + "/api      - JSON API");
  Serial.println("  http://" + WiFi.localIP().toString() + "/calibrate - Soil calibration");
  Serial.println("\\nStarting sensor monitoring...");
}

// ================================================================================
// 🔄 ARDUINO LOOP - Κύριος βρόχος προγράμματος (εκτελείται συνεχώς)
// ================================================================================

void loop() {
  // --- 📊 Sensor Data Collection ---
  temperature = bmp.readTemperature();                         // Ανάγνωση θερμοκρασίας από BMP280
  pressure = bmp.readPressure() / 100.0F;                      // Ανάγνωση πίεσης (μετατροπή Pa σε hPa)
  
  if (lightLevel != -1) {                                      // Αν το light sensor είναι διαθέσιμο
    lightLevel = lightMeter.readLightLevel();                  // Ανάγνωση φωτεινότητας
  }
  
  soilMoisture = readSoilMoisturePercent();                    // Ανάγνωση υγρασίας εδάφους με filtering
  
  // --- 🔧 System Functions ---
  calibrateSoilSensor();                                       // Calibration monitoring
  checkAlerts();                                               // Alert checking

  // --- 📺 Serial Output για Debugging ---
  Serial.print("Temperature: "); Serial.print(temperature);   // Εκτύπωση θερμοκρασίας
  Serial.print(" °C, Pressure: "); Serial.print(pressure);    // Εκτύπωση πίεσης
  Serial.print(" hPa");
  
  if (lightLevel != -1) {                                      // Αν το light είναι διαθέσιμο
    Serial.print(", Light: "); Serial.print(lightLevel);      // Εκτύπωση φωτεινότητας
    Serial.print(" lux");
  } else {                                                     // Αλλιώς
    Serial.print(", Light: N/A");                             // Εμφάνιση N/A
  }
  
  if (soilMoisture >= 0) {                                     // Αν το soil είναι διαθέσιμο
    Serial.print(", Soil: "); Serial.print(soilMoisture);     // Εκτύπωση υγρασίας εδάφους
    Serial.print(" %");
#if ENABLE_SOIL_DEBUG                                          // Αν το soil debug είναι ενεργό
    Serial.print(" (raw="); Serial.print(soilRaw);            // Εκτύπωση raw value για debugging
    Serial.print(")");
#endif
    Serial.println();                                          // Νέα γραμμή
  } else {                                                     // Αλλιώς
    Serial.println(", Soil: N/A");                            // Εμφάνιση N/A για soil
  }
  
  // --- ⏱️ Loop Timing ---
  delay(500);                                                  // Περίμενε 500ms μεταξύ iterations
}

// ================================================================================
// 📝 ΤΈΛΟΣ ΚΏΔΙΚΑ - Συνολικές γραμμές: ~400+
// ================================================================================
