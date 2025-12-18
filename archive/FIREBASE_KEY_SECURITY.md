# 🔐 Firebase Key Management Guide

## ⚠️ ΣΗΜΑΝΤΙΚΟ - Διαβάστε Προσεκτικά

### Το Key που μου έδωσες: `4JH5iFyDrTQxA6qR1x4iKiQNrCaAH3ZdrqMEpHOT`

Αυτό είναι ένα **Database Secret / Legacy Token** που δίνει **ΠΛΗΡΗ ΠΡΌΣΒΑΣΗ** στη Firebase βάση δεδομένων σου.

## ❌ ΤΙ ΝΑ ΚΑΝΕΙΣ ΤΩΡΑ (ΕΠΕΙΓΟΝ)

### 1. Διαγραφή του Token (REVOKE)

Πήγαινε στο Firebase Console και διέγραψε αυτό το token:

1. Άνοιξε https://console.firebase.google.com
2. Επίλεξε το project: **smartgreenhouse-fb494**
3. Πήγαινε σε: **Project Settings** (⚙️ πάνω αριστερά)
4. Κλικ στο tab: **Service accounts**
5. Scroll down στο **Database secrets**
6. Βρες το token `4JH5iFyDrTQxA6qR1x4iKiQNrCaAH3ZdrqMEpHOT`
7. Πάτα **Delete** / **Revoke**

### 2. Χρησιμοποίησε μόνο το Web API Key

Ο κώδικας σου ήδη χρησιμοποιεί το σωστό key:

```cpp
#define FIREBASE_AUTH "AIzaSyDwwszw4AapfTp_dkdli48vsxOZXkZwqfo"
```

Αυτό είναι το **Web API Key** που είναι ασφαλές για χρήση στον ESP32.

## ✅ Γιατί είναι ασφαλές το Web API Key;

Το Web API Key (`AIzaSy...`) δεν δίνει άμεση πρόσβαση στη βάση. Η πρόσβαση ελέγχεται από τα **Security Rules** που έφτιαξα:

```json
{
  "rules": {
    ".read": false,
    ".write": false,
    "sensors": {
      "$deviceId": {
        ".read": true,
        ".write": "auth != null || $deviceId == 'ESP32-Greenhouse'"
      }
    }
  }
}
```

## 🔒 Best Practices

### ✅ DO (Κάνε)
- Χρησιμοποίησε Web API Key για ESP32
- Χρησιμοποίησε Security Rules για έλεγχο πρόσβασης
- Κάνε τα rules όσο πιο αυστηρά γίνεται

### ❌ DON'T (Μην κάνεις)
- Μην χρησιμοποιείς Database Secrets στον ESP32
- Μην βάλεις sensitive keys στο GitHub
- Μην αφήνεις open rules (`.read: true`, `.write: true`)

## 📊 Τρέχον Setup (Σωστό)

```
ESP32 → WiFi → Firebase
         ↓
    Web API Key (AIzaSy...)
         ↓
    Security Rules Check
         ↓
    Write to: sensors/ESP32-Greenhouse/
```

## 🚨 Αν το Token διέρρευσε στο GitHub

Αν το Database Secret μπήκε στο GitHub, **ΔΙΕΓΡΑΨΕ ΤΟ ΑΜΕΣΩΣ** και:

1. Revoke το token από Firebase Console
2. Αλλαγή του Git history (αν είναι δημόσιο repo):
   ```bash
   git filter-branch --force --index-filter \
     "git rm --cached --ignore-unmatch <file-with-secret>" \
     --prune-empty --tag-name-filter cat -- --all
   ```
3. Force push:
   ```bash
   git push origin --force --all
   ```

## ✅ Επιβεβαίωση Ασφάλειας

Μετά τη διαγραφή του Database Secret, έλεγξε:

1. ESP32 συνδέεται κανονικά στο Firebase ✅
2. Μπορεί να γράψει δεδομένα ✅
3. Δεν μπορεί να διαγράψει όλη τη βάση ✅
4. Τα Security Rules λειτουργούν ✅

## 📞 Support

Αν έχεις θέματα, έλεγξε:
- Firebase Console → Realtime Database → Rules
- Firebase Console → Project Settings → Service accounts
- Serial Monitor του ESP32 για Firebase errors
