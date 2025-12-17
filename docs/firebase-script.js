// ============================================
// FIREBASE VERSION - FOR GITHUB PAGES
// ============================================

// Firebase Configuration
const firebaseConfig = {
    databaseURL: "https://smartgreenhouse-fb494-default-rtdb.firebaseio.com"
};

// Initialize Firebase (using modular SDK v9+)
import { initializeApp } from 'https://www.gstatic.com/firebasejs/10.7.1/firebase-app.js';
import { getDatabase, ref, onValue, query, orderByChild, limitToLast } from 'https://www.gstatic.com/firebasejs/10.7.1/firebase-database.js';

const app = initializeApp(firebaseConfig);
const database = getDatabase(app);

// ============================================
// CHART INITIALIZATION
// ============================================

let temperatureChart, pressureChart;
let lastTimestamp = 0; // Track last update to prevent duplicates
let lastUpdateTime = null; // Track when last data arrived

function initializeCharts() {
    console.log('📊 Initializing charts for 48h history...');
    
    const chartConfig = {
        responsive: true,
        maintainAspectRatio: false,
        animation: { duration: 500 },
        plugins: {
            legend: { display: true, position: 'top' }
        },
        scales: {
            x: {
                title: {
                    display: true,
                    text: '🕐 Time (Last 48 Hours)'
                }
            },
            y: { beginAtZero: false }
        }
    };

    // Temperature Chart
    const tempCanvas = document.getElementById('temperatureChart');
    if (tempCanvas) {
        const tempCtx = tempCanvas.getContext('2d');
        temperatureChart = new Chart(tempCtx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Temperature (°C)',
                    data: [],
                    borderColor: '#e74c3c',
                    backgroundColor: 'rgba(231, 76, 60, 0.1)',
                    tension: 0.4,
                    fill: true
                }]
            },
            options: chartConfig
        });
        console.log('✅ Temperature chart created');
    } else {
        console.error('❌ temperatureChart canvas not found');
    }

    // Pressure Chart
    const pressCanvas = document.getElementById('pressureChart');
    if (pressCanvas) {
        const pressCtx = pressCanvas.getContext('2d');
        pressureChart = new Chart(pressCtx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Pressure (hPa)',
                    data: [],
                    borderColor: '#3498db',
                    backgroundColor: 'rgba(52, 152, 219, 0.1)',
                    tension: 0.4,
                    fill: true
                }]
            },
            options: chartConfig
        });
        console.log('✅ Pressure chart created');
    } else {
        console.error('❌ pressureChart canvas not found');
    }
}

// ============================================
// FIREBASE DATA LISTENERS
// ============================================

let updateInProgress = false; // Prevent simultaneous updates

function loadHistoryData() {
    console.log('📚 Loading 48h history from Firebase...');
    
    // Calculate 48h ago timestamp
    const now = Date.now() / 1000; // Unix seconds
    const hours48Ago = now - (48 * 60 * 60);
    
    // Load history from Firebase (576 points for 48h @ 5min intervals)
    const historyRef = ref(database, 'sensors/ESP32-Greenhouse/history');
    const historyQuery = query(historyRef, orderByChild('timestamp'), limitToLast(576));
    
    onValue(historyQuery, (snapshot) => {
        const historyData = snapshot.val();
        if (!historyData) {
            console.log('⚠️ No history data found');
            return;
        }
        
        console.log('✅ Loaded', Object.keys(historyData).length, 'history points');
        
        // Convert to array and filter last 48h
        const dataPoints = Object.values(historyData)
            .filter(point => point.timestamp >= hours48Ago)
            .sort((a, b) => a.timestamp - b.timestamp);
        
        console.log('📊 Filtered to', dataPoints.length, 'points in last 48h');
        
        // SAMPLE DATA: Take every 6th point (30min intervals instead of 5min)
        // This gives us ~96 points for 48h instead of 576 (more readable!)
        const sampledPoints = dataPoints.filter((_, index) => index % 6 === 0);
        console.log('📉 Sampled to', sampledPoints.length, 'points for better readability');
        // SAMPLE DATA: Take every 6th point (30min intervals instead of 5min)
        // This gives us ~96 points for 48h instead of 576 (more readable!)
        const sampledPoints = dataPoints.filter((_, index) => index % 6 === 0);
        console.log('📉 Sampled to', sampledPoints.length, 'points for better readability');
        
        // Clear existing chart data
        if (temperatureChart) {
            temperatureChart.data.labels = [];
            temperatureChart.data.datasets[0].data = [];
        }
        if (pressureChart) {
            pressureChart.data.labels = [];
            pressureChart.data.datasets[0].data = [];
        }
        
        // Add sampled history points to charts
        sampledPoints.forEach(point => {
            const date = new Date(point.timestamp * 1000);
            const timeLabel = date.toLocaleString('el-GR', {
                month: '2-digit',
                day: '2-digit',
                hour: '2-digit',
                minute: '2-digit'
            });
            
            if (temperatureChart && point.temperature !== undefined) {
                temperatureChart.data.labels.push(timeLabel);
                temperatureChart.data.datasets[0].data.push(point.temperature);
            }
            if (pressureChart && point.pressure !== undefined) {
                pressureChart.data.labels.push(timeLabel);
                pressureChart.data.datasets[0].data.push(point.pressure);
            }
        });
        
        // Update charts
        if (temperatureChart) temperatureChart.update('none');
        if (pressureChart) pressureChart.update('none');
        
        console.log('✅ Charts loaded with 48h sampled history (~96 points)');
    }, { onlyOnce: true });
}

function startFirebaseListeners() {
    console.log('🔥 Starting Firebase listeners...');
    
    // First, load historical data
    loadHistoryData();
    
    // Then listen for real-time updates
    const sensorsRef = ref(database, 'sensors/ESP32-Greenhouse');
    onValue(sensorsRef, (snapshot) => {
        // Skip if an update is already in progress
        if (updateInProgress) {
            console.log('⏭️ Update already in progress, skipping...');
            return;
        }
        
        const rawData = snapshot.val();
        console.log('📊 Firebase raw data:', rawData);
        
        if (!rawData) {
            console.warn('⚠️ No data available');
            return;
        }
        
        // ALWAYS use data.latest if it exists
        const sensorData = rawData.latest || rawData;
        
        console.log('✅ Using sensor data:', sensorData);
        
        // Verify we have temperature before updating
        if (sensorData.temperature !== undefined) {
            updateInProgress = true;
            updateUI(sensorData);
            updateCharts(sensorData);
            // Release lock after a short delay
            setTimeout(() => { updateInProgress = false; }, 500);
        } else {
            console.error('❌ No temperature data found in:', sensorData);
            console.log('Available keys:', Object.keys(sensorData));
        }
    }, (error) => {
        console.error('❌ Firebase error:', error);
        updateInProgress = false;
    });
}

// ============================================
// UI UPDATE FUNCTIONS
// ============================================

function updateUI(data) {
    console.log('🔄 [v20251217181000] Updating UI with:', data);
    
    // Update last update timestamp
    lastUpdateTime = new Date();
    const lastUpdateElement = document.getElementById('pageLoadTime');
    if (lastUpdateElement) {
        lastUpdateElement.textContent = lastUpdateTime.toLocaleString('el-GR', {
            year: 'numeric',
            month: '2-digit',
            day: '2-digit',
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit'
        });
    }
    
    // FORCE UPDATE - Set values directly
    const tempElement = document.getElementById('temperature');
    if (tempElement) {
        if (data.temperature !== undefined) {
            tempElement.textContent = data.temperature.toFixed(1) + '°C';
            console.log('✅ Temperature FORCED to:', tempElement.textContent);
        }
    } else {
        console.error('❌ CRITICAL: temperature element NOT FOUND');
    }

    const pressElement = document.getElementById('pressure');
    if (pressElement) {
        if (data.pressure !== undefined) {
            pressElement.textContent = data.pressure.toFixed(0) + ' hPa';
            console.log('✅ Pressure FORCED to:', pressElement.textContent);
        }
    } else {
        console.error('❌ CRITICAL: pressure element NOT FOUND');
    }

    const soilElement = document.getElementById('soilMoisture');
    if (soilElement) {
        if (data.soilMoisture !== undefined) {
            soilElement.textContent = data.soilMoisture.toFixed(0) + '%';
            console.log('✅ Soil FORCED to:', soilElement.textContent);
            
            const soilStatus = document.getElementById('soilStatus');
            if (soilStatus) {
                if (data.soilMoisture < 20) {
                    soilStatus.textContent = '⚠️ Low';
                    soilStatus.className = 'card-status status-warning';
                } else if (data.soilMoisture < 50) {
                    soilStatus.textContent = '✓ Moderate';
                    soilStatus.className = 'card-status status-good';
                } else {
                    soilStatus.textContent = '✓ Optimal';
                    soilStatus.className = 'card-status status-excellent';
                }
            }
        }
    } else {
        console.error('❌ CRITICAL: soilMoisture element NOT FOUND');
    }

    const lightElement = document.getElementById('light');
    if (lightElement) {
        if (data.light !== undefined && data.light >= 0) {
            lightElement.textContent = data.light.toFixed(0) + ' lux';
            console.log('✅ Light FORCED to:', lightElement.textContent);
        } else {
            lightElement.textContent = 'N/A';
            console.log('✅ Light set to N/A');
        }
    } else {
        console.error('❌ CRITICAL: light element NOT FOUND');
    }
    
    console.log('✅ [v20251217181000] UI UPDATE COMPLETE - Live with 48h sampled history');
}

// ============================================
// CONTROL FUNCTIONS (for buttons/inputs)
// ============================================

window.updateThresholds = function() {
    const minTemp = document.getElementById('minTempInput')?.value;
    const maxTemp = document.getElementById('maxTempInput')?.value;
    console.log('🎚️ Temperature thresholds updated:', minTemp, '-', maxTemp);
};

window.toggleSound = function() {
    const button = document.getElementById('soundToggle');
    if (button.textContent.includes('ON')) {
        button.textContent = '🔇 OFF';
        console.log('🔇 Sound alerts disabled');
    } else {
        button.textContent = '🔊 ON';
        console.log('🔊 Sound alerts enabled');
    }
};

window.updateRefreshRate = function() {
    const rate = document.getElementById('refreshRate')?.value;
    console.log('⚡ Refresh rate updated:', rate, 'seconds');
};

function updateCharts(data) {
    // Don't update charts if timestamp hasn't changed
    if (data.timestamp && data.timestamp === lastTimestamp) {
        console.log('⏭️ Skipping chart update - same timestamp:', lastTimestamp);
        return;
    }
    
    if (data.timestamp) {
        console.log('📈 New timestamp:', data.timestamp, 'Previous:', lastTimestamp);
        lastTimestamp = data.timestamp;
    } else {
        console.log('⚠️ No timestamp in data, skipping chart update');
        return;
    }
    
    // Format time from Unix timestamp
    const date = new Date(data.timestamp * 1000);
    const timeLabel = date.toLocaleString('el-GR', {
        month: '2-digit',
        day: '2-digit',
        hour: '2-digit',
        minute: '2-digit'
    });

    // Calculate 48h ago cutoff
    const now = Date.now() / 1000;
    const hours48Ago = now - (48 * 60 * 60);

    // Update Temperature Chart - add new point and filter old ones
    if (temperatureChart && data.temperature !== undefined) {
        temperatureChart.data.labels.push(timeLabel);
        temperatureChart.data.datasets[0].data.push(data.temperature);
        
        // Keep last 100 points max (for live updates on top of history)
        while (temperatureChart.data.labels.length > 100) {
            temperatureChart.data.labels.shift();
            temperatureChart.data.datasets[0].data.shift();
        }
        
        temperatureChart.update('none'); // No animation to prevent flickering
        console.log('✅ Temperature chart updated (', temperatureChart.data.labels.length, 'points)');
    }

    // Update Pressure Chart - add new point and filter old ones
    if (pressureChart && data.pressure !== undefined) {
        pressureChart.data.labels.push(timeLabel);
        pressureChart.data.datasets[0].data.push(data.pressure);
        
        // Keep last 100 points max
        while (pressureChart.data.labels.length > 100) {
            pressureChart.data.labels.shift();
            pressureChart.data.datasets[0].data.shift();
        }
        
        pressureChart.update('none'); // No animation
        console.log('✅ Pressure chart updated (', pressureChart.data.labels.length, 'points)');
    }
}

// ============================================
// INITIALIZATION
// ============================================

document.addEventListener('DOMContentLoaded', () => {
    console.log('🚀 Firebase version loading...');
    
    // Wait for Chart.js to be available
    if (typeof Chart === 'undefined') {
        console.error('❌ Chart.js not loaded!');
        return;
    }
    
    console.log('✅ Chart.js loaded');
    
    // Initialize charts
    setTimeout(() => {
        initializeCharts();
        startFirebaseListeners();
        
        // Add Firebase status indicator
        const statusDiv = document.createElement('div');
        statusDiv.id = 'firebase-status';
        statusDiv.innerHTML = '🔥 Connected to Firebase';
        statusDiv.style.cssText = 'position: fixed; top: 10px; right: 10px; background: #27ae60; color: white; padding: 8px 15px; border-radius: 20px; font-size: 12px; z-index: 1000; box-shadow: 0 2px 8px rgba(0,0,0,0.2);';
        document.body.appendChild(statusDiv);
        
        console.log('✅ Firebase initialized');
    }, 100);
});
