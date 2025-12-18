# 🚀 Complete Deployment Guide

## 📦 Backend Deployment (Render.com)

### Step 1: Prepare Repository
1. **Commit all changes:**
```powershell
cd "c:\Users\mimis\Desktop\PTIXIAKI ERGASIA\thermokrasia"
git add .
git commit -m "Add cloud backend and frontend"
git push origin master
```

### Step 2: Deploy to Render
1. **Go to:** https://render.com
2. **Sign up** with GitHub account
3. **Click:** "New +" → "Web Service"
4. **Connect repository:** `Mimisnak/SmartGreenhouse`
5. **Configure:**
   - Name: `greenhouse-backend`
   - Root Directory: `cloud-backend`
   - Build Command: `npm install && npm run init-db`
   - Start Command: `npm start`
   - Environment: Node
   - Plan: Free

6. **Add Environment Variables:**
   - `NODE_ENV` = `production`
   - `PORT` = `3000`
   - `JWT_SECRET` = (click "Generate" για random value)

7. **Click "Create Web Service"**
8. **Wait 2-3 minutes** για το deploy
9. **Copy URL** (π.χ. `https://greenhouse-backend-xxxx.onrender.com`)

### Step 3: Update Frontend Config
1. **Edit** `cloud-frontend/config.js`:
```javascript
BASE_URL: 'https://greenhouse-backend-xxxx.onrender.com',
```

2. **Commit change:**
```powershell
git add cloud-frontend/config.js
git commit -m "Update API URL"
git push
```

---

## 🌐 Frontend Deployment (GitHub Pages)

### Step 1: Enable GitHub Pages
1. **Go to:** https://github.com/Mimisnak/SmartGreenhouse/settings/pages
2. **Source:** Deploy from a branch
3. **Branch:** `master`
4. **Folder:** `/cloud-frontend`
5. **Click "Save"**

### Step 2: Wait & Access
- **Wait 2-3 minutes**
- **Access at:** https://mimisnak.github.io/SmartGreenhouse/

---

## 🔧 ESP32 Configuration

### Update Backend URL
1. **Edit** `src/main.cpp`:
```cpp
const char* cloudApiUrl = "https://greenhouse-backend-xxxx.onrender.com/api/telemetry";
```

2. **Build & Upload:**
```powershell
cd "c:\Users\mimis\Desktop\PTIXIAKI ERGASIA\thermokrasia"
platformio run --target upload
```

---

## ✅ Testing

### 1. Test Backend
```powershell
curl https://greenhouse-backend-xxxx.onrender.com/health
```

### 2. Test Frontend
- Open: https://mimisnak.github.io/SmartGreenhouse/
- Login: `demo@example.com` / `demo123`

### 3. Test ESP32
- Check Serial Monitor for "✓ Data sent successfully"
- Refresh dashboard to see real-time data

---

## 🔐 Custom Domain (Optional)

### For greenhouse.mimis.dev:
1. **In Render Dashboard:**
   - Settings → Custom Domain
   - Add: `greenhouse.mimis.dev`
   
2. **In Cloudflare/DNS:**
   - Type: CNAME
   - Name: `greenhouse`
   - Target: `greenhouse-backend-xxxx.onrender.com`

---

## 📊 Monitoring

- **Backend Logs:** Render Dashboard → Logs tab
- **ESP32 Logs:** Serial Monitor (115200 baud)
- **Frontend:** Browser DevTools Console

---

## 🆘 Troubleshooting

**Backend not responding:**
- Check Render logs for errors
- Verify environment variables
- Free tier sleeps after 15min inactivity (first request takes 30s)

**Frontend can't connect:**
- Check `config.js` has correct URL
- Check CORS settings in backend
- Open browser console for errors

**ESP32 not sending data:**
- Verify WiFi connection
- Check backend URL in `main.cpp`
- Check device_id matches database

---

## 💰 Cost

- ✅ **Backend (Render):** FREE (750 hours/month)
- ✅ **Frontend (GitHub Pages):** FREE (unlimited)
- ✅ **Total:** $0/month

---

## 🎉 Next Steps After Deployment

1. Create more users via Register page
2. Add multiple ESP32 devices
3. Monitor real-time telemetry
4. Test manual watering commands
5. Share URL with farmers: `https://mimisnak.github.io/SmartGreenhouse/`
