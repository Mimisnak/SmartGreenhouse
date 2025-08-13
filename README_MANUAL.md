# 🌱 Smart Greenhouse ESP32-S3 Manual

## 📋 Περιεχόμενα
- [Περιγραφή Συστήματος](#περιγραφή-συστήματος)
- [Hardware Requirements](#hardware-requirements)
- [Καλωδιώσεις](#καλωδιώσεις)
- [Εγκατάσταση](#εγκατάσταση)
- [Χρήση](#χρήση)
- [Mobile Interface](#mobile-interface)
- [Έλεγχος Αισθητήρων](#έλεγχος-αισθητήρων)
- [API Endpoints](#api-endpoints)
- [Calibration](#calibration)
- [Troubleshooting](#troubleshooting)

## 🎯 Περιγραφή Συστήματος

Το Smart Greenhouse είναι ένα IoT σύστημα βασισμένο στο ESP32-S3 που παρακολουθεί περιβαλλοντικές συνθήκες:

- **🌡️ Θερμοκρασία** (BMP280)
- **🌪️ Βαρομετρική πίεση** (BMP280)
- **💡 Φωτεινότητα** (BH1750)
- **💧 Υγρασία εδάφους** (Capacitive Soil Sensor)
- **🌐 Web Interface** για παρακολούθηση
- **⚠️ Alert System** για ειδοποιήσεις
- **📊 Real-time data** με 2 δευτερόλεπτα ανανέωση

## 🔧 Hardware Requirements

### Κύρια Εξαρτήματα:
| Εξάρτημα | Model | Περιγραφή |
|----------|-------|-----------|
| Microcontroller | ESP32-S3 DevKitC-1 | Κύριος ελεγκτής |
| Temperature/Pressure | BMP280 | I2C sensor (0x76/0x77) |
| Light Sensor | BH1750 | I2C light meter |
| Soil Sensor | Analog capacitive | GPIO 1 (ADC) |
| Power Supply | 5V/1A | USB ή external |

### Χαρακτηριστικά ESP32-S3:
- **CPU**: Dual-core Xtensa LX7 @ 240MHz
- **RAM**: 320KB SRAM
- **Flash**: 8MB
- **WiFi**: 802.11 b/g/n
- **ADC**: 12-bit resolution
- **I2C**: Hardware I2C support

## 🔌 Καλωδιώσεις

### I2C Sensors (BMP280 & BH1750):
```
ESP32-S3    →    Sensors
GND         →    GND
3.3V        →    VCC
GPIO21      →    SDA
GPIO22      →    SCL
```

### Soil Sensor:
```
ESP32-S3    →    Soil Sensor
GND         →    GND
3.3V        →    VCC
GPIO1       →    AOUT (Analog Output)
```

### Power:
```
USB-C       →    ESP32-S3 (Programming + Power)
または
External 5V  →    VIN pin
```

## 📱 Εγκατάσταση

### 1. PlatformIO Setup:
```bash
# Install PlatformIO
pip install platformio

# Clone project
git clone <repository>
cd thermokrasia

# Install dependencies
pio lib install
```

### 2. WiFi Configuration:
Στο `src/main.cpp` αλλάξτε:
```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

### 3. Build & Upload:
```bash
# Build project
pio run

# Upload to ESP32-S3
pio run --target upload

# Monitor serial output
pio device monitor
```

### 4. Hardware Assembly:
1. Συνδέστε τα sensors σύμφωνα με το wiring diagram
2. Τοποθετήστε το soil sensor στο χώμα
3. Βεβαιωθείτε για σταθερές συνδέσεις

## 🌐 Χρήση

### Web Interface Access:
1. **Συνδεθείτε στο WiFi** του ESP32 ή βρείτε το IP
2. **Ανοίξτε browser** και πηγαίνετε στο IP
3. **Παρακολουθήστε** real-time data

### Available Pages:
```
http://[ESP32-IP]/         - Main dashboard
http://[ESP32-IP]/simple   - Lightweight view
http://[ESP32-IP]/api      - JSON data
http://[ESP32-IP]/health   - System status
http://[ESP32-IP]/metrics  - Prometheus metrics
http://[ESP32-IP]/calibrate - Soil calibration
```

## 📱 Mobile Interface

### **🎨 Χρωματική Παλέτα:**
- **Κύριο**: Πράσινο gradient (#8BC34A → #558B2F)
- **Accent**: #4CAF50, #2E7D32
- **Status OK**: Πράσινο gradient 🟢
- **Status Error**: Πορτοκαλί gradient 🟠

### **📱 Mobile Optimizations:**
- **Responsive Design**: Grid layouts καταρρέουν σε 1 στήλη < 768px
- **Touch-friendly**: Κουμπιά min 44px για εύκολη χρήση
- **Optimized Typography**: Μικρότερα fonts για κινητά
- **Chart Adaptation**: Heights 200px → 180px για μικρές οθόνες
- **Stable Colors**: Χωρίς animations που προκαλούν flashing

### **🔄 Real-time Features:**
- Auto-refresh κάθε 5 δευτερόλεπτα
- Live charts με Chart.js
- Persistent storage στο localStorage
- Συνδυασμός server + client history (48 ώρες)

## 🔍 Έλεγχος Αισθητήρων

### **Status Indicators:**
- **🟢 ΕΝΕΡΓΟΣ**: Αισθητήρας συνδεδεμένος και λειτουργεί σωστά
- **🟠 ΜΗ ΔΙΑΘΕΣΙΜΟΣ**: Αισθητήρας αποσυνδεδεμένος ή σφάλμα

### **Automatic Detection:**
```cpp
// System ελέγχει αυτόματα:
Temperature/Pressure: isNaN() ή < -50°C ή > 100°C → Error
Light: readLightLevel() < 0 ή isNaN() → Disconnected  
Soil: analogRead() <= 0 ή out of range → Disconnected
```

### **Real-time Monitoring:**
- **Live Status Updates**: Κάθε 5 δευτερόλεπτα
- **Visual Feedback**: Χρώματα αλλάζουν άμεσα
- **Error Values**: "--" για αποσυνδεδεμένους αισθητήρες
- **Serial Logging**: Συνεχής έλεγχος στο Serial Monitor

### **Troubleshooting Guide:**
1. **🌡️ BMP280 Issues:**
   - Ελέγξτε I2C συνδέσεις (SDA: GPIO16, SCL: GPIO17)
   - Δοκιμάστε διευθύνσεις 0x76 και 0x77
   - Βεβαιωθείτε για σταθερή τροφοδοσία 3.3V

2. **💡 BH1750 Issues:**
   - Βεβαιωθείτε για σωστή τάση (3.3V)
   - Καθαρίστε το φακό αισθητήρα
   - Ελέγξτε I2C bus με scanner

3. **💧 Soil Sensor Issues:**
   - Νέο calibration (χρησιμοποιήστε `/calibrate`)
   - Ελέγξτε συνδέσεις VCC/GND
   - Επιβεβαιώστε GPIO 4 connection
   - Βεβαιωθείτε ότι ο αισθητήρας είναι στο χώμα

## 🔗 API Endpoints

### 📊 Data Endpoints:

#### `/api` - JSON Data
```json
{
  "temperature": 24.5,
  "pressure": 1000.2,
  "light": 150.0,
  "soil": 65.0,
  "timestamp": 123456789
}
```

#### `/health` - System Status
```
OK
uptime_ms=123456
free_heap=45000
wifi_strength=-45
```

#### `/metrics` - Prometheus Format
```
# HELP greenhouse_temperature_c Current temperature
# TYPE greenhouse_temperature_c gauge
greenhouse_temperature_c 24.50

# HELP greenhouse_pressure_hpa Atmospheric pressure
# TYPE greenhouse_pressure_hpa gauge
greenhouse_pressure_hpa 1000.20
```

### 🛠️ Utility Endpoints:

#### `/calibrate` - Soil Calibration
- Real-time raw values
- Calibration instructions
- Suggested SOIL_DRY/WET values

## ⚙️ Calibration

### Soil Sensor Calibration:

#### Automatic Method:
1. **Visit** `http://[ESP32-IP]/calibrate`
2. **Remove sensor** from soil (air reading)
3. **Note the raw value** για DRY
4. **Dip in water** (wet reading)
5. **Note the raw value** για WET
6. **Update values** στον κώδικα

#### Manual Method:
1. **Monitor Serial output** για raw values
2. **Record air reading** (typically 30-50)
3. **Record wet reading** (typically 150-300)
4. **Update constants**:
```cpp
#define SOIL_DRY_VALUE 50    // Air reading
#define SOIL_WET_VALUE 200   // Wet reading
```

### Sensor Ranges:
| Sensor | Min | Max | Unit |
|--------|-----|-----|------|
| BMP280 Temperature | -40 | +85 | °C |
| BMP280 Pressure | 300 | 1100 | hPa |
| BH1750 Light | 1 | 65535 | lux |
| Soil Moisture | 0 | 100 | % |

## ⚠️ Alert System

### Temperature Alerts:
- **High Alert**: > 30°C
- **Low Alert**: < 10°C

### Soil Moisture Alerts:
- **Low Alert**: < 20%

### Alert Output:
- **Serial Monitor**: Emoji alerts
- **Future**: Email/SMS notifications

## 🔧 Troubleshooting

### 💻 Common Issues:

#### 1. WiFi Connection Failed:
```
Error: WiFi connection timeout
```
**Solution**:
- Check SSID/password
- Verify WiFi signal strength
- Try 2.4GHz network

#### 2. Sensor Not Found:
```
Error: BMP280 sensor not found
Error: BH1750 sensor not found
```
**Solution**:
- Check I2C wiring (SDA/SCL)
- Verify 3.3V power supply
- Test I2C address scan

#### 3. Erratic Soil Readings:
```
Soil: 0% → 100% → 0%
```
**Solution**:
- Check GPIO1 connection
- Recalibrate sensor
- Verify sensor placement

#### 4. Compilation Errors:
```
Error: Library not found
```
**Solution**:
```bash
pio lib install "Adafruit BMP280 Library"
pio lib install "BH1750"
pio lib install "ESPAsyncWebServer"
```

### 🔍 Debug Tools:

#### Serial Monitor:
```bash
pio device monitor --baud 115200
```

#### I2C Scanner:
```cpp
// Built-in I2C scanner shows connected devices
```

#### Raw Soil Values:
```cpp
#define ENABLE_SOIL_DEBUG 1  // Enable raw value display
```

### 📈 Performance Monitoring:

#### Memory Usage:
- **RAM**: ~13.6% (44KB/320KB)
- **Flash**: ~24.3% (813KB/3.3MB)

#### Refresh Rates:
- **Sensor reading**: 500ms
- **Web interface**: 2 seconds
- **Serial output**: 500ms

## 🎛️ Configuration Options

### Debug Flags:
```cpp
#define ENABLE_SOIL_DEBUG 1        // Raw soil values
#define ENABLE_REQUEST_LOG 1       // HTTP requests
#define ENABLE_CALIBRATION_MODE 1  // Calibration info
#define ENABLE_ALERTS 1            // Alert system
```

### Sensor Pins:
```cpp
#define SOIL_PIN 1                 // GPIO for soil sensor
// I2C pins: SDA=21, SCL=22 (hardware I2C)
```

### Alert Thresholds:
```cpp
#define TEMP_HIGH_ALERT 30.0       // °C
#define TEMP_LOW_ALERT 10.0        // °C
#define SOIL_LOW_ALERT 20.0        // %
```

### WiFi Settings:
```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

## 📚 Technical Specifications

### Libraries Used:
```
Adafruit_BMP280 @ 2.6.8
BH1750 @ 1.3.0
ESPAsyncWebServer @ 3.6.0
ArduinoJson @ 7.4.2
WiFi @ 2.0.0 (ESP32)
Wire @ 2.0.0 (I2C)
```

### Communication Protocols:
- **I2C**: BMP280, BH1750
- **ADC**: Soil sensor
- **WiFi**: Web interface
- **HTTP**: REST API
- **JSON**: Data exchange

### Security:
- **No authentication** (local network only)
- **HTTP only** (no HTTPS)
- **Open endpoints** (no rate limiting)

## 🎯 Future Enhancements

### Planned Features:
- [ ] HTTPS support
- [ ] User authentication
- [ ] Email/SMS alerts
- [ ] Data logging to SD card
- [ ] Mobile app
- [ ] Multiple soil sensors
- [ ] Pump control
- [ ] Weather API integration

### Hardware Expansions:
- [ ] Relay control
- [ ] Additional sensors
- [ ] Solar power
- [ ] Battery backup
- [ ] Outdoor enclosure

---

## 📞 Support

Για τεχνική υποστήριξη:
1. **Check troubleshooting** section
2. **Monitor serial output** για errors
3. **Use calibration mode** για sensor issues
4. **Verify wiring** σύμφωνα με το manual

**Happy Growing! 🌱**
