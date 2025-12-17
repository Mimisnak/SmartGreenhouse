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
    
    // Listen to current sensor data - use 'latest' path
    const sensorsRef = ref(database, 'sensors/ESP32-Greenhouse/latest');
    onValue(sensorsRef, (snapshot) => {
        const data = snapshot.val();
        if (data) {
            console.log('📊 Firebase data:', data);
            updateUI(data);
            updateCharts(data);
        } else {
            console.warn('⚠️ No data at latest path');
        }
    }, (error) => {
        console.error('❌ Firebase error:', error);
    });
}

// ============================================
// UI UPDATE FUNCTIONS
// ============================================

function updateUI(data) {
    console.log('🔄 Updating UI with:', data);
    
    // Update temperature
    const tempElement = document.getElementById('temperature');
    if (tempElement && data.temperature !== undefined) {
        tempElement.textContent = data.temperature.toFixed(1) + '°C';
    }

    // Update pressure
    const pressElement = document.getElementById('pressure');
    if (pressElement && data.pressure !== undefined) {
        pressElement.textContent = data.pressure.toFixed(0) + ' hPa';
    }

    // Update soil moisture
    const soilElement = document.getElementById('soilMoisture');
    if (soilElement && data.soilMoisture !== undefined) {
        soilElement.textContent = data.soilMoisture.toFixed(0) + '%';
        
        // Update soil status
        const soilStatus = document.getElementById('soilStatus');
        if (soilStatus) {
            if (data.soilMoisture < 20) {
                soilStatus.textContent = '⚠️ Low - Water needed!';
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

    // Update light (handle N/A)
    const lightElement = document.getElementById('light');
    if (lightElement) {
        if (data.light !== undefined && data.light !== null && data.light >= 0) {
            lightElement.textContent = data.light.toFixed(0) + ' lux';
        } else {
            lightElement.textContent = 'N/A';
        }
    }
}

function updateCharts(data) {
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
