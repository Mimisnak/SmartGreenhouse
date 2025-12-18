# Firebase Security Rules - Οδηγίες Εγκατάστασης

## 🔒 Πρόβλημα Ασφάλειας

Το Firebase σου έστειλε email γιατί η βάση δεδομένων σου έχει **insecure rules** - δηλαδή οποιοσδήποτε μπορεί να διαβάσει και να γράψει δεδομένα.

## ✅ Λύση

Άνοιξε το Firebase Console και ενημέρωσε τα Security Rules με τον κώδικα από το αρχείο `firebase.rules.json`.

### Βήματα:

1. **Άνοιξε το Firebase Console:**
   - Πήγαινε στο https://console.firebase.google.com
   - Επίλεξε το project: `smartgreenhouse-fb494`

2. **Πήγαινε στα Realtime Database Rules:**
   - Από το μενού αριστερά: **Realtime Database** → **Rules**

3. **Αντικατέστησε τα υπάρχοντα rules:**
   Αντίγραψε τον κώδικα από το αρχείο `firebase.rules.json` και πάτα **Publish**

## 📝 Τι κάνουν τα νέα Rules:

```json
{
  "rules": {
    ".read": false,           // Κανείς δεν μπορεί να διαβάσει όλη τη βάση
    ".write": false,          // Κανείς δεν μπορεί να γράψει παντού
    
    "sensors": {
      "$deviceId": {
        ".read": true,        // Όλοι μπορούν να ΔΙΑΒΑΣΟΥΝ τα δεδομένα των sensors
        ".write": "auth != null || $deviceId == 'ESP32-Greenhouse'",  
        // Μόνο authenticated users ή το ESP32 μπορούν να ΓΡΑΨΟΥΝ
        
        "latest": {
          ".validate": "newData.hasChildren(['timestamp', 'temperature', 'pressure'])"
          // Validation: πρέπει να έχει τα απαραίτητα πεδία
        }
      }
    }
  }
}
```

## 🔐 Επιπλέον Ασφάλεια (Προαιρετικά):

Αν θέλεις ακόμα πιο αυστηρή ασφάλεια, μπορείς να:

### Επιλογή 1: Περιορισμός με API Key
```json
".write": "$deviceId == 'ESP32-Greenhouse' && auth.token.apiKey == 'YOUR_SECRET_KEY'"
```

### Επιλογή 2: IP-based restriction (Firebase Functions required)
Αυτό απαιτεί Firebase Functions για να ελέγξεις το IP.

### Επιλογή 3: Χρονικός περιορισμός
```json
".write": "now < 1735689600000"  // Expire date (Unix timestamp)
```

## ⚠️ Σημαντικό:

Μετά την ενημέρωση των rules, το email warning θα σταματήσει και η βάση σου θα είναι ασφαλής!

## 🧪 Test τα Rules:

Μετά την ενημέρωση, δοκίμασε:
```bash
# Το ESP32 πρέπει να μπορεί να γράψει:
curl -X PUT https://smartgreenhouse-fb494-default-rtdb.firebaseio.com/sensors/ESP32-Greenhouse/test.json -d '{"value": 123}'

# Όλοι μπορούν να διαβάσουν:
curl https://smartgreenhouse-fb494-default-rtdb.firebaseio.com/sensors/ESP32-Greenhouse/latest.json
```
