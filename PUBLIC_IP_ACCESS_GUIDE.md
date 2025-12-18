# 🌐 Οδηγός Πρόσβασης από Public IP

## 🎯 Τι Άλλαξε

Το Smart Greenhouse web interface τώρα υποστηρίζει **δύο λειτουργίες**:

### 1️⃣ **Local Mode** (Αυτόματο)
- Όταν ανοίγεις το `http://[ESP32_IP]` από το ίδιο το ESP32
- Χρησιμοποιεί relative URLs (`/api`, `/history`)
- **Δεν χρειάζεται configuration**

### 2️⃣ **Remote Mode** (GitHub Pages / Public IP)
- Όταν ανοίγεις από GitHub Pages ή άλλο server
- Χρειάζεται να ρυθμίσεις την ESP32 IP
- Υποστηρίζει public/external IPs

---

## 🚀 Πώς να Χρησιμοποιήσεις

### Βήμα 1: Deploy στο GitHub Pages

1. Push τα αρχεία `docs/` στο GitHub:
```bash
git add docs/
git commit -m "Update web interface with public IP support"
git push
```

2. Ενεργοποίησε GitHub Pages:
   - Settings → Pages
   - Source: `Deploy from branch`
   - Branch: `master` → Folder: `/docs`
   - Save

3. Περίμενε 1-2 λεπτά και άνοιξε:
   ```
   https://[YourUsername].github.io/SmartGreenhouse/
   ```

### Βήμα 2: Ρύθμισε Port Forwarding

Για να δουλέψει από το internet, πρέπει να κάνεις **port forwarding** στο router σου:

1. **Βρες την Local IP του ESP32**:
   - Άνοιξε Serial Monitor
   - Κοίτα το `IP Address: 192.168.x.x`

2. **Μπες στο Router**:
   - Συνήθως: `http://192.168.1.1`
   - Login (username/password συνήθως: `admin/admin`)

3. **Ρύθμισε Port Forwarding**:
   - Βρες: `Advanced → Port Forwarding` ή `NAT`
   - Προσθήκη κανόνα:
     ```
     Service Name: ESP32_Greenhouse
     External Port: 80
     Internal IP: 192.168.x.x (η IP του ESP32)
     Internal Port: 80
     Protocol: TCP
     ```

4. **Βρες την Public IP σου**:
   - Άνοιξε: https://whatismyipaddress.com/
   - Κράτα την IP (π.χ. `85.123.45.67`)

### Βήμα 3: Ρύθμισε την IP στο Web Interface

1. Άνοιξε το GitHub Pages link
2. Θα δεις ένα **κίτρινο panel** που λέει "ESP32 Configuration"
3. Γράψε την Public IP σου: `85.123.45.67`
4. Πάτα **💾 Save IP**
5. Το interface θα συνδεθεί αυτόματα!

---

## 🔧 Τεχνικές Λεπτομέρειες

### Auto-Detection Logic

```javascript
// Detect if running locally or remotely
isLocalMode = hostname.match(/^\d+\.\d+\.\d+\.\d+$/) || hostname === 'localhost'

if (isLocalMode) {
    ESP32_BASE_URL = ''  // Use relative URLs
} else {
    ESP32_BASE_URL = 'http://[saved_ip]'  // Use absolute URLs
}
```

### Fetch Calls Update

Όλα τα API calls τώρα χρησιμοποιούν την `ESP32_BASE_URL`:

```javascript
// Before
fetch('/api')
fetch('/history')

// After
fetch(ESP32_BASE_URL + '/api')
fetch(ESP32_BASE_URL + '/history')
```

### LocalStorage

Η IP αποθηκεύεται στο browser με `localStorage`:
- **Key**: `esp32_ip`
- **Value**: `192.168.1.100` ή `85.123.45.67`

---

## 🛠️ Troubleshooting

### ❌ "Failed to load historical data"

**Πιθανές αιτίες:**
1. Λάθος IP address
2. ESP32 offline
3. Port forwarding δεν δουλεύει

**Λύση:**
```bash
# Test από PowerShell
Test-NetConnection -ComputerName [your_public_ip] -Port 80

# Ή curl
curl http://[your_public_ip]/api
```

### ❌ CORS Errors

Αν δεις errors σαν:
```
Access to fetch blocked by CORS policy
```

**Λύση:** Πρέπει να προσθέσεις CORS headers στο ESP32 server:

```cpp
// Στο main.cpp
server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request){
    // Add CORS headers
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonData);
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(response);
});
```

### ❌ Δεν φορτώνει δεδομένα

1. **Check Console (F12)**:
   - Δες για errors
   - Κοίτα τα fetch requests

2. **Verify IP**:
   - Πάτα **🗑️ Clear** και ξαναγράψε την IP

3. **Test Local First**:
   - Δοκίμασε πρώτα από το ίδιο WiFi
   - Αν δουλεύει local αλλά όχι public → port forwarding issue

---

## 📊 Πλεονεκτήματα

✅ **Dual Mode Support**:
   - Λειτουργεί και local και remote
   - Auto-detection χωρίς manual config

✅ **User-Friendly**:
   - Visual configuration panel
   - LocalStorage για persistence
   - Clear error messages

✅ **Flexible**:
   - Public IPs
   - Dynamic DNS (e.g., `myhouse.dyndns.org`)
   - Port customization (e.g., `192.168.1.100:8080`)

---

## 🔐 Security Note

⚠️ **ΠΡΟΣΟΧΗ**: Όταν κάνεις port forwarding, το ESP32 είναι **προσβάσιμο από το internet**.

**Προτάσεις ασφαλείας**:
1. Άλλαξε το default WiFi password
2. Πρόσθεσε authentication στο web server
3. Χρησιμοποίησε HTTPS (με reverse proxy)
4. Περιόρισε τα IPs που μπορούν να συνδεθούν (router firewall)

---

## 📱 Mobile Access

Το interface είναι **responsive** και δουλεύει τέλεια σε κινητά!

Μπορείς να:
- Προσθέσεις στην Home Screen (Add to Home Screen)
- Παρακολουθείς real-time από οπουδήποτε
- Εξάγεις δεδομένα (CSV)

---

## 🎉 Επόμενα Βήματα

1. ✅ Deploy στο GitHub Pages
2. ✅ Ρύθμισε Port Forwarding
3. ✅ Δοκίμασε από mobile
4. ⭐ Προσθήκη authentication (optional)
5. 🔐 HTTPS με Let's Encrypt (optional)

---

**Καλή επιτυχία!** 🚀🌱

Αν έχεις προβλήματα, check το browser console (F12) για debug info.
