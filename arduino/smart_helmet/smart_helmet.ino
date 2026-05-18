/*
 * Smart Helmet IoT System - Main Sketch
 * 
 * Features:
 * - Battery monitoring (voltage, percentage, capacity, solar current)
 * - Fall detection using MPU9250 accelerometer
 * - Location tracking using step detection + gyroscope heading
 * - Real-time data transmission to Vercel API
 * - Buzzer alerts (power-on, reset confirmation, fall alert)
 * - LED visual alerts
 * 
 * Hardware:
 * - ESP32 microcontroller
 * - MPU9250 (9-axis motion sensor)
 * - DS18B20 (temperature sensor)
 * - ACS712 (current sensor)
 * - B25 fuel gauge (battery monitoring)
 * - Buzzer and LED
 * 
 * Author: Smart Helmet Team
 * Version: 1.0
 */

#include "config.h"
#include "sensors.h"
#include "fall_detection.h"
#include "location_tracking.h"
#include "buzzer.h"
#include "http_client.h"

// Timing variables
unsigned long lastDataSendTime = 0;
unsigned long lastSerialPrintTime = 0;
const unsigned long SERIAL_PRINT_INTERVAL = 5000;  // Print debug info every 5 seconds

// System state
bool systemInitialized = false;
static bool buzzerStoppedByButton = false;
static bool pendingImmediateDataSend = false;

// GPIO 27: buzzer off + local location reset + notify server + push 0,0,0 to dashboard
void handleResetButtonPress() {
    if (!wasResetButtonTriggered()) return;

    Serial.println("\n════════════════════════════════════════");
    Serial.println("🔘 RESET BUTTON PRESSED (GPIO 27)!");
    Serial.println("   • Stopping buzzer");
    Serial.println("   • Resetting location to (0, 0), altitude 0 m");
    Serial.println("   • Notifying server / website");
    Serial.println("════════════════════════════════════════\n");

    if (!buzzerStoppedByButton) {
        stopBuzzer();
        buzzerStoppedByButton = true;
    }
    resetLocation();
    enableLocationTracking();

    if (isWiFiConnected()) {
        requestLocationResetOnServer();
        pendingImmediateDataSend = true;
    } else {
        Serial.println("⚠️  WiFi offline — location reset locally only");
    }

    Serial.println("✅ Full reset complete!\n");
}

void setup() {
    // Initialize Serial communication
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n=================================");
    Serial.println("SMART HELMET IoT SYSTEM");
    Serial.println("Version 1.0");
    Serial.println("=================================\n");
    
    // Initialize buzzer and LED first
    initializeBuzzer();
    
    // Initialize sensors
    Serial.println("Initializing sensors...");
    if (!initializeSensors()) {
        Serial.println("WARNING: Some sensors failed to initialize");
        Serial.println("System will continue with available sensors");
    } else {
        Serial.println("All sensors initialized successfully!");
    }
    
    // Initialize fall detection
    initializeFallDetection();
    
    // Initialize location tracking
    initializeLocationTracking();
    
    // Initialize WiFi
    Serial.println("\nConnecting to WiFi...");
    if (!initializeWiFi()) {
        Serial.println("Failed to connect to WiFi");
        Serial.println("System will continue in offline mode");
        Serial.println("Data will not be transmitted to server");
    }
    
    // Start power-on buzzer pattern (will loop until physical button is pressed)
    startPowerOnPattern();
    
    Serial.println("\n=================================");
    Serial.println("SYSTEM READY");
    Serial.println("=================================");
    Serial.println("\n🔔 Buzzer is beeping continuously...");
    Serial.println("📍 Press RESET BUTTON (GPIO 27) to:");
    Serial.println("   stop buzzer + START tracking from (0,0) on map.\n");
    
    systemInitialized = true;
    lastDataSendTime = millis();
    lastSerialPrintTime = millis();
}

void loop() {
    if (!systemInitialized) return;
    
    unsigned long currentTime = millis();
    
    // Update buzzer state machine
    updateBuzzer();
    
    // Check WiFi connection
    checkWiFiConnection();
    
    // Read all sensors
    readAllSensors();
    SensorData sensorData = getSensorData();
    
    // Reset button before location update so this cycle starts from origin
    handleResetButtonPress();
    
    // Update location tracking
    updateLocationTracking(sensorData);
    LocationData locationData = getLocationData();
    
    // Update fall detection
    updateFallDetection(sensorData.accelMagnitude, locationData.positionX, locationData.positionY);
    
    // Check if fall was detected
    if (isFallDetected()) {
        Serial.println("\n!!! FALL DETECTED !!!");
        FallEvent fallEvent = getLastFallEvent();
        
        // Start fall alert buzzer and LED
        startFallAlertPattern();
        
        // Send fall alert to server
        if (isWiFiConnected()) {
            Serial.println("Sending fall alert to server...");
            bool success = sendFallAlert(
                fallEvent.accelMagnitude,
                fallEvent.positionX,
                fallEvent.positionY
            );
            
            if (success) {
                Serial.println("Fall alert sent successfully!");
            } else {
                Serial.println("Failed to send fall alert");
            }
        } else {
            Serial.println("Cannot send fall alert - WiFi not connected");
        }
        
        // Clear fall detection flag
        clearFallDetection();
        
        // Note: Fall alert buzzer will continue until manually stopped
        // In production, you might want to add a timeout or remote acknowledgment
    }
    
    // Check for battery critical alert
    if (isBatteryCritical()) {
        static unsigned long lastBatteryWarning = 0;
        if (currentTime - lastBatteryWarning > 60000) {  // Warn every minute
            Serial.println("\n!!! WARNING: BATTERY CRITICALLY LOW !!!");
            lastBatteryWarning = currentTime;
        }
    }
    
    // Send data to server (interval or immediately after GPIO 27 reset)
    if (pendingImmediateDataSend || currentTime - lastDataSendTime >= DATA_SEND_INTERVAL) {
        handleResetButtonPress();
        if (isWiFiConnected()) {
            bool resetRequested = sendSensorData(sensorData, locationData);
            handleResetButtonPress();

            // Website "Reset Location" button sets last_reset_at; ESP32 syncs on next response
            if (resetRequested) {
                Serial.println("📡 Server requested location reset (dashboard button)");
                resetLocation();
                pendingImmediateDataSend = true;
            }
        }
        pendingImmediateDataSend = false;
        lastDataSendTime = currentTime;
    }
    
    // Print debug information periodically
    if (currentTime - lastSerialPrintTime >= SERIAL_PRINT_INTERVAL) {
        Serial.println("\n=== SYSTEM STATUS ===");
        printWiFiStatus();
        Serial.println();
        printSensorData();
        Serial.println();
        printLocationData();
        Serial.println();
        printFallStatus();
        Serial.printf("Buzzer Pattern: %s\n", getBuzzerPatternString());
        Serial.println("====================\n");
        
        lastSerialPrintTime = currentTime;
    }
    
    // Small delay to prevent watchdog timer issues
    delay(10);
}

/*
 * TESTING FUNCTIONS
 * Uncomment these lines in setup() for specific tests
 */

// Test buzzer patterns
void testBuzzerPatterns() {
    Serial.println("Testing buzzer patterns...");
    
    Serial.println("Testing power-on pattern (continuous beep)...");
    Serial.println("Note: Will beep continuously until you press GPIO27 button");
    startPowerOnPattern();
    delay(5000);
    stopBuzzer();
    delay(1000);
    
    Serial.println("Testing fall alert pattern (3 seconds)...");
    startFallAlertPattern();
    delay(3000);
    stopBuzzer();
    
    Serial.println("Buzzer tests complete!");
}

// Test sensor readings
void testSensors() {
    Serial.println("Testing sensors...");
    for (int i = 0; i < 10; i++) {
        readAllSensors();
        printSensorData();
        delay(1000);
    }
    Serial.println("Sensor tests complete!");
}

// Test location tracking
void testLocationTracking() {
    Serial.println("Testing location tracking...");
    Serial.println("Walk around for 30 seconds...");
    
    unsigned long startTime = millis();
    while (millis() - startTime < 30000) {
        readAllSensors();
        updateLocationTracking(getSensorData());
        printLocationData();
        delay(1000);
    }
    
    Serial.println("Location tracking test complete!");
}

// Test WiFi and API communication
void testAPIConnection() {
    Serial.println("Testing API connection...");
    
    if (!isWiFiConnected()) {
        Serial.println("WiFi not connected!");
        return;
    }
    
    // Send test data
    readAllSensors();
    SensorData sensorData = getSensorData();
    LocationData locationData = getLocationData();
    
    Serial.println("Sending test data to API...");
    bool success = sendSensorData(sensorData, locationData);
    
    if (success) {
        Serial.println("API connection test successful!");
    } else {
        Serial.println("API connection test failed!");
    }
}
