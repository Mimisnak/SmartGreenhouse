# 🚀 Quick Start Guide - Cloud Backend

## 📦 Installation

1. **Navigate to cloud-backend folder:**
```powershell
cd "c:\Users\mimis\Desktop\PTIXIAKI ERGASIA\thermokrasia\cloud-backend"
```

2. **Install dependencies and initialize database:**
```powershell
npm run setup
```

## ⚙️ Configuration

1. **Create `.env` file** (copy from `.env.example`):
```powershell
Copy-Item .env.example .env
```

2. **Edit `.env` file** with your settings:
```env
PORT=3000
JWT_SECRET=your-secret-key-change-this-in-production
NODE_ENV=development
```

## ▶️ Run the Server

**Development mode** (with auto-reload):
```powershell
npm run dev
```

**Production mode:**
```powershell
npm start
```

The server will start at: `http://localhost:3000`

## 🧪 Test the API

**Health check:**
```powershell
curl http://localhost:3000/health
```

**Login with demo account:**
```powershell
curl -X POST http://localhost:3000/api/auth/login `
  -H "Content-Type: application/json" `
  -d '{"email":"demo@example.com","password":"demo123"}'
```

## 📱 Open the Web Dashboard

1. **Start the backend** (if not running): `npm start`
2. **Open** `cloud-frontend/index.html` in your browser
3. **Login** with:
   - Email: `demo@example.com`
   - Password: `demo123`

## 🔧 Troubleshooting

**Port already in use:**
```powershell
# Change PORT in .env to 3001 or another free port
```

**Database errors:**
```powershell
# Reinitialize database
npm run init-db
```

**Missing dependencies:**
```powershell
npm install
```

## 📝 Demo Credentials

- **Email:** demo@example.com
- **Password:** demo123
- **Demo Device ID:** ESP32_DEMO_001

## 🌐 Next Steps

1. ✅ Backend running locally
2. ✅ Web dashboard accessible
3. ⏭️ Update ESP32 to send data to: `http://localhost:3000/api/telemetry`
4. ⏭️ Deploy to production server for public access
