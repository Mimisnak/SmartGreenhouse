# 📁 Project Structure & File Organization

## 🎯 Τρέχουσα Κατάσταση του Project

### ✅ Ενεργά Αρχεία (Χρησιμοποιούνται)

#### 📌 Κύριος Κώδικας
- **`src/main.cpp`** - Ο ΤΡΕΧΩΝ κώδικας με BH1750 (light sensor)  
  ⚠️ **ΜΗΝ ΧΡΗΣΙΜΟΠΟΙΗΣΕΙΣ** - Περιέχει κώδικα για καμένο αισθητήρα φωτός

- **`src/main_clean.cpp`** - Ο ΝΕΟΣ καθαρός κώδικας  
  ✅ **ΧΡΗΣΙΜΟΠΟΙΗΣΕ ΑΥΤΟΝ** - Χωρίς BH1750, με καθαρό code

- **`src/html_dashboard.h`** - HTML για το web dashboard  
  ✅ Χρησιμοποιείται από το main_clean.cpp

#### 📝 Configuration
- **`platformio.ini`** - PlatformIO configuration
- **`.gitignore`** - Git ignore rules

#### 📚 Documentation
- **`README.md`** - Κύριο documentation
- **`WIRING_GUIDE.md`** - Οδηγίες συνδεσμολογίας
- **`FIREBASE_SETUP.md`** - Firebase setup
- **`FIREBASE_SECURITY_RULES.md`** - Security rules οδηγίες
- **`FIREBASE_KEY_SECURITY.md`** - Key management guide
- **`firebase.rules.json`** - Firebase rules αρχείο

---

### ❌ Άχρηστα Αρχεία (Για Διαγραφή ή Archive)

#### src/
- `src/main_commented.cpp` - Παλιό αρχείο με comments
- `src/main_fixed.cpp` - Παλιό debug version
- `src/main_test.cpp` - Test version
- `src/main_working.cpp` - Backup version

**Πρόταση:** Μετακίνησε τα σε `src/archive/old_versions/`

#### Documentation (Διπλά)
- `ΑΡΧΕΙΑ_ΠΛΗΡΟΦΟΡΙΕΣ.md` - Ελληνικό documentation (διπλό)
- `ΔΟΜΗ_ΦΑΚΕΛΩΝ.md` - Ελληνικό structure guide (διπλό)
- `CONFIGURATION_GUIDE.md` - Covered by other docs
- `DEPLOYMENT_GUIDE.md` - Covered by README
- `SENSOR_REGISTRY_GUIDE.md` - Not used anymore
- `GITHUB_PAGES_GUIDE.md` - Not relevant
- `README_MANUAL.md` - Duplicate

**Πρόταση:** Μετακίνησε τα σε `docs/archive/`

---

## 🗂️ Προτεινόμενη Οργάνωση

```
thermokrasia/
├── src/
│   ├── main.cpp                    ← ΑΝΤΙΚΑΤΕΣΤΗΣΕ με main_clean.cpp
│   ├── html_dashboard.h            ← HTML για dashboard
│   └── archive/
│       ├── old_versions/
│       │   ├── main_commented.cpp
│       │   ├── main_fixed.cpp
│       │   ├── main_test.cpp
│       │   └── main_working.cpp
│       └── with_light_sensor/
│           └── main_bh1750.cpp     ← Backup του παλιού main.cpp
│
├── individual_sensors/
│   ├── README.md
│   ├── bmp280_standalone.cpp       ← Test BMP280
│   ├── soil_moisture_standalone.cpp ← Test Soil
│   ├── i2c_scanner.cpp             ← I2C debugging
│   └── bh1750_standalone.cpp       ← Για όταν πάρεις νέο αισθητήρα
│
├── backup/
│   └── (automated backups)
│
├── docs/
│   ├── index.html                  ← GitHub Pages
│   └── archive/
│       ├── old_guides/
│       └── greek_versions/
│
├── data/
│   ├── index.html                  ← SPIFFS version (optional)
│   ├── script.js
│   └── style.css
│
├── platformio.ini
├── .gitignore
│
├── README.md                       ← Main documentation
├── WIRING_GUIDE.md
├── FIREBASE_SETUP.md
├── FIREBASE_SECURITY_RULES.md
├── FIREBASE_KEY_SECURITY.md
└── firebase.rules.json
```

---

## 🔧 Sensors Status

### ✅ Ενεργοί Αισθητήρες
1. **BMP280** - Θερμοκρασία & Πίεση  
   - I2C Address: 0x76 or 0x77
   - SDA: GPIO 16, SCL: GPIO 17
   - Status: **WORKING** ✅

2. **Soil Moisture (Capacitive)** - Υγρασία Εδάφους  
   - Pin: GPIO 4 (ADC1)
   - Calibration: DRY=3285, WET=27
   - Status: **WORKING** ✅

### ❌ Ανενεργοί Αισθητήρες
1. **BH1750** - Light Sensor  
   - Status: **ΚΑΜΕΝΟΣ** 🔥
   - Action: Αφαιρέθηκε από κώδικα
   - Backup: `src/archive/with_light_sensor/main_bh1750.cpp`
   - Future: Παράγγειλε νέο → χρησιμοποίησε `individual_sensors/bh1750_standalone.cpp`

---

## 📋 Action Items

### Άμεσα (Τώρα)
1. ✅ Αντικατάστησε `src/main.cpp` με `src/main_clean.cpp`
2. ✅ Διάβασε το `FIREBASE_KEY_SECURITY.md`
3. ✅ Revoke το Database Secret token από Firebase Console
4. ✅ Ενημέρωσε τα Firebase Security Rules

### Μεσοπρόθεσμα
1. Μετακίνησε παλιά αρχεία στα archives
2. Καθάρισε duplicate documentation
3. Test το σύστημα με τον νέο κώδικα
4. Backup τη βάση Firebase

### Μακροπρόθεσμα
1. Όταν πάρεις νέο BH1750:
   - Χρησιμοποίησε `individual_sensors/bh1750_standalone.cpp` για test
   - Ενσωμάτωσε στο main.cpp
   - Ενημέρωσε το dashboard HTML

---

## 🚀 Quick Start (Μετά τον καθαρισμό)

```bash
# 1. Backup current state
cd thermokrasia
git add .
git commit -m "backup before cleanup"

# 2. Replace main.cpp
cp src/main_clean.cpp src/main.cpp

# 3. Build & Upload
platformio run --target upload

# 4. Monitor
platformio device monitor
```

---

## 📊 File Sizes (Before Cleanup)

```
src/
├── main.cpp               (~25 KB) - με BH1750
├── main_clean.cpp         (~18 KB) - χωρίς BH1750
├── main_commented.cpp     (~30 KB)
├── main_fixed.cpp         (~22 KB)
├── main_test.cpp          (~20 KB)
└── main_working.cpp       (~24 KB)
```

**Total src/: ~139 KB**  
**Μετά cleanup: ~18 KB** (87% reduction!)

---

## ⚠️ Important Notes

1. **ΜΗΝ διαγράψεις** το `backup/` folder - περιέχει working snapshots
2. **ΜΗΝ ανεβάσεις** το Database Secret token στο GitHub
3. **ΠΑΝΤΑ κάνε backup** πριν μεγάλες αλλαγές
4. **Έλεγξε το Firebase Console** μετά κάθε update

---

## 🆘 Troubleshooting

### Αν κάτι πάει στραβά:
1. Restore από `backup/` folder
2. Χρησιμοποίησε το `src/archive/with_light_sensor/main_bh1750.cpp`
3. Έλεγξε το Serial Monitor για errors
4. Ελένξε το Firebase Console για rules/errors

---

## ✅ Checklist Before Upload

- [ ] Διαβάστηκε το FIREBASE_KEY_SECURITY.md
- [ ] Revoke το Database Secret token
- [ ] Ενημερώθηκαν τα Firebase Security Rules
- [ ] Αντικαταστάθηκε το src/main.cpp
- [ ] Build test: `platformio run`
- [ ] Έλεγχος errors: `get_errors`
- [ ] Git commit του καθαρού κώδικα
