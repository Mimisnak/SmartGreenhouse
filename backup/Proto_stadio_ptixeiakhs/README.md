# Proto Στάδιο Πτυχιακής - Backup Snapshot

**Ημερομηνία:** 18 Νοεμβρίου 2025  
**Κατάσταση:** Stable, working version

## Περιεχόμενα

Αυτός ο φάκελος περιέχει το "χρυσό" snapshot της πτυχιακής εργασίας πριν από οποιεσδήποτε επιπλέον αλλαγές για cloud integration ή νέα features.

### Αρχεία

- `platformio.ini` - Ρυθμίσεις build με COM6 upload port
- `main.cpp` - Core firmware με:
  - BMP280 temperature/pressure sensor
  - BH1750 light sensor  
  - Capacitive soil moisture sensor
  - Web dashboard με charts
  - 48h data history
  - WiFi connectivity

### Χαρακτηριστικά που δουλεύουν

✅ Τοπικό web dashboard στο `http://[ESP32_IP]`  
✅ Real-time sensor readings  
✅ Historical charts (48 hours)  
✅ Dual language support (EL/EN)  
✅ Mobile responsive design  
✅ Automatic sensor detection  
✅ Graceful degradation (λειτουργία ακόμα κι αν λείπει αισθητήρας)

### Πώς να επαναφέρεις αυτήν την έκδοση

Αν χρειαστεί να γυρίσεις στην stable έκδοση:

```bash
# Backup current files first
mv src/main.cpp src/main_new.cpp
mv platformio.ini platformio_new.ini

# Restore proto version
cp backup/Proto_stadio_ptixeiakhs/main.cpp src/main.cpp
cp backup/Proto_stadio_ptixeiakhs/platformio.ini platformio.ini

# Upload to ESP32
pio run --target upload
```

### WiFi Configuration

```cpp
const char* ssid = "Vodafone-E79546683";
const char* password = "RLg2s6b73EfarXRx";
```

### Sensor Configuration

- **I2C Pins:** SDA=16, SCL=17
- **Soil Pin:** GPIO 4 (ADC)
- **BMP280 Address:** 0x76 (fallback 0x77)
- **BH1750 Address:** 0x23

### Upload Settings

- **COM Port:** COM6
- **Speed:** 115200 baud
- **Protocol:** esptool

## Σημειώσεις

Αυτή η έκδοση χρησιμοποιείται ως reference για την πτυχιακή εργασία. Όλες οι μελλοντικές αλλαγές (cloud integration, manual watering controls, multi-user support) θα γίνονται πάνω σε αντίγραφα αυτής της βάσης.
