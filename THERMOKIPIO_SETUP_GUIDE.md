# 🚀 ΘΕΡΜΟΚΗΠΙΟ - ΟΔΗΓΟΣ ΓΡΗΓΟΡΗΣ ΕΓΚΑΤΑΣΤΑΣΗΣ

## 📱 Workflow: Θερμοκήπιο → Remote Access

### Βήμα 1: Εγκατάσταση στο Θερμοκήπιο

1. **Πήγαινε στο θερμοκήπιο** με το laptop
2. **Συνδέσου στο WiFi** του θερμοκηπίου (το ίδιο που έχει ρυθμιστεί στο code)
3. **Συνδέσε το ESP32** με USB στο laptop

### Βήμα 2: Upload Code

**Option A: Με το Quick Deploy Script**
```cmd
Double-click: QUICK_DEPLOY.bat
Select: [1] Upload Code to ESP32
Wait: ~30 seconds
```

**Option B: Manual**
```powershell
cd "C:\Users\mimis\Desktop\PTIXIAKI ERGASIA\thermokrasia"
pio run --target upload
```

### Βήμα 3: Ελεγχος Local (στο θερμοκήπιο)

1. **Βρες Local IP** (από Serial Monitor):
   - Άνοιξε: `pio device monitor`
   - Κοίτα: `IP address: 192.168.x.x`

2. **Test στο browser**:
   ```
   http://192.168.x.x
   ```

3. **Επιβεβαίωση**:
   - ✅ Θερμοκρασία εμφανίζεται
   - ✅ Πίεση εμφανίζεται
   - ✅ Real-time updates

### Βήμα 4: Ενημέρωση Public IP

**Option A: Με το Script**
```cmd
Double-click: QUICK_DEPLOY.bat
Select: [2] Check Public IP
Copy η IP που εμφανίζεται
```

**Option B: Manual**
```powershell
# Από το laptop στο θερμοκήπιο
curl ifconfig.me
```

**Κράτα την IP**: π.χ. `79.166.243.145`

### Βήμα 5: Ενημέρωση Router (Αν άλλαξε η Local IP)

Αν η Local IP του ESP32 άλλαξε:

1. **Router Login**: `http://192.168.1.1`
2. **Port Forwarding**:
   ```
   External Port: 8083
   Internal IP: [η νέα local IP του ESP32]
   Internal Port: 80
   Protocol: TCP
   ```
3. **Save & Reboot**

---

## 🌍 Remote Access (από οπουδήποτε)

### Από Laptop/Desktop:

**Direct URL**:
```
http://79.166.243.145:8083
```

**Ή με Quick Deploy**:
```cmd
QUICK_DEPLOY.bat → [4] Open Web Interface (Remote)
Enter Public IP: 79.166.243.145
```

### Από Mobile (Smartphone):

1. **Άνοιξε Browser** (Chrome/Safari)
2. **Γράψε**:
   ```
   http://79.166.243.145:8083
   ```
3. **Add to Home Screen**:
   - Chrome: Menu → Add to Home screen
   - Safari: Share → Add to Home Screen
4. **Έχεις App!** 📱

---

## 🔄 Όταν Αλλάζει η Public IP

Η Public IP μπορεί να αλλάξει αν:
- Κλείσεις/ανοίξεις το router
- Ο ISP την ανανεώσει

**Λύση 1: Check Manually**
```cmd
QUICK_DEPLOY.bat → [2] Check Public IP
```

**Λύση 2: Dynamic DNS (Recommended)**

### Εγκατάσταση Dynamic DNS:

1. **Πήγαινε στο**: https://www.duckdns.org/
2. **Sign in** με GitHub
3. **Δημιούργησε domain**: `mimis-greenhouse` (free)
4. **Copy το token**

5. **Update στο ESP32 code** (μελλοντικά):
   ```cpp
   // Θα προσθέσουμε DuckDNS auto-update
   ```

6. **Μετά θα έχεις**:
   ```
   http://mimis-greenhouse.duckdns.org:8083
   ```
   (Η IP ανανεώνεται αυτόματα!)

---

## 🧪 Testing Connection

**Test από Laptop**:
```cmd
QUICK_DEPLOY.bat → [5] Test Connection
Enter IP: 79.166.243.145:8083
```

**Test από PowerShell**:
```powershell
curl http://79.166.243.145:8083/api
```

**Expected Response**:
```json
{
  "temperature": 22.5,
  "pressure": 1016.8,
  "light": -1,
  "soil": 0
}
```

---

## 📊 Troubleshooting

### ❌ "Cannot connect" από remote

**Check 1: Port Forwarding**
- Router settings → Port Forwarding
- Rule enabled για port 8083

**Check 2: ESP32 Online**
```powershell
# Από local network
ping 192.168.2.20
```

**Check 3: Firewall**
- Windows Firewall → Allow port 8083

### ❌ "Mixed Content" errors

**Λύση**: Μην χρησιμοποιείς GitHub Pages, μπες **απευθείας**:
```
http://79.166.243.145:8083
```

### ❌ Slow response

**Αιτία**: Internet speed
**Λύση**: Μείωσε το refresh rate στο web interface

---

## 🔜 Μελλοντικές Προσθήκες

### 1. Bluetooth Support

Για local access χωρίς WiFi:
```cpp
// ESP32 Bluetooth Serial
// Θα προσθέσουμε mobile app που συνδέεται με BLE
```

### 2. DuckDNS Auto-Update

Αυτόματη ανανέωση DNS:
```cpp
// HTTP request to DuckDNS API κάθε 5 λεπτά
```

### 3. HTTPS Support

Με Let's Encrypt:
```
Raspberry Pi → Nginx → ESP32
```

### 4. SMS Alerts

Ειδοποιήσεις σε κινητό:
```cpp
// Twilio API για SMS
```

---

## 📝 Quick Reference Card

**Κράτα αυτό το χαρτί:**

```
┌────────────────────────────────────┐
│   SMART GREENHOUSE ACCESS          │
├────────────────────────────────────┤
│ Local (θερμοκήπιο):                │
│   http://192.168.2.20              │
│                                    │
│ Remote (από παντού):               │
│   http://79.166.243.145:8083       │
│                                    │
│ Quick Deploy:                      │
│   QUICK_DEPLOY.bat                 │
│                                    │
│ Check Public IP:                   │
│   curl ifconfig.me                 │
└────────────────────────────────────┘
```

**Save στο mobile σου!** 📱

---

## ✅ Checklist

### Πριν φύγεις από το θερμοκήπιο:

- [ ] ESP32 upload επιτυχές
- [ ] Local access δουλεύει (http://192.168.2.x)
- [ ] Port forwarding ενημερωμένο
- [ ] Public IP σημειωμένη
- [ ] Test από mobile (4G) OK

### Για Remote Access:

- [ ] Browser: `http://[public_ip]:8083`
- [ ] Mobile: Add to Home Screen
- [ ] Bookmark την σελίδα

---

**Ready! Πήγαινε στο θερμοκήπιο!** 🌱🚀
