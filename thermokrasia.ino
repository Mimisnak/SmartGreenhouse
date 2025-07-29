#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// Create BMP280 object
Adafruit_BMP280 bmp;

// ESP32-S3 I2C pins (ΑΛΛΑΞΤΕ ΤΑ ΑΝ ΧΡΕΙΑΖΕΤΑΙ!)
// Μπορείτε να χρησιμοποιήσετε οποιαδήποτε από αυτά τα pins:
// SDA: 4, 5, 8, 9, 10, 15, 16, 17, 18, 19, 20, 21, 35, 36, 37, 38
// SCL: 4, 5, 8, 9, 10, 15, 16, 17, 18, 19, 20, 22, 35, 36, 37, 38

#define SDA_PIN 8   // Αλλάξτε αυτό αν θέλετε άλλο pin
#define SCL_PIN 9   // Αλλάξτε αυτό αν θέλετε άλλο pin

// Παραδείγματα εναλλακτικών:
// #define SDA_PIN 8   // για GPIO 8
// #define SCL_PIN 9   // για GPIO 9

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial) delay(100);
  
  Serial.println("ESP32-S3 BMP280 Temperature Monitor");
  Serial.println("====================================");
  
  // Initialize I2C with custom pins
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Initialize BMP280 sensor
  if (!bmp.begin(0x76)) {  // Try address 0x76 first
    Serial.println("Could not find BMP280 sensor at 0x76, trying 0x77...");
    if (!bmp.begin(0x77)) {  // Try address 0x77
      Serial.println("Could not find a valid BMP280 sensor!");
      Serial.println("Check wiring and I2C address.");
      while (1) delay(100);  // Stop here if sensor not found
    }
  }
  
  Serial.println("BMP280 sensor found and initialized!");
  
  // Configure sensor settings (optional)
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time */
  
  Serial.println("Setup completed. Starting measurements...");
  Serial.println();
}

void loop() {
  // Read temperature and pressure
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0; // Convert to hPa
  
  // Check if readings are valid
  if (isnan(temperature) || isnan(pressure)) {
    Serial.println("Error reading from BMP280 sensor!");
    delay(2000);
    return;
  }
  
  // Display results
  Serial.println("=== BMP280 Readings ===");
  Serial.print("Θερμοκρασία: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  Serial.print("Πίεση: ");
  Serial.print(pressure);
  Serial.println(" hPa");
  
  Serial.println("------------------------");
  Serial.println();
  
  // Wait 2 seconds before next reading
  delay(2000);
}
