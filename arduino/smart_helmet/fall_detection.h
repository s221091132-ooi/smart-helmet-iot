// fall_detection.h - Fall Detection Algorithm for Smart Helmet
// Uses MPU9250/MPU6050 accelerometer + gyroscope to detect falls

#ifndef FALL_DETECTION_H
#define FALL_DETECTION_H

#include "sensors.h"

// Fall detection thresholds (+50% sensitivity vs previous tuning)
#define IMPACT_THRESHOLD 10.5f        // ~1.07g peak (was 16.0)
#define IMPACT_DELTA_THRESHOLD 5.5f   // Sudden sample-to-sample change (was 8.0)
#define STRONG_IMPACT_THRESHOLD 20.0f // Fast confirm path (was 30.0)
#define GYRO_STILLNESS_THRESHOLD 60.0f // deg/s — more tolerant of minor movement (was 40.0)
#define ACCEL_STILLNESS_BAND 6.8f      // m/s² around 1g when stationary (was 4.5)
#define STILLNESS_DURATION 1000        // 1.0 second (was 1500)
#define STRONG_STILLNESS_DURATION 400    // Short stillness after hard impact (was 600)
#define LIGHT_STILLNESS_DURATION 250     // Near-instant confirm on sharp motion spikes
#define DEBOUNCE_DURATION 400          // Short gap before next trigger (was 3000)
#define IMPACT_CHECK_DELAY 200         // Wait before checking stillness (was 300)
#define PEAK_WINDOW_MS 375             // Wider peak capture window (was 250)

// Fall detection state machine
enum FallState {
    IDLE,
    IMPACT_DETECTED,
    CHECKING_STILLNESS,
    FALL_CONFIRMED
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
float lastImpactDelta = 0.0f;
float impactPeakMagnitude = 0.0f;
float peakWindowMax = 0.0f;
unsigned long peakWindowStartMs = 0;

static void resetImpactTracking(float seedMagnitude = 9.81f) {
    prevAccelMagnitude = seedMagnitude;
    lastImpactDelta = 0.0f;
    impactPeakMagnitude = 0.0f;
    peakWindowStartMs = millis();
    peakWindowMax = seedMagnitude;
}

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

static void trackImpactPeak(float accelMagnitude) {
    unsigned long now = millis();

    if (peakWindowStartMs == 0 || now - peakWindowStartMs > PEAK_WINDOW_MS) {
        resetImpactTracking(accelMagnitude);
        return;
    }

    if (accelMagnitude > peakWindowMax) {
        peakWindowMax = accelMagnitude;
    }
}

static unsigned long chooseStillnessDuration(float peakMagnitude, float impactDelta) {
    if (impactDelta >= IMPACT_DELTA_THRESHOLD * 1.15f || peakMagnitude >= STRONG_IMPACT_THRESHOLD) {
        return STRONG_STILLNESS_DURATION;
    }
    if (impactDelta >= IMPACT_DELTA_THRESHOLD || peakMagnitude >= IMPACT_THRESHOLD * 1.1f) {
        return LIGHT_STILLNESS_DURATION;
    }
    return STILLNESS_DURATION;
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
    resetImpactTracking(9.81f);

    Serial.println("Fall detection initialized (high sensitivity mode)");
    Serial.printf("Impact threshold: %.1f m/s² (%.2fg)\n", IMPACT_THRESHOLD, IMPACT_THRESHOLD / 9.81);
    Serial.printf("Impact delta threshold: %.1f m/s²\n", IMPACT_DELTA_THRESHOLD);
    Serial.printf("Strong impact threshold: %.1f m/s²\n", STRONG_IMPACT_THRESHOLD);
    Serial.printf("Gyro stillness threshold: %.1f deg/s\n", GYRO_STILLNESS_THRESHOLD);
    Serial.printf("Debounce between alerts: %dms\n", DEBOUNCE_DURATION);
}

// Check for impact (sudden spike or rapid magnitude change)
bool detectImpact(float accelMagnitude) {
    trackImpactPeak(accelMagnitude);

    float delta = fabsf(accelMagnitude - prevAccelMagnitude);
    lastImpactDelta = delta;
    prevAccelMagnitude = 0.5f * prevAccelMagnitude + 0.5f * accelMagnitude;

    return peakWindowMax > IMPACT_THRESHOLD || delta > IMPACT_DELTA_THRESHOLD;
}

// Stationary after impact: lenient check so slight motion still confirms
bool detectStillness(float gyroX, float gyroY, float gyroZ, float accelMagnitude) {
    float gyroMag = sqrtf(gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ);
    bool gyroStill = gyroMag < GYRO_STILLNESS_THRESHOLD;
    bool accelStable = fabsf(accelMagnitude - 9.81f) < ACCEL_STILLNESS_BAND;
    return gyroStill || accelStable;
}

// Fall detection state machine
void updateFallDetection(float accelMagnitude, float gyroX, float gyroY, float gyroZ, float posX, float posY) {
    unsigned long currentTime = millis();

    switch (fallState) {
        case IDLE:
            if (!mpuHasFreshData) {
                break;
            }
            if (currentTime - lastFallTime < DEBOUNCE_DURATION) {
                break;
            }
            if (detectImpact(accelMagnitude)) {
                impactPeakMagnitude = peakWindowMax;
                stillnessDurationRequired = chooseStillnessDuration(impactPeakMagnitude, lastImpactDelta);

                Serial.printf(
                    "FALL DETECTION: Impact detected! peak=%.1f m/s² delta=%.1f m/s² stillness=%lums\n",
                    impactPeakMagnitude,
                    lastImpactDelta,
                    stillnessDurationRequired
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
                resetImpactTracking(accelMagnitude);
            } else if (currentTime - stillnessStartTime >= stillnessDurationRequired) {
                confirmFall(accelMagnitude, posX, posY);
            }
            break;

        case FALL_CONFIRMED:
            // Stay here until main loop handles the alert and calls clearFallDetection()
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
    lastFallEvent.detected = false;
    fallState = IDLE;
    impactPeakMagnitude = 0.0f;
    resetImpactTracking(prevAccelMagnitude);
    lastFallTime = millis();
}

// Get current fall detection state (for debugging)
const char* getFallStateString() {
    switch (fallState) {
        case IDLE: return "IDLE";
        case IMPACT_DETECTED: return "IMPACT_DETECTED";
        case CHECKING_STILLNESS: return "CHECKING_STILLNESS";
        case FALL_CONFIRMED: return "FALL_CONFIRMED";
        default: return "UNKNOWN";
    }
}

// Print fall detection status
void printFallStatus() {
    Serial.printf("Fall Detection State: %s\n", getFallStateString());
    Serial.printf("  Peak window max: %.1f m/s² | prev smoothed: %.1f m/s² | last delta: %.1f m/s²\n",
                  peakWindowMax, prevAccelMagnitude, lastImpactDelta);
    if (fallState == CHECKING_STILLNESS) {
        unsigned long elapsed = millis() - stillnessStartTime;
        Serial.printf("  Stillness check: %lums / %lums\n", elapsed, stillnessDurationRequired);
    } else if (fallState == IDLE && lastFallTime > 0) {
        unsigned long elapsed = millis() - lastFallTime;
        if (elapsed < DEBOUNCE_DURATION) {
            Serial.printf("  Re-arm in: %lums\n", DEBOUNCE_DURATION - elapsed);
        }
    }
}

// Test function to simulate a fall (for debugging)
void simulateFall() {
    Serial.println("SIMULATING FALL FOR TESTING!");
    confirmFall(28.5f, 10.5f, 5.3f);
}

#endif // FALL_DETECTION_H
