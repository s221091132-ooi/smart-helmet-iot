// http_client.h - HTTP Client for sending data to Vercel API
// Handles WiFi connectivity and API communication

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "config.h"
#include "sensors.h"
#include "location_tracking.h"

// HTTP client globals
HTTPClient http;
WiFiClientSecure wifiClient;
bool wifiConnected = false;
unsigned long lastConnectionAttempt = 0;
const unsigned long CONNECTION_RETRY_INTERVAL = 5000;  // 5 seconds

// Initialize WiFi connection
bool initializeWiFi() {
    Serial.println("Initializing WiFi...");
    Serial.printf("SSID: %s\n", WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("\nWiFi connected successfully!");
        Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("Signal Strength: %d dBm\n", WiFi.RSSI());
        return true;
    } else {
        wifiConnected = false;
        Serial.println("\nWiFi connection failed!");
        return false;
    }
}

// Check WiFi connection and reconnect if needed
void checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        wifiConnected = false;
        
        unsigned long currentTime = millis();
        if (currentTime - lastConnectionAttempt >= CONNECTION_RETRY_INTERVAL) {
            Serial.println("WiFi disconnected, attempting to reconnect...");
            lastConnectionAttempt = currentTime;
            
            WiFi.disconnect();
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            
            delay(5000);
            
            if (WiFi.status() == WL_CONNECTED) {
                wifiConnected = true;
                Serial.println("WiFi reconnected successfully!");
            }
        }
    } else {
        wifiConnected = true;
    }
}

// Check if WiFi is connected
bool isWiFiConnected() {
    return wifiConnected && (WiFi.status() == WL_CONNECTED);
}

// Send sensor data to Vercel API
bool sendSensorData(SensorData sensorData, LocationData locationData) {
    if (!isWiFiConnected()) {
        Serial.println("Cannot send data: WiFi not connected");
        return false;
    }
    
    String endpoint = String(API_ENDPOINT) + "/api/helmet/data";
    
    // Set insecure mode for HTTPS (skip certificate validation)
    wifiClient.setInsecure();
    
    http.begin(wifiClient, endpoint);
    http.addHeader("Content-Type", "application/json");
    
    // Create JSON payload
    StaticJsonDocument<512> doc;
    doc["battery_voltage"] = sensorData.batteryVoltage;
    doc["battery_percentage"] = sensorData.batteryPercentage;
    doc["remaining_capacity"] = sensorData.remainingCapacity;
    doc["solar_current"] = sensorData.solarCurrent;
    doc["temperature"] = sensorData.temperature;
    
    JsonObject accel = doc.createNestedObject("accel");
    accel["x"] = sensorData.accelX;
    accel["y"] = sensorData.accelY;
    accel["z"] = sensorData.accelZ;
    
    JsonObject gyro = doc.createNestedObject("gyro");
    gyro["x"] = sensorData.gyroX;
    gyro["y"] = sensorData.gyroY;
    gyro["z"] = sensorData.gyroZ;
    
    JsonObject position = doc.createNestedObject("position");
    position["x"] = locationData.positionX;
    position["y"] = locationData.positionY;
    
    doc["direction"] = locationData.direction;
    doc["speed"] = locationData.speed;
    doc["altitude"] = locationData.altitude;
    doc["distance_traveled"] = locationData.distanceTraveled;
    
    String jsonPayload;
    serializeJson(doc, jsonPayload);
    
    Serial.println("Sending sensor data to API...");
    Serial.println(jsonPayload);
    
    int httpResponseCode = http.POST(jsonPayload);
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.printf("HTTP Response code: %d\n", httpResponseCode);
        Serial.printf("Response: %s\n", response.c_str());
        
        // Check if response contains reset flag
        StaticJsonDocument<128> responseDoc;
        DeserializationError error = deserializeJson(responseDoc, response);
        
        if (!error && responseDoc.containsKey("reset_location")) {
            bool shouldReset = responseDoc["reset_location"];
            Serial.println("\n────────────────────────────────────────");
            Serial.println("📡 SERVER RESPONSE ANALYSIS:");
            Serial.println("────────────────────────────────────────");
            Serial.printf("   reset_location flag: %s\n", shouldReset ? "TRUE" : "FALSE");
            
            if (shouldReset) {
                Serial.println("   ⚠️  Server is requesting location reset!");
                Serial.println("   Reason: last_reset_at timestamp < 3 seconds old");
                Serial.println("   This will trigger ESP32 to call resetLocation()");
                Serial.println("────────────────────────────────────────\n");
                http.end();
                return true;  // Signal that reset was requested
            } else {
                Serial.println("   ✅ No reset requested (normal operation)");
                Serial.println("────────────────────────────────────────\n");
            }
        } else {
            if (error) {
                Serial.printf("   ⚠️  JSON parse error: %s\n", error.c_str());
            } else {
                Serial.println("   ℹ️  No reset_location field in response");
            }
            Serial.println("────────────────────────────────────────\n");
        }
        
        http.end();
        return httpResponseCode == 200;
    } else {
        Serial.printf("HTTP POST failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
        http.end();
        return false;
    }
}

// Send fall alert to Vercel API
bool sendFallAlert(float accelMagnitude, float posX, float posY) {
    if (!isWiFiConnected()) {
        Serial.println("Cannot send fall alert: WiFi not connected");
        return false;
    }
    
    String endpoint = String(API_ENDPOINT) + "/api/helmet/fall";
    
    // Set insecure mode for HTTPS (skip certificate validation)
    wifiClient.setInsecure();
    
    http.begin(wifiClient, endpoint);
    http.addHeader("Content-Type", "application/json");
    
    // Create JSON payload
    StaticJsonDocument<256> doc;
    doc["position_x"] = posX;
    doc["position_y"] = posY;
    doc["accel_magnitude"] = accelMagnitude;
    
    String jsonPayload;
    serializeJson(doc, jsonPayload);
    
    Serial.println("Sending fall alert to API...");
    Serial.println(jsonPayload);
    
    int httpResponseCode = http.POST(jsonPayload);
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.printf("HTTP Response code: %d\n", httpResponseCode);
        Serial.printf("Response: %s\n", response.c_str());
        http.end();
        return httpResponseCode == 201 || httpResponseCode == 200;
    } else {
        Serial.printf("HTTP POST failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
        http.end();
        return false;
    }
}

// Tell server that location was reset (physical button or sync with dashboard)
bool requestLocationResetOnServer() {
    if (!isWiFiConnected()) {
        Serial.println("Cannot request location reset: WiFi not connected");
        return false;
    }
    
    String endpoint = String(API_ENDPOINT) + "/api/helmet/reset-location";
    wifiClient.setInsecure();
    http.begin(wifiClient, endpoint);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST("{}");
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.printf("Location reset API response (%d): %s\n", httpResponseCode, response.c_str());
        http.end();
        return httpResponseCode == 200;
    }
    
    Serial.printf("Location reset API failed: %s\n", http.errorToString(httpResponseCode).c_str());
    http.end();
    return false;
}

// Check for location reset command from server
bool checkForLocationReset() {
    if (!isWiFiConnected()) {
        return false;
    }
    
    String endpoint = String(API_ENDPOINT) + "/api/helmet/status";
    
    http.begin(wifiClient, endpoint);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, response);
        
        if (!error && doc.containsKey("reset_requested")) {
            bool resetRequested = doc["reset_requested"];
            http.end();
            return resetRequested;
        }
    }
    
    http.end();
    return false;
}

// Print WiFi status for debugging
void printWiFiStatus() {
    if (isWiFiConnected()) {
        Serial.println("=== WiFi Status ===");
        Serial.printf("Status: Connected\n");
        Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
        Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("Signal Strength: %d dBm\n", WiFi.RSSI());
        Serial.printf("MAC Address: %s\n", WiFi.macAddress().c_str());
        Serial.println("===================");
    } else {
        Serial.println("WiFi: Not Connected");
    }
}

// Get WiFi signal strength as percentage
int getWiFiSignalStrength() {
    if (!isWiFiConnected()) return 0;
    
    int rssi = WiFi.RSSI();
    // Convert RSSI to percentage (approximate)
    if (rssi >= -50) return 100;
    if (rssi <= -100) return 0;
    return 2 * (rssi + 100);
}

#endif // HTTP_CLIENT_H
