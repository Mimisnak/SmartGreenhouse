const express = require('express');
const { getDatabase } = require('../db/database');
const { authenticateToken } = require('../middleware/auth');

const router = express.Router();

// Get user's devices
router.get('/', authenticateToken, (req, res) => {
  const db = getDatabase();

  try {
    const devices = db.prepare(`
      SELECT id, device_id, name, location, active, created_at, last_seen
      FROM devices
      WHERE user_id = ?
      ORDER BY name
    `).all(req.user.user_id);

    res.json({ devices });
  } catch (error) {
    console.error('Devices query error:', error);
    res.status(500).json({ error: 'Failed to retrieve devices' });
  }
});

// Register new device
router.post('/', authenticateToken, (req, res) => {
  const { device_id, name, location } = req.body;

  if (!device_id || !name) {
    return res.status(400).json({ error: 'device_id and name required' });
  }

  const db = getDatabase();

  try {
    const result = db.prepare(`
      INSERT INTO devices (device_id, user_id, name, location)
      VALUES (?, ?, ?, ?)
    `).run(device_id, req.user.user_id, name, location || null);

    res.status(201).json({
      message: 'Device registered',
      device: {
        id: result.lastInsertRowid,
        device_id,
        name,
        location
      }
    });
  } catch (error) {
    if (error.code === 'SQLITE_CONSTRAINT') {
      res.status(409).json({ error: 'Device already registered' });
    } else {
      console.error('Device registration error:', error);
      res.status(500).json({ error: 'Failed to register device' });
    }
  }
});

// Update device
router.patch('/:deviceId', authenticateToken, (req, res) => {
  const { deviceId } = req.params;
  const { name, location, active } = req.body;

  const db = getDatabase();

  try {
    const updates = [];
    const params = [];

    if (name !== undefined) {
      updates.push('name = ?');
      params.push(name);
    }
    if (location !== undefined) {
      updates.push('location = ?');
      params.push(location);
    }
    if (active !== undefined) {
      updates.push('active = ?');
      params.push(active ? 1 : 0);
    }

    if (updates.length === 0) {
      return res.status(400).json({ error: 'No fields to update' });
    }

    params.push(deviceId, req.user.user_id);

    const result = db.prepare(`
      UPDATE devices
      SET ${updates.join(', ')}
      WHERE device_id = ? AND user_id = ?
    `).run(...params);

    if (result.changes === 0) {
      return res.status(404).json({ error: 'Device not found' });
    }

    res.json({ message: 'Device updated' });
  } catch (error) {
    console.error('Device update error:', error);
    res.status(500).json({ error: 'Failed to update device' });
  }
});

// Delete device
router.delete('/:deviceId', authenticateToken, (req, res) => {
  const { deviceId } = req.params;
  const db = getDatabase();

  try {
    const result = db.prepare(`
      DELETE FROM devices
      WHERE device_id = ? AND user_id = ?
    `).run(deviceId, req.user.user_id);

    if (result.changes === 0) {
      return res.status(404).json({ error: 'Device not found' });
    }

    res.json({ message: 'Device deleted' });
  } catch (error) {
    console.error('Device deletion error:', error);
    res.status(500).json({ error: 'Failed to delete device' });
  }
});

module.exports = router;
