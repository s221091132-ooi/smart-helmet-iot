// fall_detection.h - Fall Detection Algorithm for Smart Helmet
// Uses MPU9250/MPU6050 accelerometer + gyroscope to detect falls

#ifndef FALL_DETECTION_H
#define FALL_DETECTION_H

#include "sensors.h"

// Fall detection thresholds
#define IMPACT_THRESHOLD 16.0        // ~1.6g total magnitude spike on impact
#define IMPACT_DELTA_THRESHOLD 8.0   // Sudden change in m/s² between samples
#define STRONG_IMPACT_THRESHOLD 30.0   // Skip long stillness wait on hard impacts
#define GYRO_STILLNESS_THRESHOLD 40.0f // deg/s — allow minor post-impact twitch
#define ACCEL_STILLNESS_BAND 4.5f      // m/s² — magnitude near 1g when stationary
#define STILLNESS_DURATION 1500        // 1.5 seconds in milliseconds
#define STRONG_STILLNESS_DURATION 600  // Short stillness after hard impact
#define DEBOUNCE_DURATION 3000         // 3 seconds between detections
#define IMPACT_CHECK_DELAY 300         // Wait before checking stillness
#define PEAK_WINDOW_MS 250             // Track impact peaks over this window

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
unsigned long stillnessDurationRequired = STILLNESS_DURATION;
FallEvent lastFallEvent;
bool fallConfirmedLatch = false;
float prevAccelMagnitude = 9.81f;
float impactPeakMagnitude = 0.0f;
float peakWindowMax = 0.0f;
unsigned long peakWindowStartMs = 0;

static void confirmFall(float accelMagnitude, float posX, float posY) {
    unsigned long currentTime = millis();

    Serial.println("FALL DETECTION: FALL CONFIRMED!");
    fallState = FALL_CONFIRMED;
    fallConfirmedLatch = true;

    lastFallEvent.detected = true;
    lastFallEvent.timestamp = currentTime;
    lastFallEvent.accelMagnitude = accelMagnitude;
    lastFallEvent.positionX = posX;
    lastFallEvent.positionY = posY;
    lastFallTime = currentTime;
}

static void resetPeakWindow(float seedMagnitude) {
    peakWindowStartMs = millis();
    peakWindowMax = seedMagnitude;
}

static void trackImpactPeak(float accelMagnitude) {
    unsigned long now = millis();

    if (peakWindowStartMs == 0 || now - peakWindowStartMs > PEAK_WINDOW_MS) {
        resetPeakWindow(accelMagnitude);
        return;
    }

    if (accelMagnitude > peakWindowMax) {
        peakWindowMax = accelMagnitude;
    }
}

// Initialize fall detection
void initializeFallDetection() {
    fallState = IDLE;
    impactTime = 0;
    stillnessStartTime = 0;
    lastFallTime = 0;
    stillnessDurationRequired = STILLNESS_DURATION;
    lastFallEvent.detected = false;
    fallConfirmedLatch = false;
    prevAccelMagnitude = 9.81f;
    impactPeakMagnitude = 0.0f;
    resetPeakWindow(9.81f);

    Serial.println("Fall detection initialized");
    Serial.printf("Impact threshold: %.1f m/s² (%.2fg)\n", IMPACT_THRESHOLD, IMPACT_THRESHOLD / 9.81);
    Serial.printf("Impact delta threshold: %.1f m/s²\n", IMPACT_DELTA_THRESHOLD);
    Serial.printf("Strong impact threshold: %.1f m/s²\n", STRONG_IMPACT_THRESHOLD);
    Serial.printf("Gyro stillness threshold: %.1f deg/s\n", GYRO_STILLNESS_THRESHOLD);
}

// Check for impact (sudden spike or rapid magnitude change)
bool detectImpact(float accelMagnitude) {
    trackImpactPeak(accelMagnitude);

    float delta = fabsf(accelMagnitude - prevAccelMagnitude);
    prevAccelMagnitude = 0.7f * prevAccelMagnitude + 0.3f * accelMagnitude;

    return peakWindowMax > IMPACT_THRESHOLD || delta > IMPACT_DELTA_THRESHOLD;
}

// Stationary after impact: low gyro rotation + accel magnitude near 1g
bool detectStillness(float gyroX, float gyroY, float gyroZ, float accelMagnitude) {
    float gyroMag = sqrtf(gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ);
    bool gyroStill = gyroMag < GYRO_STILLNESS_THRESHOLD;
    bool accelStable = fabsf(accelMagnitude - 9.81f) < ACCEL_STILLNESS_BAND;
    return gyroStill && accelStable;
}

// Fall detection state machine
void updateFallDetection(float accelMagnitude, float gyroX, float gyroY, float gyroZ, float posX, float posY) {
    unsigned long currentTime = millis();

    switch (fallState) {
        case IDLE:
            if (!mpuHasFreshData) {
                break;
            }
            if (detectImpact(accelMagnitude)) {
                impactPeakMagnitude = peakWindowMax;
                stillnessDurationRequired =
                    (impactPeakMagnitude >= STRONG_IMPACT_THRESHOLD) ? STRONG_STILLNESS_DURATION : STILLNESS_DURATION;

                Serial.printf(
                    "FALL DETECTION: Impact detected! peak=%.1f m/s² delta-path=%.1f m/s²\n",
                    impactPeakMagnitude,
                    accelMagnitude
                );
                fallState = IMPACT_DETECTED;
                impactTime = currentTime;
            }
            break;

        case IMPACT_DETECTED:
            if (currentTime - impactTime >= IMPACT_CHECK_DELAY) {
                Serial.println("FALL DETECTION: Checking for stillness...");
                fallState = CHECKING_STILLNESS;
                stillnessStartTime = currentTime;
            }
            break;

        case CHECKING_STILLNESS:
            if (!detectStillness(gyroX, gyroY, gyroZ, accelMagnitude)) {
                Serial.println("FALL DETECTION: Movement detected, false alarm");
                fallState = IDLE;
                impactPeakMagnitude = 0.0f;
            } else if (currentTime - stillnessStartTime >= stillnessDurationRequired) {
                confirmFall(accelMagnitude, posX, posY);
            }
            break;

        case FALL_CONFIRMED:
            // Stay here until main loop handles the alert and calls clearFallDetection()
            break;

        case DEBOUNCE:
            if (currentTime - lastFallTime >= DEBOUNCE_DURATION) {
                Serial.println("FALL DETECTION: Debounce period ended, ready for new detection");
                fallState = IDLE;
                impactPeakMagnitude = 0.0f;
            }
            break;
    }
}

// Check if a new fall has been detected (call this in main loop)
bool isFallDetected() {
    return fallState == FALL_CONFIRMED || fallConfirmedLatch;
}

// Get the last fall event
FallEvent getLastFallEvent() {
    return lastFallEvent;
}

// Clear the fall detected flag (call after handling the fall)
void clearFallDetection() {
    fallConfirmedLatch = false;
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
    Serial.printf("  Peak window max: %.1f m/s² | prev smoothed: %.1f m/s²\n", peakWindowMax, prevAccelMagnitude);
    if (fallState == CHECKING_STILLNESS) {
        unsigned long elapsed = millis() - stillnessStartTime;
        Serial.printf("  Stillness check: %lums / %lums\n", elapsed, stillnessDurationRequired);
    } else if (fallState == DEBOUNCE) {
        unsigned long elapsed = millis() - lastFallTime;
        Serial.printf("  Debounce: %lums / %dms\n", elapsed, DEBOUNCE_DURATION);
    }
}

// Test function to simulate a fall (for debugging)
void simulateFall() {
    Serial.println("SIMULATING FALL FOR TESTING!");
    confirmFall(28.5f, 10.5f, 5.3f);
}

#endif // FALL_DETECTION_H
