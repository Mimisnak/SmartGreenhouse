# Sensor Registry & Cloud Integration - Documentation

## Επισκόπηση

Η νέα έκδοση του firmware προσθέτει:
1. **Sensor Registry System** - Οργανωμένη διαχείριση αισθητήρων
2. **Cloud Integration Infrastructure** - Έτοιμο για αποστολή δεδομένων σε backend
3. **New API Endpoint** - `/sensors` για πληροφορίες αισθητήρων

## 1. Sensor Registry System

### Δομή

```cpp
enum SensorType {
  SENSOR_BMP280_TEMP,       // Θερμοκρασία
  SENSOR_BMP280_PRESSURE,   // Πίεση
  SENSOR_BH1750_LIGHT,      // Φωτισμός
  SENSOR_SOIL_MOISTURE,     // Υγρασία εδάφους
  SENSOR_COUNT              // Αριθμός αισθητήρων
};

struct SensorInfo {
  const char* name;          // Όνομα αισθητήρα
  const char* unit;          // Μονάδα μέτρησης
  bool enabled;              // Ενεργοποιημένος;
  bool available;            // Διαθέσιμος τώρα;
  float lastValue;           // Τελευταία τιμή
  unsigned long lastRead;    // Timestamp τελευταίας ανάγνωσης
};
```

### Χρήση

Το registry ενημερώνεται αυτόματα σε κάθε loop iteration:

```cpp
void updateSensorRegistry() {
  // Ελέγχει κάθε αισθητήρα
  // Ενημερώνει available/lastValue/lastRead
}
```

### Πρόσβαση στα δεδομένα

```cpp
// Έλεγχος αν ο αισθητήρας θερμοκρασίας είναι διαθέσιμος
if (sensors[SENSOR_BMP280_TEMP].available) {
  float temp = sensors[SENSOR_BMP280_TEMP].lastValue;
  Serial.printf("Temperature: %.1f %s\n", temp, sensors[SENSOR_BMP280_TEMP].unit);
}

// Loop σε όλους τους αισθητήρες
for (int i = 0; i < SENSOR_COUNT; i++) {
  if (sensors[i].enabled && sensors[i].available) {
    Serial.printf("%s: %.1f %s\n", 
                  sensors[i].name, 
                  sensors[i].lastValue, 
                  sensors[i].unit);
  }
}
```

## 2. Cloud Integration

### Configuration

```cpp
// Ενεργοποίηση/απενεργοποίηση cloud sync
#define ENABLE_CLOUD_SYNC false  // Άλλαξε σε true όταν έχεις backend

// Cloud backend URL
const char* cloudApiUrl = "https://greenhouse.mimis.dev/api/telemetry";

// Device ID (unique για κάθε ESP32)
const char* deviceId = "greenhouse-proto-001";

// Συχνότητα αποστολής (milliseconds)
#define CLOUD_SYNC_INTERVAL 60000  // 1 λεπτό
```

### JSON Payload Format

Όταν στέλνει δεδομένα στο cloud, το payload είναι:

```json
{
  "device_id": "greenhouse-proto-001",
  "timestamp": 1234567890,
  "sensors": [
    {
      "type": "Temperature",
      "value": 25.4,
      "unit": "°C"
    },
    {
      "type": "Pressure",
      "value": 1013.2,
      "unit": "hPa"
    },
    {
      "type": "Light",
      "value": 450,
      "unit": "lux"
    },
    {
      "type": "Soil Moisture",
      "value": 65.3,
      "unit": "%"
    }
  ]
}
```

### Ενεργοποίηση Cloud Sync

**Βήμα 1:** Στήσε backend server που δέχεται POST requests στο `/api/telemetry`

**Βήμα 2:** Ενημέρωσε τη διεύθυνση:
```cpp
const char* cloudApiUrl = "https://your-backend.com/api/telemetry";
```

**Βήμα 3:** Ενεργοποίησε το sync:
```cpp
#define ENABLE_CLOUD_SYNC true
```

**Βήμα 4:** Upload firmware στο ESP32

### Error Handling

Το cloud sync έχει graceful fallback:
- Δεν crash αν το WiFi χαθεί
- Δεν μπλοκάρει το loop αν ο server είναι κάτω
- Συνεχίζει να δουλεύει τοπικά κανονικά

## 3. New API Endpoint: `/sensors`

### Request

```bash
GET http://192.168.2.20/sensors
```

### Response

```json
{
  "sensors": [
    {
      "name": "Temperature",
      "unit": "°C",
      "enabled": true,
      "available": true,
      "value": 25.4,
      "last_read": 1234567890
    },
    {
      "name": "Pressure",
      "unit": "hPa",
      "enabled": true,
      "available": true,
      "value": 1013.2,
      "last_read": 1234567890
    },
    {
      "name": "Light",
      "unit": "lux",
      "enabled": true,
      "available": false,
      "value": 0,
      "last_read": 0
    },
    {
      "name": "Soil Moisture",
      "unit": "%",
      "enabled": true,
      "available": true,
      "value": 65.3,
      "last_read": 1234567890
    }
  ],
  "device_id": "greenhouse-proto-001",
  "cloud_sync_enabled": false
}
```

### Use Cases

- **Debugging**: Δες ποιοι αισθητήρες είναι online
- **Monitoring**: Track last_read timestamps
- **Integration**: External apps μπορούν να query τη λίστα αισθητήρων

## 4. Backward Compatibility

✅ **Όλα τα υπάρχοντα features δουλεύουν ακριβώς όπως πριν:**

- Web dashboard στο `/`
- API endpoint στο `/api`
- History endpoint στο `/history`
- Health check στο `/health`
- Status στο `/status`

Το νέο registry system τρέχει "παράλληλα" χωρίς να αλλάξει την υπάρχουσα λειτουργία.

## 5. Επόμενα Βήματα

### Για Cloud Integration

1. **Backend Development**
   - Node.js/Express ή Python/FastAPI
   - Endpoint: `POST /api/telemetry`
   - Database: PostgreSQL/MySQL για αποθήκευση
   - Authentication: JWT tokens

2. **Frontend Dashboard**
   - Deploy στο `greenhouse.mimis.dev`
   - Charts με δεδομένα από backend
   - Login system
   - Multi-device support

3. **Manual Control**
   - Backend endpoint: `POST /api/command`
   - ESP32 polling: `GET /api/pending-commands`
   - UI buttons για χειροκίνητο πότισμα

### Για Νέους Αισθητήρες

Αν θες να προσθέσεις νέο αισθητήρα (π.χ. CO₂):

**1. Update το enum:**
```cpp
enum SensorType {
  SENSOR_BMP280_TEMP,
  SENSOR_BMP280_PRESSURE,
  SENSOR_BH1750_LIGHT,
  SENSOR_SOIL_MOISTURE,
  SENSOR_CO2,           // <-- Νέος
  SENSOR_COUNT
};
```

**2. Update το array:**
```cpp
SensorInfo sensors[SENSOR_COUNT] = {
  {"Temperature", "°C", true, false, 0.0, 0},
  {"Pressure", "hPa", true, false, 0.0, 0},
  {"Light", "lux", true, false, 0.0, 0},
  {"Soil Moisture", "%", true, false, 0.0, 0},
  {"CO2", "ppm", true, false, 0.0, 0}  // <-- Νέος
};
```

**3. Update το registry:**
```cpp
void updateSensorRegistry() {
  // ... existing sensors ...
  
  // CO2 sensor
  float co2 = readCO2Sensor();
  if (co2 > 0 && co2 < 5000) {
    sensors[SENSOR_CO2].available = true;
    sensors[SENSOR_CO2].lastValue = co2;
    sensors[SENSOR_CO2].lastRead = millis();
  } else {
    sensors[SENSOR_CO2].available = false;
  }
}
```

Αυτό θα δουλέψει αυτόματα με το cloud sync και το `/sensors` endpoint!

## Support

Για ερωτήσεις ή προβλήματα:
- GitHub: https://github.com/Mimisnak/SmartGreenhouse
- Email: contact@mimis.dev
