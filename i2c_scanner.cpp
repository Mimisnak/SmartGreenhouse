#include <Arduino.h>
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("I2C Scanner");
  Serial.println("Scanning for I2C devices...");
  
  // Initialize I2C with custom pins
  Wire.begin(8, 9); // SDA=8, SCL=9
  
  byte error, address;
  int nDevices = 0;
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
    Serial.println("Check your wiring:");
    Serial.println("- VCC to 3.3V (NOT 5V!)");
    Serial.println("- GND to GND");
    Serial.println("- SDA to GPIO 8");
    Serial.println("- SCL to GPIO 9");
  }
  else {
    Serial.print("Found ");
    Serial.print(nDevices);
    Serial.println(" device(s)");
  }
}

void loop() {
  delay(5000);
}
