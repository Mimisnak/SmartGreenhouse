/*
 * Soil Moisture Sensor - Standalone
 * Analog sensor on GPIO 1
 */
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

const char* ssid = "Vodafone-E79546683";
const char* password = "RLg2s6b73EfarXRx";

#define SOIL_PIN 1
#define SOIL_DRY_VALUE 3500   // Calibrate: reading when completely dry
#define SOIL_WET_VALUE 1200   // Calibrate: reading when fully wet

AsyncWebServer server(80);

float soilMoisture = 0.0;
int soilRaw = 0;

float readSoilMoisturePercent() {
  soilRaw = analogRead(SOIL_PIN);
  if (soilRaw <= 0) return -1;
  
  int pct = map(soilRaw, SOIL_DRY_VALUE, SOIL_WET_VALUE, 0, 100);
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  
  return (float)pct;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Soil Moisture Sensor Test");
  Serial.printf("Using GPIO %d for analog reading\n", SOIL_PIN);
  Serial.printf("Dry calibration: %d, Wet calibration: %d\n", SOIL_DRY_VALUE, SOIL_WET_VALUE);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Setup web server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Soil Moisture Sensor</title>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
    html += "<style>body{font-family:Arial;margin:20px;background:#f0f0f0;} .container{max-width:600px;margin:0 auto;background:white;padding:20px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);} .sensor{background:linear-gradient(135deg,#27ae60,#2ecc71);color:white;padding:20px;border-radius:8px;margin:10px 0;text-align:center;} .value{font-size:2em;font-weight:bold;margin:10px 0;} .unit{font-size:1.2em;opacity:0.9;} .raw{background:#ecf0f1;color:#2c3e50;padding:10px;border-radius:5px;margin:10px 0;} .update{color:#666;font-size:0.9em;margin-top:20px;} .status{padding:10px;border-radius:5px;margin:10px 0;text-align:center;font-weight:bold;} .wet{background:#e8f5e8;color:#2e7d32;} .normal{background:#fff3e0;color:#ef6c00;} .dry{background:#ffebee;color:#c62828;} .calibration{background:#e3f2fd;color:#1976d2;padding:15px;border-radius:5px;margin:20px 0;} .calibration h3{margin-top:0;}</style></head><body>";
    html += "<div class='container'><h1>🌱 Soil Moisture Sensor</h1>";
    html += "<div class='sensor'><h2>Soil Moisture</h2><div class='value' id='moisture'>--</div><div class='unit'>%</div></div>";
    html += "<div class='raw'><strong>Raw ADC Value:</strong> <span id='raw'>--</span></div>";
    html += "<div class='status' id='status'>--</div>";
    html += "<div class='calibration'><h3>📏 Calibration Guide</h3><p><strong>For accurate readings:</strong></p><ul><li>🌵 <strong>Dry:</strong> Remove sensor from soil, measure raw value</li><li>💧 <strong>Wet:</strong> Dip sensor in water, measure raw value</li><li>⚙️ Update SOIL_DRY_VALUE and SOIL_WET_VALUE in code</li></ul><p><em>Current: Dry=" + String(SOIL_DRY_VALUE) + ", Wet=" + String(SOIL_WET_VALUE) + "</em></p></div>";
    html += "<div class='update' id='lastUpdate'>Last update: --</div>";
    html += "<script>function updateData(){fetch('/api').then(r=>r.json()).then(d=>{document.getElementById('moisture').textContent=d.soil>=0?d.soil.toFixed(0):'--';document.getElementById('raw').textContent=d.soil_raw;var status=document.getElementById('status');if(d.soil>=70){status.className='status wet';status.textContent='💧 Very Wet (70-100%)';}else if(d.soil>=40){status.className='status normal';status.textContent='🌿 Optimal (40-70%)';}else if(d.soil>=20){status.className='status normal';status.textContent='🌱 Needs Water (20-40%)';}else if(d.soil>=0){status.className='status dry';status.textContent='🌵 Very Dry (<20%)';}else{status.className='status dry';status.textContent='❌ Sensor Error';}document.getElementById('lastUpdate').textContent='Last update: '+new Date().toLocaleString();}).catch(e=>console.error(e));}updateData();setInterval(updateData,2000);</script>";
    html += "</div></body></html>";
    request->send(200, "text/html", html);
  });
  
  server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument doc;
    doc["soil"] = soilMoisture;
    doc["soil_raw"] = soilRaw;
    doc["timestamp"] = millis();
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  soilMoisture = readSoilMoisturePercent();
  
  Serial.print("Soil Moisture: ");
  if (soilMoisture >= 0) {
    Serial.print(soilMoisture);
    Serial.print("% (raw=");
    Serial.print(soilRaw);
    Serial.println(")");
  } else {
    Serial.println("N/A");
  }
  
  delay(1000);
}
