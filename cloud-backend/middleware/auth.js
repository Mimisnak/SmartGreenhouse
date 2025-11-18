const jwt = require('jsonwebtoken');

function authenticateToken(req, res, next) {
  const authHeader = req.headers['authorization'];
  const token = authHeader && authHeader.split(' ')[1]; // Bearer TOKEN

  if (!token) {
    return res.status(401).json({ error: 'Access token required' });
  }

  jwt.verify(token, process.env.JWT_SECRET, (err, user) => {
    if (err) {
      return res.status(403).json({ error: 'Invalid or expired token' });
    }
    req.user = user;
    next();
  });
}

function authenticateDevice(req, res, next) {
  // Device authentication via device_id (can be enhanced with API keys)
  const deviceId = req.body.device_id || req.query.device_id;
  
  if (!deviceId) {
    return res.status(401).json({ error: 'Device ID required' });
  }

  // TODO: Validate device exists and belongs to a user
  req.deviceId = deviceId;
  next();
}

module.exports = { authenticateToken, authenticateDevice };
