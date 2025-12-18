# 🚀 QUICK START - Upload & Test

## ✅ Build Successful!
```
RAM:   16.7% (54684 bytes / 327680 bytes)
Flash: 35.1% (1172025 bytes / 3342336 bytes)
Status: ✅ SUCCESS
```

## 📤 Upload Steps

### 1. Connect ESP32
Σύνδεσε το ESP32-S3 στο USB

### 2. Upload Firmware
```powershell
cd "C:\Users\mimis\Desktop\PTIXIAKI ERGASIA\thermokrasia"
pio run --target upload
```

### 3. Monitor Serial Output
```powershell
pio device monitor
```

## 🔍 Τι να Ψάξεις στο Serial Monitor

### ✅ NTP Sync (ΣΗΜΑΝΤΙΚΟ!)
```
Initializing NTP time...
Time synchronized with NTP server
Current time: 2025-12-17 18:30:45
```

### ✅ History Recording
```
📊 History added: 1/288 @ 18:30:00 | Temp: 24.5°C (Min: 24.5, Max: 24.5)
📊 History added: 2/288 @ 18:35:00 | Temp: 24.6°C (Min: 24.5, Max: 24.6)
```

### ✅ Firebase Sync
```
📅 Timestamp: 2025-12-17 18:30:00 (1734453000 ms)
✅ Data sent to Firebase successfully
📊 History saved (#1)
```

## 🌐 Test στο Browser

### 1. Βρες το IP του ESP32
Από το Serial Monitor:
```
WiFi connected!
IP address: 192.168.1.XXX
```

### 2. Άνοιξε στο Browser
```
http://192.168.1.XXX/
```

### 3. Άνοιξε Developer Console (F12)
Θα δεις:
```
🔄 Loading historical data from ESP32...
📦 Raw history data received: {temperature: [...], timestamps: [...]}
📅 Sample #0: Unix=1734453000, Date=17/12/2025, 18:30:00
✅ Loaded 12 historical data points
📅 Time range: 17/12/2025, 17:30:00 to 18:30:00
📊 Charts updated with 12 data points
```

### 4. Έλεγξε τα Γραφήματα
- ✅ X-axis: Πραγματικές ώρες (17:30, 17:35, 17:40...)
- ✅ Tooltips: Hover για λεπτομέρειες
- ✅ Auto-update: Νέα data κάθε 5 δευτερόλεπτα
- ✅ History reload: Κάθε 5 λεπτά

## 🐛 Troubleshooting

### ⚠️ Αν δεν βλέπεις timestamps
1. Έλεγξε NTP sync στο Serial Monitor
2. Βεβαιώσου ότι το WiFi είναι συνδεδεμένο
3. Περίμενε 5-10 λεπτά για πρώτες καταγραφές

### ⚠️ Αν τα γραφήματα είναι άδεια
1. Περίμενε 5 λεπτά (HISTORY_INTERVAL)
2. Κάνε refresh τη σελίδα
3. Έλεγξε το `/history` endpoint: `http://192.168.1.XXX/history`

### ⚠️ Αν το NTP δεν συγχρονίζει
1. Έλεγξε το WiFi connection
2. Δοκίμασε άλλο NTP server στο `main.cpp`:
```cpp
const char* ntpServer = "time.google.com";  // Alternative
// const char* ntpServer = "0.gr.pool.ntp.org";  // Greece
```

## 🎯 Τι Άλλαξε (Summary)

### ESP32 Side
- ✅ **Unix timestamps** αντί για millis()
- ✅ **NTP sync** για πραγματική ώρα
- ✅ **Debug logs** με αναγνώσιμες ώρες

### Browser Side
- ✅ **Smart timestamp parsing** από Unix timestamps
- ✅ **Auto-reload history** κάθε 5 λεπτά
- ✅ **Better charts** με axis labels & tooltips
- ✅ **Debug mode** για troubleshooting

## 📊 Expected Behavior

```
Minute 0:  ESP32 boots → NTP sync → WiFi connected
Minute 5:  First history entry → 📊 @ 18:30:00
Minute 10: Second entry → 📊 @ 18:35:00
...
Hour 1:    12 entries (288 max for 24h)
```

## 🎉 Success Indicators

✅ Serial Monitor shows **real timestamps**  
✅ Browser console shows **parsed Unix timestamps**  
✅ Charts show **correct times** on X-axis  
✅ History **persists** after page refresh  
✅ Firebase shows **timestampStr** with readable dates  

---

**Εάν όλα αυτά δουλεύουν = 100% SUCCESS! 🚀**

Για οποιαδήποτε ερώτηση, άνοιξε issue ή check το TIMESTAMP_FIX_2025-12-17.md
