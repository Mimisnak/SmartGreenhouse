# 📡 GitHub Pages Setup - SmartGreenhouse

Αυτό το αρχείο σου δίνει οδηγίες για να ρυθμίσεις το GitHub Pages και να έχεις **απομακρυσμένη πρόσβαση** στο ESP32-S3 σου από οπουδήποτε στον κόσμο!

## 🌍 Πώς Λειτουργεί

1. **ESP32-S3** → Τοπικό δίκτυο (π.χ. 192.168.2.19:80)
2. **Router Port Forwarding** → Εξωτερική πόρτα (π.χ. 8080) → ESP32-S3
3. **GitHub Pages** → Στατική ιστοσελίδα που συνδέεται στο public IP σου

## 🔧 Βήμα 1: Ρύθμιση Router (Port Forwarding)

### Μπες στο Router Admin Panel:
- Άνοιξε browser και πήγαινε σε: `192.168.1.1` ή `192.168.2.1`
- Μπες με username/password (συνήθως: admin/admin ή admin/password)

### Βρες το Port Forwarding:
- Ψάξε για "Port Forwarding", "Virtual Server", ή "NAT"
- Δημιούργησε **νέο κανόνα**:

```
Service Name: ESP32-SmartGreenhouse
External Port: 8080 (ή οποιαδήποτε πόρτα θέλεις)
Internal IP: 192.168.2.19 (το IP του ESP32-S3)
Internal Port: 80
Protocol: TCP
Status: Enable
```

### Αποθήκευσε και Επανεκκίνησε το Router!

## 🌐 Βήμα 2: Βρες το Public IP σου

1. Πήγαινε στο: [https://whatismyipaddress.com/](https://whatismyipaddress.com/)
2. Γράψε το IP (π.χ. `123.456.789.123`)
3. **Σημαντικό:** Το IP αυτό αλλάζει κάποιες φορές, αν έχεις dynamic IP

## ✅ Βήμα 3: Ενεργοποίηση GitHub Pages

### Στο GitHub Repository:
1. Πήγαινε στο: [https://github.com/Mimisnak/SmartGreenhouse](https://github.com/Mimisnak/SmartGreenhouse)
2. Κάνε κλικ στο **Settings**
3. Πήγαινε στο **Pages** (αριστερά)
4. Στο **Source** επίλεξε: **Deploy from a branch**
5. Branch: **master** ή **main**
6. Folder: **/ (root)**
7. Κάνε κλικ **Save**

### Η σελίδα σου θα είναι διαθέσιμη στο:
```
https://mimisnak.github.io/SmartGreenhouse/
```

## 🚀 Βήμα 4: Χρήση

1. **Άνοιξε το GitHub Pages**: https://mimisnak.github.io/SmartGreenhouse/
2. **Συμπλήρωσε στη σελίδα**:
   - **Public IP/Domain**: `123.456.789.123` (το δικό σου IP)
   - **External Port**: `8080` (η πόρτα που έβαλες στο router)
3. **Κάνε κλικ**: "Αποθήκευση Ρυθμίσεων"
4. **Κάνε κλικ**: "Test Connection"
5. **Αν είναι OK**: "Σύνδεση στο ESP32-S3"

## 🔥 Advanced: Dynamic DNS (για σταθερό domain)

Αν το IP σου αλλάζει συχνά, μπορείς να χρησιμοποιήσεις:

### No-IP (Δωρεάν):
1. Εγγραφή στο: [https://www.noip.com/](https://www.noip.com/)
2. Δημιουργία hostname: `myesp32.ddns.net`
3. Download και εγκατάσταση No-IP DUC client
4. Χρήση του domain αντί για IP στο GitHub Pages

### DuckDNS (Δωρεάν):
1. Εγγραφή στο: [https://www.duckdns.org/](https://www.duckdns.org/)
2. Δημιουργία subdomain: `myesp32.duckdns.org`
3. Ρύθμιση auto-update

## 🛡️ Ασφάλεια

### Προτάσεις:
- **Μη χρησιμοποιείς πόρτα 80** (χρησιμοποίησε 8080, 8888, κλπ)
- **Άλλαξε το default password** του ESP32-S3 (αν υπάρχει)
- **Κλείσε το port forwarding** όταν δεν το χρειάζεσαι
- **Χρησιμοποίησε HTTPS** όπου είναι δυνατό

## 🧪 Testing

### Τοπικά (ίδιο δίκτυο):
```
http://192.168.2.19/
```

### Απομακρυσμένα (από παντού):
```
http://YOUR-PUBLIC-IP:8080/
```

### GitHub Pages:
```
https://mimisnak.github.io/SmartGreenhouse/
```

## 🚨 Troubleshooting

### Δεν συνδέεται;
1. **Έλεγξε το ESP32-S3**: Είναι ανοιχτό; Συνδεδεμένο στο WiFi;
2. **Έλεγξε το Router**: Port forwarding ενεργό; Σωστό Internal IP;
3. **Έλεγξε το Public IP**: Άλλαξε; Ανανέωσε στο GitHub Pages
4. **Έλεγξε το Firewall**: Μπλοκάρει την πόρτα 8080;

### Test Commands:
```bash
# Test local connection
curl http://192.168.2.19/data

# Test external connection (από άλλο δίκτυο)
curl http://YOUR-PUBLIC-IP:8080/data
```

## 📱 Mobile Access

Μπορείς να αποθηκεύσεις το GitHub Pages link στην αρχική οθόνη του κινητού σου:

1. Άνοιξε το link στον browser
2. Menu → "Add to Home Screen"
3. Τώρα έχεις app icon! 🎉

---

**🎯 Αποτέλεσμα**: Απομακρυσμένη πρόσβαση στο ESP32-S3 σου από οπουδήποτε στον κόσμο μέσω GitHub Pages!
