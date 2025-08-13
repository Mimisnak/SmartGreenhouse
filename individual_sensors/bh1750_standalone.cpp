/*
 * BH1750 Light Sensor - Standalone
 * GPIO: SDA=16, SCL=17
 */
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <BH1750.h>
#include <ArduinoJson.h>
#include <Wire.h>

const char* ssid = "Vodafone-E79546683";
const char* password = "RLg2s6b73EfarXRx";

#define SDA_PIN 16
#define SCL_PIN 17

BH1750 lightMeter;
AsyncWebServer server(80);

float lightLevel = 0.0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("I2C initialized with SDA=16, SCL=17");
  
  // Initialize BH1750
  if (!lightMeter.begin()) {
    Serial.println("BH1750 sensor not found!");
    while(1);
  }
  
  Serial.println("BH1750 initialized successfully!");
  
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
    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>BH1750 Light Sensor</title>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
    html += "<style>body{font-family:Arial;margin:20px;background:#f0f0f0;} .container{max-width:600px;margin:0 auto;background:white;padding:20px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);} .sensor{background:linear-gradient(135deg,#f39c12,#f1c40f);color:white;padding:20px;border-radius:8px;margin:10px 0;text-align:center;} .value{font-size:2em;font-weight:bold;margin:10px 0;} .unit{font-size:1.2em;opacity:0.9;} .update{color:#666;font-size:0.9em;margin-top:20px;} .status{padding:10px;border-radius:5px;margin:10px 0;text-align:center;font-weight:bold;} .bright{background:#e8f5e8;color:#2e7d32;} .normal{background:#fff3e0;color:#ef6c00;} .dark{background:#e3f2fd;color:#1976d2;}</style></head><body>";
    html += "<div class='container'><h1>☀️ BH1750 Light Sensor</h1>";
    html += "<div class='sensor'><h2>Light Level</h2><div class='value' id='light'>--</div><div class='unit'>lux</div></div>";
    html += "<div class='status' id='status'>--</div>";
    html += "<div class='update' id='lastUpdate'>Last update: --</div>";
    html += "<script>function updateData(){fetch('/api').then(r=>r.json()).then(d=>{document.getElementById('light').textContent=d.light.toFixed(0);var status=document.getElementById('status');if(d.light>1000){status.className='status bright';status.textContent='☀️ Very Bright (>1000 lux)';}else if(d.light>200){status.className='status normal';status.textContent='🌤️ Normal Light (200-1000 lux)';}else if(d.light>50){status.className='status normal';status.textContent='🌥️ Dim Light (50-200 lux)';}else{status.className='status dark';status.textContent='🌙 Dark (<50 lux)';}document.getElementById('lastUpdate').textContent='Last update: '+new Date().toLocaleString();}).catch(e=>console.error(e));}updateData();setInterval(updateData,2000);</script>";
    html += "</div></body></html>";
    request->send(200, "text/html", html);
  });
  
  server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument doc;
    doc["light"] = lightLevel;
    doc["timestamp"] = millis();
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  lightLevel = lightMeter.readLightLevel();
  
  Serial.print("Light: ");
  Serial.print(lightLevel);
  Serial.println(" lux");
  
  delay(1000);
}
