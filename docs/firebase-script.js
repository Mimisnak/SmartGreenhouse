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

let temperatureChart, pressureChart, lightChart, soilChart, historyChart;

function initializeCharts() {
    const chartConfig = {
        responsive: true,
        maintainAspectRatio: false,
        animation: { duration: 750 },
        plugins: {
            legend: { display: false }
        },
        scales: {
            y: { beginAtZero: false }
        }
    };

    // Temperature Chart
    const tempCtx = document.getElementById('temperatureChart')?.getContext('2d');
    if (tempCtx) {
        temperatureChart = new Chart(tempCtx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Temperature (°C)',
                    data: [],
                    borderColor: '#e74c3c',
                    backgroundColor: 'rgba(231, 76, 60, 0.1)',
                    tension: 0.4
                }]
            },
            options: chartConfig
        });
    }

    // Pressure Chart
    const pressCtx = document.getElementById('pressureChart')?.getContext('2d');
    if (pressCtx) {
        pressureChart = new Chart(pressCtx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Pressure (hPa)',
                    data: [],
                    borderColor: '#3498db',
                    backgroundColor: 'rgba(52, 152, 219, 0.1)',
                    tension: 0.4
                }]
            },
            options: chartConfig
        });
    }

    // Light Chart
    const lightCtx = document.getElementById('lightChart')?.getContext('2d');
    if (lightCtx) {
        lightChart = new Chart(lightCtx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Light (lux)',
                    data: [],
                    borderColor: '#f39c12',
                    backgroundColor: 'rgba(243, 156, 18, 0.1)',
                    tension: 0.4
                }]
            },
            options: chartConfig
        });
    }

    // Soil Moisture Chart
    const soilCtx = document.getElementById('soilChart')?.getContext('2d');
    if (soilCtx) {
        soilChart = new Chart(soilCtx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Soil Moisture (%)',
                    data: [],
                    borderColor: '#27ae60',
                    backgroundColor: 'rgba(39, 174, 96, 0.1)',
                    tension: 0.4
                }]
            },
            options: chartConfig
        });
    }
}

// ============================================
// FIREBASE DATA LISTENERS
// ============================================

function startFirebaseListeners() {
    console.log('🔥 Starting Firebase listeners...');
    
    // Listen to current sensor data
    const sensorsRef = ref(database, 'sensors/ESP32-Greenhouse');
    onValue(sensorsRef, (snapshot) => {
        const data = snapshot.val();
        if (data) {
            console.log('📊 Firebase data received:', data);
            updateUI(data);
            updateCharts(data);
        }
    });

    // Listen to history (last 288 points = 24h @ 5min intervals)
    const historyRef = query(
        ref(database, 'history/ESP32-Greenhouse'),
        orderByChild('timestamp'),
        limitToLast(288)
    );
    
    onValue(historyRef, (snapshot) => {
        const historyData = [];
        snapshot.forEach((childSnapshot) => {
            historyData.push(childSnapshot.val());
        });
        
        if (historyData.length > 0) {
            console.log(`📈 History loaded: ${historyData.length} points`);
            updateHistoryChart(historyData);
        }
    });
}

// ============================================
// UI UPDATE FUNCTIONS
// ============================================

function updateUI(data) {
    // Update temperature
    const tempElement = document.querySelector('.temp-value');
    if (tempElement && data.temperature !== undefined) {
        tempElement.textContent = data.temperature.toFixed(1) + '°C';
    }

    // Update pressure
    const pressElement = document.querySelector('.pressure-value');
    if (pressElement && data.pressure !== undefined) {
        pressElement.textContent = data.pressure.toFixed(0) + ' hPa';
    }

    // Update light
    const lightElement = document.querySelector('.light-value');
    if (lightElement) {
        if (data.light !== undefined && data.light !== null && data.light >= 0) {
            lightElement.textContent = data.light.toFixed(0) + ' lux';
        } else {
            lightElement.textContent = 'N/A';
        }
    }

    // Update soil moisture
    const soilElement = document.querySelector('.soil-value');
    if (soilElement && data.soilMoisture !== undefined) {
        soilElement.textContent = data.soilMoisture.toFixed(0) + '%';
        
        // Update soil status indicator
        const soilStatus = document.querySelector('.soil-status');
        if (soilStatus) {
            if (data.soilMoisture < 20) {
                soilStatus.innerHTML = '⚠️ Low - Water needed!';
                soilStatus.className = 'soil-status status-low';
            } else if (data.soilMoisture < 50) {
                soilStatus.innerHTML = '✓ Moderate';
                soilStatus.className = 'soil-status status-moderate';
            } else {
                soilStatus.innerHTML = '✓ Good';
                soilStatus.className = 'soil-status status-good';
            }
        }
    }

    // Update timestamp
    const timestampElement = document.querySelector('.last-update');
    if (timestampElement && data.timestamp) {
        const date = new Date(data.timestamp * 1000);
        timestampElement.textContent = 'Last update: ' + date.toLocaleString('el-GR');
    }
}

function updateCharts(data) {
    const now = new Date().toLocaleTimeString();

    // Update Temperature Chart
    if (temperatureChart && data.temperature !== undefined) {
        temperatureChart.data.labels.push(now);
        temperatureChart.data.datasets[0].data.push(data.temperature);
        if (temperatureChart.data.labels.length > 20) {
            temperatureChart.data.labels.shift();
            temperatureChart.data.datasets[0].data.shift();
        }
        temperatureChart.update('none');
    }

    // Update Pressure Chart
    if (pressureChart && data.pressure !== undefined) {
        pressureChart.data.labels.push(now);
        pressureChart.data.datasets[0].data.push(data.pressure);
        if (pressureChart.data.labels.length > 20) {
            pressureChart.data.labels.shift();
            pressureChart.data.datasets[0].data.shift();
        }
        pressureChart.update('none');
    }

    // Update Light Chart (if available)
    if (lightChart && data.light !== undefined && data.light !== null && data.light >= 0) {
        lightChart.data.labels.push(now);
        lightChart.data.datasets[0].data.push(data.light);
        if (lightChart.data.labels.length > 20) {
            lightChart.data.labels.shift();
            lightChart.data.datasets[0].data.shift();
        }
        lightChart.update('none');
    }

    // Update Soil Moisture Chart
    if (soilChart && data.soilMoisture !== undefined) {
        soilChart.data.labels.push(now);
        soilChart.data.datasets[0].data.push(data.soilMoisture);
        if (soilChart.data.labels.length > 20) {
            soilChart.data.labels.shift();
            soilChart.data.datasets[0].data.shift();
        }
        soilChart.update('none');
    }
}

function updateHistoryChart(historyData) {
    if (!historyChart) {
        const ctx = document.getElementById('historyChart')?.getContext('2d');
        if (!ctx) return;

        historyChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [
                    {
                        label: 'Temperature (°C)',
                        data: [],
                        borderColor: '#e74c3c',
                        backgroundColor: 'rgba(231, 76, 60, 0.1)',
                        yAxisID: 'y-temp',
                        tension: 0.3
                    },
                    {
                        label: 'Soil Moisture (%)',
                        data: [],
                        borderColor: '#27ae60',
                        backgroundColor: 'rgba(39, 174, 96, 0.1)',
                        yAxisID: 'y-soil',
                        tension: 0.3
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    title: {
                        display: true,
                        text: '24-Hour History',
                        font: { size: 16 }
                    },
                    legend: {
                        display: true,
                        position: 'top'
                    }
                },
                scales: {
                    x: {
                        title: { display: true, text: 'Time' }
                    },
                    'y-temp': {
                        type: 'linear',
                        position: 'left',
                        title: { display: true, text: 'Temperature (°C)' }
                    },
                    'y-soil': {
                        type: 'linear',
                        position: 'right',
                        title: { display: true, text: 'Soil Moisture (%)' },
                        grid: { drawOnChartArea: false }
                    }
                }
            }
        });
    }

    // Clear existing data
    historyChart.data.labels = [];
    historyChart.data.datasets[0].data = [];
    historyChart.data.datasets[1].data = [];

    // Add history data
    historyData.forEach(point => {
        if (point.timestamp) {
            const date = new Date(point.timestamp * 1000);
            const timeLabel = date.toLocaleString('el-GR', { 
                month: 'short', 
                day: 'numeric', 
                hour: '2-digit', 
                minute: '2-digit' 
            });
            
            historyChart.data.labels.push(timeLabel);
            historyChart.data.datasets[0].data.push(point.temperature || null);
            historyChart.data.datasets[1].data.push(point.soilMoisture || null);
        }
    });

    historyChart.update();
}

// ============================================
// INITIALIZATION
// ============================================

document.addEventListener('DOMContentLoaded', () => {
    console.log('🚀 Firebase version loaded');
    initializeCharts();
    startFirebaseListeners();
    
    // Add connection status indicator
    const statusDiv = document.createElement('div');
    statusDiv.className = 'firebase-status';
    statusDiv.innerHTML = '🔥 Connected to Firebase';
    statusDiv.style.cssText = 'position: fixed; top: 10px; right: 10px; background: #27ae60; color: white; padding: 8px 15px; border-radius: 20px; font-size: 12px; z-index: 1000; box-shadow: 0 2px 8px rgba(0,0,0,0.2);';
    document.body.appendChild(statusDiv);
});
