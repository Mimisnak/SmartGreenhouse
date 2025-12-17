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

function initializeCharts() {
    console.log('📊 Initializing charts...');
    
    const chartConfig = {
        responsive: true,
        maintainAspectRatio: false,
        animation: { duration: 500 },
        plugins: {
            legend: { display: true, position: 'top' }
        },
        scales: {
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

function startFirebaseListeners() {
    console.log('🔥 Starting Firebase listeners...');
    
    // Listen to the entire ESP32-Greenhouse node
    const sensorsRef = ref(database, 'sensors/ESP32-Greenhouse');
    onValue(sensorsRef, (snapshot) => {
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
            updateUI(sensorData);
            updateCharts(sensorData);
        } else {
            console.error('❌ No temperature data found in:', sensorData);
            console.log('Available keys:', Object.keys(sensorData));
        }
    }, (error) => {
        console.error('❌ Firebase error:', error);
    });
}

// ============================================
// UI UPDATE FUNCTIONS
// ============================================

function updateUI(data) {
    console.log('🔄 [v20251217172200] Updating UI with:', data);
    
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
    
    console.log('✅ [v20251217172200] UI UPDATE COMPLETE');
}

function updateCharts(data) {
    // Don't update charts if timestamp hasn't changed
    if (data.timestamp && data.timestamp === lastTimestamp) {
        console.log('⏭️ Skipping chart update - same timestamp');
        return;
    }
    
    if (data.timestamp) {
        lastTimestamp = data.timestamp;
    }
    
    const now = new Date().toLocaleTimeString('el-GR', { hour: '2-digit', minute: '2-digit' });

    // Update Temperature Chart
    if (temperatureChart && data.temperature !== undefined) {
        temperatureChart.data.labels.push(now);
        temperatureChart.data.datasets[0].data.push(data.temperature);
        
        // Keep last 50 points
        if (temperatureChart.data.labels.length > 50) {
            temperatureChart.data.labels.shift();
            temperatureChart.data.datasets[0].data.shift();
        }
        temperatureChart.update('none');
        console.log('📈 Temperature chart updated');
    }

    // Update Pressure Chart
    if (pressureChart && data.pressure !== undefined) {
        pressureChart.data.labels.push(now);
        pressureChart.data.datasets[0].data.push(data.pressure);
        
        // Keep last 50 points
        if (pressureChart.data.labels.length > 50) {
            pressureChart.data.labels.shift();
            pressureChart.data.datasets[0].data.shift();
        }
        pressureChart.update('none');
        console.log('📈 Pressure chart updated');
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
