/*
 * BMP280 Temperature & Pressure Sensor - Standalone
 * GPIO: SDA=16, SCL=17
 */
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_BMP280.h>
#include <ArduinoJson.h>
#include <Wire.h>

const char* ssid = "Vodafone-E79546683";
const char* password = "RLg2s6b73EfarXRx";

#define SDA_PIN 16
#define SCL_PIN 17

Adafruit_BMP280 bmp;
AsyncWebServer server(80);

float temperature = 0.0;
float pressure = 0.0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("I2C initialized with SDA=16, SCL=17");
  
  // Initialize BMP280
  if (!bmp.begin(0x76)) {
    Serial.println("Trying alternative address...");
    if (!bmp.begin(0x77)) {
      Serial.println("Could not find a valid BMP280 sensor!");
      while(1);
    }
  }
  
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
  
  Serial.println("BMP280 initialized successfully!");
  
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
    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>BMP280 Sensor</title>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
    html += "<style>body{font-family:Arial;margin:20px;background:#f0f0f0;} .container{max-width:600px;margin:0 auto;background:white;padding:20px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);} .sensor{background:linear-gradient(135deg,#667eea,#764ba2);color:white;padding:20px;border-radius:8px;margin:10px 0;text-align:center;} .value{font-size:2em;font-weight:bold;margin:10px 0;} .unit{font-size:1.2em;opacity:0.9;} .update{color:#666;font-size:0.9em;margin-top:20px;}</style></head><body>";
    html += "<div class='container'><h1>🌡️ BMP280 Sensor Monitor</h1>";
    html += "<div class='sensor'><h2>Temperature</h2><div class='value' id='temp'>--</div><div class='unit'>°C</div></div>";
    html += "<div class='sensor'><h2>Pressure</h2><div class='value' id='press'>--</div><div class='unit'>hPa</div></div>";
    html += "<div class='update' id='lastUpdate'>Last update: --</div>";
    html += "<script>function updateData(){fetch('/api').then(r=>r.json()).then(d=>{document.getElementById('temp').textContent=d.temperature.toFixed(1);document.getElementById('press').textContent=d.pressure.toFixed(1);document.getElementById('lastUpdate').textContent='Last update: '+new Date().toLocaleString();}).catch(e=>console.error(e));}updateData();setInterval(updateData,2000);</script>";
    html += "</div></body></html>";
    request->send(200, "text/html", html);
  });
  
  server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument doc;
    doc["temperature"] = temperature;
    doc["pressure"] = pressure;
    doc["timestamp"] = millis();
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  temperature = bmp.readTemperature();
  pressure = bmp.readPressure() / 100.0F;
  
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");
  
  delay(1000);
}
