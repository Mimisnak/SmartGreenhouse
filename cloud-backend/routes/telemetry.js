const express = require('express');
const { getDatabase } = require('../db/database');
const { authenticateDevice } = require('../middleware/auth');

const router = express.Router();

// Receive telemetry data from ESP32
router.post('/', authenticateDevice, (req, res) => {
  const { device_id, timestamp, sensors } = req.body;

  if (!sensors || !Array.isArray(sensors)) {
    return res.status(400).json({ error: 'Invalid sensor data format' });
  }

  const db = getDatabase();

  try {
    // Update device last_seen
    db.prepare('UPDATE devices SET last_seen = CURRENT_TIMESTAMP WHERE device_id = ?')
      .run(device_id);

    // Insert telemetry data
    const insert = db.prepare(
      'INSERT INTO telemetry (device_id, sensor_type, value, unit) VALUES (?, ?, ?, ?)'
    );

    const insertMany = db.transaction((sensorData) => {
      for (const sensor of sensorData) {
        insert.run(device_id, sensor.type, sensor.value, sensor.unit);
      }
    });

    insertMany(sensors);

    res.json({ 
      message: 'Telemetry received', 
      count: sensors.length 
    });
  } catch (error) {
    console.error('Telemetry error:', error);
    res.status(500).json({ error: 'Failed to store telemetry' });
  }
});

// Get latest telemetry for a device
router.get('/latest/:deviceId', (req, res) => {
  const { deviceId } = req.params;
  const db = getDatabase();

  try {
    const data = db.prepare(`
      SELECT sensor_type, value, unit, MAX(timestamp) as timestamp
      FROM telemetry
      WHERE device_id = ?
      GROUP BY sensor_type
      ORDER BY timestamp DESC
    `).all(deviceId);

    res.json({ device_id: deviceId, sensors: data });
  } catch (error) {
    console.error('Query error:', error);
    res.status(500).json({ error: 'Failed to retrieve data' });
  }
});

// Get telemetry history
router.get('/history/:deviceId', (req, res) => {
  const { deviceId } = req.params;
  const { hours = 24, sensor_type } = req.query;
  
  const db = getDatabase();

  try {
    let query = `
      SELECT sensor_type, value, unit, timestamp
      FROM telemetry
      WHERE device_id = ?
      AND timestamp >= datetime('now', '-${parseInt(hours)} hours')
    `;
    
    const params = [deviceId];
    
    if (sensor_type) {
      query += ' AND sensor_type = ?';
      params.push(sensor_type);
    }
    
    query += ' ORDER BY timestamp DESC LIMIT 1000';

    const data = db.prepare(query).all(...params);

    res.json({ 
      device_id: deviceId,
      hours: parseInt(hours),
      count: data.length,
      data 
    });
  } catch (error) {
    console.error('History error:', error);
    res.status(500).json({ error: 'Failed to retrieve history' });
  }
});

module.exports = router;
