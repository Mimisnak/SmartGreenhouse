// API Configuration
const API_CONFIG = {
  // Backend URL
  BASE_URL: 'https://greenhouse-backend-e5or.onrender.com',
  
  // API endpoints
  ENDPOINTS: {
    LOGIN: '/api/auth/login',
    REGISTER: '/api/auth/register',
    DEVICES: '/api/devices',
    TELEMETRY: '/api/telemetry',
    COMMANDS: '/api/commands'
  }
};

// Export for use in script.js
window.API_CONFIG = API_CONFIG;
