# 🔥 Firebase Setup Guide

## 1. Create Firebase Project

1. Go to https://console.firebase.google.com/
2. Click **"Add project"**
3. Name: `SmartGreenhouse`
4. Disable Google Analytics (not needed)
5. Click **"Create project"**

## 2. Enable Firebase Services

### Firestore Database
1. In Firebase Console → **Build** → **Firestore Database**
2. Click **"Create database"**
3. Choose **"Start in test mode"** (we'll secure it later)
4. Location: **europe-west** (closest to Greece)
5. Click **"Enable"**

### Authentication
1. In Firebase Console → **Build** → **Authentication**
2. Click **"Get started"**
3. Click **"Email/Password"** → **Enable** → **Save**

### Hosting
1. In Firebase Console → **Build** → **Hosting**
2. Click **"Get started"**
3. Follow the wizard (we'll deploy via CLI)

## 3. Register Web App

1. In Firebase Console → Project Overview (⚙️ gear icon)
2. Click **"Add app"** → **Web** (</> icon)
3. App nickname: `greenhouse-web`
4. Check **"Also set up Firebase Hosting"**
5. Click **"Register app"**
6. **COPY** the `firebaseConfig` object (looks like this):

```javascript
const firebaseConfig = {
  apiKey: "AIza...",
  authDomain: "smartgreenhouse-xxxxx.firebaseapp.com",
  projectId: "smartgreenhouse-xxxxx",
  storageBucket: "smartgreenhouse-xxxxx.appspot.com",
  messagingSenderId: "123456789",
  appId: "1:123456789:web:xxxxx"
};
```

7. **PASTE IT HERE** when ready!

## 4. Install Firebase CLI (on your PC)

Open PowerShell and run:
```powershell
npm install -g firebase-tools
firebase login
```

## 5. Deploy

I'll create all the Firebase files, then you run:
```powershell
cd cloud-frontend
firebase deploy
```

## Why Firebase is Better

✅ **Always online** - no sleep/wake delays
✅ **Free tier generous** - 50k reads/day, 20k writes/day
✅ **No CORS issues** - built-in support
✅ **Real-time database** - instant updates
✅ **Built-in authentication** - no JWT hassle
✅ **Global CDN** - fast everywhere
✅ **Easy ESP32 integration** - REST API

---

**Ready? Start with Step 1 and tell me when you have the firebaseConfig!**
