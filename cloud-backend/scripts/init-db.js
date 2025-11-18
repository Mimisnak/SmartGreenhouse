const Database = require('better-sqlite3');
const bcrypt = require('bcryptjs');
const path = require('path');

// Create database directory if it doesn't exist
const dbPath = path.join(__dirname, '..', 'db', 'greenhouse.db');

console.log('🔧 Initializing database...');

const db = new Database(dbPath);

// Enable foreign keys
db.pragma('foreign_keys = ON');

// Create tables
console.log('📋 Creating tables...');

db.exec(`
  CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    email TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    name TEXT NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
  );

  CREATE TABLE IF NOT EXISTS devices (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    device_id TEXT UNIQUE NOT NULL,
    user_id INTEGER NOT NULL,
    name TEXT NOT NULL,
    location TEXT,
    active INTEGER DEFAULT 1,
    last_seen DATETIME,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
  );

  CREATE TABLE IF NOT EXISTS telemetry (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    device_id TEXT NOT NULL,
    sensor_type TEXT NOT NULL,
    value REAL NOT NULL,
    unit TEXT NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (device_id) REFERENCES devices(device_id) ON DELETE CASCADE
  );

  CREATE TABLE IF NOT EXISTS commands (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    device_id TEXT NOT NULL,
    command_type TEXT NOT NULL,
    parameters TEXT,
    status TEXT DEFAULT 'pending',
    result TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    executed_at DATETIME,
    FOREIGN KEY (device_id) REFERENCES devices(device_id) ON DELETE CASCADE
  );

  CREATE INDEX IF NOT EXISTS idx_telemetry_device_time ON telemetry(device_id, timestamp);
  CREATE INDEX IF NOT EXISTS idx_commands_device_status ON commands(device_id, status);
`);

console.log('✅ Tables created successfully');

// Create demo user
console.log('👤 Creating demo user...');
const demoPassword = 'demo123';
const hashedPassword = bcrypt.hashSync(demoPassword, 10);

try {
  const insertUser = db.prepare(`
    INSERT OR IGNORE INTO users (email, password_hash, name)
    VALUES (?, ?, ?)
  `);
  
  const result = insertUser.run('demo@example.com', hashedPassword, 'Demo User');
  
  if (result.changes > 0) {
    console.log('✅ Demo user created:');
    console.log('   Email: demo@example.com');
    console.log('   Password: demo123');
  } else {
    console.log('ℹ️  Demo user already exists');
  }
} catch (error) {
  console.error('❌ Error creating demo user:', error.message);
}

// Create demo device
console.log('📱 Creating demo device...');
try {
  const user = db.prepare('SELECT id FROM users WHERE email = ?').get('demo@example.com');
  
  if (user) {
    const insertDevice = db.prepare(`
      INSERT OR IGNORE INTO devices (device_id, user_id, name, location, active)
      VALUES (?, ?, ?, ?, ?)
    `);
    
    const result = insertDevice.run('ESP32_DEMO_001', user.id, 'Demo Greenhouse', 'Athens, Greece', 1);
    
    if (result.changes > 0) {
      console.log('✅ Demo device created: ESP32_DEMO_001');
    } else {
      console.log('ℹ️  Demo device already exists');
    }
  }
} catch (error) {
  console.error('❌ Error creating demo device:', error.message);
}

db.close();

console.log('\n🎉 Database initialization complete!');
console.log('\n📝 Next steps:');
console.log('   1. Run: npm start');
console.log('   2. Open: http://localhost:3000');
console.log('   3. Login with demo@example.com / demo123');
