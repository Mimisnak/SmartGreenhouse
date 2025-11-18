const express = require('express');
const { getDatabase } = require('../db/database');
const { authenticateToken, authenticateDevice } = require('../middleware/auth');

const router = express.Router();

// User sends command to device
router.post('/', authenticateToken, (req, res) => {
  const { device_id, command_type, parameters } = req.body;

  if (!device_id || !command_type) {
    return res.status(400).json({ error: 'device_id and command_type required' });
  }

  const db = getDatabase();

  try {
    // Verify device belongs to user
    const device = db.prepare(`
      SELECT id FROM devices WHERE device_id = ? AND user_id = ?
    `).get(device_id, req.user.user_id);

    if (!device) {
      return res.status(404).json({ error: 'Device not found' });
    }

    // Insert command
    const result = db.prepare(`
      INSERT INTO commands (device_id, command_type, parameters)
      VALUES (?, ?, ?)
    `).run(device_id, command_type, JSON.stringify(parameters || {}));

    res.status(201).json({
      message: 'Command queued',
      command_id: result.lastInsertRowid
    });
  } catch (error) {
    console.error('Command error:', error);
    res.status(500).json({ error: 'Failed to queue command' });
  }
});

// Device polls for pending commands
router.get('/pending', authenticateDevice, (req, res) => {
  const deviceId = req.deviceId;
  const db = getDatabase();

  try {
    const commands = db.prepare(`
      SELECT id, command_type, parameters, created_at
      FROM commands
      WHERE device_id = ? AND status = 'pending'
      ORDER BY created_at ASC
      LIMIT 10
    `).all(deviceId);

    res.json({ commands });
  } catch (error) {
    console.error('Pending commands error:', error);
    res.status(500).json({ error: 'Failed to retrieve commands' });
  }
});

// Device reports command execution
router.patch('/:commandId/complete', authenticateDevice, (req, res) => {
  const { commandId } = req.params;
  const { success } = req.body;

  const db = getDatabase();

  try {
    const result = db.prepare(`
      UPDATE commands
      SET status = ?, executed_at = CURRENT_TIMESTAMP
      WHERE id = ? AND device_id = ?
    `).run(success ? 'completed' : 'failed', commandId, req.deviceId);

    if (result.changes === 0) {
      return res.status(404).json({ error: 'Command not found' });
    }

    res.json({ message: 'Command status updated' });
  } catch (error) {
    console.error('Command completion error:', error);
    res.status(500).json({ error: 'Failed to update command' });
  }
});

module.exports = router;
