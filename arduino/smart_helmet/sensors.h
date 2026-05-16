// sensors.h - Sensor reading functions for Smart Helmet
// Handles MPU9250, DS18B20, ACS712, and B25 fuel gauge

#ifndef SENSORS_H
#define SENSORS_H

#include <Wire.h>
#include <MPU9250.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Pin definitions
#define ONE_WIRE_BUS 4           // DS18B20 temperature sensor
#define ACS712_PIN 35            // ACS712 current sensor (analog) - MOVED from 34 to 35
#define BATTERY_VOLTAGE_PIN 34   // Battery voltage sensor (analog) - YOUR ACTUAL WIRING
#define BUZZER_PIN 25            // Buzzer output
#define LED_PIN 26               // LED output
#define RESET_BUTTON_PIN 27      // Reset location button (with pull-up)

// Battery configuration
#define BATTERY_MAX_VOLTAGE 4.2  // Maximum LiPo voltage (fully charged)
#define BATTERY_MIN_VOLTAGE 3.0  // Minimum safe voltage (cutoff - don't discharge below this!)
#define BATTERY_CAPACITY 2000    // Battery capacity in mAh
#define VOLTAGE_DIVIDER_RATIO 5.98  // Voltage divider ratio - Calibrated with multimeter (3.83V actual)

// LiPo discharge curve reference:
// 4.20V = 100% | 4.00V = ~90% | 3.85V = ~75% | 3.70V = ~50% | 3.50V = ~25% | 3.00V = 0%

// ACS712 configuration (5A model)
#define ACS712_SENSITIVITY 0.185  // 185mV per Amp for 5A model
#define ACS712_OFFSET 2.5         // Offset voltage at zero current

// MPU9250 sensor object
MPU9250 mpu;

// DS18B20 temperature sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

// Sensor data structure
struct SensorData {
    // Battery data
    float batteryVoltage;
    int batteryPercentage;
    int remainingCapacity;
    float solarCurrent;
    
    // Temperature
    float temperature;
    
    // Accelerometer (raw values)
    int16_t accelX;
    int16_t accelY;
    int16_t accelZ;
    
    // Gyroscope (raw values)
    int16_t gyroX;
    int16_t gyroY;
    int16_t gyroZ;
    
    // Magnetometer (for heading)
    int16_t magX;
    int16_t magY;
    int16_t magZ;
    
    // Acceleration magnitude (calculated)
    float accelMagnitude;
};

SensorData currentSensorData;

// Initialize all sensors
bool initializeSensors() {
    bool success = true;
    
    // Initialize I2C
    Wire.begin();
    Wire.setClock(400000);  // 400kHz I2C
    
    // Initialize MPU9250
    if (!mpu.setup(0x68)) {  // 0x68 is default MPU9250 address
        Serial.println("ERROR: MPU9250 initialization failed!");
        success = false;
    } else {
        Serial.println("MPU9250 initialized successfully");
        
        // Calibrate gyroscope and accelerometer
        delay(2000);
        Serial.println("Calibrating MPU9250...");
        mpu.calibrateAccelGyro();
        Serial.println("MPU9250 calibration complete");
    }
    
    // Initialize DS18B20 temperature sensor
    tempSensor.begin();
    int deviceCount = tempSensor.getDeviceCount();
    if (deviceCount == 0) {
        Serial.println("WARNING: DS18B20 not found!");
        success = false;
    } else {
        Serial.print("DS18B20 found: ");
        Serial.print(deviceCount);
        Serial.println(" device(s)");
        tempSensor.setResolution(12);  // 12-bit resolution (0.0625°C)
    }
    
    // Initialize analog pins
    pinMode(ACS712_PIN, INPUT);
    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
    
    // Initialize output pins
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    
    // Initialize button pin with internal pull-up
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
    
    Serial.println("Analog sensors and button configured");
    
    return success;
}

// Read battery voltage from ADC with voltage divider
float readBatteryVoltage() {
    int rawValue = analogRead(BATTERY_VOLTAGE_PIN);
    // ESP32 ADC: 0-4095 for 0-3.3V
    float voltage = (rawValue / 4095.0) * 3.3 * VOLTAGE_DIVIDER_RATIO;
    return voltage;
}

// Calculate battery percentage from voltage using LiPo discharge curve
int calculateBatteryPercentage(float voltage) {
    if (voltage >= BATTERY_MAX_VOLTAGE) return 100;
    if (voltage <= BATTERY_MIN_VOLTAGE) return 0;
    
    // Improved LiPo discharge curve approximation
    // More accurate than linear for LiPo batteries
    float percentage;
    
    if (voltage >= 4.0) {
        // 4.2V - 4.0V: 100% - 90% (nearly flat)
        percentage = 90.0 + ((voltage - 4.0) / 0.2) * 10.0;
    } else if (voltage >= 3.7) {
        // 4.0V - 3.7V: 90% - 40% (gradual)
        percentage = 40.0 + ((voltage - 3.7) / 0.3) * 50.0;
    } else if (voltage >= 3.5) {
        // 3.7V - 3.5V: 40% - 15% (steeper)
        percentage = 15.0 + ((voltage - 3.5) / 0.2) * 25.0;
    } else {
        // 3.5V - 3.0V: 15% - 0% (very steep, critical zone)
        percentage = ((voltage - 3.0) / 0.5) * 15.0;
    }
    
    return constrain((int)percentage, 0, 100);
}

// Calculate remaining capacity in mAh
int calculateRemainingCapacity(int percentage) {
    return (BATTERY_CAPACITY * percentage) / 100;
}

// Read solar charging current from ACS712
float readSolarCurrent() {
    int rawValue = analogRead(ACS712_PIN);
    // Convert ADC to voltage
    float voltage = (rawValue / 4095.0) * 3.3;
    
    // Calculate current: (Voltage - Offset) / Sensitivity
    float current = (voltage - ACS712_OFFSET) / ACS712_SENSITIVITY;
    
    // Return absolute value (charging current)
    return abs(current);
}

// Read temperature from DS18B20
float readTemperature() {
    tempSensor.requestTemperatures();
    float temp = tempSensor.getTempCByIndex(0);
    
    // Check for sensor error
    if (temp == DEVICE_DISCONNECTED_C || temp == 85.0) {
        Serial.println("WARNING: DS18B20 read error!");
        return 25.0;  // Return default room temperature
    }
    
    return temp;
}

// Read MPU9250 accelerometer, gyroscope, and magnetometer
void readMPU9250() {
    if (mpu.update()) {
        // Read accelerometer (convert to raw 16-bit values for consistency)
        currentSensorData.accelX = (int16_t)(mpu.getAccX() * 16384);  // Scale to ±2g range
        currentSensorData.accelY = (int16_t)(mpu.getAccY() * 16384);
        currentSensorData.accelZ = (int16_t)(mpu.getAccZ() * 16384);
        
        // Read gyroscope (convert to raw 16-bit values)
        currentSensorData.gyroX = (int16_t)(mpu.getGyroX() * 131);  // Scale to ±250°/s range
        currentSensorData.gyroY = (int16_t)(mpu.getGyroY() * 131);
        currentSensorData.gyroZ = (int16_t)(mpu.getGyroZ() * 131);
        
        // Read magnetometer
        currentSensorData.magX = (int16_t)mpu.getMagX();
        currentSensorData.magY = (int16_t)mpu.getMagY();
        currentSensorData.magZ = (int16_t)mpu.getMagZ();
        
        // Calculate acceleration magnitude in m/s²
        float ax = mpu.getAccX() * 9.81;  // Convert g to m/s²
        float ay = mpu.getAccY() * 9.81;
        float az = mpu.getAccZ() * 9.81;
        currentSensorData.accelMagnitude = sqrt(ax*ax + ay*ay + az*az);
    }
}

// Read all sensors and update currentSensorData
void readAllSensors() {
    // Read battery system
    currentSensorData.batteryVoltage = readBatteryVoltage();
    currentSensorData.batteryPercentage = calculateBatteryPercentage(currentSensorData.batteryVoltage);
    currentSensorData.remainingCapacity = calculateRemainingCapacity(currentSensorData.batteryPercentage);
    currentSensorData.solarCurrent = readSolarCurrent();
    
    // Read temperature
    currentSensorData.temperature = readTemperature();
    
    // Read MPU9250
    readMPU9250();
}

// Print sensor data to Serial for debugging
void printSensorData() {
    Serial.println("=== Sensor Readings ===");
    Serial.printf("Battery: %.2fV (%d%%) - %dmAh remaining\n", 
                  currentSensorData.batteryVoltage,
                  currentSensorData.batteryPercentage,
                  currentSensorData.remainingCapacity);
    Serial.printf("Solar Current: %.2fA\n", currentSensorData.solarCurrent);
    Serial.printf("Temperature: %.1f°C\n", currentSensorData.temperature);
    Serial.printf("Accel: X=%d Y=%d Z=%d (Mag=%.2f m/s²)\n",
                  currentSensorData.accelX,
                  currentSensorData.accelY,
                  currentSensorData.accelZ,
                  currentSensorData.accelMagnitude);
    Serial.printf("Gyro: X=%d Y=%d Z=%d\n",
                  currentSensorData.gyroX,
                  currentSensorData.gyroY,
                  currentSensorData.gyroZ);
    Serial.printf("Mag: X=%d Y=%d Z=%d\n",
                  currentSensorData.magX,
                  currentSensorData.magY,
                  currentSensorData.magZ);
    Serial.println("=====================");
}

// Check if battery is critically low
bool isBatteryCritical() {
    return currentSensorData.batteryPercentage < 10;
}

// Check if reset button is pressed
// Button connected between GPIO 27 and GND (using internal pull-up)
// Returns true when button is pressed
bool isResetButtonPressed() {
    static unsigned long lastDebounceTime = 0;
    static bool lastButtonState = HIGH;
    static bool buttonPressed = false;
    const unsigned long debounceDelay = 50; // 50ms debounce
    
    bool reading = digitalRead(RESET_BUTTON_PIN);
    
    // If button state changed, reset debounce timer
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    // If button state has been stable for debounceDelay
    if ((millis() - lastDebounceTime) > debounceDelay) {
        // If button is pressed (LOW because of pull-up)
        if (reading == LOW && !buttonPressed) {
            buttonPressed = true;
            Serial.println("\n🔘🔘🔘 RESET BUTTON PRESSED! 🔘🔘🔘");
            Serial.printf("    GPIO %d detected LOW signal\n", RESET_BUTTON_PIN);
            Serial.println("    Action: Location will be reset");
            return true;
        }
        // If button is released
        if (reading == HIGH) {
            if (buttonPressed) {
                Serial.println("🔘 Reset button released");
            }
            buttonPressed = false;
        }
    }
    
    lastButtonState = reading;
    return false;
}

// Get current sensor data
SensorData getSensorData() {
    return currentSensorData;
}

#endif // SENSORS_H
