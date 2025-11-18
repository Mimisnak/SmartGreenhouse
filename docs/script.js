// Use API_CONFIG from config.js
const API_URL = window.API_CONFIG.BASE_URL + '/api';

let token = localStorage.getItem('token');
let currentUser = null;
let selectedDevice = null;
let charts = {};

// Initialize
document.addEventListener('DOMContentLoaded', () => {
    if (token) {
        showDashboard();
        loadDevices();
    } else {
        showLogin();
    }
});

// Auth Functions
function showLogin() {
    document.getElementById('loginForm').style.display = 'block';
    document.getElementById('registerForm').style.display = 'none';
}

function showRegister() {
    document.getElementById('loginForm').style.display = 'none';
    document.getElementById('registerForm').style.display = 'block';
}

async function login() {
    const email = document.getElementById('loginEmail').value;
    const password = document.getElementById('loginPassword').value;

    try {
        const response = await fetch(`${API_URL}/auth/login`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ email, password })
        });

        const data = await response.json();

        if (!response.ok) throw new Error(data.error);

        token = data.token;
        currentUser = data.user;
        localStorage.setItem('token', token);
        
        showDashboard();
        loadDevices();
    } catch (error) {
        showError(error.message);
    }
}

async function register() {
    const name = document.getElementById('registerName').value;
    const email = document.getElementById('registerEmail').value;
    const password = document.getElementById('registerPassword').value;

    try {
        const response = await fetch(`${API_URL}/auth/register`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ name, email, password })
        });

        const data = await response.json();

        if (!response.ok) throw new Error(data.error);

        alert('Εγγραφή επιτυχής! Μπορείτε να συνδεθείτε.');
        showLogin();
    } catch (error) {
        showError(error.message);
    }
}

function logout() {
    localStorage.removeItem('token');
    token = null;
    currentUser = null;
    window.location.reload();
}

function showDashboard() {
    document.getElementById('loginScreen').style.display = 'none';
    document.getElementById('dashboardScreen').style.display = 'block';
    if (currentUser) {
        document.getElementById('userName').textContent = currentUser.name || currentUser.email;
    }
}

function showError(message) {
    const errorDiv = document.getElementById('authError');
    errorDiv.textContent = message;
    errorDiv.style.display = 'block';
    setTimeout(() => errorDiv.style.display = 'none', 5000);
}

// Device Management
async function loadDevices() {
    try {
        const response = await fetch(`${API_URL}/devices`, {
            headers: { 'Authorization': `Bearer ${token}` }
        });

        const data = await response.json();
        const devicesList = document.getElementById('devicesList');
        
        if (data.devices.length === 0) {
            devicesList.innerHTML = '<p>Δεν έχετε προσθέσει συσκευές ακόμα.</p>';
            return;
        }

        devicesList.innerHTML = data.devices.map(device => `
            <div class="device-card" onclick="selectDevice('${device.device_id}')">
                <h3>${device.name}</h3>
                <p>${device.location || 'Χωρίς τοποθεσία'}</p>
                <span class="status ${device.active ? 'active' : 'inactive'}">
                    ${device.active ? '🟢 Ενεργό' : '🔴 Ανενεργό'}
                </span>
                <p class="last-seen">Τελευταία σύνδεση: ${formatDate(device.last_seen)}</p>
            </div>
        `).join('');
    } catch (error) {
        console.error('Load devices error:', error);
    }
}

function showAddDevice() {
    document.getElementById('addDeviceModal').style.display = 'flex';
}

function closeAddDevice() {
    document.getElementById('addDeviceModal').style.display = 'none';
}

async function addDevice() {
    const deviceId = document.getElementById('newDeviceId').value;
    const name = document.getElementById('newDeviceName').value;
    const location = document.getElementById('newDeviceLocation').value;

    try {
        const response = await fetch(`${API_URL}/devices`, {
            method: 'POST',
            headers: {
                'Authorization': `Bearer ${token}`,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ device_id: deviceId, name, location })
        });

        if (!response.ok) throw new Error('Failed to add device');

        closeAddDevice();
        loadDevices();
    } catch (error) {
        alert(error.message);
    }
}

// Device Details
async function selectDevice(deviceId) {
    selectedDevice = deviceId;
    
    document.querySelector('.devices-section').style.display = 'none';
    document.getElementById('deviceDetails').style.display = 'block';

    // Load latest data
    await loadDeviceData(deviceId);
    
    // Start auto-refresh
    setInterval(() => loadDeviceData(deviceId), 10000);
}

async function loadDeviceData(deviceId) {
    try {
        const response = await fetch(`${API_URL}/telemetry/latest/${deviceId}`);
        const data = await response.json();

        renderSensorCards(data.sensors);
        loadCharts(deviceId);
    } catch (error) {
        console.error('Load device data error:', error);
    }
}

function renderSensorCards(sensors) {
    const grid = document.getElementById('sensorGrid');
    
    grid.innerHTML = sensors.map(sensor => `
        <div class="sensor-card">
            <div class="sensor-icon">${getSensorIcon(sensor.sensor_type)}</div>
            <h4>${sensor.sensor_type}</h4>
            <div class="sensor-value">${sensor.value.toFixed(1)} ${sensor.unit}</div>
            <div class="sensor-time">${formatDate(sensor.timestamp)}</div>
        </div>
    `).join('');
}

function getSensorIcon(type) {
    const icons = {
        'Temperature': '🌡️',
        'Pressure': '📊',
        'Light': '☀️',
        'Soil Moisture': '🌱'
    };
    return icons[type] || '📈';
}

async function loadCharts(deviceId) {
    try {
        const response = await fetch(`${API_URL}/telemetry/history/${deviceId}?hours=24`);
        const data = await response.json();

        // Group by sensor type
        const byType = {};
        data.data.forEach(point => {
            if (!byType[point.sensor_type]) byType[point.sensor_type] = [];
            byType[point.sensor_type].push(point);
        });

        // Create/update charts
        if (byType['Temperature']) {
            createChart('tempChart', 'Θερμοκρασία (24h)', byType['Temperature'], '°C');
        }
        if (byType['Soil Moisture']) {
            createChart('soilChart', 'Υγρασία Εδάφους (24h)', byType['Soil Moisture'], '%');
        }
    } catch (error) {
        console.error('Load charts error:', error);
    }
}

function createChart(canvasId, label, data, unit) {
    const ctx = document.getElementById(canvasId).getContext('2d');
    
    if (charts[canvasId]) {
        charts[canvasId].destroy();
    }

    const reversedData = [...data].reverse();
    
    charts[canvasId] = new Chart(ctx, {
        type: 'line',
        data: {
            labels: reversedData.map(d => new Date(d.timestamp).toLocaleTimeString()),
            datasets: [{
                label: `${label} (${unit})`,
                data: reversedData.map(d => d.value),
                borderColor: 'rgb(76, 175, 80)',
                backgroundColor: 'rgba(76, 175, 80, 0.1)',
                tension: 0.4,
                fill: true
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: { display: true }
            }
        }
    });
}

// Manual Control
async function sendCommand(commandType, parameters) {
    if (!selectedDevice) return;

    try {
        const response = await fetch(`${API_URL}/commands`, {
            method: 'POST',
            headers: {
                'Authorization': `Bearer ${token}`,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                device_id: selectedDevice,
                command_type: commandType,
                parameters
            })
        });

        if (!response.ok) throw new Error('Command failed');

        alert('Εντολή στάλθηκε επιτυχώς!');
    } catch (error) {
        alert(error.message);
    }
}

function closeDeviceDetails() {
    document.getElementById('deviceDetails').style.display = 'none';
    document.querySelector('.devices-section').style.display = 'block';
}

// Utilities
function formatDate(dateString) {
    if (!dateString) return 'Ποτέ';
    const date = new Date(dateString);
    return date.toLocaleString('el-GR');
}
