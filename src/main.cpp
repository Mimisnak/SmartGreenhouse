/*
 * Smart Greenhouse - Temperature, Pressure & Light Monitoring
 */
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_BMP280.h>
#include <ArduinoJson.h>
#include <BH1750.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <FirebaseESP32.h>

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

// --- Cloud Configuration (Firebase) ---
#define ENABLE_CLOUD_SYNC true  // Cloud sync enabled for remote access

// Firebase Configuration
#define FIREBASE_HOST "smartgreenhouse-fb494-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "AIzaSyDwwszw4AapfTp_dkdli48vsxOZXkZwqfo"  // Use Web API Key for simple auth
#define DEVICE_ID "ESP32-Greenhouse"

FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

const char* cloudApiUrl = "";  // Not used - using Firebase directly
const char* deviceId = DEVICE_ID;
unsigned long lastCloudSync = 0;
#define CLOUD_SYNC_INTERVAL 30000  // Send data every 30 seconds

// --- Soil Moisture Configuration ---
// Adjust SOIL_PIN to your actual analog pin. Choose an ADC1 capable pin.
// Calibrate SOIL_DRY_VALUE (reading in completely dry air) and SOIL_WET_VALUE (reading fully wet)
// then percentage = map(raw, SOIL_DRY_VALUE, SOIL_WET_VALUE, 0, 100) (clamped)
#define SOIL_PIN 4  // GPIO 4 - excellent ADC1 pin for soil sensor
#define SOIL_DRY_VALUE 3285  // Capacitive sensor reading in air (dry = 0%)
#define SOIL_WET_VALUE 27    // Capacitive sensor in wet soil (wet = 100%)

const char* ssid = "Vodafone-E79546683";
const char* password = "RLg2s6b73EfarXRx";

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
#define MAX_HISTORY_POINTS 576
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

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Configure soil sensor pin with pull-down to prevent floating
  pinMode(SOIL_PIN, INPUT_PULLDOWN);
  
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
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println(); Serial.println("WiFi connected!"); Serial.print("IP address: "); Serial.println(WiFi.localIP());
  
  // Initialize Firebase
  Serial.println("Initializing Firebase...");
  firebaseConfig.host = FIREBASE_HOST;
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase initialized!");
  
  setupWebServer();
  server.begin();
  Serial.println("HTTP server started");
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
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  float t0 = temperature;
  float p0 = pressure;
  float l0 = lightLevel;
  float s0 = soilMoisture;
  bool lightAvail = (l0 != -1 && l0 > 0);
  bool soilAvail  = (s0 >= 0);
  AsyncResponseStream *res = request->beginResponseStream("text/html");
  auto esc = [](String v){return v;}; // placeholder if we later need escaping
  res->print(F("<!DOCTYPE html><html lang='el'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'>"));
  res->print(F("<title>Smart Greenhouse</title><style>*{margin:0;padding:0;box-sizing:border-box;}body{font-family:'Segoe UI',Tahoma,Geneva,Verdana,sans-serif;background:linear-gradient(135deg,#8BC34A 0%,#689F38 50%,#558B2F 100%);min-height:100vh;padding:10px;color:#333;overflow-x:hidden;}@keyframes backgroundShift{0%,100%{background:linear-gradient(135deg,#8BC34A 0%,#689F38 50%,#558B2F 100%);}50%{background:linear-gradient(135deg,#4CAF50 0%,#388E3C 50%,#2E7D32 100%);}} .container{max-width:1200px;margin:0 auto;background:rgba(255,255,255,0.95);border-radius:15px;box-shadow:0 8px 25px rgba(46,125,50,0.2);padding:20px;} .header{text-align:center;margin-bottom:25px;position:relative;background:linear-gradient(135deg,#2E7D32,#388E3C,#4CAF50);color:white;padding:20px;border-radius:12px;} .main-title{font-size:2em;margin-bottom:8px;font-weight:700;} .subtitle{font-size:0.95em;margin-bottom:15px;opacity:0.9;} .language-switcher{position:absolute;top:10px;right:10px;display:flex;gap:5px;} .language-btn{padding:5px 10px;border:2px solid rgba(255,255,255,0.3);background:transparent;color:white;border-radius:12px;cursor:pointer;transition:.3s;font-size:11px;font-weight:600;min-height:32px;min-width:36px;} .language-btn.active,.language-btn:hover{background:rgba(255,255,255,0.2);transform:translateY(-1px);} .status-cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:15px;margin-bottom:20px;} .card{background:rgba(255,255,255,0.95);border-radius:12px;padding:18px;text-align:center;position:relative;overflow:hidden;transition:.3s;border-left:4px solid #4CAF50;} .card:hover{transform:translateY(-2px);box-shadow:0 6px 18px rgba(76,175,80,0.2);} .card-icon{font-size:2em;margin-bottom:8px;display:block;color:#4CAF50;} .card-title{font-size:0.95em;color:#2E7D32;margin-bottom:5px;font-weight:600;} .card-value{font-size:1.8em;font-weight:700;color:#2E7D32;margin-bottom:2px;} .card-unit{color:#558B2F;font-size:0.85em;margin-bottom:4px;} .card-status{font-size:.75em;font-weight:600;padding:4px 8px;border-radius:10px;display:inline-block;} .ok{background:linear-gradient(135deg,#4CAF50,#66BB6A);color:white;} .na{background:linear-gradient(135deg,#FF9800,#FFB74D);color:white;} .charts-section{margin-top:15px;background:rgba(255,255,255,0.95);border-radius:12px;padding:15px;border-left:4px solid #4CAF50;} .charts-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));gap:15px;} .chart-container{background:white;border-radius:10px;padding:12px;box-shadow:0 3px 12px rgba(76,175,80,0.1);position:relative;border-left:3px solid #4CAF50;} .chart-title{text-align:center;margin-bottom:8px;font-size:0.9em;font-weight:600;color:#2E7D32;} .chart-wrapper{position:relative;height:220px;margin-bottom:6px;} .footer{margin-top:20px;text-align:center;color:white;font-size:0.85em;background:linear-gradient(135deg,#2E7D32,#388E3C);padding:12px;border-radius:10px;} .footer a{color:#C8E6C9;text-decoration:none;font-weight:600;transition:.3s;} .footer a:hover{text-decoration:underline;color:white;} .update-time{color:#A5D6A7;font-size:.75em;margin-top:6px;font-style:italic;} @media (max-width:768px){body{padding:5px;}.container{padding:12px;margin:5px;border-radius:10px;}.header{padding:15px;}.main-title{font-size:1.6em;}.subtitle{font-size:0.9em;}.language-switcher{position:static;justify-content:center;margin-top:10px;}.status-cards{grid-template-columns:1fr;gap:12px;}.card{padding:15px;}.card-icon{font-size:1.8em;}.card-value{font-size:1.6em;}.charts-grid{grid-template-columns:1fr;gap:12px;}.chart-wrapper{height:200px;}.charts-section{padding:12px;}} @media (max-width:480px){.container{padding:8px;}.header{padding:12px;}.main-title{font-size:1.4em;}.card{padding:12px;}.card-icon{font-size:1.6em;}.card-value{font-size:1.4em;}.chart-wrapper{height:180px;}.footer{padding:10px;font-size:0.8em;}}</style></head><body>"));
  res->print(F("<div class='container'><div class='header'><div class='language-switcher'><button class='language-btn active' onclick='setLanguage(\"el\")' id='btn-el'>EL</button><button class='language-btn' onclick='setLanguage(\"en\")' id='btn-en'>EN</button></div><h1 class='main-title' data-el='Smart Greenhouse' data-en='Smart Greenhouse'>Smart Greenhouse</h1><p class='subtitle' data-el='Σύστημα Παρακολούθησης Φυτών' data-en='Plant Monitoring System'>Σύστημα Παρακολούθησης Φυτών</p></div>"));
  res->print(F("<div class='status-cards'>"));
  // Temperature card
  res->print(F("<div class='card temperature-card'><span class='card-icon'>🌡️</span><div class='card-title' data-el='Θερμοκρασία' data-en='Temperature'>Θερμοκρασία</div><div class='card-value' id='temperature'>"));
  if(isnan(t0)) res->print(F("--")); else res->print(String(t0,1));
  res->print(F("</div><div class='card-unit'>°C</div><div class='card-status ok' id='tempStatus' data-el='ΕΝΕΡΓΟΣ' data-en='ACTIVE'>ΕΝΕΡΓΟΣ</div></div>"));
  // Pressure card
  res->print(F("<div class='card pressure-card'><span class='card-icon'>📊</span><div class='card-title' data-el='Ατμοσφαιρική Πίεση' data-en='Atmospheric Pressure'>Ατμοσφαιρική Πίεση</div><div class='card-value' id='pressure'>"));
  if(isnan(p0)) res->print(F("--")); else res->print(String(p0,1));
  res->print(F("</div><div class='card-unit'>hPa</div><div class='card-status ok' id='pressStatus' data-el='ΕΝΕΡΓΟΣ' data-en='ACTIVE'>ΕΝΕΡΓΟΣ</div></div>"));
  // Light card
  res->print(F("<div class='card light-card'><span class='card-icon'>☀️</span><div class='card-title' data-el='Φωτισμός' data-en='Light Level'>Φωτισμός</div><div class='card-value' id='light'>"));
  if(lightAvail) res->print(String(l0,0)); else res->print(F("--"));
  res->print(F("</div><div class='card-unit'>lux</div><div class='card-status "));
  res->print(lightAvail?F("ok' id='lightStatus' data-el='ΕΝΕΡΓΟΣ' data-en='ACTIVE'>ΕΝΕΡΓΟΣ</div></div>"):F("na' id='lightStatus' data-el='ΜΗ ΔΙΑΘΕΣΙΜΟΣ' data-en='UNAVAILABLE'>ΜΗ ΔΙΑΘΕΣΙΜΟΣ</div></div>"));
  // Soil card
  res->print(F("<div class='card soil-card'><span class='card-icon'>🌱</span><div class='card-title' data-el='Υγρασία Εδάφους' data-en='Soil Moisture'>Υγρασία Εδάφους</div><div class='card-value' id='soil'>"));
  if(soilAvail) res->print(String(s0,0)); else res->print(F("--"));
  res->print(F("</div><div class='card-unit'>%</div><div class='card-status "));
  res->print(soilAvail?F("ok' id='soilStatus' data-el='ΕΝΕΡΓΟΣ' data-en='ACTIVE'>ΕΝΕΡΓΟΣ</div></div>"):F("na' id='soilStatus' data-el='ΜΗ ΔΙΑΘΕΣΙΜΟΣ' data-en='UNAVAILABLE'>ΜΗ ΔΙΑΘΕΣΙΜΟΣ</div></div>"));
  res->print(F("</div>")); // status-cards
  // Charts
  res->print(F("<div class='charts-section'><div class='charts-grid'>"));
  res->print(F("<div class='chart-container'><h3 class='chart-title' data-el='Ιστορικό Θερμοκρασίας (48 ώρες)' data-en='Temperature History (48h)'>Ιστορικό Θερμοκρασίας (48 ώρες)</h3><div class='chart-wrapper'><canvas id='tempChart'></canvas></div></div>"));
  res->print(F("<div class='chart-container'><h3 class='chart-title' data-el='Ιστορικό Πίεσης (48 ώρες)' data-en='Pressure History (48h)'>Ιστορικό Πίεσης (48 ώρες)</h3><div class='chart-wrapper'><canvas id='pressChart'></canvas></div></div>"));
  res->print(F("<div class='chart-container'><h3 class='chart-title' data-el='Ιστορικό Φωτισμού (48 ώρες)' data-en='Light History (48h)'>Ιστορικό Φωτισμού (48 ώρες)</h3><div class='chart-wrapper'><canvas id='lightChart'></canvas></div></div>"));
  res->print(F("<div class='chart-container'><h3 class='chart-title' data-el='Ιστορικό Υγρασίας Εδάφους (48 ώρες)' data-en='Soil Moisture History (48h)'>Ιστορικό Υγρασίας Εδάφους (48 ώρες)</h3><div class='chart-wrapper'><canvas id='soilChart'></canvas></div></div>"));
  res->print(F("</div></div>"));
  res->print(F("<div class='footer'><p>&copy; 2025 <a href='https://github.com/Mimisnak/SmartGreenhouse' target='_blank'>Smart Greenhouse System</a> | <span data-el='Developer by:' data-en='Developer by:'>Developer by:</span> <a href='https://mimis.dev' target='_blank'>mimis.dev</a></p><div class='update-time' id='lastUpdate' data-el='Τελευταία ενημέρωση: αρχική' data-en='Last update: initial'>Τελευταία ενημέρωση: αρχική</div></div></div>"));
  // JS with improved sensor disconnection detection and stable colors
  res->print(F("<script src='https://cdn.jsdelivr.net/npm/chart.js'></script><script>var tempChart=null,pressChart=null,lightChart=null,soilChart=null;var dataHistory={temperature:[],pressure:[],light:[],soil:[],timestamps:[],realTimestamps:[]};var currentLanguage='el',lastUpdateTime=null;const WINDOW_MS=48*60*60*1000;function trimOld(){var now=Date.now();while(dataHistory.realTimestamps.length && now-dataHistory.realTimestamps[0].getTime()>WINDOW_MS){dataHistory.realTimestamps.shift();dataHistory.timestamps.shift();dataHistory.temperature.shift();dataHistory.pressure.shift();dataHistory.light.shift();dataHistory.soil.shift();}}function loadServerHistory(){return fetch('/history').then(r=>r.json()).then(d=>{if(d.temperature && d.temperature.length>0){for(var i=0;i<d.temperature.length;i++){var t=new Date(d.timestamps[i]);dataHistory.realTimestamps.push(t);dataHistory.timestamps.push(t.toLocaleTimeString());dataHistory.temperature.push(d.temperature[i]);dataHistory.pressure.push(d.pressure[i]);dataHistory.light.push(d.light[i]);dataHistory.soil.push(d.soil[i]);lastUpdateTime=t;}}}).catch(e=>{console.log('No server history available')});}function loadStoredData(){var s=localStorage.getItem('greenhouse-data-v2');if(!s)return;try{var arr=JSON.parse(s);var now=Date.now();arr.forEach(it=>{var t=new Date(it.timestamp);if(now-t.getTime()<=WINDOW_MS){var existing=dataHistory.realTimestamps.find(rt=>Math.abs(rt.getTime()-t.getTime())<60000);if(!existing){dataHistory.realTimestamps.push(t);dataHistory.timestamps.push(t.toLocaleTimeString());dataHistory.temperature.push(it.temperature);dataHistory.pressure.push(it.pressure);dataHistory.light.push(it.light);dataHistory.soil.push(it.soil);lastUpdateTime=t;}}});}catch(e){}}function saveData(){try{var out=[];for(var i=0;i<dataHistory.realTimestamps.length;i++){out.push({timestamp:dataHistory.realTimestamps[i].toISOString(),temperature:dataHistory.temperature[i],pressure:dataHistory.pressure[i],light:dataHistory.light[i],soil:dataHistory.soil[i]});}localStorage.setItem('greenhouse-data-v2',JSON.stringify(out));}catch(e){}}function makeChart(ctx,label,color,data){return new Chart(ctx,{type:'line',data:{labels:dataHistory.timestamps,datasets:[{label:label,data:data,borderColor:color,backgroundColor:color.replace('rgb','rgba').replace(')',',0.15)'),tension:.35,fill:true,spanGaps:true}]},options:{responsive:true,maintainAspectRatio:false,plugins:{legend:{display:false}},animation:{duration:0},scales:{x:{ticks:{maxRotation:0,minRotation:0}}}}});}function initCharts(){tempChart=makeChart(document.getElementById('tempChart').getContext('2d'),'Temperature (°C)','rgb(76,175,80)',dataHistory.temperature);pressChart=makeChart(document.getElementById('pressChart').getContext('2d'),'Pressure (hPa)','rgb(46,125,50)',dataHistory.pressure);lightChart=makeChart(document.getElementById('lightChart').getContext('2d'),'Light (lux)','rgb(139,195,74)',dataHistory.light);soilChart=makeChart(document.getElementById('soilChart').getContext('2d'),'Soil (%)','rgb(102,187,106)',dataHistory.soil);}function refreshCharts(){[tempChart,pressChart,lightChart,soilChart].forEach((c,i)=>{if(!c)return;c.data.labels=dataHistory.timestamps.slice();var arr=i===0?dataHistory.temperature:i===1?dataHistory.pressure:i===2?dataHistory.light:dataHistory.soil;c.data.datasets[0].data=arr.slice();c.update('none');});}function updateData(){fetch('/api').then(r=>r.json()).then(d=>{if(d.temperature>-900 && d.temperature<100){document.getElementById('temperature').textContent=d.temperature.toFixed(1);document.getElementById('tempStatus').className='card-status ok';document.getElementById('tempStatus').textContent=currentLanguage==='el'?'ΕΝΕΡΓΟΣ':'ACTIVE';}else{document.getElementById('temperature').textContent='--';document.getElementById('tempStatus').className='card-status na';document.getElementById('tempStatus').textContent=currentLanguage==='el'?'ΜΗ ΔΙΑΘΕΣΙΜΟΣ':'UNAVAILABLE';}if(d.pressure>-900 && d.pressure<2000){document.getElementById('pressure').textContent=d.pressure.toFixed(1);document.getElementById('pressStatus').className='card-status ok';document.getElementById('pressStatus').textContent=currentLanguage==='el'?'ΕΝΕΡΓΟΣ':'ACTIVE';}else{document.getElementById('pressure').textContent='--';document.getElementById('pressStatus').className='card-status na';document.getElementById('pressStatus').textContent=currentLanguage==='el'?'ΜΗ ΔΙΑΘΕΣΙΜΟΣ':'UNAVAILABLE';}if(d.light>=0){document.getElementById('light').textContent=d.light.toFixed(0);document.getElementById('lightStatus').className='card-status ok';document.getElementById('lightStatus').textContent=currentLanguage==='el'?'ΕΝΕΡΓΟΣ':'ACTIVE';}else{document.getElementById('light').textContent='--';document.getElementById('lightStatus').className='card-status na';document.getElementById('lightStatus').textContent=currentLanguage==='el'?'ΜΗ ΔΙΑΘΕΣΙΜΟΣ':'UNAVAILABLE';}if(d.soil>=0){document.getElementById('soil').textContent=d.soil.toFixed(0);document.getElementById('soilStatus').className='card-status ok';document.getElementById('soilStatus').textContent=currentLanguage==='el'?'ΕΝΕΡΓΟΣ':'ACTIVE';}else{document.getElementById('soil').textContent='--';document.getElementById('soilStatus').className='card-status na';document.getElementById('soilStatus').textContent=currentLanguage==='el'?'ΜΗ ΔΙΑΘΕΣΙΜΟΣ':'UNAVAILABLE';}var now=new Date();var existing=dataHistory.realTimestamps.find(rt=>Math.abs(rt.getTime()-now.getTime())<60000);if(!existing){dataHistory.temperature.push(d.temperature>-900?d.temperature:null);dataHistory.pressure.push(d.pressure>-900?d.pressure:null);dataHistory.light.push(d.light>=0?d.light:null);dataHistory.soil.push(d.soil>=0?d.soil:null);dataHistory.realTimestamps.push(now);dataHistory.timestamps.push(now.toLocaleTimeString());trimOld();refreshCharts();saveData();}lastUpdateTime=now;document.getElementById('lastUpdate').textContent=(currentLanguage==='el'?'Τελευταία ενημέρωση: ':'Last update: ')+now.toLocaleString(currentLanguage==='el'?'el-GR':'en-US');}).catch(e=>{});}function setLanguage(lang){currentLanguage=lang;document.querySelectorAll('.language-btn').forEach(b=>b.classList.remove('active'));document.getElementById('btn-'+lang).classList.add('active');document.querySelectorAll('[data-'+lang+']').forEach(el=>{el.textContent=el.getAttribute('data-'+lang)});if(lastUpdateTime){document.getElementById('lastUpdate').textContent=(currentLanguage==='el'?'Τελευταία ενημέρωση: ':'Last update: ')+lastUpdateTime.toLocaleString(currentLanguage==='el'?'el-GR':'en-US')}localStorage.setItem('preferred-language',lang);}window.onload=function(){document.body.style.animation='none';document.body.style.background='linear-gradient(135deg,#8BC34A 0%,#689F38 50%,#558B2F 100%)';var savedLang=localStorage.getItem('preferred-language')||'el';setLanguage(savedLang);loadServerHistory().then(()=>{loadStoredData();dataHistory.realTimestamps.sort((a,b)=>a.getTime()-b.getTime());dataHistory.timestamps=dataHistory.realTimestamps.map(t=>t.toLocaleTimeString());initCharts();refreshCharts();updateData();setInterval(updateData,5000);});}</script></body></html>"));
  logRequest(request,200);
  request->send(res);
  });

  server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument doc; doc["temperature"]=temperature; doc["pressure"]=pressure; doc["light"]= (lightLevel!=-1)?lightLevel:0; doc["soil"]= (soilMoisture>=0)?soilMoisture:-1; doc["timestamp"]=millis(); String res; serializeJson(doc,res); request->send(200,"application/json",res);
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
    logRequest(request,200);
    request->send(200, "application/json", response);
  });
  
  server.onNotFound([](AsyncWebServerRequest *request){ logRequest(request,404); request->send(404,"text/plain","File not found"); });
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
    
    // Add current reading to circular buffer
    sensorHistory[historyIndex].temperature = temperature;
    sensorHistory[historyIndex].pressure = pressure;
    sensorHistory[historyIndex].lightLevel = lightLevel;
    sensorHistory[historyIndex].soilMoisture = soilMoisture;
    sensorHistory[historyIndex].timestamp = currentTime;
    
    historyIndex = (historyIndex + 1) % MAX_HISTORY_POINTS;
    if (historyCount < MAX_HISTORY_POINTS) {
      historyCount++;
    }
    
    Serial.print("History added: "); 
    Serial.print(historyCount); 
    Serial.print("/"); 
    Serial.println(MAX_HISTORY_POINTS);
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

// Send telemetry data to cloud backend
void sendToCloud() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cloud sync skipped: WiFi not connected");
    return;
  }
  
  // Send to Firebase Realtime Database
  String path = String("sensors/") + DEVICE_ID + "/latest";
  
  FirebaseJson json;
  json.add("timestamp", millis());
  json.add("temperature", sensors[SENSOR_BMP280_TEMP].available ? sensors[SENSOR_BMP280_TEMP].lastValue : -999);
  json.add("pressure", sensors[SENSOR_BMP280_PRESSURE].available ? sensors[SENSOR_BMP280_PRESSURE].lastValue : -999);
  json.add("light", sensors[SENSOR_BH1750_LIGHT].available ? sensors[SENSOR_BH1750_LIGHT].lastValue : -999);
  json.add("soilMoisture", sensors[SENSOR_SOIL_MOISTURE].available ? sensors[SENSOR_SOIL_MOISTURE].lastValue : -999);
  
  if (Firebase.setJSON(firebaseData, path.c_str(), json)) {
    Serial.println("✅ Data sent to Firebase successfully");
  } else {
    Serial.print("❌ Firebase error: ");
    Serial.println(firebaseData.errorReason());
  }
  
  // Also store in history (optional - for charts)
  String historyPath = String("sensors/") + DEVICE_ID + "/history/" + String(millis());
  Firebase.setJSON(firebaseData, historyPath.c_str(), json);
}
