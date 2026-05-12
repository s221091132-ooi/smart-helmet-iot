// fall_detection.h - Fall Detection Algorithm for Smart Helmet
// Uses MPU9250 accelerometer data to detect falls

#ifndef FALL_DETECTION_H
#define FALL_DETECTION_H

#include "sensors.h"

// Fall detection thresholds
#define IMPACT_THRESHOLD 24.5        // 2.5g in m/s² (sudden acceleration)
#define STILLNESS_THRESHOLD 4.9      // 0.5g in m/s² (lack of movement)
#define STILLNESS_DURATION 2000      // 2 seconds in milliseconds
#define DEBOUNCE_DURATION 3000       // 3 seconds between detections
#define IMPACT_CHECK_DELAY 500       // Wait 500ms after impact before checking stillness

// Fall detection state machine
enum FallState {
    IDLE,
    IMPACT_DETECTED,
    CHECKING_STILLNESS,
    FALL_CONFIRMED,
    DEBOUNCE
};

// Fall detection data
struct FallEvent {
    bool detected;
    unsigned long timestamp;
    float accelMagnitude;
    float positionX;
    float positionY;
};

FallState fallState = IDLE;
unsigned long impactTime = 0;
unsigned long stillnessStartTime = 0;
unsigned long lastFallTime = 0;
FallEvent lastFallEvent;

// Initialize fall detection
void initializeFallDetection() {
    fallState = IDLE;
    impactTime = 0;
    stillnessStartTime = 0;
    lastFallTime = 0;
    lastFallEvent.detected = false;
    
    Serial.println("Fall detection initialized");
    Serial.printf("Impact threshold: %.1f m/s² (%.2fg)\n", IMPACT_THRESHOLD, IMPACT_THRESHOLD / 9.81);
    Serial.printf("Stillness threshold: %.1f m/s² (%.2fg)\n", STILLNESS_THRESHOLD, STILLNESS_THRESHOLD / 9.81);
}

// Check for impact (sudden acceleration)
bool detectImpact(float accelMagnitude) {
    return accelMagnitude > IMPACT_THRESHOLD;
}

// Check for stillness (minimal movement)
bool detectStillness(float accelMagnitude) {
    return accelMagnitude < STILLNESS_THRESHOLD;
}

// Fall detection state machine
void updateFallDetection(float accelMagnitude, float posX, float posY) {
    unsigned long currentTime = millis();
    
    switch (fallState) {
        case IDLE:
            // Check for impact
            if (detectImpact(accelMagnitude)) {
                Serial.println("FALL DETECTION: Impact detected!");
                fallState = IMPACT_DETECTED;
                impactTime = currentTime;
            }
            break;
            
        case IMPACT_DETECTED:
            // Wait for specified delay before checking stillness
            if (currentTime - impactTime >= IMPACT_CHECK_DELAY) {
                Serial.println("FALL DETECTION: Checking for stillness...");
                fallState = CHECKING_STILLNESS;
                stillnessStartTime = currentTime;
            }
            break;
            
        case CHECKING_STILLNESS:
            // Check if person is still moving
            if (!detectStillness(accelMagnitude)) {
                // Person is still moving, not a fall
                Serial.println("FALL DETECTION: Movement detected, false alarm");
                fallState = IDLE;
            } else if (currentTime - stillnessStartTime >= STILLNESS_DURATION) {
                // Person has been still for required duration - FALL CONFIRMED
                Serial.println("FALL DETECTION: FALL CONFIRMED!");
                fallState = FALL_CONFIRMED;
                
                // Record fall event
                lastFallEvent.detected = true;
                lastFallEvent.timestamp = currentTime;
                lastFallEvent.accelMagnitude = accelMagnitude;
                lastFallEvent.positionX = posX;
                lastFallEvent.positionY = posY;
                lastFallTime = currentTime;
            }
            break;
            
        case FALL_CONFIRMED:
            // Fall has been confirmed, waiting to transition to debounce
            fallState = DEBOUNCE;
            break;
            
        case DEBOUNCE:
            // Debounce period - ignore new detections
            if (currentTime - lastFallTime >= DEBOUNCE_DURATION) {
                Serial.println("FALL DETECTION: Debounce period ended, ready for new detection");
                fallState = IDLE;
            }
            break;
    }
}

// Check if a new fall has been detected (call this in main loop)
bool isFallDetected() {
    if (fallState == FALL_CONFIRMED) {
        return true;
    }
    return false;
}

// Get the last fall event
FallEvent getLastFallEvent() {
    return lastFallEvent;
}

// Clear the fall detected flag (call after handling the fall)
void clearFallDetection() {
    if (fallState == FALL_CONFIRMED) {
        fallState = DEBOUNCE;
    }
    lastFallEvent.detected = false;
}

// Get current fall detection state (for debugging)
const char* getFallStateString() {
    switch (fallState) {
        case IDLE: return "IDLE";
        case IMPACT_DETECTED: return "IMPACT_DETECTED";
        case CHECKING_STILLNESS: return "CHECKING_STILLNESS";
        case FALL_CONFIRMED: return "FALL_CONFIRMED";
        case DEBOUNCE: return "DEBOUNCE";
        default: return "UNKNOWN";
    }
}

// Print fall detection status
void printFallStatus() {
    Serial.printf("Fall Detection State: %s\n", getFallStateString());
    if (fallState == CHECKING_STILLNESS) {
        unsigned long elapsed = millis() - stillnessStartTime;
        Serial.printf("  Stillness check: %lums / %dms\n", elapsed, STILLNESS_DURATION);
    } else if (fallState == DEBOUNCE) {
        unsigned long elapsed = millis() - lastFallTime;
        Serial.printf("  Debounce: %lums / %dms\n", elapsed, DEBOUNCE_DURATION);
    }
}

// Test function to simulate a fall (for debugging)
void simulateFall() {
    Serial.println("SIMULATING FALL FOR TESTING!");
    fallState = FALL_CONFIRMED;
    lastFallEvent.detected = true;
    lastFallEvent.timestamp = millis();
    lastFallEvent.accelMagnitude = 28.5;
    lastFallEvent.positionX = 10.5;
    lastFallEvent.positionY = 5.3;
    lastFallTime = millis();
}

#endif // FALL_DETECTION_H
