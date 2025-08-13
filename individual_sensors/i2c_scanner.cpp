/*
 * I2C Scanner - Detect connected I2C devices
 * GPIO: SDA=16, SCL=17
 */
#include <Wire.h>

#define SDA_PIN 16
#define SCL_PIN 17

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("I2C Scanner - ESP32-S3");
  Serial.println("SDA = GPIO 16, SCL = GPIO 17");
  Serial.println("Scanning...");
}

void loop() {
  byte error, address;
  int deviceCount = 0;
  
  Serial.println("\n--- I2C Scan ---");
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      
      // Identify common sensors
      switch(address) {
        case 0x23: Serial.println(" (BH1750 Light Sensor)"); break;
        case 0x76: Serial.println(" (BMP280 Temp/Pressure - Primary)"); break;
        case 0x77: Serial.println(" (BMP280 Temp/Pressure - Alternative)"); break;
        case 0x48: Serial.println(" (ADS1115 ADC)"); break;
        case 0x68: Serial.println(" (DS3231 RTC or MPU6050 IMU)"); break;
        case 0x3C: Serial.println(" (OLED Display 128x64)"); break;
        case 0x3D: Serial.println(" (OLED Display 128x64 Alt)"); break;
        case 0x50: Serial.println(" (EEPROM)"); break;
        case 0x57: Serial.println(" (EEPROM)"); break;
        default: Serial.println(" (Unknown device)"); break;
      }
      deviceCount++;
    }
    else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("No I2C devices found");
    Serial.println("Check connections:");
    Serial.println("- SDA to GPIO 16");
    Serial.println("- SCL to GPIO 17");
    Serial.println("- VCC to 3.3V");
    Serial.println("- GND to GND");
  } else {
    Serial.print("Found ");
    Serial.print(deviceCount);
    Serial.println(" device(s)");
  }
  
  delay(5000); // Scan every 5 seconds
}
