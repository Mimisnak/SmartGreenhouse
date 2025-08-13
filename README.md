# 🌱 Smart Greenhouse - Advanced Plant Monitoring System

[![ESP32-S3](https://img.shields.io/badge/ESP32-S3-blue)](https://www.espressif.com/en/products/socs/esp32-s3)
[![BMP280](https://img.shields.io/badge/Sensor-BMP280-green)](https://www.bosch-sensortec.com/products/environmental-sensors/pressure-sensors/bmp280/)
[![BH1750](https://img.shields.io/badge/Sensor-BH1750-yellow)](https://www.rohm.com/products/sensors-mems/ambient-light-sensor-ics/bh1750fvi)
[![MIT License](https://img.shields.io/badge/License-MIT-red)](LICENSE)

> **Προηγμένο σύστημα παρακολούθησης φυτών με δυο αισθητήρες, web interface και real-time monitoring**

## 📋 Περιεχόμενα

- [Εισαγωγή](#εισαγωγή)
- [Χαρακτηριστικά](#χαρακτηριστικά)
- [Hardware Requirements](#hardware-requirements)
- [Συνδεσμολογία](#συνδεσμολογία)
- [Software Installation](#software-installation)
- [Configuration](#configuration)
- [Web Interface](#web-interface)
- [API Documentation](#api-documentation)
- [Τεχνικές Λεπτομέρειες](#τεχνικές-λεπτομέρειες)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## 🌟 Εισαγωγή

Το **Smart Greenhouse** είναι ένα ολοκληρωμένο σύστημα IoT για την παρακολούθηση περιβαλλοντικών συνθηκών σε θερμοκήπια ή εσωτερικούς χώρους καλλιέργειας. Χρησιμοποιεί το ESP32-S3 microcontroller με δυο υψηλής ακρίβειας αισθητήρες για να παρέχει real-time δεδομένα μέσω ενός modern web interface.

### 🎯 Σκοπός του Project

- **Παρακολούθηση Θερμοκρασίας**: Συνεχής μέτρηση για βέλτιστες συνθήκες ανάπτυξης
- **Μέτρηση Ατμοσφαιρικής Πίεσης**: Προβλέψεις καιρικών αλλαγών
- **Φωτομέτρηση**: Βελτιστοποίηση φωτισμού φυτών
- **Remote Monitoring**: Πρόσβαση από οπουδήποτε μέσω web browser
- **Data Persistence**: Αποθήκευση ιστορικών δεδομένων
- **Mobile Responsive**: Πλήρης υποστήριξη κινητών συσκευών

## ✨ Χαρακτηριστικά

### 🔧 Hardware Features
- ✅ **Triple Sensor Support**: BMP280 + BH1750 + Soil Moisture
- ✅ **Mixed Communication**: I2C + ADC interfaces
- ✅ **Auto Sensor Detection**: Αυτόματη αναγνώριση αισθητήρων
- ✅ **Graceful Degradation**: Λειτουργία ακόμα και αν λείπει ένας αισθητήρας
- ✅ **Corrosion Resistant**: Capacitive soil sensor χωρίς διάβρωση
- ✅ **Watchdog Timer**: Αυτόματη επανεκκίνηση σε περίπτωση προβλήματος

### 🌐 Network Features
- ✅ **WiFi Auto-Reconnection**: Αυτόματη επανασύνδεση κάθε 30 δευτερόλεπτα
- ✅ **Connection Monitoring**: Συνεχής παρακολούθηση δικτύου
- ✅ **Network Resilience**: Ανθεκτικότητα σε διακοπές δικτύου
- ✅ **HTTP Web Server**: AsyncWebServer για γρήγορες αποκρίσεις
- ✅ **Remote Data Transmission**: Αποστολή δεδομένων σε δημόσια IP κάθε 60 δευτερόλεπτα
- ✅ **Dual Connectivity**: Τοπικός server + remote cloud integration

### 📊 Data Management
- ✅ **Server-Side Storage**: Αποθήκευση δεδομένων στο ESP32 (νέο!)
- ✅ **48-hour Retention**: Διατήρηση ιστορικών δεδομένων για 48 ώρες
- ✅ **30-minute Intervals**: Αυτόματη αποθήκευση κάθε 30 λεπτά
- ✅ **Persistence**: Δεδομένα παραμένουν μετά από browser refresh
- ✅ **Real-time Updates**: Ανανέωση δεδομένων κάθε 5 δευτερόλεπτα
- ✅ **Automatic Cleanup**: Αυτόματος καθαρισμός παλιών δεδομένων
- ✅ **Data Validation**: Έλεγχος εγκυρότητας δεδομένων

### 🎨 User Interface
- ✅ **Modern Design**: Glassmorphism design με animations
- ✅ **Dark/Light Theme**: Toggle μεταξύ θεμάτων
- ✅ **Dual Language**: Ελληνικά και Αγγλικά
- ✅ **Responsive Layout**: Optimized για όλες τις συσκευές
- ✅ **Interactive Charts**: Chart.js με zoom και pan
- ✅ **Beta Notification**: Εισαγωγική οθόνη με πληροφορίες developer

### 📈 Monitoring Features
- ✅ **Sensor Status Indicators**: Real-time κατάσταση αισθητήρων
- ✅ **Connection Status**: Παρακολούθηση συνδεσιμότητας
- ✅ **Error Handling**: Graceful error management
- ✅ **System Health**: Monitoring του συστήματος

## 🛠 Hardware Requirements

### Κύρια Εξαρτήματα

| Εξάρτημα | Μοντέλο | Περιγραφή | Κόστος (€) |
|----------|---------|-----------|------------|
| **Microcontroller** | ESP32-S3 N16R8 | 16MB Flash, 8MB PSRAM | ~15-20 |
| **Θερμοκρασία/Πίεση** | BMP280 | Ακρίβεια ±1°C, ±1hPa | ~3-5 |
| **Φωτισμός** | BH1750/GY-302 | 1-65535 lux range | ~2-4 |
| **Υγρασία Εδάφους** | Capacitive Soil Sensor | Corrosion resistant, 0-100% | ~4-6 |
| **Καλώδια** | Dupont Wires | I2C + Analog connections | ~1-2 |
| **Breadboard** | Half-size | Προαιρετικό για testing | ~2-3 |

### Τεχνικές Προδιαγραφές

#### ESP32-S3 N16R8
- **CPU**: Xtensa dual-core 32-bit LX7 @ 240MHz
- **Memory**: 512KB SRAM + 8MB PSRAM
- **Flash**: 16MB
- **WiFi**: 802.11 b/g/n (2.4 GHz)
- **GPIO**: 45 programmable pins
- **I2C**: Hardware I2C support
- **Power**: 3.3V operation

#### BMP280 Sensor
- **Pressure Range**: 300-1100 hPa
- **Pressure Accuracy**: ±1 hPa
- **Temperature Range**: -40 to +85°C
- **Temperature Accuracy**: ±1°C
- **Interface**: I2C (0x76/0x77)
- **Power**: 1.8-3.6V

#### BH1750 Light Sensor
- **Range**: 1-65535 lux
- **Accuracy**: ±20%
- **Resolution**: 1 lux
- **Interface**: I2C (0x23)
- **Power**: 2.4-3.6V

#### Capacitive Soil Moisture Sensor
- **Range**: 0-100% soil moisture
- **Output**: Analog 0-3.3V
- **Accuracy**: ±3%
- **Interface**: ADC pin (GPIO 4)
- **Power**: 3.3-5V
- **Features**: Corrosion resistant, waterproof

## 🔌 Συνδεσμολογία

### I2C Bus Configuration

```
ESP32-S3          BMP280          BH1750/GY-302    Soil Moisture
--------          ------          -------------    -------------
GPIO 16 (SDA) --- SDA ----------- SDA              
GPIO 17 (SCL) --- SCL ----------- SCL              
GPIO 4 (ADC) ----------------------------- -------- AOUT
3.3V ------------ VCC ----------- VCC ------------ VCC
GND ------------- GND ----------- GND ------------ GND
```

### Σχηματικό Διάγραμμα

```
                    ESP32-S3 N16R8
                   ┌─────────────────┐
                   │                 │
    ┌─────────────▶│ GPIO 16 (SDA)   │
    │              │ GPIO 17 (SCL)   │◀─────────────┐
    │              │ GPIO 4 (ADC)    │◀───────────┐ │
    │              │ 3.3V            │◀─────────┐ │ │
    │              │ GND             │◀───────┐ │ │ │
    │              └─────────────────┘        │ │ │ │
    │                                         │ │ │ │
    │               BMP280                    │ │ │ │
    │              ┌─────────┐                │ │ │ │
    └─────────────▶│ SDA     │                │ │ │ │
                   │ SCL     │◀───────────────┘ │ │ │
                   │ VCC     │◀─────────────────┘ │ │
                   │ GND     │◀─────────────────────┘ │
                   └─────────┘                       │
                                                     │
                   BH1750/GY-302                     │
                  ┌─────────┐                        │
    ┌────────────▶│ SDA     │                        │
    │             │ SCL     │◀─────────────────────┐ │
    │             │ VCC     │◀───────────────────┐ │ │
    │             │ GND     │◀─────────────────┐ │ │ │
    │             └─────────┘                  │ │ │ │
    │                                          │ │ │ │
    │           Capacitive Soil Sensor         │ │ │ │
    │          ┌─────────┐                     │ │ │ │
    │          │ AOUT    │◀────────────────────┘ │ │ │
    │          │ VCC     │◀──────────────────────┘ │ │
    │          │ GND     │◀────────────────────────┘ │
    │          └─────────┘                           │
    │                                                │
    └────────────────────────────────────────────────┘
```

### Pin Mapping

| ESP32-S3 Pin | Λειτουργία | Σύνδεση |
|--------------|------------|---------|
| GPIO 16 | SDA (I2C Data) | BMP280 SDA + BH1750 SDA |
| GPIO 17 | SCL (I2C Clock) | BMP280 SCL + BH1750 SCL |
| GPIO 4 | ADC (Analog Input) | Soil Moisture AOUT |
| 3.3V | Power Supply | All sensors VCC |
| GND | Ground | All sensors GND |

### I2C Addresses

| Device | Address | Alternative |
|--------|---------|-------------|
| BMP280 | 0x76 | 0x77 |
| BH1750 | 0x23 | - |

## 💻 Software Installation

### Προαπαιτούμενα

1. **PlatformIO IDE**
   ```bash
   # Εγκατάσταση PlatformIO Core
   pip install platformio
   
   # Ή χρήση VS Code Extension
   # Εγκατάσταση PlatformIO IDE extension
   ```

2. **Git** (για clone του repository)
   ```bash
   git clone https://github.com/Mimisnak/SmartGreenhouse.git
   cd SmartGreenhouse
   ```

### Dependencies

Το project χρησιμοποιεί τις ακόλουθες βιβλιοθήκες (αυτόματη εγκατάσταση μέσω `platformio.ini`):

```ini
lib_deps = 
    WiFi                    # ESP32 WiFi library
    ESPAsyncWebServer       # Async web server
    adafruit/Adafruit BMP280 Library@^2.6.8
    ArduinoJson@^7.0.4      # JSON parsing
    claws/BH1750@^1.3.0     # Light sensor library
    Wire                    # I2C communication
```

### Build και Upload

```bash
# Build project
pio run

# Upload στο ESP32-S3
pio run --target upload

# Monitor serial output
pio device monitor
```

### Alternative με Arduino IDE

1. **Εγκατάσταση ESP32 Board Package**
   - Προσθήκη URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Board Manager → ESP32 → Install

2. **Εγκατάσταση Libraries**
   - Adafruit BMP280 Library
   - BH1750 Library by claws
   - ESPAsyncWebServer
   - ArduinoJson

3. **Board Configuration**
   - Board: "ESP32S3 Dev Module"
   - USB CDC On Boot: "Enabled"
   - Flash Size: "16MB"

## ⚙️ Configuration

### WiFi Setup

Επεξεργασία του αρχείου `src/main.cpp`:

```cpp
// WiFi credentials - ΑΛΛΑΞΤΕ ΤΑ ΣΤΟΙΧΕΙΑ ΣΑΣ
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

### I2C Pin Configuration

Εάν θέλετε να χρησιμοποιήσετε διαφορετικά pins:

```cpp
// I2C pins - Προσαρμόστε ανάλογα με τη συνδεσμολογία σας
#define SDA_PIN 16  // Data pin
#define SCL_PIN 17  // Clock pin
```

### Sensor Configuration

```cpp
// BMP280 Settings
bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,     // Operating mode
    Adafruit_BMP280::SAMPLING_X2,     // Temperature sampling
    Adafruit_BMP280::SAMPLING_X16,    // Pressure sampling
    Adafruit_BMP280::FILTER_X16,      // Filtering
    Adafruit_BMP280::STANDBY_MS_500   // Standby time
);
```

### Watchdog Timer

```cpp
#define WDT_TIMEOUT 60 // Timeout σε δευτερόλεπτα
```

## 🌐 Web Interface

### Κύρια Χαρακτηριστικά

#### 🎨 Design Elements

- **Glassmorphism**: Σύγχρονο διαφανές design
- **Gradient Backgrounds**: Δυναμικά χρωματικά σχήματα
- **Smooth Animations**: Transitions και hover effects
- **Cards Layout**: Οργανωμένη παρουσίαση δεδομένων

#### 📱 Responsive Design

```css
/* Mobile First Approach */
@media (max-width: 768px) {
    .status-cards {
        grid-template-columns: 1fr;
    }
    .chart-wrapper {
        height: 300px;
    }
}

@media (max-width: 480px) {
    .main-title {
        font-size: 2em;
    }
    .card-value {
        font-size: 2.2em;
    }
}
```

### 📊 Interactive Charts

Τα γραφήματα χρησιμοποιούν **Chart.js** με τα εξής χαρακτηριστικά:

- **Zoom Support**: Mouse wheel και pinch-to-zoom
- **Pan Support**: Drag για navigation
- **Responsive**: Αυτόματη προσαρμογή σε όλες τις οθόνες
- **Real-time Updates**: Live data updates χωρίς flickering

```javascript
// Chart Configuration Example
var chartOptions = {
    responsive: true,
    maintainAspectRatio: false,
    plugins: {
        zoom: {
            zoom: {
                wheel: { enabled: true },
                pinch: { enabled: true },
                mode: 'x'
            },
            pan: {
                enabled: true,
                mode: 'x'
            }
        }
    }
};
```

### 🎭 Theme System

#### Light Theme
- **Background**: Linear gradient (blue to purple)
- **Cards**: Semi-transparent white
- **Text**: Dark colors για καλή αντίθεση

#### Dark Theme
- **Background**: Dark gradient (navy to dark purple)
- **Cards**: Semi-transparent dark
- **Text**: Light colors

```javascript
function toggleTheme() {
    currentTheme = currentTheme === 'light' ? 'dark' : 'light';
    document.body.setAttribute('data-theme', currentTheme);
    localStorage.setItem('preferred-theme', currentTheme);
}
```

### 🌍 Multi-language Support

Υποστήριξη Ελληνικών και Αγγλικών με:

```html
<!-- Data attributes για κείμενα -->
<span data-el="Θερμοκρασία" data-en="Temperature">Θερμοκρασία</span>
```

```javascript
function setLanguage(lang) {
    document.querySelectorAll('[data-' + lang + ']').forEach(element => {
        element.textContent = element.getAttribute('data-' + lang);
    });
    localStorage.setItem('preferred-language', lang);
}
```

## 📡 API Documentation

### Endpoints

#### GET `/`
**Περιγραφή**: Κύρια σελίδα με το web interface

**Response**: HTML page

#### GET `/api`
**Περιγραφή**: Real-time sensor data

**Response Format**:
```json
{
    "temperature": 25.4,
    "pressure": 1013.2,
    "light": 450,
    "soilMoisture": 65.3,
    "timestamp": 1234567890
}
```

**Response Fields**:
- `temperature`: Θερμοκρασία σε °C
- `pressure`: Πίεση σε hPa
- `light`: Φωτισμός σε lux (0 αν δεν διατίθεται)
- `soilMoisture`: Υγρασία εδάφους σε ποσοστό (0-100%)
- `timestamp`: Proper timestamp σε Unix format

#### GET `/history`
**Περιγραφή**: Historical sensor data (48-hour retention)

**Response Format**:
```json
{
    "count": 96,
    "data": [
        {
            "temperature": 25.4,
            "pressure": 1013.2,
            "light": 450,
            "soilMoisture": 65.3,
            "timestamp": 1234567890
        },
        ...
    ]
}
```

**Response Fields**:
- `count`: Αριθμός ιστορικών σημείων
- `data`: Array με ιστορικά δεδομένα (max 96 entries)
- Κάθε entry περιέχει: `temperature`, `pressure`, `light`, `soilMoisture`, `timestamp`

### Error Handling

- **404 Not Found**: Για άγνωστα endpoints
- **Connection Errors**: Graceful handling στο frontend
- **Sensor Errors**: Προβολή κατάστασης στο UI

## 🔧 Τεχνικές Λεπτομέρειες

### Αρχιτεκτονική Συστήματος

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Web Browser   │◀──▶│   ESP32-S3      │◀──▶│    Sensors      │
│                 │    │                 │    │                 │
│ • HTML/CSS/JS   │    │ • AsyncWebServer│    │ • BMP280        │
│ • Chart.js      │    │ • WiFi Client   │    │ • BH1750        │
│ • LocalStorage  │    │ • I2C Master    │    │ • I2C Slaves    │
│ • Responsive UI │    │ • Watchdog      │    │ • Auto-detect   │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

### Data Flow

```
Sensors → I2C → ESP32 → JSON → HTTP → Browser → Charts → LocalStorage
   ↑                                                          ↓
   └──────────── Error Recovery ←── Network Monitor ←─────────┘
```

### Memory Management

- **Stack Size**: Optimized για ESP32-S3
- **Heap Usage**: Careful string handling
- **PSRAM**: Utilized για large data structures
- **Flash**: Efficient storage του web content

### Performance Optimization

1. **Compressed JavaScript**: Minified functions
2. **Efficient JSON**: Compact data structures
3. **Async Operations**: Non-blocking I/O
4. **Chart Updates**: Optimized redraws
5. **Memory Cleanup**: Automatic garbage collection

### Security Considerations

- **No Authentication**: Local network only
- **Input Validation**: Sensor data sanitization
- **XSS Protection**: Safe HTML generation
- **Network Isolation**: WiFi network dependency

## 🔍 Monitoring & Debugging

### Serial Monitor Output

```
ESP32-S3 Watchdog timer initialized
I2C initialized with SDA=16, SCL=17
Scanning I2C bus...
Found I2C device at address 0x23
Found I2C device at address 0x76
Found 2 I2C device(s)

BH1750 initialized successfully!
Connecting to WiFi.......
WiFi connected!
IP address: 192.168.1.100
HTTP server started

Temperature: 25.4 °C, Pressure: 1013.2 hPa, Light: 450 lux
```

### Browser Console Logs

```javascript
// Data loading
Loading 145 stored data points

// Sensor status
Sensors reconnected successfully

// System events
New day detected: clearing stored data
Chart zoom to 24h for temp
```

### Health Monitoring

1. **Watchdog Timer**: Αυτόματη επανεκκίνηση
2. **WiFi Monitor**: Connection status
3. **Sensor Check**: I2C communication
4. **Memory Usage**: Heap monitoring
5. **Update Frequency**: Data consistency

## ❗ Troubleshooting

### Συχνά Προβλήματα

#### 1. Δεν συνδέεται στο WiFi

**Συμπτώματα**:
- Συνεχείς τελείες στο Serial Monitor
- Δεν εμφανίζεται IP address

**Λύσεις**:
```cpp
// Έλεγχος credentials
const char* ssid = "CORRECT_WIFI_NAME";
const char* password = "CORRECT_PASSWORD";

// Έλεγχος WiFi band (μόνο 2.4GHz)
// ESP32 δεν υποστηρίζει 5GHz
```

#### 2. I2C Sensors δεν ανιχνεύονται

**Συμπτώματα**:
```
Found 0 I2C device(s)
BMP280 sensor not found!
```

**Λύσεις**:
1. **Έλεγχος συνδέσεων**:
   ```
   ESP32    Sensor
   -----    ------
   GPIO16 → SDA
   GPIO17 → SCL
   3.3V   → VCC
   GND    → GND
   ```

2. **Έλεγχος τάσης**: 3.3V (όχι 5V)

3. **Pull-up resistors**: Εσωτερικά ενεργοποιημένα

#### 3. Web Interface δεν φορτώνει

**Συμπτώματα**:
- Timeout στο browser
- "Server not found"

**Λύσεις**:
1. **Έλεγχος IP**: Χρήση σωστής διεύθυνσης
2. **Network**: Ίδιο WiFi network
3. **Firewall**: Disable local restrictions

#### 4. Charts δεν εμφανίζονται

**Συμπτώματα**:
- Κενά chart containers
- JavaScript errors

**Λύσεις**:
1. **Internet Connection**: Chart.js CDN access
2. **Browser Compatibility**: Modern browsers only
3. **JavaScript Errors**: Check console

#### 5. Data δεν αποθηκεύονται

**Συμπτώματα**:
- Charts reset στο page reload
- "No stored data found"

**Λύσεις**:
1. **Browser Storage**: Enable localStorage
2. **Private Mode**: Disable incognito
3. **Storage Quota**: Clear browser data

### Debug Commands

```cpp
// Enable debug output
#define DEBUG_SERIAL 1

#if DEBUG_SERIAL
    Serial.println("Debug info here");
#endif

// I2C Scanner
void scanI2C() {
    for (byte i = 8; i < 120; i++) {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0) {
            Serial.printf("I2C device at 0x%02X\n", i);
        }
    }
}
```

### Performance Issues

#### Slow Response Times

**Αιτίες**:
- WiFi signal strength
- Network congestion
- ESP32 overload

**Λύσεις**:
```cpp
// Reduce update frequency
setInterval(updateData, 10000); // 10 seconds instead of 5

// Optimize data processing
// Use efficient data structures
```

#### Memory Issues

**Συμπτώματα**:
```
ESP32: Out of memory
Watchdog timeout reset
```

**Λύσεις**:
```cpp
// Monitor heap usage
Serial.printf("Free heap: %d\n", ESP.getFreeHeap());

// Use PSRAM for large data
// Optimize string operations
```

## 📂 Project Structure

```
SmartGreenhouse/
├── src/
│   └── main.cpp                 # Κύριος κώδικας ESP32
├── data/                        # Web assets (optional)
│   ├── index.html              # Static HTML
│   ├── style.css               # CSS styles
│   └── script.js               # JavaScript
├── backup/                      # Version backups
│   ├── README.md               # Backup information
│   └── [timestamped folders]/  # Dated backups
├── platformio.ini              # PlatformIO configuration
├── README.md                   # This file
├── LICENSE                     # MIT License
├── WIRING_GUIDE.md            # Hardware guide
└── GITHUB_PAGES_GUIDE.md      # Deployment guide
```

### Κύρια Αρχεία

- **`src/main.cpp`**: Core ESP32 application
- **`platformio.ini`**: Build configuration
- **`backup/`**: Versioned code snapshots
- **Documentation**: Comprehensive guides

## 🚀 Deployment Options

### 1. Local Development
```bash
pio run --target upload
# Access via ESP32 IP address
```

### 2. GitHub Pages (Static)
- Deploy static version to GitHub Pages
- Useful για demo purposes
- No real sensor data

### 3. Cloud Integration
- **ThingSpeak**: IoT data platform
- **Blynk**: Mobile app integration
- **MQTT**: Broker communication

## 🤝 Contributing

### Πώς να συνεισφέρετε

1. **Fork** το repository
2. **Create** feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** changes (`git commit -m 'Add amazing feature'`)
4. **Push** to branch (`git push origin feature/amazing-feature`)
5. **Open** Pull Request

### Development Guidelines

- **Code Style**: Consistent indentation
- **Comments**: Document complex functions
- **Testing**: Verify on hardware
- **Documentation**: Update README

### Feature Requests

Επιθυμητές προσθήκες:
- [ ] MQTT Support
- [ ] Email Alerts
- [ ] Mobile App
- [ ] SD Card Logging
- [ ] Multi-device Support
- [ ] Weather Forecast Integration

## 📊 Performance Metrics

### Typical Values

| Metric | Value | Unit |
|--------|-------|------|
| Update Rate | 5 | seconds |
| Response Time | <100 | ms |
| Memory Usage | <60% | ESP32 heap |
| Power Consumption | ~150 | mA |
| WiFi Range | 50+ | meters |

### Sensor Accuracy

| Parameter | Accuracy | Range |
|-----------|----------|-------|
| Temperature | ±1°C | -40 to +85°C |
| Pressure | ±1 hPa | 300-1100 hPa |
| Light | ±20% | 1-65535 lux |
| Soil Moisture | ±3% | 0-100% |

## 📚 References & Resources

### Documentation
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [BMP280 Datasheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmp280-ds001.pdf)
- [BH1750 Datasheet](https://www.mouser.com/datasheet/2/348/bh1750fvi-e-186247.pdf)

### Libraries
- [Adafruit BMP280](https://github.com/adafruit/Adafruit_BMP280_Library)
- [BH1750 by claws](https://github.com/claws/BH1750)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [ArduinoJson](https://arduinojson.org/)

### Tools
- [PlatformIO](https://platformio.org/)
- [Chart.js](https://www.chartjs.org/)
- [VS Code](https://code.visualstudio.com/)

## 📄 License

Αυτό το project είναι licensed υπό την **MIT License** - δείτε το αρχείο [LICENSE](LICENSE) για λεπτομέρειες.

```
MIT License

Copyright (c) 2025 mimis.dev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
```

## 👨‍💻 Developer

**mimis.dev**  
🌐 Website: [https://mimis.dev](https://mimis.dev)  
📧 Email: contact@mimis.dev  
🐙 GitHub: [@Mimisnak](https://github.com/Mimisnak)

---

## 📞 Support

Για questions, issues, ή contributions:

- 🐛 **Issues**: [GitHub Issues](https://github.com/Mimisnak/SmartGreenhouse/issues)
- 💬 **Discussions**: [GitHub Discussions](https://github.com/Mimisnak/SmartGreenhouse/discussions)
- 📧 **Email**: contact@mimis.dev

---

<div align="center">

**🌱 Happy Growing! 🌱**

*Made with ❤️ by [mimis.dev](https://mimis.dev)*

</div>
