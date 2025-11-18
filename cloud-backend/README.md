# Cloud Backend - Smart Greenhouse

Node.js/Express backend για cloud integration του Smart Greenhouse system.

## Features

- ✅ User authentication (JWT)
- ✅ Device registration & management
- ✅ Telemetry data storage (SQLite)
- ✅ Real-time sensor data API
- ✅ Command queue for manual control
- ✅ Historical data retrieval
- ✅ Rate limiting & security headers

## Installation

```bash
# Install dependencies
npm install

# Copy environment file
copy .env.example .env

# Edit .env and set JWT_SECRET
notepad .env

# Initialize database
npm run init-db

# Start server
npm start

# Or in development mode with auto-reload
npm run dev
```

## API Endpoints

### Authentication

**Register User**
```http
POST /api/auth/register
Content-Type: application/json

{
  "email": "user@example.com",
  "password": "secure_password",
  "name": "John Doe"
}
```

**Login**
```http
POST /api/auth/login
Content-Type: application/json

{
  "email": "user@example.com",
  "password": "secure_password"
}

Response:
{
  "token": "eyJhbGc...",
  "user": {
    "id": 1,
    "email": "user@example.com",
    "name": "John Doe"
  }
}
```

### Devices

**List User Devices** (requires auth)
```http
GET /api/devices
Authorization: Bearer {token}
```

**Register Device** (requires auth)
```http
POST /api/devices
Authorization: Bearer {token}
Content-Type: application/json

{
  "device_id": "greenhouse-proto-001",
  "name": "My Greenhouse",
  "location": "Backyard"
}
```

### Telemetry

**Send Telemetry** (from ESP32)
```http
POST /api/telemetry
Content-Type: application/json

{
  "device_id": "greenhouse-proto-001",
  "timestamp": 1234567890,
  "sensors": [
    {
      "type": "Temperature",
      "value": 25.4,
      "unit": "°C"
    },
    {
      "type": "Soil Moisture",
      "value": 65.3,
      "unit": "%"
    }
  ]
}
```

**Get Latest Data**
```http
GET /api/telemetry/latest/greenhouse-proto-001
```

**Get Historical Data**
```http
GET /api/telemetry/history/greenhouse-proto-001?hours=24&sensor_type=Temperature
```

### Commands (Manual Control)

**Send Command** (requires auth)
```http
POST /api/commands
Authorization: Bearer {token}
Content-Type: application/json

{
  "device_id": "greenhouse-proto-001",
  "command_type": "WATER_NOW",
  "parameters": {
    "duration": 30
  }
}
```

**Poll Pending Commands** (from ESP32)
```http
GET /api/commands/pending?device_id=greenhouse-proto-001
```

**Report Command Completion** (from ESP32)
```http
PATCH /api/commands/123/complete?device_id=greenhouse-proto-001
Content-Type: application/json

{
  "success": true
}
```

## Database Schema

### users
- id, email, password_hash, name, created_at, last_login

### devices
- id, device_id (unique), user_id, name, location, active, created_at, last_seen

### telemetry
- id, device_id, sensor_type, value, unit, timestamp

### commands
- id, device_id, command_type, parameters, status, created_at, executed_at

## Environment Variables

```env
PORT=3000
NODE_ENV=development
JWT_SECRET=your-super-secret-jwt-key-change-this
DB_PATH=./data/greenhouse.db
ALLOWED_ORIGINS=http://localhost:3000,https://greenhouse.mimis.dev
RATE_LIMIT_WINDOW_MS=900000
RATE_LIMIT_MAX_REQUESTS=100
```

## Deployment

### Local Testing
```bash
npm start
```

### Production (Railway/Render/etc)
1. Push code to GitHub
2. Connect repository to hosting platform
3. Set environment variables
4. Deploy

### Cloudflare Tunnel (για τοπική δημόσια έκθεση)
```bash
npm install -g cloudflared
cloudflared tunnel --url http://localhost:3000
```

## Security Notes

- ⚠️ **CHANGE JWT_SECRET** in production
- Use HTTPS in production
- Configure CORS origins properly
- Consider API key authentication for devices
- Set up database backups

## Development

```bash
# Watch mode with auto-reload
npm run dev

# Test endpoints
curl http://localhost:3000/health
```

## Next Steps

1. Update ESP32 firmware to send data to this backend
2. Deploy frontend to `greenhouse.mimis.dev`
3. Set up SSL/TLS certificates
4. Configure production database (PostgreSQL recommended)
5. Implement WebSocket for real-time updates (optional)
