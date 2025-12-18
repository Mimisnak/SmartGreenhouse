// ==================== ESP32 IP CONFIGURATION ====================
// Auto-detect if running locally on ESP32 or remotely (GitHub Pages)
let ESP32_BASE_URL = '';
let isLocalMode = false;

function detectAndConfigureESP32() {
    // Check if running on ESP32 (local) or GitHub Pages (remote)
    const hostname = window.location.hostname;
    isLocalMode = hostname.match(/^\d+\.\d+\.\d+\.\d+$/) || hostname === 'localhost';
    
    if (isLocalMode) {
        // Running on ESP32 - use relative URLs
        ESP32_BASE_URL = '';
        console.log('✅ Local Mode: Running on ESP32 at ' + window.location.host);
        hideIPConfig();
    } else {
        // Running remotely (GitHub Pages) - need ESP32 IP
        const savedIP = localStorage.getItem('esp32_ip');
        if (savedIP) {
            ESP32_BASE_URL = `http://${savedIP}`;
            console.log('✅ Remote Mode: Using saved ESP32 IP ' + savedIP);
        } else {
            console.warn('⚠️ Remote Mode: ESP32 IP not configured');
            showIPConfig();
        }
    }
}

function showIPConfig() {
    const configDiv = document.getElementById('ipConfigSection');
    if (configDiv) configDiv.style.display = 'block';
}

function hideIPConfig() {
    const configDiv = document.getElementById('ipConfigSection');
    if (configDiv) configDiv.style.display = 'none';
}

function saveESP32IP() {
    const ipInput = document.getElementById('esp32IpInput');
    const ip = ipInput.value.trim();
    
    // Validate IP format
    const ipPattern = /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}(:\d+)?$/;
    if (!ipPattern.test(ip)) {
        alert('⚠️ Invalid IP format! Use: 192.168.1.100 or 192.168.1.100:80');
        return;
    }
    
    localStorage.setItem('esp32_ip', ip);
    ESP32_BASE_URL = `http://${ip}`;
    hideIPConfig();
    showNotification(`✅ ESP32 IP saved: ${ip}`, 'success');
    console.log('✅ ESP32 IP configured:', ESP32_BASE_URL);
    
    // Reload data with new IP
    loadHistoricalData();
    updateLiveValues();
}

function clearESP32IP() {
    localStorage.removeItem('esp32_ip');
    ESP32_BASE_URL = '';
    document.getElementById('esp32IpInput').value = '';
    showIPConfig();
    showNotification('🗑️ ESP32 IP cleared', 'info');
}

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
let soilMoistureHistory = []; // Track last 5 readings to detect unstable sensor
let currentTheme = 'light';
let isPlaying = true;
let refreshRate = 2000;
let weatherEffect = 'none';
let DEBUG_MODE = true; // Set to false to disable debug logs

// Initialize when page loads
document.addEventListener('DOMContentLoaded', function() {
    detectAndConfigureESP32(); // Detect ESP32 IP or prompt for configuration
    loadSettings();
    initializeCharts();
    createWeatherEffects();
    loadHistoricalData(); // Load historical data on startup
    updateLiveValues(); // Initial live values update
    updateCharts(); // Initial charts update
    startAutoUpdate();
    generatePlantCareRecommendations();
});

// Load historical data from ESP32 server
async function loadHistoricalData() {
    try {
        const url = ESP32_BASE_URL + '/history';
        if (DEBUG_MODE) console.log('🔄 Loading historical data from:', url);
        const response = await fetch(url);
        
        if (!response.ok) {
            console.warn('No historical data available yet');
            return;
        }
        
        const historyData = await response.json();
        if (DEBUG_MODE) {
            console.log('📦 Raw history data received:', historyData);
            console.log(`📊 Data arrays - Temp: ${historyData.temperature?.length}, Timestamps: ${historyData.timestamps?.length}`);
        }
        
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
                // ESP32 sends 'soil' in /history endpoint, not 'soilMoisture'
                dataHistory.soilMoisture.push(historyData.soil[i] !== undefined ? historyData.soil[i] : 0);
                dataHistory.light.push(historyData.light[i] !== undefined ? historyData.light[i] : 0);
                
                // Convert Unix timestamp (seconds) to JavaScript Date
                // Timestamps come from ESP32 as Unix timestamps in seconds
                const unixTimestamp = historyData.timestamps[i];
                const timestamp = new Date(unixTimestamp * 1000); // Convert seconds to milliseconds
                dataHistory.timestamps.push(timestamp.toISOString());
                
                if (DEBUG_MODE && i < 3) {
                    console.log(`📅 Sample #${i}: Unix=${unixTimestamp}, Date=${timestamp.toLocaleString()}`);
                }
            }
            
            // Update charts with historical data
            updateChartsWithHistory();
            
            console.log(`✅ Loaded ${historyData.temperature.length} historical data points`);
            console.log(`📅 Time range: ${new Date(historyData.timestamps[0] * 1000).toLocaleString()} to ${new Date(historyData.timestamps[historyData.timestamps.length-1] * 1000).toLocaleString()}`);
            showNotification(`Φορτώθηκαν ${historyData.temperature.length} ιστορικά δεδομένα!`, 'success');
        }
        
    } catch (error) {
        console.warn('Failed to load historical data:', error);
    }
}

// Update charts with historical data
function updateChartsWithHistory() {
    if (dataHistory.timestamps.length === 0) return;
    
    // Create time labels for charts with smart formatting
    const labels = dataHistory.timestamps.map(timestamp => {
        const date = new Date(timestamp);
        const now = new Date();
        const diffHours = Math.abs(now - date) / 36e5; // Hours difference
        
        // If data is from today, show time only
        if (date.toDateString() === now.toDateString()) {
            return date.toLocaleTimeString('el-GR', { hour: '2-digit', minute: '2-digit' });
        }
        // If older data, show date and time
        else if (diffHours < 48) {
            return date.toLocaleDateString('el-GR', { month: 'short', day: 'numeric', hour: '2-digit', minute: '2-digit' });
        }
        // For very old data, show date only
        else {
            return date.toLocaleDateString('el-GR', { month: 'short', day: 'numeric' });
        }
    });
    
    // Update temperature chart
    temperatureChart.data.labels = labels;
    temperatureChart.data.datasets[0].data = [...dataHistory.temperature];
    temperatureChart.update('none'); // No animation for smoother update
    
    // Update pressure chart  
    pressureChart.data.labels = labels;
    pressureChart.data.datasets[0].data = [...dataHistory.pressure];
    pressureChart.update('none');
    
    // Update soil moisture chart
    soilMoistureChart.data.labels = labels;
    soilMoistureChart.data.datasets[0].data = [...dataHistory.soilMoisture];
    soilMoistureChart.update('none');
    
    // Update light chart
    lightChart.data.labels = labels;
    lightChart.data.datasets[0].data = [...dataHistory.light];
    lightChart.update('none');
    
    console.log(`📊 Charts updated with ${labels.length} data points`);
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
                tension: 0.4,
                pointRadius: 3,
                pointHoverRadius: 5
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: true,
            interaction: {
                mode: 'index',
                intersect: false
            },
            scales: {
                x: {
                    display: true,
                    title: {
                        display: true,
                        text: 'Χρόνος'
                    },
                    ticks: {
                        maxRotation: 45,
                        minRotation: 0,
                        autoSkip: true,
                        maxTicksLimit: 8
                    }
                },
                y: {
                    beginAtZero: false,
                    title: {
                        display: true,
                        text: '°C'
                    }
                }
            },
            plugins: {
                legend: {
                    display: true,
                    position: 'top'
                },
                tooltip: {
                    enabled: true,
                    mode: 'index',
                    intersect: false,
                    callbacks: {
                        title: function(context) {
                            return 'Ώρα: ' + context[0].label;
                        },
                        label: function(context) {
                            return context.dataset.label + ': ' + context.parsed.y.toFixed(1) + '°C';
                        }
                    }
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
                tension: 0.4,
                pointRadius: 3,
                pointHoverRadius: 5
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: true,
            interaction: {
                mode: 'index',
                intersect: false
            },
            scales: {
                x: {
                    display: true,
                    title: {
                        display: true,
                        text: 'Χρόνος'
                    },
                    ticks: {
                        maxRotation: 45,
                        minRotation: 0,
                        autoSkip: true,
                        maxTicksLimit: 8
                    }
                },
                y: {
                    beginAtZero: false,
                    title: {
                        display: true,
                        text: 'hPa'
                    }
                }
            },
            plugins: {
                legend: {
                    display: true,
                    position: 'top'
                },
                tooltip: {
                    enabled: true,
                    callbacks: {
                        label: function(context) {
                            return context.dataset.label + ': ' + context.parsed.y.toFixed(1) + ' hPa';
                        }
                    }
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
                tension: 0.4,
                pointRadius: 3,
                pointHoverRadius: 5
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: true,
            interaction: {
                mode: 'index',
                intersect: false
            },
            scales: {
                x: {
                    display: true,
                    title: {
                        display: true,
                        text: 'Χρόνος'
                    },
                    ticks: {
                        maxRotation: 45,
                        minRotation: 0,
                        autoSkip: true,
                        maxTicksLimit: 8
                    }
                },
                y: {
                    beginAtZero: true,
                    max: 100,
                    title: {
                        display: true,
                        text: '%'
                    }
                }
            },
            plugins: {
                legend: {
                    display: true,
                    position: 'top'
                },
                tooltip: {
                    enabled: true,
                    callbacks: {
                        label: function(context) {
                            return context.dataset.label + ': ' + context.parsed.y.toFixed(0) + '%';
                        }
                    }
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
                tension: 0.4,
                pointRadius: 3,
                pointHoverRadius: 5
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: true,
            interaction: {
                mode: 'index',
                intersect: false
            },
            scales: {
                x: {
                    display: true,
                    title: {
                        display: true,
                        text: 'Χρόνος'
                    },
                    ticks: {
                        maxRotation: 45,
                        minRotation: 0,
                        autoSkip: true,
                        maxTicksLimit: 8
                    }
                },
                y: {
                    beginAtZero: true,
                    title: {
                        display: true,
                        text: 'lux'
                    }
                }
            },
            plugins: {
                legend: {
                    display: true,
                    position: 'top'
                },
                tooltip: {
                    enabled: true,
                    callbacks: {
                        label: function(context) {
                            return context.dataset.label + ': ' + context.parsed.y.toFixed(0) + ' lux';
                        }
                    }
                }
            }
        }
    });
}

// Update sensor data and charts
// Update live sensor values (called every 5 seconds)
async function updateLiveValues() {
    try {
        const url = ESP32_BASE_URL + '/api';
        const response = await fetch(url);
        
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }
        
        const data = await response.json();
        
        // Update status cards displays
        document.getElementById('temperature').textContent = data.temperature + '°C';
        document.getElementById('pressure').textContent = data.pressure + ' hPa';
        
        // Track soil moisture history for stability check
        soilMoistureHistory.push(data.soilMoisture);
        if (soilMoistureHistory.length > 5) {
            soilMoistureHistory.shift(); // Keep only last 5 readings
        }
        
        // Check if sensor is unstable (rapid changes indicate disconnected sensor)
        let isUnstable = false;
        if (soilMoistureHistory.length >= 3) {
            // Check if any consecutive readings differ by more than 15%
            for (let i = 1; i < soilMoistureHistory.length; i++) {
                const diff = Math.abs(soilMoistureHistory[i] - soilMoistureHistory[i-1]);
                if (diff > 15) {
                    isUnstable = true;
                    break;
                }
            }
        }
        
        // Display soil moisture based on stability
        if (isUnstable) {
            document.getElementById('soilMoisture').textContent = '--';
            document.getElementById('soilStatus').textContent = 'ΑΝΕΝΕΡΓΟΣ';
            document.getElementById('soilStatus').className = 'card-status status-critical';
        } else {
            document.getElementById('soilMoisture').textContent = data.soilMoisture + '%';
            updateStatusIndicator('soilStatus', data.soilMoisture, 40, 80);
        }
        
        document.getElementById('light').textContent = data.light + ' lux';
        
        // Update status bar with live values
        const soilDisplay = isUnstable ? '--' : data.soilMoisture + '%';
        document.getElementById('liveValues').textContent = 
            `🌡️ ${data.temperature}°C | 🔘 ${data.pressure} hPa | 💧 ${soilDisplay} | ☀️ ${data.light} lux`;
        
        // Update WiFi signal strength
        if (data.wifiRSSI !== undefined) {
            const rssi = data.wifiRSSI;
            document.getElementById('wifiSignal').textContent = rssi + ' dBm';
            // RSSI ranges: Excellent > -50, Good > -60, Fair > -70, Weak > -80, Poor <= -80
            let wifiStatus = document.getElementById('wifiStatus');
            if (rssi > -50) {
                wifiStatus.className = 'card-status status-excellent';
                wifiStatus.textContent = '📶 Άριστο';
            } else if (rssi > -60) {
                wifiStatus.className = 'card-status status-good';
                wifiStatus.textContent = '📶 Καλό';
            } else if (rssi > -70) {
                wifiStatus.className = 'card-status status-warning';
                wifiStatus.textContent = '⚠️ Μέτριο';
            } else if (rssi > -80) {
                wifiStatus.className = 'card-status status-warning';
                wifiStatus.textContent = '⚠️ Αδύναμο';
            } else {
                wifiStatus.className = 'card-status status-critical';
                wifiStatus.textContent = '❌ Πολύ Αδύναμο';
            }
        }
        
        // Update status indicators
        updateStatusIndicator('tempStatus', data.temperature, 18, 28);
        updateStatusIndicator('pressureStatus', data.pressure, 1000, 1030);
        // soilStatus already updated above based on 99% check
        updateStatusIndicator('lightStatus', data.light, 200, 2000);
        
        // Update system info (totalReadings, min/max temp)
        if (data.totalReadings !== undefined) {
            document.getElementById('totalReadings').textContent = data.totalReadings;
        }
        if (data.minTemperature !== undefined && data.minTemperature < 999) {
            document.getElementById('minTemp').textContent = data.minTemperature.toFixed(1) + '°C';
        }
        if (data.maxTemperature !== undefined && data.maxTemperature > -999) {
            document.getElementById('maxTemp').textContent = data.maxTemperature.toFixed(1) + '°C';
        }
        
        // Update last update time
        const now = new Date();
        document.getElementById('pageLoadTime').textContent = now.toLocaleTimeString('el-GR');
        document.getElementById('systemStatus').textContent = 'Live ✅';
    } catch (error) {
        console.error('Failed to update live values:', error);
        document.getElementById('systemStatus').textContent = 'Error ❌';
    }
}

// Update charts with latest data (called every 6 minutes)
async function updateCharts() {
    try {
        const url = ESP32_BASE_URL + '/api';
        const response = await fetch(url);
        
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }
        
        const data = await response.json();
        
        // Add current data point to charts
        const now = new Date();
        const timeLabel = now.toLocaleTimeString('el-GR', { hour: '2-digit', minute: '2-digit' });
        
        updateChart(temperatureChart, data.temperature, timeLabel);
        updateChart(pressureChart, data.pressure, timeLabel);
        updateChart(soilMoistureChart, data.soilMoisture, timeLabel);
        updateChart(lightChart, data.light, timeLabel);
        
        // Add to local history for persistence
        dataHistory.temperature.push(data.temperature);
        dataHistory.pressure.push(data.pressure);
        dataHistory.soilMoisture.push(data.soilMoisture);
        dataHistory.light.push(data.light);
        dataHistory.timestamps.push(now.toISOString());
        
        // Keep only last 96 points in browser memory (48h @ 30min intervals)
        if (dataHistory.temperature.length > 96) {
            dataHistory.temperature.shift();
            dataHistory.pressure.shift();
            dataHistory.soilMoisture.shift();
            dataHistory.light.shift();
            dataHistory.timestamps.shift();
        }
        
        // Note: Historical data is now managed by ESP32 server, not browser
        // Data persistence survives page refreshes thanks to server-side storage
        
        // Don't show notification on every update (too noisy)
        // showNotification('Δεδομένα ενημερώθηκαν επιτυχώς!', 'success');
        
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
    // Add new data point
    chart.data.labels.push(label);
    chart.data.datasets[0].data.push(value);
    
    // Keep only last 50 data points in real-time view (about 4 minutes at 5s interval)
    // Historical data (24h) is loaded separately from server
    const maxPoints = 50;
    if (chart.data.labels.length > maxPoints) {
        chart.data.labels.shift();
        chart.data.datasets[0].data.shift();
    }
    
    // Update without animation for smoother experience
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

// Unified startAutoUpdate with history reload every 5 minutes
let liveValuesInterval = null;
let chartsUpdateInterval = null;
let historyReloadInterval = null;

function startAutoUpdate() {
    // Stop any existing intervals
    if (liveValuesInterval) clearInterval(liveValuesInterval);
    if (chartsUpdateInterval) clearInterval(chartsUpdateInterval);
    if (historyReloadInterval) clearInterval(historyReloadInterval);
    
    // Update live values every 5 seconds
    updateLiveValues(); // Initial update
    liveValuesInterval = setInterval(updateLiveValues, 5000);
    
    // Update charts every 5 minutes (300000ms)
    updateCharts(); // Initial update
    chartsUpdateInterval = setInterval(updateCharts, 300000);
    
    // Reload historical data every 5 minutes to sync with ESP32
    historyReloadInterval = setInterval(() => {
        if (DEBUG_MODE) console.log('🔄 Auto-reloading historical data...');
        loadHistoricalData();
    }, 300000); // 5 minutes = 300000ms
    
    if (DEBUG_MODE) console.log('✅ Auto-update started: Live values=5s, Charts=5min, History reload=5min');
}

// Stop automatic updates
function stopAutoUpdate() {
    if (liveValuesInterval) {
        clearInterval(liveValuesInterval);
        liveValuesInterval = null;
    }
    if (chartsUpdateInterval) {
        clearInterval(chartsUpdateInterval);
        chartsUpdateInterval = null;
    }
    if (historyReloadInterval) {
        clearInterval(historyReloadInterval);
        historyReloadInterval = null;
    }
    if (DEBUG_MODE) console.log('⏸️ Auto-update stopped');
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

// Handle page visibility changes to save resources
document.addEventListener('visibilitychange', function() {
    if (document.hidden) {
        stopAutoUpdate();
    } else {
        startAutoUpdate();
        updateData(); // Immediate update when page becomes visible
        loadHistoricalData(); // Also reload history when page becomes visible
    }
});

// Handle page unload
window.addEventListener('beforeunload', function() {
    stopAutoUpdate();
});
