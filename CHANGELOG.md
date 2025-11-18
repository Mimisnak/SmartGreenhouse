# Changelog - Smart Greenhouse System

## [Proto+] - 18 Νοέμβριος 2025

### Προστέθηκαν

#### Sensor Registry System
- **Structured sensor management**: Νέο enum `SensorType` και struct `SensorInfo` για οργανωμένη διαχείριση αισθητήρων
- **Sensor array**: Πίνακας `sensors[]` που κρατάει metadata για κάθε αισθητήρα (name, unit, enabled, available, lastValue, lastRead)
- **Auto-detection**: Η συνάρτηση `updateSensorRegistry()` ενημερώνει αυτόματα την κατάσταση κάθε αισθητήρα

#### Cloud Integration Infrastructure
- **HTTPClient support**: Προστέθηκε `#include <HTTPClient.h>`
- **Cloud configuration**: Μεταβλητές για cloud API URL, device ID, sync interval
- **sendToCloud() function**: Έτοιμη συνάρτηση για αποστολή δεδομένων σε backend (disabled by default)
- **ENABLE_CLOUD_SYNC flag**: Εύκολη ενεργοποίηση/απενεργοποίηση cloud sync

#### New API Endpoints
- **GET /sensors**: Επιστρέφει λίστα με όλους τους αισθητήρες, την κατάσταση τους και τις τιμές τους
  ```json
  {
    "sensors": [
      {
        "name": "Temperature",
        "unit": "°C",
        "enabled": true,
        "available": true,
        "value": 25.4,
        "last_read": 12345678
      },
      ...
    ],
    "device_id": "greenhouse-proto-001",
    "cloud_sync_enabled": false
  }
  ```

### Αλλάχθηκαν

#### Loop Function
- Προστέθηκε κλήση `updateSensorRegistry()` για ενημέρωση καταστάσεων
- Προστέθηκε conditional cloud sync (μόνο όταν `ENABLE_CLOUD_SYNC == true`)

### Τεχνικές Λεπτομέρειες

#### Sensor Registry Structure
```cpp
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
```

#### Cloud Sync Behavior
- **Interval**: 60 δευτερόλεπτα (configurable)
- **Format**: JSON με device_id + array αισθητήρων
- **Fallback**: Graceful handling αν WiFi χάσει ή server δεν απαντά
- **Control**: `ENABLE_CLOUD_SYNC` flag για enable/disable

### Backward Compatibility

✅ Όλη η υπάρχουσα λειτουργία διατηρήθηκε:
- Web dashboard δουλεύει ακριβώς όπως πριν
- Υπάρχοντα API endpoints (`/api`, `/health`, `/status`) αμετάβλητα
- Sensor readings και history function κανονικά
- WiFi connection logic αμετάβλητη

### Backup

Δημιουργήθηκε πλήρες backup στο:
```
backup/Proto_stadio_ptixeiakhs/
├── platformio.ini
├── main.cpp (placeholder)
└── README.md
```

### Επόμενα Βήματα (Future Work)

Όταν είσαι έτοιμος:
1. **Cloud Backend**: Node.js/Python API server με endpoints για telemetry
2. **Authentication**: User login system για πολλαπλά devices
3. **Manual Controls**: Web UI για χειροκίνητο πότισμα/εντολές
4. **Subdomain Setup**: `greenhouse.mimis.dev` deployment

---

## [Proto] - Baseline (before 18 Nov 2025)

Stable version με:
- 3 sensors (BMP280, BH1750, Soil)
- Local web dashboard
- 48h history storage
- Charts and real-time updates
- Mobile responsive UI
