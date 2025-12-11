// API Configuration - Local ESP32 Only (No Authentication)
const API_CONFIG = {
  // Note: Access ESP32 directly via local IP (e.g., http://192.168.1.100)
  // No backend server required
  BASE_URL: '',  // Empty - using direct ESP32 connection
  
  // Legacy endpoints (not used in local mode)
  ENDPOINTS: {
    TELEMETRY: '/api/telemetry',
    DEVICES: '/api/devices'
  }
};

// Export for use in script.js
window.API_CONFIG = API_CONFIG;
