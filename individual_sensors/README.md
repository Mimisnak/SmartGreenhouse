# Individual Sensor Testing - Smart Greenhouse

Ξεχωριστοί κώδικες για δοκιμή κάθε αισθητήρα ανεξάρτητα.

## 📂 Περιεχόμενα

### 🌡️ BMP280 - Temperature & Pressure
**Αρχείο:** `bmp280_standalone.cpp`
- Θερμοκρασία (°C) και ατμοσφαιρική πίεση (hPa)
- I2C addresses: 0x76 ή 0x77
- Web interface με real-time data
- API endpoint: `/api`

### ☀️ BH1750 - Light Sensor  
**Αρχείο:** `bh1750_standalone.cpp`
- Φωτισμός σε lux
- I2C address: 0x23
- Κατηγοριοποίηση φωτός (Very Bright, Normal, Dim, Dark)
- Έγχρωμο web interface

### 🌱 Soil Moisture Sensor
**Αρχείο:** `soil_moisture_standalone.cpp`
- Υγρασία εδάφους σε %
- Analog GPIO 1
- Raw ADC τιμές για calibration
- Οδηγίες calibration στο web interface

### 🔍 I2C Scanner
**Αρχείο:** `i2c_scanner.cpp`
- Εντοπισμός I2C συσκευών
- Αναγνώριση γνωστών αισθητήρων
- Οδηγίες σύνδεσης

## 🚀 Χρήση

### 1. Επιλογή αισθητήρα
Αντίγραψε τον κώδικα που θέλεις στο `src/main.cpp`:
```bash
copy "individual_sensors\bmp280_standalone.cpp" "src\main.cpp"
```

### 2. Build & Upload
```bash
platformio run --target upload
```

### 3. Monitor
```bash
platformio device monitor
```

### 4. Web Access
Άνοιξε browser στη IP που εμφανίζεται στο serial monitor.

## ⚙️ Hardware Configuration

### I2C Sensors (BMP280, BH1750)
```
ESP32-S3    Sensor
GPIO 16  -> SDA
GPIO 17  -> SCL  
3.3V     -> VCC
GND      -> GND
```

### Analog Sensor (Soil Moisture)
```
ESP32-S3    Sensor
GPIO 1   -> Signal/Analog Out
3.3V     -> VCC (ή 5V αν υποστηρίζεται)
GND      -> GND
```

## 🔧 Calibration

### Soil Moisture Sensor
1. Τρέξε τον `soil_moisture_standalone.cpp`
2. Βγάλε τον αισθητήρα από το χώμα → σημείωσε Raw τιμή (DRY)
3. Βάλε τον αισθητήρα σε νερό → σημείωσε Raw τιμή (WET)  
4. Ενημέρωσε στον κώδικα:
   ```cpp
   #define SOIL_DRY_VALUE XXXX  // Raw τιμή στεγνού
   #define SOIL_WET_VALUE YYYY  // Raw τιμή βρεγμένου
   ```

## 📊 Features κάθε αισθητήρα

| Αισθητήρας | Web UI | API | Real-time | Calibration | 
|------------|---------|-----|-----------|-------------|
| BMP280     | ✅      | ✅  | ✅        | Auto        |
| BH1750     | ✅      | ✅  | ✅        | Auto        |
| Soil       | ✅      | ✅  | ✅        | Manual      |
| I2C Scanner| Serial  | ❌  | ✅        | N/A         |

## 🔙 Επιστροφή στο πλήρες σύστημα
```bash
copy "backup\2025-08-11_18-34-00_complete_4_sensors\main_4_sensors_working.cpp" "src\main.cpp"
```

## 💡 Tips
- Χρησιμοποίησε I2C Scanner για έλεγχο συνδέσεων
- Κάθε standalone κώδικας έχει το δικό του web interface
- Calibration του soil sensor είναι κρίσιμο για σωστές μετρήσεις
- Όλοι οι κώδικες είναι ready-to-use με WiFi + web server
