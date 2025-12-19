/*
 * Smart Greenhouse - Temperature, Pressure & Light Monitoring
 * LOCAL IP ONLY MODE - No Cloud/Firebase
 */
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_BMP280.h>
#include <ArduinoJson.h>
#include <BH1750.h>
#include <Wire.h>
#include <LittleFS.h>
#include <HTTPClient.h>
// #include <FirebaseESP32.h>  // DISABLED - Local IP only
#include <FastLED.h>

// RGB LED Configuration (WS2812 addressable LED on ESP32-S3)
#define LED_PIN 48
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

// Water Pump Relay Configuration (JQC-3FF-S-Z)
#define RELAY_PIN 5        // GPIO5 για το relay
// Το relay module έχει ενσωματωμένα LED indicators στην πλακέτα

// --- Sensor Registry Structure ---
enum SensorType {
  SENSOR_BMP280_TEMP,
  SENSOR_BMP280_PRESSURE,
  SENSOR_BH1750_LIGHT,
  SENSOR_SOIL_MOISTURE,
  SENSOR_COUNT
};

struct SensorInfo {
  const char* name;
  const char* unit;
  bool enabled;
  bool available;
  float lastValue;
  unsigned long lastRead;
};

SensorInfo sensors[SENSOR_COUNT] = {
  {"Temperature", "°C", true, false, 0.0, 0},
  {"Pressure", "hPa", true, false, 0.0, 0},
  {"Light", "lux", true, false, 0.0, 0},
  {"Soil Moisture", "%", true, false, 0.0, 0}
};

// --- Cloud Configuration (DISABLED - Local IP Only) ---
#define ENABLE_CLOUD_SYNC false  // Cloud sync DISABLED

/*
// Firebase Configuration (COMMENTED OUT)
#define FIREBASE_HOST "smartgreenhouse-fb494-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "AIzaSyDwwszw4AapfTp_dkdli48vsxOZXkZwqfo"
#define DEVICE_ID "ESP32-Greenhouse"

FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;
*/

const char* deviceId = "ESP32-Greenhouse";
unsigned long lastCloudSync = 0;
#define CLOUD_SYNC_INTERVAL 300000  // 5 minutes (not used in local mode)

// --- Soil Moisture Configuration ---
// Adjust SOIL_PIN to your actual analog pin. Choose an ADC1 capable pin.
// Calibrate SOIL_DRY_VALUE (reading in completely dry air) and SOIL_WET_VALUE (reading fully wet)
// then percentage = map(raw, SOIL_DRY_VALUE, SOIL_WET_VALUE, 0, 100) (clamped)
#define SOIL_PIN 4  // GPIO 4 - excellent ADC1 pin for soil sensor
#define SOIL_DRY_VALUE 3285  // Capacitive sensor reading in air (dry = 0%)
#define SOIL_WET_VALUE 27    // Capacitive sensor in wet soil (wet = 100%)

// LED indicator for cloud sync
const char* ssid = "Vodafone-E79546683";
const char* password = "RLg2s6b73EfarXRx";

// NTP Configuration for accurate timestamps
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7200;  // GMT+2 (Greece winter)
const int daylightOffset_sec = 3600;  // DST offset
bool timeInitialized = false;

// I2C Bus 0 pins (for BMP280)
#define SDA_PIN 16
#define SCL_PIN 17

// I2C Bus 1 pins (for BH1750 - separate bus to avoid conflicts)
#define SDA_PIN_1 8
#define SCL_PIN_1 9

Adafruit_BMP280 bmp;
TwoWire I2C_1 = TwoWire(1);  // Second I2C bus
BH1750 lightMeter(0x23);  // Initialize with address
AsyncWebServer server(80);

// Legacy global variables (kept for compatibility)
float temperature = 0.0;
float pressure = 0.0;
float lightLevel = 0.0;
float soilMoisture = -1.0; // percent, -1 means not initialized
int soilRaw = -1;          // last raw ADC reading

// Sensor management functions
void updateSensorRegistry();
void sendToCloud();

// History storage - 48 hours of data (one reading every 5 minutes = 576 points)
#define MAX_HISTORY_POINTS 288  // 24 hours at 5-minute intervals (24*60/5=288)
#define MAX_FIREBASE_HISTORY 288  // Keep last 24 hours in Firebase
struct SensorReading {
  float temperature;
  float pressure;
  float lightLevel;
  float soilMoisture;
  unsigned long timestamp;
};

SensorReading sensorHistory[MAX_HISTORY_POINTS];
int historyIndex = 0;
int historyCount = 0;
int totalReadingsCount = 0;  // Total readings sent to Firebase
float minTemperature = 999.0;  // Track min temp in 24h window
float maxTemperature = -999.0; // Track max temp in 24h window
unsigned long lastHistoryUpdate = 0;
#define HISTORY_INTERVAL 300000  // 5 minutes in milliseconds

#define ENABLE_SOIL_DEBUG 1
#define ENABLE_REQUEST_LOG 1
#define ENABLE_CALIBRATION_MODE 1  // Set to 1 to see calibration values
#define ENABLE_ALERTS 1            // Enable temperature/soil alerts

// Alert thresholds
#define TEMP_HIGH_ALERT 30.0       // Alert if temperature > 30°C
#define TEMP_LOW_ALERT 10.0        // Alert if temperature < 10°C  
#define SOIL_LOW_ALERT 20.0        // Alert if soil moisture < 20%

// Watering System Configuration
bool autoWateringEnabled = false;  // Auto watering mode
float soilMinThreshold = 30.0;     // Start watering below this %
float soilMaxThreshold = 90.0;     // Stop watering at this %
bool isWatering = false;            // Current watering state
unsigned long wateringStartTime = 0;
unsigned long manualWateringDuration = 15000;  // 15 seconds for manual watering
bool manualWateringActive = false;

static const char* httpMethodName(AsyncWebServerRequest *r){
  switch(r->method()){
    case HTTP_GET: return "GET";
    case HTTP_POST: return "POST";
    case HTTP_PUT: return "PUT";
    case HTTP_PATCH: return "PATCH";
    case HTTP_DELETE: return "DELETE";
    case HTTP_OPTIONS: return "OPTIONS";
    default: return "OTHER";
  }
}
void logRequest(AsyncWebServerRequest *request, int status){
#if ENABLE_REQUEST_LOG
  IPAddress ip = request->client()->remoteIP();
  Serial.printf("REQ %s %s FROM %s -> %d\n", httpMethodName(request), request->url().c_str(), ip.toString().c_str(), status);
#endif
}

bool initializeBMP280();
bool initializeBH1750();
float readSoilMoisturePercent();
void setupWebServer();
void checkAlerts();
void calibrateSoilSensor();
void addToHistory();
void startWatering();
void stopWatering();
void handleAutoWatering();

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize LittleFS for serving web files
  if(!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed");
    return;
  }
  Serial.println("LittleFS Mounted Successfully");
  
  // Configure soil sensor pin with pull-down to prevent floating
  pinMode(SOIL_PIN, INPUT_PULLDOWN);
  
  // Configure water pump relay (module has built-in LED indicators)
  // Active-HIGH relay: HIGH=ON, LOW=OFF
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);      // Relay OFF (pump off) - active HIGH
  Serial.println("Water pump relay initialized (OFF)");
  
  // Initialize I2C Bus 0 for BMP280
  Wire.begin(SDA_PIN, SCL_PIN, 100000);
  Serial.printf("I2C Bus 0 initialized: SDA=%d, SCL=%d (BMP280)\n", SDA_PIN, SCL_PIN);
  
  // Initialize I2C Bus 1 for BH1750
  I2C_1.begin(SDA_PIN_1, SCL_PIN_1, 100000);
  Serial.printf("I2C Bus 1 initialized: SDA=%d, SCL=%d (BH1750)\n", SDA_PIN_1, SCL_PIN_1);
  
  delay(100);
  
  // Scan I2C Bus 1 to find devices
  Serial.println("Scanning I2C Bus 1 (GPIO 8/9)...");
  byte count = 0;
  for (byte i = 1; i < 127; i++) {
    I2C_1.beginTransmission(i);
    byte error = I2C_1.endTransmission();
    if (error == 0) {
      Serial.printf("Found device at address 0x%02X\n", i);
      count++;
    }
  }
  Serial.printf("Found %d device(s) on Bus 1\n", count);
  
  // Scan I2C Bus 0 to see if BH1750 is there instead
  Serial.println("Scanning I2C Bus 0 (GPIO 16/17)...");
  count = 0;
  for (byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("Found device at address 0x%02X\n", i);
      count++;
    }
  }
  Serial.printf("Found %d device(s) on Bus 0\n", count);
  
  Serial.println("Initializing sensors...");
  
  if (!initializeBMP280()) { 
    Serial.println("Could not find a valid BMP280 sensor, check wiring! (continuing without BMP)"); 
  }
  
  // Small delay between sensor initializations
  delay(50);
  
  if (!initializeBH1750()) { 
    Serial.println("BH1750 sensor not found - continuing without light sensor"); 
    lightLevel = -1; 
  }
  // Initial soil moisture read (will stay -1 if pin not connected / invalid)
  soilMoisture = readSoilMoisturePercent();
  
  // Initialize RGB LED (WS2812)
  FastLED.addLeds<WS2812, LED_PIN, RGB>(leds, NUM_LEDS);  // Changed to RGB color order
  FastLED.setBrightness(50);  // 50/255 brightness
  leds[0] = CRGB::Black;  // Off initially
  FastLED.show();
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println(); Serial.println("WiFi connected!"); Serial.print("IP address: "); Serial.println(WiFi.localIP());
  
  // Initialize NTP for accurate timestamps
  Serial.println("Initializing NTP time...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.println("Time synchronized with NTP server");
    Serial.print("Current time: ");
    Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
    timeInitialized = true;
  } else {
    Serial.println("Failed to sync time with NTP - timestamps may be incorrect");
    timeInitialized = false;
  }
  
  /*
  // Firebase Initialization (DISABLED - Local IP Only)
  Serial.println("Initializing Firebase...");
  firebaseConfig.host = FIREBASE_HOST;
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase initialized!");
  */
  Serial.println("Cloud sync DISABLED - Local IP only mode");
  
  setupWebServer();
  server.begin();
  Serial.println("HTTP server started - Access at http://192.168.2.20");
}

bool initializeBMP280() {
  if (!bmp.begin(0x76)) {
    Serial.println("Trying alternative address...");
    if (!bmp.begin(0x77)) return false;
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
  return true;
}

bool initializeBH1750() {
  // Configure BH1750 to use second I2C bus
  lightMeter.configure(BH1750::CONTINUOUS_HIGH_RES_MODE);
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &I2C_1)) {
    Serial.println("BH1750 initialized successfully on Bus 1!");
    return true;
  }
  // Try alternative address
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x5C, &I2C_1)) {
    Serial.println("BH1750 initialized successfully on Bus 1 (0x5C)!");
    return true;
  }
  return false;
}

float readSoilMoisturePercent() {
  // Take multiple readings for stability 
  int readings[5];
  int validReadings = 0;
  
  for (int i = 0; i < 5; i++) {
    int reading = analogRead(SOIL_PIN);
    if (reading > 0) {
      readings[validReadings] = reading;
      validReadings++;
    }
    delay(10); // Small delay between readings
  }
  
  if (validReadings == 0) return -1; // No valid readings
  
  // Calculate average
  long sum = 0;
  for (int i = 0; i < validReadings; i++) {
    sum += readings[i];
  }
  soilRaw = sum / validReadings;
  
  // REVERSE APPROACH: Use raw value, add minimal smoothing only for extreme noise
  static int lastRaw = -1;
  static float lastPercent = -1;
  
  // Only smooth for extreme spikes (over 400 raw units difference)
  if (lastRaw != -1) {
    int diff = abs(soilRaw - lastRaw);
    if (diff > 400) { // Only for extreme spikes
      soilRaw = (soilRaw + lastRaw) / 2;
    }
  }
  lastRaw = soilRaw;
  
  int pct = map(soilRaw, SOIL_DRY_VALUE, SOIL_WET_VALUE, 0, 100);
  if (pct < 0) pct = 0; 
  if (pct > 100) pct = 100;
  
  float currentPercent = (float)pct;
  
  // Minimal percentage smoothing - only for very large jumps
  if (lastPercent != -1) {
    float diff = abs(currentPercent - lastPercent);
    if (diff > 80) { // Only smooth very extreme percentage changes
      currentPercent = (currentPercent + lastPercent) / 2;
    }
  }
  lastPercent = currentPercent;
  
  return currentPercent;
}

void calibrateSoilSensor() {
#if ENABLE_CALIBRATION_MODE
  static int minRaw = 4095, maxRaw = 0;
  static unsigned long lastDebug = 0;
  
  if (soilRaw > 0) {
    if (soilRaw < minRaw) minRaw = soilRaw;
    if (soilRaw > maxRaw) maxRaw = soilRaw;
    
    // Print debug info every 5 seconds
    if (millis() - lastDebug > 5000) { 
      Serial.printf("📊 CALIBRATION: Range %d-%d | Current: %d | Suggest: DRY=%d, WET=%d\n", 
                    minRaw, maxRaw, soilRaw, minRaw, maxRaw);
      lastDebug = millis();
    }
  }
#endif
}

void checkAlerts() {
#if ENABLE_ALERTS
  static unsigned long lastAlert = 0;
  if (millis() - lastAlert > 30000) { // Check every 30 seconds
    lastAlert = millis();
    
    // Temperature alerts
    if (temperature > TEMP_HIGH_ALERT) {
      Serial.printf("🔥 ALERT: High temperature %.1f°C (threshold: %.1f°C)\n", 
                    temperature, TEMP_HIGH_ALERT);
    }
    if (temperature < TEMP_LOW_ALERT && temperature > 0) {
      Serial.printf("🧊 ALERT: Low temperature %.1f°C (threshold: %.1f°C)\n", 
                    temperature, TEMP_LOW_ALERT);
    }
    
    // Soil moisture alerts  
    if (soilMoisture >= 0 && soilMoisture < SOIL_LOW_ALERT) {
      Serial.printf("🌵 ALERT: Low soil moisture %.0f%% (threshold: %.0f%%)\n", 
                    soilMoisture, SOIL_LOW_ALERT);
    }
  }
#endif
}

void setupWebServer() {
  // Serve main page from LittleFS
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    logRequest(request, 200);
    request->send(LittleFS, "/index.html", "text/html");
  });

  // CORS preflight handler for OPTIONS requests
  server.on("/api", HTTP_OPTIONS, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(204);
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(response);
  });
  
  server.on("/history", HTTP_OPTIONS, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(204);
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(response);
  });

  server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument doc; 
    doc["temperature"]=temperature; 
    doc["pressure"]=pressure; 
    doc["light"]= lightLevel;  // Send -1 when disconnected, not 0
    doc["soil"]= soilMoisture;  // Send actual value including -1 or 0
    doc["timestamp"]=millis();
    doc["minTemperature"]=minTemperature;
    doc["maxTemperature"]=maxTemperature;
    doc["totalReadings"]=totalReadingsCount;
    doc["wifiRSSI"]=WiFi.RSSI();  // WiFi signal strength
    String res; serializeJson(doc,res); 
    
    // Add CORS headers for remote access (GitHub Pages)
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", res);
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(response);
    logRequest(request,200);
  });
  server.on("/health", HTTP_GET, [](AsyncWebServerRequest *request){
    String body = "OK\n";
    body += "uptime_ms=" + String(millis()) + "\n";
    body += "free_heap=" + String(ESP.getFreeHeap()) + "\n";
    body += "bmp=" + String(temperature!=0.0 || pressure!=0.0 ? 1:0) + "\n";
    body += "light_sensor=" + String(lightLevel!=-1?1:0) + "\n";
    body += "soil_sensor=" + String(soilMoisture>=0?1:0) + "\n";
  logRequest(request,200);
  request->send(200,"text/plain",body);
  });
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument doc; doc["uptime_ms"] = millis(); doc["free_heap"] = ESP.getFreeHeap(); doc["light_sensor"] = (lightLevel!=-1); doc["soil_sensor"] = (soilMoisture>=0); doc["bmp_sensor"] = (temperature!=0.0 || pressure!=0.0); String res; serializeJson(doc,res); request->send(200,"application/json",res);
  logRequest(request,200);
  });
  
  // Sensor registry endpoint
  server.on("/sensors", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument doc;
    JsonArray sensorArray = doc["sensors"].to<JsonArray>();
    
    for (int i = 0; i < SENSOR_COUNT; i++) {
      JsonObject sensor = sensorArray.add<JsonObject>();
      sensor["name"] = sensors[i].name;
      sensor["unit"] = sensors[i].unit;
      sensor["enabled"] = sensors[i].enabled;
      sensor["available"] = sensors[i].available;
      sensor["value"] = sensors[i].lastValue;
      sensor["last_read"] = sensors[i].lastRead;
    }
    
    doc["device_id"] = deviceId;
    doc["cloud_sync_enabled"] = ENABLE_CLOUD_SYNC;
    
    String res;
    serializeJson(doc, res);
    logRequest(request,200);
    request->send(200, "application/json", res);
  });
  
  // Lightweight plain HTML page (no heavy CSS) for quick remote check
  server.on("/simple", HTTP_GET, [](AsyncWebServerRequest *request){
    String p = F("<!DOCTYPE html><html><head><meta charset='utf-8'><title>Greenhouse Simple</title><meta name='viewport' content='width=device-width,initial-scale=1'><style>body{font-family:Arial;margin:10px;}table{border-collapse:collapse;}td,th{border:1px solid #888;padding:6px;}code{background:#eee;padding:2px 4px;border-radius:4px;}</style></head><body><h2>Smart Greenhouse - Simple</h2><div id='ip'></div><table><thead><tr><th>Metric</th><th>Value</th></tr></thead><tbody><tr><td>Temperature (°C)</td><td id='t'>--</td></tr><tr><td>Pressure (hPa)</td><td id='p'>--</td></tr><tr><td>Light (lux)</td><td id='l'>--</td></tr><tr><td>Soil (%)</td><td id='s'>--</td></tr><tr><td>Uptime (s)</td><td id='u'>--</td></tr></tbody></table><p>API: <code>/api</code>, Health: <code>/health</code>, Metrics: <code>/metrics</code></p><script>function g(id){return document.getElementById(id);}function upd(){fetch('/api').then(r=>r.json()).then(d=>{g('t').textContent=d.temperature.toFixed(1);g('p').textContent=d.pressure.toFixed(2);g('l').textContent=d.light>0?d.light.toFixed(0):'N/A';g('s').textContent=d.soil>=0?d.soil.toFixed(0):'N/A';g('u').textContent=(d.timestamp/1000).toFixed(0);});}upd();setInterval(upd,2000);</script></body></html>");
  logRequest(request,200);
  request->send(200, "text/html", p);
  });
  // Prometheus-like metrics endpoint
  server.on("/metrics", HTTP_GET, [](AsyncWebServerRequest *request){
    String m;
    m  = F("# HELP greenhouse_temperature_c Current temperature in Celsius\n");
    m += F("# TYPE greenhouse_temperature_c gauge\n");
    m += String("greenhouse_temperature_c ")+String(temperature,2)+"\n";
    m += F("# HELP greenhouse_pressure_hpa Current pressure hPa\n# TYPE greenhouse_pressure_hpa gauge\n");
    m += String("greenhouse_pressure_hpa ")+String(pressure,2)+"\n";
    m += F("# HELP greenhouse_light_lux Light level in lux\n# TYPE greenhouse_light_lux gauge\n");
    m += String("greenhouse_light_lux ")+String(lightLevel!=-1?lightLevel:0,2)+"\n";
    m += F("# HELP greenhouse_soil_percent Soil moisture percent\n# TYPE greenhouse_soil_percent gauge\n");
    m += String("greenhouse_soil_percent ")+String(soilMoisture>=0?soilMoisture:0,2)+"\n";
    m += F("# HELP greenhouse_uptime_ms Uptime in milliseconds\n# TYPE greenhouse_uptime_ms counter\n");
    m += String("greenhouse_uptime_ms ")+String(millis())+"\n";
    m += F("# HELP greenhouse_free_heap_bytes Free heap bytes\n# TYPE greenhouse_free_heap_bytes gauge\n");
    m += String("greenhouse_free_heap_bytes ")+String(ESP.getFreeHeap())+"\n";
    logRequest(request,200);
    request->send(200, "text/plain; version=0.0.4", m);
  });
  
  // ==================== WATERING API ENDPOINTS ====================
  
  // Get watering status
  server.on("/water/status", HTTP_GET, [](AsyncWebServerRequest *request){
    StaticJsonDocument<256> doc;
    doc["isWatering"] = isWatering;
    doc["autoEnabled"] = autoWateringEnabled;
    doc["minThreshold"] = soilMinThreshold;
    doc["maxThreshold"] = soilMaxThreshold;
    doc["currentSoilMoisture"] = soilMoisture;
    doc["manualWateringActive"] = manualWateringActive;
    
    String response;
    serializeJson(doc, response);
    
    AsyncWebServerResponse *resp = request->beginResponse(200, "application/json", response);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    request->send(resp);
    logRequest(request, 200);
  });
  
  // Enable/Disable auto watering
  server.on("/water/auto", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, 
  [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, data, len);
    
    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    if (doc.containsKey("enabled")) {
      autoWateringEnabled = doc["enabled"];
      Serial.printf("Auto watering %s\n", autoWateringEnabled ? "ENABLED" : "DISABLED");
    }
    if (doc.containsKey("minThreshold")) {
      soilMinThreshold = doc["minThreshold"];
      Serial.printf("Min threshold set to %.1f%%\n", soilMinThreshold);
    }
    if (doc.containsKey("maxThreshold")) {
      soilMaxThreshold = doc["maxThreshold"];
      Serial.printf("Max threshold set to %.1f%%\n", soilMaxThreshold);
    }
    
    StaticJsonDocument<128> response;
    response["success"] = true;
    response["autoMode"] = autoWateringEnabled;
    response["minThreshold"] = soilMinThreshold;
    response["maxThreshold"] = soilMaxThreshold;
    
    String res;
    serializeJson(response, res);
    
    AsyncWebServerResponse *resp = request->beginResponse(200, "application/json", res);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    request->send(resp);
    logRequest(request, 200);
  });
  
  // OPTIONS for CORS preflight
  server.on("/water/auto", HTTP_OPTIONS, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(204);
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(response);
  });
  
  // Manual watering (15 seconds)
  server.on("/water/manual", HTTP_POST, [](AsyncWebServerRequest *request){
    if (!manualWateringActive && !isWatering) {
      manualWateringActive = true;
      startWatering();
      Serial.println("🚿 Manual watering started (15s timer)");
      
      StaticJsonDocument<128> response;
      response["success"] = true;
      response["message"] = "Manual watering started (15s)";
      
      String res;
      serializeJson(response, res);
      
      AsyncWebServerResponse *resp = request->beginResponse(200, "application/json", res);
      resp->addHeader("Access-Control-Allow-Origin", "*");
      request->send(resp);
      logRequest(request, 200);
    } else {
      request->send(400, "application/json", "{\"error\":\"Watering already active\"}");
      logRequest(request, 400);
    }
  });
  
  // OPTIONS for CORS preflight
  server.on("/water/manual", HTTP_OPTIONS, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(204);
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(response);
  });

  // Calibration helper endpoint
  server.on("/calibrate", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Soil Calibration</title>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1'><style>body{font-family:Arial;margin:20px;background:#f0f0f0;} .container{max-width:600px;margin:0 auto;background:white;padding:20px;border-radius:10px;} .raw{font-size:2em;text-align:center;margin:20px 0;padding:20px;background:#e3f2fd;border-radius:8px;} .step{background:#f5f5f5;padding:15px;margin:10px 0;border-radius:5px;} .code{background:#333;color:#0f0;padding:10px;border-radius:5px;font-family:monospace;}</style></head><body>";
    html += "<div class='container'><h1>🌱 Soil Sensor Calibration</h1>";
    html += "<div class='raw'>Current Raw: <span id='raw'>" + String(soilRaw) + "</span></div>";
    html += "<div class='step'><h3>Step 1: Dry Measurement</h3><p>Remove sensor from soil and measure in air.</p><div class='code'>Current: " + String(soilRaw) + "</div></div>";
    html += "<div class='step'><h3>Step 2: Wet Measurement</h3><p>Dip sensor in water and measure.</p></div>";
    html += "<div class='step'><h3>Step 3: Update Code</h3><div class='code'>#define SOIL_DRY_VALUE " + String(soilRaw) + "<br>#define SOIL_WET_VALUE [wet_value]</div></div>";
    html += "<script>setInterval(()=>fetch('/api').then(r=>r.json()).then(d=>document.getElementById('raw').textContent=d.soil_raw),2000);</script>";
    html += "</div></body></html>";
    logRequest(request,200);
    request->send(200, "text/html", html);
  });
  
  // History endpoint for charts
  server.on("/history", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument doc;
    JsonArray temps = doc["temperature"].to<JsonArray>();
    JsonArray pressures = doc["pressure"].to<JsonArray>();
    JsonArray lights = doc["light"].to<JsonArray>();
    JsonArray soils = doc["soil"].to<JsonArray>();
    JsonArray timestamps = doc["timestamps"].to<JsonArray>();
    
    // Add historical data in chronological order
    int start = (historyCount < MAX_HISTORY_POINTS) ? 0 : historyIndex;
    for (int i = 0; i < historyCount; i++) {
      int idx = (start + i) % MAX_HISTORY_POINTS;
      temps.add(sensorHistory[idx].temperature);
      pressures.add(sensorHistory[idx].pressure);
      lights.add((sensorHistory[idx].lightLevel != -1) ? sensorHistory[idx].lightLevel : 0);
      soils.add((sensorHistory[idx].soilMoisture >= 0) ? sensorHistory[idx].soilMoisture : 0);
      timestamps.add(sensorHistory[idx].timestamp);
    }
    
    String response;
    serializeJson(doc, response);
    
    // Add CORS headers for remote access (GitHub Pages)
    AsyncWebServerResponse *resp = request->beginResponse(200, "application/json", response);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(resp);
    logRequest(request,200);
  });
  
  server.onNotFound([](AsyncWebServerRequest *request){ logRequest(request,404); request->send(404,"text/plain","File not found"); });
}

// ==================== WATERING SYSTEM FUNCTIONS ====================

// Start/Stop water pump
void startWatering() {
  if (!isWatering) {
    digitalWrite(RELAY_PIN, HIGH);       // Turn ON relay (pump ON) - active HIGH
    isWatering = true;
    wateringStartTime = millis();
    Serial.println("💧 WATERING STARTED - Relay ON (active HIGH)");
  }
}

void stopWatering() {
  if (isWatering) {
    digitalWrite(RELAY_PIN, LOW);        // Turn OFF relay (pump OFF) - active HIGH
    isWatering = false;
    manualWateringActive = false;
    Serial.println("🛑 WATERING STOPPED - Relay OFF (active HIGH)");
  }
}

// Automatic watering logic
void handleAutoWatering() {
  // Handle manual watering timer (15 seconds)
  if (manualWateringActive) {
    if (millis() - wateringStartTime >= manualWateringDuration) {
      Serial.println("⏱️ Manual watering timer expired (15s)");
      stopWatering();
    }
    return; // Skip auto logic during manual watering
  }
  
  // Auto watering logic
  if (autoWateringEnabled && soilMoisture >= 0) {
    if (!isWatering && soilMoisture < soilMinThreshold) {
      Serial.printf("🌱 Soil too dry (%.1f%% < %.1f%%), starting auto watering\n", 
                    soilMoisture, soilMinThreshold);
      startWatering();
    }
    else if (isWatering && soilMoisture >= soilMaxThreshold) {
      Serial.printf("✅ Soil optimal (%.1f%% >= %.1f%%), stopping auto watering\n", 
                    soilMoisture, soilMaxThreshold);
      stopWatering();
    }
  }
}

void loop() {
  // Check BMP280 sensor connection
  float newTemp = bmp.readTemperature();
  float newPressure = bmp.readPressure() / 100.0F;
  
  // Validate BMP280 readings (NaN means disconnected)
  if (isnan(newTemp) || isnan(newPressure) || newTemp < -50 || newTemp > 100) {
    temperature = -999; // Error indicator
    pressure = -999;    // Error indicator
  } else {
    temperature = newTemp;
    pressure = newPressure;
  }
  
  // Check BH1750 sensor connection
  if (lightLevel != -1) {
    float newLight = lightMeter.readLightLevel();
    if (isnan(newLight) || newLight < 0) {
      lightLevel = -1; // Mark as disconnected
    } else {
      lightLevel = newLight;
    }
  }
  
  // Check soil sensor connection
  soilMoisture = readSoilMoisturePercent();
  
  // Add to history every 5 minutes
  addToHistory();
  
  // Update sensor registry
  updateSensorRegistry();
  
  // Calibration and alerts
  calibrateSoilSensor();
  checkAlerts();
  
  // Automatic watering control
  handleAutoWatering();
  
  // Cloud sync (if enabled)
  if (ENABLE_CLOUD_SYNC && millis() - lastCloudSync > CLOUD_SYNC_INTERVAL) {
    sendToCloud();
    lastCloudSync = millis();
  }
  
  Serial.print("Temperature: "); Serial.print(temperature); Serial.print(" °C, Pressure: "); Serial.print(pressure); Serial.print(" hPa");
  if (lightLevel != -1) { Serial.print(", Light: "); Serial.print(lightLevel); Serial.print(" lux"); } else { Serial.print(", Light: N/A"); }
  
  // Direct ADC read for troubleshooting
  int directADC = analogRead(SOIL_PIN);
  
  if (soilMoisture >= 0) {
    Serial.print(", Soil: "); Serial.print(soilMoisture); Serial.print(" %");
#if ENABLE_SOIL_DEBUG
    Serial.print(" (filtered="); Serial.print(soilRaw); Serial.print(", direct="); Serial.print(directADC); Serial.print(")");
#endif
    Serial.println();
  } else {
    Serial.println(", Soil: N/A");
  }
  delay(500);
}

void addToHistory() {
  unsigned long currentTime = millis();
  if (currentTime - lastHistoryUpdate >= HISTORY_INTERVAL) {
    lastHistoryUpdate = currentTime;
    
    // Get current Unix timestamp (seconds since epoch)
    time_t now;
    time(&now);
    unsigned long unixTimestamp = (unsigned long)now;
    
    // Add current reading to circular buffer with UNIX TIMESTAMP
    sensorHistory[historyIndex].temperature = temperature;
    sensorHistory[historyIndex].pressure = pressure;
    sensorHistory[historyIndex].lightLevel = lightLevel;
    sensorHistory[historyIndex].soilMoisture = soilMoisture;
    sensorHistory[historyIndex].timestamp = unixTimestamp;  // UNIX timestamp, not millis!
    
    historyIndex = (historyIndex + 1) % MAX_HISTORY_POINTS;
    if (historyCount < MAX_HISTORY_POINTS) {
      historyCount++;
    }
    
    // Increment total readings counter
    totalReadingsCount++;
    
    // Calculate min/max temperature from last 24h of history
    minTemperature = 999.0;
    maxTemperature = -999.0;
    for (int i = 0; i < historyCount; i++) {
      float t = sensorHistory[i].temperature;
      if (t > -50 && t < 100) {  // Valid temperature range
        if (t < minTemperature) minTemperature = t;
        if (t > maxTemperature) maxTemperature = t;
      }
    }
    
    // Format timestamp for display
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    
    Serial.print("📊 History added: "); 
    Serial.print(historyCount); 
    Serial.print("/"); 
    Serial.print(MAX_HISTORY_POINTS);
    Serial.print(" @ ");
    Serial.print(timeStr);
    Serial.print(" | Temp: ");
    Serial.print(temperature);
    Serial.print("°C (Min: ");
    Serial.print(minTemperature);
    Serial.print(", Max: ");
    Serial.print(maxTemperature);
    Serial.println(")");
  }
}

// Update sensor registry with current values
void updateSensorRegistry() {
  unsigned long now = millis();
  
  // Temperature sensor (BMP280)
  if (temperature > -50 && temperature < 100) {
    sensors[SENSOR_BMP280_TEMP].available = true;
    sensors[SENSOR_BMP280_TEMP].lastValue = temperature;
    sensors[SENSOR_BMP280_TEMP].lastRead = now;
  } else {
    sensors[SENSOR_BMP280_TEMP].available = false;
  }
  
  // Pressure sensor (BMP280)
  if (pressure > 300 && pressure < 1100) {
    sensors[SENSOR_BMP280_PRESSURE].available = true;
    sensors[SENSOR_BMP280_PRESSURE].lastValue = pressure;
    sensors[SENSOR_BMP280_PRESSURE].lastRead = now;
  } else {
    sensors[SENSOR_BMP280_PRESSURE].available = false;
  }
  
  // Light sensor (BH1750)
  if (lightLevel >= 0) {
    sensors[SENSOR_BH1750_LIGHT].available = true;
    sensors[SENSOR_BH1750_LIGHT].lastValue = lightLevel;
    sensors[SENSOR_BH1750_LIGHT].lastRead = now;
  } else {
    sensors[SENSOR_BH1750_LIGHT].available = false;
  }
  
  // Soil moisture sensor
  if (soilMoisture >= 0) {
    sensors[SENSOR_SOIL_MOISTURE].available = true;
    sensors[SENSOR_SOIL_MOISTURE].lastValue = soilMoisture;
    sensors[SENSOR_SOIL_MOISTURE].lastRead = now;
  } else {
    sensors[SENSOR_SOIL_MOISTURE].available = false;
  }
}

// Send telemetry data to cloud backend (DISABLED - Local IP Only)
void sendToCloud() {
  // FUNCTION DISABLED - No Firebase/Cloud uploads
  // This system works in LOCAL IP MODE ONLY
  // Access at: http://192.168.2.20 or via Port Forwarding
  Serial.println("ℹ️ Cloud sync disabled - running in Local IP only mode");
  return;
}
