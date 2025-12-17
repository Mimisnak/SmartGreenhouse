# ⚡ ΓΡΗΓΟΡΕΣ ΕΝΕΡΓΕΙΕΣ - Smart Greenhouse

## 🔴 ΕΠΕΙΓΟΝΤΑ (Πριν το Upload)

### 1. Firebase Token Revoke (2 λεπτά)
```
https://console.firebase.google.com
→ smartgreenhouse-fb494  
→ ⚙️ Project Settings
→ Service accounts tab
→ Database secrets section
→ Βρες: 4JH5iFyDrTQxA6qR1x4iKiQNrCaAH3ZdrqMEpHOT
→ Πάτα DELETE ή REVOKE
```

### 2. Firebase Security Rules (3 λεπτά)
```
https://console.firebase.google.com
→ smartgreenhouse-fb494
→ Realtime Database (αριστερό menu)
→ Rules tab (πάνω)
→ Πάτα Edit
→ Αντικατάστησε όλο το περιεχόμενο με:
```

```json
{
  "rules": {
    ".read": false,
    ".write": false,
    "sensors": {
      "$deviceId": {
        ".read": true,
        ".write": "auth != null || $deviceId == 'ESP32-Greenhouse'",
        "latest": {
          ".validate": "newData.hasChildren(['timestamp', 'temperature', 'pressure'])"
        },
        "history": {
          "$timestamp": {
            ".validate": "newData.hasChildren(['timestamp', 'temperature', 'pressure'])"
          }
        },
        "stats": {
          "totalReadings": {
            ".validate": "newData.isNumber()"
          }
        }
      }
    }
  }
}
```

```
→ Πάτα PUBLISH (κόκκινο κουμπί)
```

---

## 🟡 ΠΡΟΑΙΡΕΤΙΚΑ (Αλλά Συνιστώμενα)

### 3. Clean Firebase Database (1 λεπτά)
```
Firebase Console → Realtime Database → Data tab
→ sensors/ESP32-Greenhouse/history/
→ Αν έχει >300 entries, διέγραψε τα παλιά
→ Κράτα μόνο τις τελευταίες 24 ώρες
```

### 4. Archive Old Files (5 λεπτά)
```powershell
cd "c:\Users\mimis\Desktop\PTIXIAKI ERGASIA\thermokrasia"

# Δημιούργησε folders
New-Item -ItemType Directory -Path "src\archive\old_versions" -Force
New-Item -ItemType Directory -Path "docs\archive" -Force

# Move old code versions
Move-Item "src\main_commented.cpp" "src\archive\old_versions\"
Move-Item "src\main_fixed.cpp" "src\archive\old_versions\"
Move-Item "src\main_test.cpp" "src\archive\old_versions\"
Move-Item "src\main_working.cpp" "src\archive\old_versions\"

# Move duplicate docs
Move-Item "ΑΡΧΕΙΑ_ΠΛΗΡΟΦΟΡΙΕΣ.md" "docs\archive\"
Move-Item "ΔΟΜΗ_ΦΑΚΕΛΩΝ.md" "docs\archive\"
Move-Item "CONFIGURATION_GUIDE.md" "docs\archive\"
```

---

## 🟢 BUILD & UPLOAD

### 5. Replace main.cpp
```powershell
# Έχει ήδη γίνει backup του παλιού main.cpp ✅
# Τώρα αντικατάστησε το:

cd "c:\Users\mimis\Desktop\PTIXIAKI ERGASIA\thermokrasia"
Copy-Item "src\main_clean.cpp" "src\main.cpp" -Force
```

### 6. Build Test
```powershell
platformio run
```

**Έλεγξε για errors. Αν δεις "SUCCESS" →  συνέχισε**

### 7. Upload
```powershell
platformio run --target upload
```

### 8. Monitor
```powershell
platformio device monitor --baud 115200
```

**Περίμενε να δεις:**
```
=== Smart Greenhouse System ===
I2C initialized: SDA=16, SCL=17
BMP280 OK
Soil sensor: OK
WiFi connected!
IP: 192.168.x.x
Firebase OK
Web server started
=== System Ready ===
```

---

## ✅ VERIFICATION

### 9. Test Dashboard
```
1. Από το Serial Monitor, βρες το IP
2. Άνοιξε browser: http://192.168.x.x/
3. Έλεγξε:
   ✓ Θερμοκρασία δείχνει τιμή (όχι --)
   ✓ Πίεση δείχνει τιμή
   ✓ Υγρασία εδάφους δείχνει τιμή
   ✓ System Info → Total Readings αυξάνεται
   ✓ Charts φορτώνουν
```

### 10. Test Firebase
```
Firebase Console → Realtime Database → Data
→ sensors/ESP32-Greenhouse/

Έλεγξε:
✓ latest/ - Ενημερώνεται κάθε 5 λεπτά
✓ history/ - Νέα entries προστίθενται
✓ stats/totalReadings - Αυξάνεται
✓ stats/minTemperature - Τιμή από 24h
✓ stats/maxTemperature - Τιμή από 24h
```

---

## 🔧 TROUBLESHOOTING

### Αν WiFi δεν συνδέεται:
```
Έλεγξε platformio.ini:
- SSID: "Vodafone-E79546683"
- Password: "RLg2s6b73EfarXRx"
```

### Αν BMP280 not found:
```
Serial Monitor θα δείξει: "ERROR: BMP280 not found!"
Έλεγξε:
- Καλώδια: SDA=GPIO16, SCL=GPIO17
- Τροφοδοσία: 3.3V (όχι 5V!)
- I2C address: 0x76 ή 0x77
```

### Αν Firebase δεν γράφει:
```
1. Έλεγξε Serial Monitor για: "Firebase ERROR: ..."
2. Επιβεβαίωσε ότι έκανες revoke το Database Secret
3. Επιβεβαίωσε ότι ενημέρωσες τα Rules
4. Έλεγξε WiFi connection
```

### Αν Dashboard δεν φορτώνει:
```
1. Δοκίμασε: http://IP/health
2. Θα δεις: "OK" + sensor status
3. Αν όχι → restart ESP32
```

---

## 📱 Quick Links

- **Firebase Console:** https://console.firebase.google.com
- **Project:** smartgreenhouse-fb494
- **GitHub Repo:** https://github.com/Mimisnak/SmartGreenhouse

---

## 📋 Checklist Ολοκλήρωσης

Tick όταν κάνεις κάθε βήμα:

- [ ] 1. Firebase Token Revoke
- [ ] 2. Firebase Security Rules Update
- [ ] 3. (Optional) Clean Firebase Database
- [ ] 4. (Optional) Archive Old Files
- [ ] 5. Replace main.cpp
- [ ] 6. Build Test (no errors)
- [ ] 7. Upload to ESP32
- [ ] 8. Monitor Serial Output
- [ ] 9. Test Dashboard (http://IP/)
- [ ] 10. Test Firebase Database

**Όταν όλα είναι ✓, είσαι έτοιμος!** 🎉

---

## 🆘 Αν κάτι πάει στραβά:

### RESTORE BACKUP:
```powershell
cd "c:\Users\mimis\Desktop\PTIXIAKI ERGASIA\thermokrasia"
Copy-Item "src\archive\main_with_bh1750_backup.cpp" "src\main.cpp" -Force
platformio run --target upload
```

---

**Χρόνος εκτέλεσης:** ~15 λεπτά συνολικά  
**Δυσκολία:** Εύκολο (copy-paste)  
**Αποτέλεσμα:** Καθαρό, ασφαλές, οργανωμένο project ✅
