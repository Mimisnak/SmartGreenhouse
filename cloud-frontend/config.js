// API Configuration
const API_CONFIG = {
  // Change this to your deployed backend URL
  // For local testing: 'http://localhost:3000'
  // For production: 'https://your-app.onrender.com'
  BASE_URL: 'https://greenhouse-api-mimis.onrender.com',
  
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
