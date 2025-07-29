# 🌐 GitHub Pages Setup - Βήμα προς Βήμα

## 📋 Προαπαιτούμενα
- ✅ GitHub account
- ✅ ESP32-S3 με τη web έκδοση του κώδικα
- ✅ Wi-Fi σύνδεση

---

## 🚀 Βήμα 1: GitHub Repository Setup

### 1.1 Δημιουργία Repository
1. Μπείτε στο **GitHub.com**
2. Κλικ στο **"New repository"** (πράσινο κουμπί)
3. Repository name: `thermokrasia` ή `esp32-thermometer`
4. Κάντε το **Public** (απαραίτητο για GitHub Pages)
5. ✅ Τσεκάρετε **"Add a README file"**
6. Κλικ **"Create repository"**

### 1.2 Upload αρχείων
1. Κλικ **"uploading an existing file"**
2. Drag & drop τα αρχεία:
   - `index.html` (το dashboard)
   - `README_WEB.md`
   - `WIRING_GUIDE.md` (προαιρετικά)
3. Commit message: "Add ESP32-S3 Web Dashboard"
4. Κλικ **"Commit changes"**

---

## 🔧 Βήμα 2: GitHub Pages Activation

### 2.1 Settings
1. Στο repository, κλικ **"Settings"** (τελευταία καρτέλα)
2. Scroll down στην αριστερή μπάρα
3. Κλικ **"Pages"**

### 2.2 Configuration
1. **Source:** Deploy from a branch
2. **Branch:** main
3. **Folder:** / (root)
4. Κλικ **"Save"**

### 2.3 Περιμένετε deployment
- ⏱️ 2-5 λεπτά για πρώτη φορά
- Θα εμφανιστεί: **"Your site is published at..."**
- URL format: `https://yourusername.github.io/thermokrasia`

---

## 📱 Βήμα 3: ESP32-S3 Setup

### 3.1 Wi-Fi Configuration
1. Ανοίξτε `src/main.cpp`
2. Βρείτε τις γραμμές:
   ```cpp
   const char* ssid = "YOUR_WIFI_NAME";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```
3. Αλλάξτε με τα δικά σας στοιχεία:
   ```cpp
   const char* ssid = "WIND_FIBER_123";
   const char* password = "mypassword123";
   ```

### 3.2 Upload κώδικα
1. Συνδέστε ESP32-S3 στο PC
2. Τρέξτε: `pio run --target upload`
3. Περιμένετε upload completion

### 3.3 Βρείτε το IP Address
1. Ανοίξτε Serial Monitor: `pio device monitor`
2. Αναζητήστε:
   ```
   ✅ Wi-Fi συνδέθηκε επιτυχώς!
   🌐 IP Address: 192.168.1.100
   ```
3. **Σημειώστε το IP address!**

---

## 🌐 Βήμα 4: Σύνδεση & Τεστ

### 4.1 Τοπική πρόσβαση (ίδιο Wi-Fi)
1. Στον browser: `http://192.168.1.100` (το IP σας)
2. Θα δείτε τη θερμοκρασία αμέσως!

### 4.2 Remote πρόσβαση (GitHub Pages)
1. Ανοίξτε: `https://yourusername.github.io/thermokrasia`
2. Εισάγετε το IP: `192.168.1.100`
3. Κλικ **"Σύνδεση"**
4. 🎉 **Success!** Βλέπετε τα δεδομένα από οπουδήποτε!

---

## 🛠️ Troubleshooting

### ❌ GitHub Pages δεν λειτουργεί
**Πρόβλημα:** `404 Not Found`
**Λύσεις:**
- Βεβαιωθείτε το repository είναι **Public**
- Ελέγξτε ότι το `index.html` είναι στο root folder
- Περιμένετε 5-10 λεπτά για propagation
- Check: Settings → Pages → URL είναι σωστό

### ❌ ESP32-S3 δεν συνδέεται
**Πρόβλημα:** `❌ Αποτυχία σύνδεσης Wi-Fi!`
**Λύσεις:**
- Ελέγξτε το Wi-Fi όνομα (case-sensitive)
- Ελέγξτε τον κωδικό (special characters)
- Βεβαιωθείτε για 2.4GHz Wi-Fi (όχι 5GHz μόνο)
- Δοκιμάστε hotspot από το κινητό

### ❌ Dashboard δεν συνδέεται στο ESP32-S3
**Πρόβλημα:** `❌ Σφάλμα σύνδεσης`
**Αιτίες & Λύσεις:**

#### 🏠 Τοπική χρήση (ίδιο Wi-Fi)
- ✅ **Πρώτα δοκιμάστε:** Άμεση πρόσβαση `http://ESP32_IP`
- ✅ **IP address:** Βεβαιωθείτε είναι σωστό (check Serial Monitor)
- ✅ **Ping test:** `ping 192.168.1.100` από command prompt

#### 🌍 Remote πρόσβαση (διαφορετικό δίκτυο)
**Προχωρημένο setup - απαιτεί router configuration:**

1. **Port Forwarding:**
   - Router admin panel (συνήθως 192.168.1.1)
   - Forward port 80 → ESP32-S3 IP
   - Χρησιμοποιήστε εξωτερικό IP αντί για local

2. **Dynamic DNS (προαιρετικά):**
   - Χρησιμοποιήστε service όπως No-IP
   - Για σταθερό domain name

### ❌ Λάθος θερμοκρασία (179°C)
**Πρόβλημα:** I2C communication errors
**Λύσεις:**
- Ελέγξτε συνδέσεις SDA/SCL
- Μην αγγίζετε καλώδια κατά λειτουργία
- Κοντύτερα καλώδια
- Restart ESP32-S3

---

## 🎯 Best Practices

### 🔒 Security
- **Local Use:** Προτιμήστε local access (πιο ασφαλές)
- **Router Settings:** Disable WPS, enable WPA3
- **Guest Network:** Χρησιμοποιήστε guest Wi-Fi για IoT devices

### ⚡ Performance  
- **Wi-Fi Signal:** Κοντά στο router για καλύτερο signal
- **Update Frequency:** 2 δευτερόλεπτα είναι optimal
- **Browser Cache:** Κάντε refresh αν δεν ενημερώνεται

### 📱 Mobile Access
- **Bookmark:** Προσθέστε στο home screen (PWA-like)
- **Auto-refresh:** Το dashboard ανανεώνεται αυτόματα
- **Offline:** Η local πρόσβαση δουλεύει χωρίς internet

---

## 🚀 Advanced Features (μελλοντικά)

### 📊 Data Logging
```cpp
// Προσθήκη SD card logging
#include <SD.h>
void logToSD(float temp, float pressure) {
  File file = SD.open("/data.csv", FILE_APPEND);
  file.printf("%.2f,%.2f,%lu\n", temp, pressure, millis());
  file.close();
}
```

### 📧 Email Alerts
```cpp
// Προσθήκη email notifications
if (temperature > 30.0) {
  sendEmail("High temperature alert: " + String(temperature) + "°C");
}
```

### 📈 Charts
```javascript
// Chart.js integration στο dashboard
const chart = new Chart(ctx, {
  type: 'line',
  data: temperatureData,
  options: chartOptions
});
```

---

## 🏆 Επιτυχία!

Τώρα έχετε ένα **professional IoT θερμόμετρο** με:

✅ **Real-time monitoring** από οπουδήποτε
✅ **Professional web interface** 
✅ **Mobile-friendly design**
✅ **Greek language support**
✅ **Automatic reconnection**
✅ **GitHub Pages hosting**

**Απολαύστε τη νέα σας smart θερμοκρασία!** 🌡️🎉

---

### 📞 Support
Αν χρειάζεστε βοήθεια:
1. Ελέγξτε το Serial Monitor για errors
2. Δοκιμάστε local access πρώτα  
3. Restart ESP32-S3 σε περίπτωση προβλημάτων
4. GitHub Issues για bug reports
