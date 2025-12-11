// Global variables for charts and settings
let temperatureChart = null;
let pressureChart = null;
let soilMoistureChart = null;
let lightChart = null;
let dataHistory = {
    temperature: [],
    pressure: [],
    soilMoisture: [],
    light: [],
    timestamps: []
};
let currentTheme = 'light';
let isPlaying = true;
let refreshRate = 2000;
let weatherEffect = 'none';

// Initialize when page loads
document.addEventListener('DOMContentLoaded', function() {
    loadSettings();
    initializeCharts();
    createWeatherEffects();
    loadHistoricalData(); // Load historical data on startup
    updateData();
    startAutoUpdate();
    generatePlantCareRecommendations();
});

// Load historical data from ESP32 server
async function loadHistoricalData() {
    try {
        const response = await fetch('/history');
        
        if (!response.ok) {
            console.warn('No historical data available yet');
            return;
        }
        
        const historyData = await response.json();
        
        if (historyData.temperature && historyData.temperature.length > 0) {
            // Clear existing data
            dataHistory.temperature = [];
            dataHistory.pressure = [];
            dataHistory.soilMoisture = [];
            dataHistory.light = [];
            dataHistory.timestamps = [];
            
            // Load server data (arrays from ESP32)
            for (let i = 0; i < historyData.temperature.length; i++) {
                dataHistory.temperature.push(historyData.temperature[i]);
                dataHistory.pressure.push(historyData.pressure[i]);
                dataHistory.soilMoisture.push(historyData.soil[i] || 0);
                dataHistory.light.push(historyData.light[i] || 0);
                // Convert ESP32 millis() to readable time
                const timestamp = new Date(Date.now() - (historyData.timestamps.length - 1 - i) * 300000);
                dataHistory.timestamps.push(timestamp.toISOString());
            }
            
            // Update charts with historical data
            updateChartsWithHistory();
            
            console.log(`Loaded ${historyData.temperature.length} historical data points`);
            showNotification(`Φορτώθηκαν ${historyData.temperature.length} ιστορικά δεδομένα!`, 'success');
        }
        
    } catch (error) {
        console.warn('Failed to load historical data:', error);
    }
}

// Update charts with historical data
function updateChartsWithHistory() {
    if (dataHistory.timestamps.length === 0) return;
    
    // Create time labels for charts
    const labels = dataHistory.timestamps.map(timestamp => 
        new Date(timestamp).toLocaleTimeString()
    );
    
    // Update temperature chart
    temperatureChart.data.labels = labels;
    temperatureChart.data.datasets[0].data = [...dataHistory.temperature];
    temperatureChart.update();
    
    // Update pressure chart  
    pressureChart.data.labels = labels;
    pressureChart.data.datasets[0].data = [...dataHistory.pressure];
    pressureChart.update();
    
    // Update soil moisture chart
    soilMoistureChart.data.labels = labels;
    soilMoistureChart.data.datasets[0].data = [...dataHistory.soilMoisture];
    soilMoistureChart.update();
    
    // Update light chart
    lightChart.data.labels = labels;
    lightChart.data.datasets[0].data = [...dataHistory.light];
    lightChart.update();
}

// Initialize Chart.js charts
function initializeCharts() {
    const tempCtx = document.getElementById('temperatureChart').getContext('2d');
    const pressCtx = document.getElementById('pressureChart').getContext('2d');
    const soilCtx = document.getElementById('soilMoistureChart').getContext('2d');
    const lightCtx = document.getElementById('lightChart').getContext('2d');
    
    temperatureChart = new Chart(tempCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Θερμοκρασία (°C)',
                data: [],
                borderColor: '#28a745',
                backgroundColor: 'rgba(40, 167, 69, 0.1)',
                fill: true,
                tension: 0.4
            }]
        },
        options: {
            responsive: true,
            scales: {
                y: {
                    beginAtZero: false
                }
            },
            plugins: {
                legend: {
                    display: true,
                    position: 'top'
                }
            }
        }
    });
    
    pressureChart = new Chart(pressCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Πίεση (hPa)',
                data: [],
                borderColor: '#17a2b8',
                backgroundColor: 'rgba(23, 162, 184, 0.1)',
                fill: true,
                tension: 0.4
            }]
        },
        options: {
            responsive: true,
            scales: {
                y: {
                    beginAtZero: false
                }
            },
            plugins: {
                legend: {
                    display: true,
                    position: 'top'
                }
            }
        }
    });
    
    soilMoistureChart = new Chart(soilCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Υγρασία Εδάφους (%)',
                data: [],
                borderColor: '#007bff',
                backgroundColor: 'rgba(0, 123, 255, 0.1)',
                fill: true,
                tension: 0.4
            }]
        },
        options: {
            responsive: true,
            scales: {
                y: {
                    beginAtZero: true,
                    max: 100
                }
            },
            plugins: {
                legend: {
                    display: true,
                    position: 'top'
                }
            }
        }
    });
    
    lightChart = new Chart(lightCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Φωτισμός (lux)',
                data: [],
                borderColor: '#ffc107',
                backgroundColor: 'rgba(255, 193, 7, 0.1)',
                fill: true,
                tension: 0.4
            }]
        },
        options: {
            responsive: true,
            scales: {
                y: {
                    beginAtZero: true
                }
            },
            plugins: {
                legend: {
                    display: true,
                    position: 'top'
                }
            }
        }
    });
}

// Update sensor data and charts
async function updateData() {
    try {
        const response = await fetch('/api');
        
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }
        
        const data = await response.json();
        
        // Update status cards displays
        document.getElementById('temperature').textContent = data.temperature + '°C';
        document.getElementById('pressure').textContent = data.pressure + ' hPa';
        document.getElementById('soilMoisture').textContent = data.soilMoisture + '%';
        document.getElementById('light').textContent = data.light + ' lux';
        
        // Update status indicators
        updateStatusIndicator('tempStatus', data.temperature, 18, 28);
        updateStatusIndicator('pressureStatus', data.pressure, 1000, 1030);
        updateStatusIndicator('soilStatus', data.soilMoisture, 40, 80);
        updateStatusIndicator('lightStatus', data.light, 200, 2000);
        
        // Add current data point to charts (real-time update)
        const now = new Date().toLocaleTimeString();
        updateChart(temperatureChart, data.temperature, now);
        updateChart(pressureChart, data.pressure, now);
        updateChart(soilMoistureChart, data.soilMoisture, now);
        updateChart(lightChart, data.light, now);
        
        // Note: Historical data is now managed by ESP32 server, not browser
        // Data persistence survives page refreshes thanks to server-side storage
        
        showNotification('Δεδομένα ενημερώθηκαν επιτυχώς!', 'success');
        
    } catch (error) {
        console.error('Update failed:', error);
        showNotification(`Σφάλμα: ${error.message}`, 'error');
    }
}

// Update status indicator with color coding
function updateStatusIndicator(elementId, value, minGood, maxGood) {
    const statusEl = document.getElementById(elementId);
    if (!statusEl) return;
    
    let statusClass, statusText;
    if (value >= minGood && value <= maxGood) {
        statusClass = 'status-excellent';
        statusText = 'Εξαιρετικό';
    } else if (value >= minGood - 5 && value <= maxGood + 5) {
        statusClass = 'status-good';
        statusText = 'Καλό';
    } else {
        statusClass = 'status-warning';
        statusText = 'Προσοχή';
    }
    
    statusEl.className = `card-status ${statusClass}`;
    statusEl.textContent = statusText;
}

// Update status card with color coding (legacy function)
function updateStatusCard(type, value, unit, icon, minGood, maxGood) {
    const card = document.querySelector(`[data-sensor="${type}"]`);
    if (!card) return;
    
    const valueEl = card.querySelector('.card-value');
    const statusEl = card.querySelector('.card-status');
    const iconEl = card.querySelector('.card-icon');
    
    valueEl.textContent = value;
    iconEl.textContent = icon;
    
    let statusClass, statusText;
    if (value >= minGood && value <= maxGood) {
        statusClass = 'status-excellent';
        statusText = 'Εξαιρετικό';
    } else if (value >= minGood - 5 && value <= maxGood + 5) {
        statusClass = 'status-good';
        statusText = 'Καλό';
    } else {
        statusClass = 'status-warning';
        statusText = 'Προσοχή';
    }
    
    statusEl.className = `card-status ${statusClass}`;
    statusEl.textContent = statusText;
}

// Update chart with new data
function updateChart(chart, value, label) {
    chart.data.labels.push(label);
    chart.data.datasets[0].data.push(value);
    
    // Keep only last 20 data points
    if (chart.data.labels.length > 20) {
        chart.data.labels.shift();
        chart.data.datasets[0].data.shift();
    }
    
    chart.update('none');
}

// Theme management
function setTheme(theme) {
    currentTheme = theme;
    document.body.className = theme === 'dark' ? 'dark-theme' : '';
    
    // Update active button
    document.querySelectorAll('.theme-btn').forEach(btn => {
        btn.classList.remove('active');
    });
    event.target.classList.add('active');
    
    saveSettings();
    showNotification(`Θέμα "${theme}" ενεργοποιήθηκε`, 'success');
}

// Weather effects
function createWeatherEffects() {
    const container = document.querySelector('.weather-effects');
    
    if (weatherEffect === 'rain') {
        createRainEffect(container);
    } else if (weatherEffect === 'snow') {
        createSnowEffect(container);
    } else if (weatherEffect === 'particles') {
        createParticleEffect(container);
    }
}

function createRainEffect(container) {
    container.innerHTML = '';
    for (let i = 0; i < 50; i++) {
        const drop = document.createElement('div');
        drop.className = 'particle';
        drop.style.left = Math.random() * 100 + '%';
        drop.style.animationDuration = (Math.random() * 3 + 2) + 's';
        drop.style.animationDelay = Math.random() * 2 + 's';
        drop.style.width = '2px';
        drop.style.height = '20px';
        drop.style.borderRadius = '0';
        container.appendChild(drop);
    }
}

function createSnowEffect(container) {
    container.innerHTML = '';
    for (let i = 0; i < 30; i++) {
        const flake = document.createElement('div');
        flake.className = 'particle';
        flake.style.left = Math.random() * 100 + '%';
        flake.style.animationDuration = (Math.random() * 5 + 3) + 's';
        flake.style.animationDelay = Math.random() * 3 + 's';
        flake.style.width = '8px';
        flake.style.height = '8px';
        container.appendChild(flake);
    }
}

function createParticleEffect(container) {
    container.innerHTML = '';
    for (let i = 0; i < 20; i++) {
        const particle = document.createElement('div');
        particle.className = 'particle';
        particle.style.left = Math.random() * 100 + '%';
        particle.style.animationDuration = (Math.random() * 8 + 4) + 's';
        particle.style.animationDelay = Math.random() * 4 + 's';
        particle.style.width = '6px';
        particle.style.height = '6px';
        particle.style.background = 'rgba(40, 167, 69, 0.7)';
        container.appendChild(particle);
    }
}

function setWeatherEffect(effect) {
    weatherEffect = effect;
    createWeatherEffects();
    saveSettings();
    showNotification(`Εφέ "${effect}" ενεργοποιήθηκε`, 'success');
}

// Controls
function togglePlayPause() {
    isPlaying = !isPlaying;
    const button = event.target;
    
    if (isPlaying) {
        startAutoUpdate();
        button.textContent = '⏸️ Παύση';
        showNotification('Αυτόματη ενημέρωση ενεργοποιήθηκε', 'success');
    } else {
        clearInterval(updateInterval);
        button.textContent = '▶️ Έναρξη';
        showNotification('Αυτόματη ενημέρωση σταμάτησε', 'warning');
    }
}

function updateRefreshRate() {
    refreshRate = parseInt(document.getElementById('refresh-rate').value) * 1000;
    if (isPlaying) {
        clearInterval(updateInterval);
        startAutoUpdate();
    }
    saveSettings();
    showNotification(`Ρυθμός ανανέωσης: ${refreshRate/1000}s`, 'success');
}

function setTemperatureThreshold() {
    const threshold = parseFloat(document.getElementById('temp-threshold').value);
    localStorage.setItem('temperatureThreshold', threshold);
    showNotification(`Όριο θερμοκρασίας: ${threshold}°C`, 'success');
}

function setPressureThreshold() {
    const threshold = parseFloat(document.getElementById('pressure-threshold').value);
    localStorage.setItem('pressureThreshold', threshold);
    showNotification(`Όριο πίεσης: ${threshold} hPa`, 'success');
}

// Data export functions
function exportCSV() {
    let csv = 'Timestamp,Temperature,Pressure\n';
    for (let i = 0; i < dataHistory.timestamps.length; i++) {
        csv += `${dataHistory.timestamps[i]},${dataHistory.temperature[i]},${dataHistory.pressure[i]}\n`;
    }
    
    downloadFile(csv, 'greenhouse-data.csv', 'text/csv');
    showNotification('Δεδομένα εξήχθησαν σε CSV', 'success');
}

function exportJSON() {
    const jsonData = JSON.stringify({
        exportDate: new Date().toISOString(),
        data: {
            timestamps: dataHistory.timestamps,
            temperature: dataHistory.temperature,
            pressure: dataHistory.pressure
        },
        statistics: {
            temperatureAvg: dataHistory.temperature.reduce((a, b) => a + b, 0) / dataHistory.temperature.length,
            pressureAvg: dataHistory.pressure.reduce((a, b) => a + b, 0) / dataHistory.pressure.length,
            dataPoints: dataHistory.timestamps.length
        }
    }, null, 2);
    
    downloadFile(jsonData, 'greenhouse-data.json', 'application/json');
    showNotification('Δεδομένα εξήχθησαν σε JSON', 'success');
}

function generateReport() {
    const report = `
GREENHOUSE MONITORING REPORT
Generated: ${new Date().toLocaleString()}

CURRENT CONDITIONS:
- Temperature: ${document.getElementById('temperature-value').textContent}°C
- Pressure: ${document.getElementById('pressure-value').textContent} hPa
- Last Update: ${document.getElementById('last-update').textContent}

SYSTEM STATUS:
- Device IP: ${document.getElementById('device-ip').textContent}
- WiFi Signal: ${document.getElementById('wifi-strength').textContent}
- Uptime: ${document.getElementById('uptime').textContent}
- Free Memory: ${document.getElementById('memory-free').textContent}

STATISTICS (24h):
- Data Points: ${dataHistory.timestamps.length}
- Avg Temperature: ${(dataHistory.temperature.reduce((a, b) => a + b, 0) / dataHistory.temperature.length).toFixed(1)}°C
- Avg Pressure: ${(dataHistory.pressure.reduce((a, b) => a + b, 0) / dataHistory.pressure.length).toFixed(1)} hPa
    `;
    
    downloadFile(report, 'greenhouse-report.txt', 'text/plain');
    showNotification('Αναφορά δημιουργήθηκε', 'success');
}

// Plant care AI recommendations
function generatePlantCareRecommendations() {
    const recommendations = [
        { icon: '💧', text: 'Ελέγξτε την υγρασία του εδάφους καθημερινά' },
        { icon: '🌱', text: 'Περιστρέψτε τα φυτά για ομοιόμορφη ανάπτυξη' },
        { icon: '✂️', text: 'Αφαιρέστε νεκρά φύλλα για καλύτερη υγεία' },
        { icon: '🌡️', text: 'Διατηρήστε σταθερή θερμοκρασία 20-25°C' },
        { icon: '🍃', text: 'Εξασφαλίστε καλό αερισμό στον χώρο' }
    ];
    
    const container = document.querySelector('.plant-care');
    if (!container) return;
    
    container.innerHTML = '<h2>🤖 AI Συστάσεις Φροντίδας</h2>';
    
    recommendations.forEach(rec => {
        const div = document.createElement('div');
        div.className = 'care-recommendation';
        div.innerHTML = `
            <span class="care-icon">${rec.icon}</span>
            <span>${rec.text}</span>
        `;
        container.appendChild(div);
    });
}

// Utility functions
function downloadFile(content, filename, mimeType) {
    const blob = new Blob([content], { type: mimeType });
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    window.URL.revokeObjectURL(url);
}

function showNotification(message, type) {
    const notification = document.createElement('div');
    notification.className = `notification ${type}`;
    notification.textContent = message;
    
    document.body.appendChild(notification);
    
    setTimeout(() => notification.classList.add('show'), 100);
    setTimeout(() => {
        notification.classList.remove('show');
        setTimeout(() => document.body.removeChild(notification), 300);
    }, 3000);
}

function formatUptime(seconds) {
    if (!seconds) return '--';
    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    return `${days}d ${hours}h ${minutes}m`;
}

function startAutoUpdate() {
    if (updateInterval) clearInterval(updateInterval);
    updateInterval = setInterval(updateData, refreshRate);
}

function saveSettings() {
    localStorage.setItem('greenhouse-settings', JSON.stringify({
        theme: currentTheme,
        refreshRate: refreshRate,
        weatherEffect: weatherEffect
    }));
}

function loadSettings() {
    const saved = localStorage.getItem('greenhouse-settings');
    if (saved) {
        const settings = JSON.parse(saved);
        currentTheme = settings.theme || 'light';
        refreshRate = settings.refreshRate || 2000;
        weatherEffect = settings.weatherEffect || 'none';
        
        if (currentTheme === 'dark') {
            document.body.className = 'dark-theme';
        }
    }
}
function formatUptime(seconds) {
    if (!seconds) return '--';
    
    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    
    if (days > 0) {
        return `${days}d ${hours}h ${minutes}m`;
    } else if (hours > 0) {
        return `${hours}h ${minutes}m`;
    } else {
        return `${minutes}m`;
    }
}

// Start automatic updates every 5 seconds
function startAutoUpdate() {
    if (updateInterval) {
        clearInterval(updateInterval);
    }
    
    updateInterval = setInterval(updateData, 5000);
}

// Stop automatic updates
function stopAutoUpdate() {
    if (updateInterval) {
        clearInterval(updateInterval);
        updateInterval = null;
    }
}

// Handle page visibility changes to save resources
document.addEventListener('visibilitychange', function() {
    if (document.hidden) {
        stopAutoUpdate();
    } else {
        startAutoUpdate();
        updateData(); // Immediate update when page becomes visible
    }
});

// Handle page unload
window.addEventListener('beforeunload', function() {
    stopAutoUpdate();
});

// Firebase chart update function
window.updateFirebaseCharts = function(entries) {
    if (!entries || entries.length === 0) return;
    
    const timestamps = entries.map(e => new Date(e.timestamp).toLocaleTimeString());
    const temps = entries.map(e => e.temperature || 0);
    const pressures = entries.map(e => e.pressure || 0);
    const soils = entries.map(e => e.soilMoisture || 0);
    const lights = entries.map(e => e.light || 0);
    
    // Update temperature chart
    if (temperatureChart) {
        temperatureChart.data.labels = timestamps;
        temperatureChart.data.datasets[0].data = temps;
        temperatureChart.update('none');
    }
    
    // Update pressure chart
    if (pressureChart) {
        pressureChart.data.labels = timestamps;
        pressureChart.data.datasets[0].data = pressures;
        pressureChart.update('none');
    }
    
    // Update soil chart
    if (soilMoistureChart) {
        soilMoistureChart.data.labels = timestamps;
        soilMoistureChart.data.datasets[0].data = soils;
        soilMoistureChart.update('none');
    }
    
    // Update light chart
    if (lightChart) {
        lightChart.data.labels = timestamps;
        lightChart.data.datasets[0].data = lights;
        lightChart.update('none');
    }
    
    console.log(`📊 Charts updated with ${entries.length} data points`);
};
