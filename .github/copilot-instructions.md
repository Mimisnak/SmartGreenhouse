# Copilot Instructions

<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

This is an ESP32-S3 Arduino project for temperature monitoring using the BMP280 sensor.

## Project Context
- Hardware: ESP32-S3 N16R8 microcontroller
- Sensor: BMP280 (temperature and atmospheric pressure sensor)
- Communication: I2C protocol
- Purpose: Temperature measurement and monitoring

## Code Style Guidelines
- Use Arduino IDE compatible code
- Follow Arduino naming conventions
- Include proper pin definitions for ESP32-S3
- Use libraries: Adafruit_BMP280, Wire
- Add error handling for sensor initialization
- Include serial output for debugging

## Hardware Configuration
- BMP280 connected via I2C (SDA/SCL pins)
- Power supply: 3.3V from ESP32-S3
- I2C address: typically 0x76 or 0x77
