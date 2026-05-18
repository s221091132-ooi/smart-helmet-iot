// sensors.h - Sensor reading functions for Smart Helmet
// Handles MPU9250 or MPU6050, LM35, ACS712

#ifndef SENSORS_H
#define SENSORS_H

#include <Wire.h>
#include <MPU9250.h>

// Reset button interrupt flag (short presses missed while HTTP blocks loop)
volatile bool g_resetButtonInterrupt = false;

void IRAM_ATTR onResetButtonInterrupt() {
    g_resetButtonInterrupt = true;
}

// Pin definitions
#define LM35_PIN 32              // LM35 temperature sensor (analog) - CHANGED from pin 4 to 32 (more stable)
#define ACS712_PIN 35            // ACS712 current sensor (analog) - MOVED from 34 to 35
#define BATTERY_VOLTAGE_PIN 34   // Battery voltage sensor (analog) - YOUR ACTUAL WIRING
#define BUZZER_PIN 25            // Buzzer output
#define LED_PIN 26               // LED output
#define RESET_BUTTON_PIN 27      // Reset button (active LOW with internal pullup)

// Battery configuration
#define BATTERY_MAX_VOLTAGE 4.2  // Maximum LiPo voltage (fully charged)
#define BATTERY_MIN_VOLTAGE 3.0  // Minimum safe voltage (cutoff - don't discharge below this!)
#define BATTERY_CAPACITY 2000    // Battery capacity in mAh
#define VOLTAGE_DIVIDER_RATIO 5.98  // Voltage divider ratio - Calibrated with multimeter (3.83V actual)

// LiPo discharge curve reference:
// 4.20V = 100% | 4.00V = ~90% | 3.85V = ~75% | 3.70V = ~50% | 3.50V = ~25% | 3.00V = 0%

// LM35 configuration
#define LM35_V_REF 3.3           // ESP32 reference voltage
#define LM35_ADC_RESOLUTION 4095.0  // ESP32 12-bit ADC (0 to 4095)
#define LM35_CALIBRATION_OFFSET 6.0  // Calibration offset in °C (adjust if needed)

// ACS712 configuration (5A model)
#define ACS712_SENSITIVITY 0.185  // 185mV per Amp for 5A model
float ACS712_OFFSET = 2.5;        // Offset voltage at zero current (will be calibrated at startup)

#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

// MPU6050 fallback (same I2C bus as MPU9250)
#define MPU6050_ADDR_68 0x68
#define MPU6050_ADDR_69 0x69
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_WHO_AM_I 0x75
#define MPU6050_ACCEL_XOUT_H 0x3B

enum ImuType { IMU_NONE, IMU_MPU9250, IMU_MPU6050 };
ImuType activeImu = IMU_NONE;
uint8_t mpu6050Addr = MPU6050_ADDR_68;

MPU9250 mpu;
bool mpuInitialized = false;
bool mpuHasFreshData = false;
float mpu6050HeadingDeg = 0.0f;
unsigned long mpu6050LastReadMs = 0;

struct SensorData {
    float batteryVoltage;
    int batteryPercentage;
    int remainingCapacity;
    float solarCurrent;
    float temperature;
    int16_t accelX;
    int16_t accelY;
    int16_t accelZ;
    float gyroX;
    float gyroY;
    float gyroZ;
    float magX;
    float magY;
    float magZ;
    float headingDeg;
    float accelMagnitude;
};

SensorData currentSensorData;

int16_t mpu6050Read16(uint8_t reg) {
    Wire.beginTransmission(mpu6050Addr);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(mpu6050Addr, (uint8_t)2);
    if (Wire.available() < 2) return 0;
    return ((int16_t)Wire.read() << 8) | Wire.read();
}

bool mpu6050WriteByte(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(mpu6050Addr);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool initMPU6050(uint8_t addr) {
    mpu6050Addr = addr;
    if (!mpu6050WriteByte(MPU6050_PWR_MGMT_1, 0x00)) return false;
    Wire.beginTransmission(mpu6050Addr);
    Wire.write(MPU6050_WHO_AM_I);
    Wire.endTransmission(false);
    Wire.requestFrom(mpu6050Addr, (uint8_t)1);
    if (Wire.available() < 1) return false;
    uint8_t who = Wire.read();
    if (who != 0x68 && who != 0x71) return false;
    mpu6050HeadingDeg = 0.0f;
    mpu6050LastReadMs = millis();
    return true;
}

void readMPU6050() {
    int16_t ax = mpu6050Read16(MPU6050_ACCEL_XOUT_H);
    int16_t ay = mpu6050Read16(MPU6050_ACCEL_XOUT_H + 2);
    int16_t az = mpu6050Read16(MPU6050_ACCEL_XOUT_H + 4);
    int16_t gz = mpu6050Read16(0x47);

    currentSensorData.accelX = ax;
    currentSensorData.accelY = ay;
    currentSensorData.accelZ = az;

    float gyroZ = gz / 131.0f;
    currentSensorData.gyroX = mpu6050Read16(0x43) / 131.0f;
    currentSensorData.gyroY = mpu6050Read16(0x45) / 131.0f;
    currentSensorData.gyroZ = gyroZ;
    currentSensorData.magX = 0;
    currentSensorData.magY = 0;
    currentSensorData.magZ = 0;

    unsigned long now = millis();
    float dt = (now - mpu6050LastReadMs) / 1000.0f;
    if (dt > 0.0f && dt < 0.25f) {
        mpu6050HeadingDeg += gyroZ * dt;
        while (mpu6050HeadingDeg < 0.0f) mpu6050HeadingDeg += 360.0f;
        while (mpu6050HeadingDeg >= 360.0f) mpu6050HeadingDeg -= 360.0f;
    }
    mpu6050LastReadMs = now;
    currentSensorData.headingDeg = mpu6050HeadingDeg;

    float axMs = (ax / 16384.0f) * 9.81f;
    float ayMs = (ay / 16384.0f) * 9.81f;
    float azMs = (az / 16384.0f) * 9.81f;
    currentSensorData.accelMagnitude = sqrt(axMs * axMs + ayMs * ayMs + azMs * azMs);
    mpuHasFreshData = true;
}

// Initialize all sensors
bool initializeSensors() {
    bool success = true;
    
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(400000);

    // Try MPU9250 first, then MPU6050 (X/Y step tracking works with both)
    if (mpu.setup(0x68) || mpu.setup(0x69)) {
        activeImu = IMU_MPU9250;
        mpuInitialized = true;
        mpu.ahrs(true);
        Serial.println("MPU9250 OK — X/Y map + compass heading");

        delay(2000);
        Serial.println("Calibrating accel/gyro (keep still)...");
        mpu.calibrateAccelGyro();
        Serial.println("Rotate figure-8 for magnetometer (15 sec)...");
        mpu.calibrateMag();
    } else if (initMPU6050(MPU6050_ADDR_68) || initMPU6050(MPU6050_ADDR_69)) {
        activeImu = IMU_MPU6050;
        mpuInitialized = true;
        Serial.printf("MPU6050 OK at 0x%02X — X/Y map (forward=+Y, gyro heading)\n", mpu6050Addr);
        Serial.println("  Face helmet forward at startup = map North (+Y)");
        delay(1000);
    } else {
        activeImu = IMU_NONE;
        mpuInitialized = false;
        Serial.println("WARNING: No IMU on I2C — X/Y tracking disabled");
        Serial.println("  Check SDA=21, SCL=22, VCC=3.3V, GND");
    }
    
    // Initialize analog pins for temperature and current sensors
    pinMode(LM35_PIN, INPUT);
    pinMode(ACS712_PIN, INPUT);
    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
    Serial.println("LM35 temperature sensor configured on GPIO 32 (pin 4 had conflict with I2C)");
    
    // Initialize output pins
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    
    // Initialize reset button (active LOW with internal pullup)
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(RESET_BUTTON_PIN), onResetButtonInterrupt, FALLING);
    Serial.println("Reset button configured on GPIO 27 (active LOW, interrupt on press)");
    
    // Calibrate ACS712 current sensor (find zero-current offset)
    Serial.println("Calibrating ACS712 current sensor...");
    Serial.println("  Ensure no solar current is flowing!");
    delay(1000);
    
    long rawZero = 0;
    for (int i = 0; i < 100; i++) {
        rawZero += analogRead(ACS712_PIN);
        delay(10);
    }
    ACS712_OFFSET = (rawZero / 100.0 / 4095.0) * 3.3;
    
    Serial.print("  ACS712 zero-current offset: ");
    Serial.print(ACS712_OFFSET, 3);
    Serial.println(" V");
    Serial.println("  ACS712 calibration complete!");
    
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

// Read solar charging current from ACS712 with averaging and deadband filter
float readSolarCurrent() {
    // Average 50 samples to reduce noise
    long totalRaw = 0;
    for (int i = 0; i < 50; i++) {
        totalRaw += analogRead(ACS712_PIN);
        delay(2);
    }
    float avgRaw = totalRaw / 50.0;
    
    // Convert ADC to voltage
    float voltage = (avgRaw / 4095.0) * 3.3;
    
    // Calculate current: (Voltage - Offset) / Sensitivity
    float current = (voltage - ACS712_OFFSET) / ACS712_SENSITIVITY;
    
    // Deadband filter: values between -0.03A and +0.03A are noise, set to 0
    if (current > -0.03 && current < 0.03) {
        current = 0.0;
    }
    
    // Return absolute value (charging current is always positive)
    return abs(current);
}

// Read temperature from LM35
float readTemperature() {
    // Average 10 samples for better accuracy
    long totalRaw = 0;
    for (int i = 0; i < 10; i++) {
        totalRaw += analogRead(LM35_PIN);
        delay(2);
    }
    float avgRaw = totalRaw / 10.0;
    
    // Convert the raw value into the actual voltage arriving at the ESP32 pin
    float tempVoltage = (avgRaw / LM35_ADC_RESOLUTION) * LM35_V_REF;
    
    // Convert the voltage to Celsius (LM35: 10mV per degree Celsius)
    float temperatureC = (tempVoltage * 100.0) + LM35_CALIBRATION_OFFSET;
    
    // Sanity check: if temperature is out of reasonable range, return default
    if (temperatureC < -10 || temperatureC > 100) {
        Serial.println("WARNING: LM35 read error!");
        return 25.0;  // Return default room temperature
    }
    
    return temperatureC;
}

// Read MPU9250 accelerometer, gyroscope, and magnetometer
void readMPU9250() {
    if (!mpuInitialized) {
        mpuHasFreshData = false;
        return;
    }
    if (mpu.update()) {
        mpuHasFreshData = true;
        // Accelerometer as raw 16-bit (±2g) for API compatibility
        currentSensorData.accelX = (int16_t)(mpu.getAccX() * 16384);
        currentSensorData.accelY = (int16_t)(mpu.getAccY() * 16384);
        currentSensorData.accelZ = (int16_t)(mpu.getAccZ() * 16384);
        
        // Gyro deg/s and magnetometer uT — use library units directly (no double scaling)
        currentSensorData.gyroX = mpu.getGyroX();
        currentSensorData.gyroY = mpu.getGyroY();
        currentSensorData.gyroZ = mpu.getGyroZ();
        currentSensorData.magX = mpu.getMagX();
        currentSensorData.magY = mpu.getMagY();
        currentSensorData.magZ = mpu.getMagZ();
        
        // Yaw from onboard Madgwick filter (-180..180) → compass 0..360
        float yaw = mpu.getYaw();
        if (yaw < 0.0f) yaw += 360.0f;
        currentSensorData.headingDeg = yaw;
        
        float ax = mpu.getAccX() * 9.81;
        float ay = mpu.getAccY() * 9.81;
        float az = mpu.getAccZ() * 9.81;
        currentSensorData.accelMagnitude = sqrt(ax * ax + ay * ay + az * az);
    } else {
        mpuHasFreshData = false;
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
    
    if (activeImu == IMU_MPU9250) {
        readMPU9250();
    } else if (activeImu == IMU_MPU6050) {
        readMPU6050();
    } else {
        mpuHasFreshData = false;
    }
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
    Serial.printf("Gyro: X=%.1f Y=%.1f Z=%.1f deg/s\n",
                  currentSensorData.gyroX,
                  currentSensorData.gyroY,
                  currentSensorData.gyroZ);
    Serial.printf("Mag: X=%.1f Y=%.1f Z=%.1f uT | Heading: %.1f°\n",
                  currentSensorData.magX,
                  currentSensorData.magY,
                  currentSensorData.magZ,
                  currentSensorData.headingDeg);
    Serial.println("=====================");
}

// Check if battery is critically low
bool isBatteryCritical() {
    return currentSensorData.batteryPercentage < 10;
}

// Read reset button (active LOW with INPUT_PULLUP)
// Returns true while button is physically pressed
bool isResetButtonPressed() {
    return digitalRead(RESET_BUTTON_PIN) == LOW;
}

// True on short tap (interrupt) or while held (poll backup)
bool wasResetButtonTriggered() {
    if (g_resetButtonInterrupt) {
        g_resetButtonInterrupt = false;
        return true;
    }
    return isResetButtonPressed();
}

// Reset gyro-integrated heading (MPU6050) when GPIO27 / reset location
void resetImuHeading() {
    mpu6050HeadingDeg = 0.0f;
    currentSensorData.headingDeg = 0.0f;
}

// Get current sensor data
SensorData getSensorData() {
    return currentSensorData;
}

#endif // SENSORS_H
