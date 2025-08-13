# ⚙️ Smart Greenhouse Configuration Guide

## 🔧 Quick Setup Checklist

### 1. Hardware Connections
```
ESP32-S3 Pin → Component
─────────────────────────
GND          → All GND pins
3.3V         → All VCC pins  
GPIO16       → SDA (I2C Data)
GPIO17       → SCL (I2C Clock)
GPIO1        → Soil Sensor AOUT
```

### 2. Code Configuration
```cpp
// WiFi Settings
const char* ssid = "YOUR_WIFI_NAME";      // ✏️ Change this
const char* password = "YOUR_PASSWORD";    // ✏️ Change this

// Soil Calibration (after testing)
#define SOIL_DRY_VALUE 50    // ✏️ Air reading
#define SOIL_WET_VALUE 200   // ✏️ Water reading

// Alert Thresholds
#define TEMP_HIGH_ALERT 30.0   // ✏️ High temp alert
#define TEMP_LOW_ALERT 10.0    // ✏️ Low temp alert  
#define SOIL_LOW_ALERT 20.0    // ✏️ Low soil alert
```

### 3. Debug Options
```cpp
#define ENABLE_SOIL_DEBUG 1      // 1=Show raw values
#define ENABLE_REQUEST_LOG 1     // 1=Log HTTP requests
#define ENABLE_CALIBRATION_MODE 1 // 1=Show calibration
#define ENABLE_ALERTS 1          // 1=Enable alerts
```

## 📋 Calibration Procedure

### Step 1: Initial Upload
1. Set conservative values: `SOIL_DRY_VALUE 50`, `SOIL_WET_VALUE 200`
2. Upload code to ESP32-S3
3. Open Serial Monitor (115200 baud)

### Step 2: Find Dry Value
1. Remove soil sensor from soil (air)
2. Note the raw value in Serial Monitor
3. Wait for stable readings (2-3 minutes)

### Step 3: Find Wet Value  
1. Dip sensor in water or very wet soil
2. Note the raw value in Serial Monitor
3. Wait for stable readings

### Step 4: Update Code
1. Set `SOIL_DRY_VALUE` to air reading
2. Set `SOIL_WET_VALUE` to water reading
3. Re-upload code

### Step 5: Verify
1. Check that air shows ~0%
2. Check that water shows ~100%
3. Test intermediate values

## 🌐 Network Setup

### WiFi Connection
- Uses 2.4GHz WiFi only
- WPA/WPA2 security supported
- DHCP automatic IP assignment

### Access Methods
```
Method 1: Find IP in Serial Monitor
✅ Recommended for development

Method 2: Router Admin Panel  
✅ Check DHCP client list

Method 3: Network Scanner
✅ Use apps like "Network Scanner"
```

### Endpoints
```
http://[IP]/         → Main dashboard
http://[IP]/simple   → Mobile-friendly
http://[IP]/api      → JSON data
http://[IP]/calibrate → Calibration helper
```

## 🔍 Troubleshooting

### Common Issues & Solutions

#### "WiFi connection failed"
- ✅ Check SSID/password spelling
- ✅ Ensure 2.4GHz network
- ✅ Check signal strength

#### "BMP280 sensor not found"  
- ✅ Check I2C wiring (SDA/SCL)
- ✅ Verify 3.3V power
- ✅ Try different I2C address

#### "Soil readings erratic"
- ✅ Check GPIO1 connection  
- ✅ Recalibrate sensor
- ✅ Ensure stable power

#### "No web interface"
- ✅ Check IP address in Serial
- ✅ Verify WiFi connection
- ✅ Try different browser

### Diagnostic Commands
```bash
# Build project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output  
pio device monitor --baud 115200

# Clean build
pio run --target clean
```

## 📊 Expected Values

### Normal Ranges
```
Temperature: 15-35°C (indoor)
Pressure:    950-1050 hPa (sea level)
Light:       0-1000 lux (indoor)
             10000+ lux (outdoor)
Soil:        0-100% (calibrated)
```

### Soil Calibration Examples
```
Sensor Type          Dry    Wet
─────────────────────────────────
Capacitive v1.2      45     180
Capacitive v2.0      30     250  
Resistive analog     100    400
```

## 🎯 Performance Tips

### Optimize for Stability
1. Use quality jumper wires
2. Secure all connections
3. Provide stable 3.3V power
4. Shield from interference

### Web Performance
- Simple page loads faster
- JSON API for custom apps
- 2-second refresh rate optimal

### Power Consumption
- ~80mA during WiFi operation
- ~40mA in deep sleep mode
- Use power bank for portable use

---

**Ready to grow! 🌱**
