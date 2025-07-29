# 🌡️ ESP32-S3 Θερμόμετρο με Web Interface

Ένα ολοκληρωμένο σύστημα παρακολούθησης θερμοκρασίας που συνδυάζει:
- **ESP32-S3 N16R8** microcontroller
- **BMP280** αισθητήρας θερμοκρασίας/πίεσης  
- **Web Server** για πρόσβαση από browser
- **GitHub Pages** dashboard για παρακολούθηση

## 🚀 Χαρακτηριστικά

### ESP32-S3 Web Server
- ✅ Wi-Fi connectivity
- ✅ Real-time JSON API
- ✅ Responsive web interface
- ✅ CORS support για external access
- ✅ Automatic sensor validation
- ✅ Error handling & recovery

### GitHub Pages Dashboard  
- ✅ Professional responsive design
- ✅ Real-time data updates
- ✅ Connection status monitoring
- ✅ Greek language interface
- ✅ Mobile-friendly layout
- ✅ Auto-reconnection functionality

## 📁 Δομή Project

```
thermokrasia/
├── src/
│   ├── main.cpp           # Αρχικός κώδικας (μόνο Serial)
│   └── main_web.cpp       # Νέος κώδικας με Web Server
├── index.html             # GitHub Pages Dashboard
├── platformio.ini         # PlatformIO configuration
├── WIRING_GUIDE.md       # Οδηγίες σύνδεσης hardware
└── README_WEB.md         # Αυτό το αρχείο
```

## 🔧 Setup Instructions

### Βήμα 1: Hardware Setup
Ακολουθήστε τις οδηγίες στο `WIRING_GUIDE.md`:
- ESP32-S3 N16R8
- BMP280 sensor (I2C στα GPIO 8 & 9)
- Wi-Fi network access

### Βήμα 2: ESP32-S3 Programming

1. **Αντιγράψτε τον κώδικα:**
   ```bash
   cp src/main_web.cpp src/main.cpp
   ```

2. **Ενημερώστε Wi-Fi credentials:**
   Στο αρχείο `src/main.cpp`, αλλάξτε:
   ```cpp
   const char* ssid = "YOUR_WIFI_NAME";        // Το όνομα του Wi-Fi σας
   const char* password = "YOUR_WIFI_PASSWORD"; // Τον κωδικό του Wi-Fi σας
   ```

3. **Upload στο ESP32-S3:**
   ```bash
   pio run --target upload
   ```

4. **Monitor για IP address:**
   ```bash
   pio device monitor
   ```
   Βρείτε το IP address στο Serial Monitor, π.χ. `192.168.1.100`

### Βήμα 3: GitHub Pages Setup

1. **Fork/Clone αυτό το repository**

2. **Enable GitHub Pages:**
   - Settings → Pages
   - Source: Deploy from a branch
   - Branch: main
   - Folder: / (root)

3. **Access Dashboard:**
   Μετά από λίγα λεπτά: `https://yourusername.github.io/thermokrasia`

## 🌐 Χρήση

### Τοπική πρόσβαση (ίδιο Wi-Fi)
1. Βρείτε το IP του ESP32-S3 στο Serial Monitor
2. Ανοίξτε browser: `http://192.168.1.100` (example IP)
3. Βλέπετε real-time δεδομένα αμέσως

### Remote πρόσβαση (GitHub Pages)
1. Ανοίξτε το GitHub Pages link
2. Εισάγετε το IP του ESP32-S3
3. Κλικ "Σύνδεση"
4. Παρακολουθείτε δεδομένα από οπουδήποτε!

## 📊 API Endpoints

### GET `/api`
Επιστρέφει JSON με δεδομένα αισθητήρα:
```json
{
  "temperature": 23.45,
  "pressure": 1013.25,
  "timestamp": 12345678,
  "sensor_status": "OK",
  "uptime": 3600,
  "device": "ESP32-S3",
  "sensor": "BMP280"
}
```

### GET `/`
Επιστρέφει built-in web interface

## 🛠️ Troubleshooting

### ESP32-S3 δεν συνδέεται στο Wi-Fi
```
❌ Αποτυχία σύνδεσης Wi-Fi!
```
**Λύσεις:**
- Ελέγξτε το όνομα Wi-Fi (ssid)
- Ελέγξτε τον κωδικό Wi-Fi (password)  
- Βεβαιωθείτε ότι το Wi-Fi είναι 2.4GHz (όχι 5GHz μόνο)
- Ελέγξτε εάν το Wi-Fi έχει guest isolation

### GitHub Pages δεν μπορεί να συνδεθεί
```
❌ Σφάλμα σύνδεσης
```
**Αιτίες:**
- **CORS Issues:** Το ESP32-S3 έχει built-in CORS support
- **Different Networks:** Το ESP32-S3 πρέπει να είναι προσβάσιμο από το internet
- **Firewall:** Router firewall μπορεί να μπλοκάρει external connections

**Λύσεις:**
1. **Port Forwarding:** Στο router, forward το port 80 στο ESP32-S3 IP
2. **Local Use:** Χρησιμοποιήστε το `index.html` τοπικά (file://)
3. **VPN:** Χρησιμοποιήστε VPN για secure remote access

### Λάθος μετρήσεις θερμοκρασίας
```
⚠️ ΑΝΙΧΝΕΥΣΗ ΛΑΘΟΣ ΜΕΤΡΗΣΗΣ!
Θερμοκρασία: 179.19 °C
```
**Λύσεις:**
- Ελέγξτε I2C συνδέσεις (SDA/SCL)
- Μην αγγίζετε τα καλώδια κατά τη λειτουργία
- Χρησιμοποιήστε κοντύτερα καλώδια
- Προσθέστε pull-up resistors 4.7kΩ (προαιρετικά)

## 📱 Features του Dashboard

### Real-time Updates
- ⏱️ Ενημέρωση κάθε 2 δευτερόλεπτα
- 🔄 Auto-reconnection σε περίπτωση αποσύνδεσης
- 📊 Live status indicators

### Responsive Design
- 📱 Mobile-friendly interface
- 🎨 Professional gradient design
- 🌡️ Large, readable temperature display
- 📈 Chart placeholder για μελλοντικές εκδόσεις

### Greek Language Support
- 🇬🇷 Πλήρης υποστήριξη ελληνικών
- 📅 Ελληνική μορφοποίηση ημερομηνίας/ώρας
- 💬 Ελληνικά error messages

## 🔒 Security Notes

### Local Network Only (Προτεινόμενο)
- Χρησιμοποιήστε το ESP32-S3 μόνο στο τοπικό δίκτυο
- Access μέσω local IP (192.168.x.x)
- GitHub Pages για monitoring interface

### Internet Access (Προχωρημένο)
- Port forwarding στο router (port 80)
- HTTPS reverse proxy (nginx/cloudflare)
- Authentication layer (basic auth)

## 📈 Μελλοντικές Βελτιώσεις

- [ ] 📊 Chart.js γραφήματα θερμοκρασίας
- [ ] 💾 Data logging σε SD card
- [ ] 📧 Email alerts για extreme temperatures
- [ ] 🔔 Push notifications
- [ ] 📱 Progressive Web App (PWA)
- [ ] 🌍 Multiple sensor support
- [ ] 📊 Historical data export

## 🆘 Support

Αν αντιμετωπίζετε προβλήματα:

1. **Ελέγξτε το Serial Monitor** για debug messages
2. **Δοκιμάστε τοπική πρόσβαση** πρώτα (`http://ESP32_IP`)
3. **Ελέγξτε τις συνδέσεις** BMP280 
4. **Restart** το ESP32-S3
5. **Ελέγξτε το Wi-Fi** signal strength

## 🏆 Πλεονεκτήματα

### Τοπικό vs Cloud
- ✅ **Πλήρης έλεγχος** των δεδομένων σας
- ✅ **Καμία εξάρτηση** από cloud services  
- ✅ **Γρήγορη απόκριση** (local network)
- ✅ **Δωρεάν** hosting (GitHub Pages)
- ✅ **Offline capable** (local web interface)

### ESP32-S3 vs άλλα microcontrollers
- ✅ **Dual-core** performance
- ✅ **Wi-Fi built-in** (δεν χρειάζεται external module)
- ✅ **Μεγάλη μνήμη** (8MB PSRAM)
- ✅ **Arduino IDE compatible**
- ✅ **Οικονομικό** (~$10-15)

Απολαύστε το νέο σας IoT θερμόμετρο! 🌡️🚀
