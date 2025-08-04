#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// Wi-Fi credentials - ΑΛΛΑΞΤΕ ΑΥΤΑ!
const char* ssid = "Vodafone-E79546683";              
const char* password = "RLg2s6b73EfarXRx";       

// Create objects
Adafruit_BMP280 bmp;
AsyncWebServer server(80);

// ESP32-S3 I2C pins
#define SDA_PIN 8   
#define SCL_PIN 9   
#define LED_PIN 48    // Built-in RGB LED on ESP32-S3

bool sensorInitialized = false;
float currentTemperature = 0.0;
float currentPressure = 0.0;
unsigned long lastReading = 0;
unsigned long bootTime = 0;

// Statistics variables
float minTemperature = 999.0;
float maxTemperature = -999.0;
float totalTemperature = 0.0;
int readingCount = 0;

bool initializeSensor() {
  Wire.begin(SDA_PIN, SCL_PIN);
  
  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 not found at 0x76, trying 0x77...");
    if (!bmp.begin(0x77)) {
      Serial.println("BMP280 sensor not found!");
      return false;
    }
  }
  
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
                  
  Serial.println("BMP280 sensor initialized successfully!");
  return true;
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  
  Serial.println("Scanning for WiFi networks...");
  int n = WiFi.scanNetworks();
  Serial.printf("Found %d networks:\n", n);
  for (int i = 0; i < n; ++i) {
    Serial.printf("%d: %s (%d dBm) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Open" : "Encrypted");
  }
  
  Serial.printf("Connecting to WiFi: %s\n", ssid);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(1000);
    Serial.printf("Attempt %d/30 - Status: %d\n", attempts + 1, WiFi.status());
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected successfully!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("DNS: ");
    Serial.println(WiFi.dnsIP());
  } else {
    Serial.println("WiFi connection failed! Starting Access Point...");
    Serial.printf("Final status: %d\n", WiFi.status());
    
    // Start Access Point mode
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32-ThermoKrasia", "12345678");
    
    Serial.println("Access Point started!");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("Connect to: ESP32-ThermoKrasia");
    Serial.println("Password: 12345678");
  }
}

void initLittleFS() {
  if (!LittleFS.begin()) {
    Serial.println("LittleFS Mount Failed");
    return;
  }
  Serial.println("LittleFS mounted successfully!");
}

void setupWebServer() {
  // CORS headers for all responses
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
  
  // Root handler with Smart Greenhouse interface
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"(
<!DOCTYPE html>
<html lang="el">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Smart Greenhouse - Advanced Plant Monitoring System</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #134e5e 0%, #71b280 50%, #a8e6cf 100%);
            min-height: 100vh;
            color: #2c3e50;
            overflow-x: hidden;
        }
        
        .floating-leaves {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            pointer-events: none;
            z-index: 1;
        }
        
        .leaf {
            position: absolute;
            font-size: 20px;
            opacity: 0.3;
            animation: float 10s infinite linear;
        }
        
        @keyframes float {
            0% { transform: translateY(-100px) rotate(0deg); }
            100% { transform: translateY(100vh) rotate(360deg); }
        }
        
        .container {
            max-width: 1400px;
            margin: 0 auto;
            padding: 20px;
            position: relative;
            z-index: 2;
        }
        
        .header {
            background: linear-gradient(135deg, #27ae60, #2ecc71, #58d68d);
            padding: 40px;
            text-align: center;
            color: white;
            border-radius: 25px;
            box-shadow: 0 25px 50px rgba(0, 0, 0, 0.2);
            margin-bottom: 30px;
            position: relative;
            overflow: hidden;
        }
        
        .header h1 {
            font-size: 3.5em;
            margin-bottom: 15px;
            text-shadow: 3px 3px 6px rgba(0,0,0,0.3);
            position: relative;
            z-index: 1;
        }
        
        .header p {
            font-size: 1.3em;
            opacity: 0.95;
            position: relative;
            z-index: 1;
        }
        
        .status-dashboard {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 25px;
            margin-bottom: 40px;
        }
        
        .status-card {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            padding: 30px;
            box-shadow: 0 15px 35px rgba(0, 0, 0, 0.1);
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255, 255, 255, 0.2);
            transition: all 0.4s ease;
            position: relative;
            overflow: hidden;
        }
        
        .status-card:hover {
            transform: translateY(-10px) scale(1.03);
            box-shadow: 0 25px 50px rgba(0, 0, 0, 0.2);
        }
        
        .card-header {
            display: flex;
            align-items: center;
            margin-bottom: 20px;
            position: relative;
            z-index: 1;
        }
        
        .card-icon {
            font-size: 2.5em;
            margin-right: 15px;
            filter: drop-shadow(2px 2px 4px rgba(0,0,0,0.2));
        }
        
        .card-title {
            font-size: 1.4em;
            font-weight: bold;
            color: #27ae60;
        }
        
        .card-value {
            font-size: 2.8em;
            font-weight: bold;
            margin: 15px 0;
            text-align: center;
            color: #2c3e50;
            text-shadow: 1px 1px 2px rgba(0,0,0,0.1);
            position: relative;
            z-index: 1;
        }
        
        .card-status {
            text-align: center;
            padding: 10px;
            border-radius: 15px;
            font-weight: bold;
            margin-top: 15px;
            position: relative;
            z-index: 1;
        }
        
        .status-excellent { background: linear-gradient(135deg, #27ae60, #2ecc71); color: white; }
        .status-good { background: linear-gradient(135deg, #f39c12, #e67e22); color: white; }
        .status-warning { background: linear-gradient(135deg, #e74c3c, #c0392b); color: white; }
        .status-offline { background: linear-gradient(135deg, #95a5a6, #7f8c8d); color: white; }
        
        .system-info {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        
        .info-card {
            background: rgba(255, 255, 255, 0.9);
            padding: 20px;
            border-radius: 15px;
            text-align: center;
            box-shadow: 0 8px 20px rgba(0, 0, 0, 0.1);
            transition: all 0.3s ease;
        }
        
        .info-card:hover {
            transform: translateY(-3px);
            box-shadow: 0 12px 30px rgba(0, 0, 0, 0.15);
        }
        
        .info-label {
            font-size: 0.9em;
            color: #7f8c8d;
            margin-bottom: 8px;
        }
        
        .info-value {
            font-size: 1.4em;
            font-weight: bold;
            color: #27ae60;
        }
        
        .footer {
            background: linear-gradient(135deg, #2c3e50, #34495e);
            color: white;
            padding: 30px;
            text-align: center;
            border-radius: 20px;
            margin-top: 40px;
        }
        
        @media (max-width: 768px) {
            .header h1 { font-size: 2.5em; }
            .container { padding: 15px; }
            .status-dashboard { grid-template-columns: 1fr; }
        }
    </style>
</head>
<body>
    <div class="floating-leaves">
        <div class="leaf" style="left: 10%; animation-delay: 0s;">🍃</div>
        <div class="leaf" style="left: 20%; animation-delay: 2s;">🌿</div>
        <div class="leaf" style="left: 30%; animation-delay: 4s;">🍀</div>
        <div class="leaf" style="left: 40%; animation-delay: 6s;">🌱</div>
        <div class="leaf" style="left: 50%; animation-delay: 8s;">🍃</div>
        <div class="leaf" style="left: 60%; animation-delay: 1s;">🌿</div>
        <div class="leaf" style="left: 70%; animation-delay: 3s;">🍀</div>
        <div class="leaf" style="left: 80%; animation-delay: 5s;">🌱</div>
        <div class="leaf" style="left: 90%; animation-delay: 7s;">🍃</div>
    </div>

    <div class="container">
        <div class="header">
            <h1>🌱 Smart Greenhouse</h1>
            <p>Advanced Plant Monitoring & Environmental Control System</p>
        </div>

        <div class="status-dashboard">
            <div class="status-card">
                <div class="card-header">
                    <span class="card-icon">🌡️</span>
                    <span class="card-title">Temperature</span>
                </div>
                <div class="card-value" id="temperature">--.-°C</div>
                <div class="card-status status-excellent" id="tempStatus">Loading...</div>
            </div>

            <div class="status-card">
                <div class="card-header">
                    <span class="card-icon">📊</span>
                    <span class="card-title">Pressure</span>
                </div>
                <div class="card-value" id="pressure">---- hPa</div>
                <div class="card-status status-excellent" id="pressureStatus">Loading...</div>
            </div>

            <div class="status-card">
                <div class="card-header">
                    <span class="card-icon">💧</span>
                    <span class="card-title">Soil Moisture</span>
                </div>
                <div class="card-value" id="soilMoisture">--%</div>
                <div class="card-status status-offline">Coming Soon</div>
            </div>

            <div class="status-card">
                <div class="card-header">
                    <span class="card-icon">☀️</span>
                    <span class="card-title">Light Level</span>
                </div>
                <div class="card-value" id="lightLevel">---- lux</div>
                <div class="card-status status-offline">Coming Soon</div>
            </div>
        </div>

        <div class="system-info">
            <div class="info-card">
                <div class="info-label">IP Address</div>
                <div class="info-value" id="deviceIP">---.---.---.---</div>
            </div>
            <div class="info-card">
                <div class="info-label">Signal Strength</div>
                <div class="info-value" id="signalStrength">--- dBm</div>
            </div>
            <div class="info-card">
                <div class="info-label">Uptime</div>
                <div class="info-value" id="systemUptime">--- min</div>
            </div>
            <div class="info-card">
                <div class="info-label">Total Readings</div>
                <div class="info-value" id="totalReadings">---</div>
            </div>
            <div class="info-card">
                <div class="info-label">Min Temperature</div>
                <div class="info-value" id="minTemp">--.-°C</div>
            </div>
            <div class="info-card">
                <div class="info-label">Max Temperature</div>
                <div class="info-value" id="maxTemp">--.-°C</div>
            </div>
        </div>

        <div class="footer">
            <h3>🌱 Smart Greenhouse Monitoring System</h3>
            <p>Advanced IoT Solution for Optimal Plant Growth | ESP32-S3 Powered</p>
        </div>
    </div>

    <script>
        let updateCounter = 0;

        function updateGreenhouse() {
            updateCounter++;
            
            fetch('/api')
                .then(response => response.json())
                .then(data => {
                    // Update main sensors
                    document.getElementById('temperature').textContent = data.temperature.toFixed(1) + '°C';
                    document.getElementById('pressure').textContent = data.pressure.toFixed(1) + ' hPa';
                    
                    // Update system info
                    document.getElementById('deviceIP').textContent = data.wifi_ip;
                    document.getElementById('signalStrength').textContent = data.wifi_rssi + ' dBm';
                    document.getElementById('systemUptime').textContent = Math.floor(data.uptime / 60) + ' min';
                    document.getElementById('totalReadings').textContent = data.reading_count;
                    document.getElementById('minTemp').textContent = data.min_temperature.toFixed(1) + '°C';
                    document.getElementById('maxTemp').textContent = data.max_temperature.toFixed(1) + '°C';
                    
                    // Update temperature status
                    const tempStatus = document.getElementById('tempStatus');
                    const temp = data.temperature;
                    if (temp >= 18 && temp <= 25) {
                        tempStatus.textContent = 'Ideal for plants';
                        tempStatus.className = 'card-status status-excellent';
                    } else if (temp >= 15 && temp <= 30) {
                        tempStatus.textContent = 'Acceptable';
                        tempStatus.className = 'card-status status-good';
                    } else {
                        tempStatus.textContent = 'Out of range';
                        tempStatus.className = 'card-status status-warning';
                    }
                    
                    // Update pressure status
                    const pressureStatus = document.getElementById('pressureStatus');
                    pressureStatus.textContent = 'Normal pressure';
                    pressureStatus.className = 'card-status status-excellent';
                    
                    console.log(`Greenhouse Update #${updateCounter} - Temp: ${temp}°C, Pressure: ${data.pressure.toFixed(1)} hPa`);
                })
                .catch(error => {
                    console.error('Greenhouse Error:', error);
                    document.getElementById('temperature').textContent = 'Error';
                    document.getElementById('pressure').textContent = 'Error';
                });
        }

        // Initialize
        updateGreenhouse();
        setInterval(updateGreenhouse, 2000);
        
        // Add interactive effects
        document.querySelectorAll('.status-card, .info-card').forEach(card => {
            card.addEventListener('mouseenter', function() {
                this.style.transform = this.style.transform.includes('scale') 
                    ? this.style.transform 
                    : (this.style.transform || '') + ' scale(1.05)';
            });
            
            card.addEventListener('mouseleave', function() {
                this.style.transform = this.style.transform.replace(' scale(1.05)', '');
            });
        });
    </script>
</body>
</html>
)";
    
    request->send(200, "text/html", html);
  });
  
  // API endpoint for sensor data
  server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument doc;
    
    // Sensor data
    doc["temperature"] = currentTemperature;
    doc["pressure"] = currentPressure;
    doc["timestamp"] = millis();
    doc["uptime"] = (millis() - bootTime) / 1000; // uptime in seconds
    
    // Device info
    doc["device"] = "ESP32-S3";
    doc["sensor"] = "BMP280";
    doc["wifi_connected"] = WiFi.status() == WL_CONNECTED;
    doc["wifi_ip"] = WiFi.localIP().toString();
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["sensor_status"] = sensorInitialized ? "OK" : "ERROR";
    
    // Statistics
    doc["min_temperature"] = minTemperature;
    doc["max_temperature"] = maxTemperature;
    doc["avg_temperature"] = readingCount > 0 ? totalTemperature / readingCount : 0;
    doc["reading_count"] = readingCount;
    
    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
  });
  
  // Handle OPTIONS requests for CORS
  server.on("/api", HTTP_OPTIONS, [](AsyncWebServerRequest *request){
    request->send(200);
  });
  
  // 404 handler
  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "File not found");
  });
  
  server.begin();
  Serial.println("Web server started!");
}

void readSensor() {
  if (!sensorInitialized) return;
  
  float temp = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F; // Convert to hPa
  
  if (!isnan(temp) && !isnan(pressure)) {
    currentTemperature = temp;
    currentPressure = pressure;
    
    // Update statistics
    if (temp < minTemperature) minTemperature = temp;
    if (temp > maxTemperature) maxTemperature = temp;
    totalTemperature += temp;
    readingCount++;
    
    Serial.printf("Temperature: %.2f°C, Pressure: %.2f hPa\n", temp, pressure);
  } else {
    Serial.println("Failed to read sensor data");
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);  // Give time for serial monitor to connect
  bootTime = millis();
  
  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // Turn on LED at start
  
  Serial.println("ESP32-S3 Temperature Monitor Starting...");
  Serial.println("==================================================");
  
  // Initialize LittleFS first
  initLittleFS();
  
  // Initialize sensor
  sensorInitialized = initializeSensor();
  
  // Initialize WiFi
  initWiFi();
  
  // Setup web server
  setupWebServer();
  
  Serial.println("==================================================");
  Serial.printf("Access your monitor at: http://%s\n", WiFi.localIP().toString().c_str());
  Serial.printf("API endpoint: http://%s/api\n", WiFi.localIP().toString().c_str());
  Serial.println("==================================================");
  
  lastReading = millis();
}

void loop() {
  // LED heartbeat - blink every 2 seconds to show it's alive
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  if (millis() - lastBlink >= 2000) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    lastBlink = millis();
  }
  
  // Read sensor every 3 seconds
  if (millis() - lastReading >= 3000) {
    readSensor();
    lastReading = millis();
  }
  
  // Check WiFi connection every 30 seconds
  static unsigned long lastWiFiCheck = 0;
  if (millis() - lastWiFiCheck >= 30000) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected, reconnecting...");
      initWiFi();
    }
    lastWiFiCheck = millis();
  }
  
  delay(100);
}
