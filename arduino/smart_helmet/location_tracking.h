// location_tracking.h - 2D location (X, Y) for live map
// Forward at heading 0° = +Y (North). Right = +X (East). Left = -X.
// Each step moves STEP_LENGTH meters along current heading; distance_traveled sums steps.

#ifndef LOCATION_TRACKING_H
#define LOCATION_TRACKING_H

#include "sensors.h"
#include "debug_agent_log.h"
#include <math.h>

// Step = spike in |accelMag - adaptiveBaseline| (fixed 9.81 fails when helmet is tilted)
#define STEP_DYNAMIC_THRESHOLD 0.85f  // m/s² above local baseline (walking impulse)
#define STEP_BASELINE_LOCK 1.15f        // below this deviation, baseline slowly tracks magnitude
#define STEP_BASELINE_ALPHA 0.02f     // how fast baseline follows tilt (~50 frames)
#define STEP_LENGTH 0.71f             // meters/step (~2.3 ft)
#define STEP_MIN_INTERVAL 240         // ms between steps
#define SMOOTHING_ALPHA 0.25f

// If compass is 90° off on your mount, change this (e.g. 90 or -90)
#define HEADING_MOUNT_OFFSET 0.0f

// Cardinal direction thresholds
#define NORTH_MIN 337.5
#define NORTH_MAX 22.5
#define NORTHEAST_MIN 22.5
#define NORTHEAST_MAX 67.5
#define EAST_MIN 67.5
#define EAST_MAX 112.5
#define SOUTHEAST_MIN 112.5
#define SOUTHEAST_MAX 157.5
#define SOUTH_MIN 157.5
#define SOUTH_MAX 202.5
#define SOUTHWEST_MIN 202.5
#define SOUTHWEST_MAX 247.5
#define WEST_MIN 247.5
#define WEST_MAX 292.5
#define NORTHWEST_MIN 292.5
#define NORTHWEST_MAX 337.5

// Location tracking data
struct LocationData {
    float positionX;          // X coordinate in meters
    float positionY;          // Y coordinate in meters
    float heading;            // Current heading in degrees (0=N, 90=E, 180=S, 270=W)
    float speed;              // Current speed in km/h
    float altitude;           // Not used for map (kept at 0)
    float distanceTraveled;   // Total distance traveled in meters
    int stepCount;            // Total steps counted
    String direction;         // Cardinal direction (N, NE, E, SE, S, SW, W, NW)
};

LocationData location;

// False until GPIO 27 reset — then steps update X/Y and distance
bool locationTrackingEnabled = false;

// Step detection state
float smoothedAccelMag = 9.81f;
float lastDynamicAccel = 0.0f;
float stepMagBaseline = 9.81f;   // adapts to helmet tilt (|a| is often ~10 m/s², not 9.81)
unsigned long lastStepTime = 0;
unsigned long lastUpdateTime = 0;
unsigned long lastTrackDebugMs = 0;
// Initialize location tracking
void initializeLocationTracking() {
    location.positionX = 0.0;
    location.positionY = 0.0;
    location.heading = 0.0;
    location.speed = 0.0;
    location.altitude = 0.0f;
    location.distanceTraveled = 0.0;
    location.stepCount = 0;
    location.direction = "N";
    
    smoothedAccelMag = 9.81f;
    lastDynamicAccel = 0.0f;
    stepMagBaseline = 9.81f;
    lastStepTime = 0;
    lastUpdateTime = millis();
    locationTrackingEnabled = false;
    Serial.println("Map at (0,0) — press RESET (GPIO27) to start step tracking");
}

void enableLocationTracking() {
    locationTrackingEnabled = true;
    // Seed baseline from current orientation so first steps can cross threshold
    SensorData s = getSensorData();
    stepMagBaseline = s.accelMagnitude;
    smoothedAccelMag = s.accelMagnitude;
    lastDynamicAccel = 0.0f;
    Serial.println("TRACKING ON — walk with helmet on; watch Serial for STEP ok");
    // #region agent log
    agentLog("H1", "location_tracking.h:enableLocationTracking", "tracking_on_baseline",
             stepMagBaseline, smoothedAccelMag, (float)location.stepCount, 0);
    // #endregion
}

// Reset location to origin (0, 0) with altitude and movement cleared
void resetLocation() {
    location.positionX = 0.0;
    location.positionY = 0.0;
    location.heading = 0.0;
    location.speed = 0.0;
    location.altitude = 0.0;
    location.distanceTraveled = 0.0;
    location.stepCount = 0;
    location.direction = "N";
    
    smoothedAccelMag = 9.81f;
    lastDynamicAccel = 0.0f;
    stepMagBaseline = 9.81f;
    lastStepTime = 0;
    lastUpdateTime = millis();
    resetImuHeading();
    Serial.println("Location reset to origin (0, 0), altitude 0 m");
}

float normalizeHeading(float heading) {
    while (heading < 0.0f) heading += 360.0f;
    while (heading >= 360.0f) heading -= 360.0f;
    return heading;
}

float smoothAccelMagnitude(float magnitude) {
    smoothedAccelMag = SMOOTHING_ALPHA * magnitude + (1.0f - SMOOTHING_ALPHA) * smoothedAccelMag;
    return smoothedAccelMag;
}

// Detect steps: impulse above slowly-adapting magnitude baseline (tilted helmet OK)
bool detectStep(float accelMagnitude) {
    unsigned long currentTime = millis();
    float smoothed = smoothAccelMagnitude(accelMagnitude);

    // Track "resting" |a| when not in a big spike (helmet tilt changes total magnitude)
    float devFromBase = fabsf(accelMagnitude - stepMagBaseline);
    if (devFromBase < STEP_BASELINE_LOCK) {
        stepMagBaseline =
            (1.0f - STEP_BASELINE_ALPHA) * stepMagBaseline + STEP_BASELINE_ALPHA * accelMagnitude;
    }

    float dynamic = fabsf(smoothed - stepMagBaseline);

    bool isPeak = (dynamic > STEP_DYNAMIC_THRESHOLD) &&
                  (lastDynamicAccel <= STEP_DYNAMIC_THRESHOLD);
    bool enoughTimePassed = (lastStepTime == 0) ||
                            ((currentTime - lastStepTime) >= STEP_MIN_INTERVAL);

    lastDynamicAccel = dynamic;

    if (isPeak && enoughTimePassed) {
        if (lastStepTime > 0) {
            unsigned long intervalMs = currentTime - lastStepTime;
            if (intervalMs >= STEP_MIN_INTERVAL && intervalMs <= 2500) {
                float intervalSec = intervalMs / 1000.0f;
                location.speed = (STEP_LENGTH / intervalSec) * 3.6f;
                if (location.speed > 12.0f) location.speed = 12.0f;
            }
        }
        lastStepTime = currentTime;
        Serial.printf("STEP ok dyn=%.2f mag=%.2f base=%.2f\n", dynamic, smoothed, stepMagBaseline);
        // #region agent log
        agentLog("H4", "location_tracking.h:detectStep", "step_fired", dynamic, smoothed, stepMagBaseline,
                 location.stepCount + 1);
        // #endregion
        return true;
    }

    return false;
}

// Convert heading to cardinal direction
String headingToDirection(float heading) {
    if ((heading >= NORTH_MIN && heading <= 360.0) || (heading >= 0.0 && heading < NORTH_MAX)) {
        return "N";
    } else if (heading >= NORTHEAST_MIN && heading < NORTHEAST_MAX) {
        return "NE";
    } else if (heading >= EAST_MIN && heading < EAST_MAX) {
        return "E";
    } else if (heading >= SOUTHEAST_MIN && heading < SOUTHEAST_MAX) {
        return "SE";
    } else if (heading >= SOUTH_MIN && heading < SOUTH_MAX) {
        return "S";
    } else if (heading >= SOUTHWEST_MIN && heading < SOUTHWEST_MAX) {
        return "SW";
    } else if (heading >= WEST_MIN && heading < WEST_MAX) {
        return "W";
    } else if (heading >= NORTHWEST_MIN && heading < NORTHWEST_MAX) {
        return "NW";
    }
    return "N";  // Default
}

// Update position based on step and heading
void updatePosition() {
    // Convert heading to radians
    float headingRad = location.heading * PI / 180.0;
    
    // Calculate displacement (East = X, North = Y)
    float deltaX = STEP_LENGTH * sin(headingRad);
    float deltaY = STEP_LENGTH * cos(headingRad);
    
    // Update position
    location.positionX += deltaX;
    location.positionY += deltaY;
    
    // Update distance traveled
    location.distanceTraveled += STEP_LENGTH;
    
    location.stepCount++;

    Serial.printf("MAP: (X=%.2f, Y=%.2f) dist=%.2fm steps=%d heading=%.0f°\n",
                  location.positionX, location.positionY,
                  location.distanceTraveled, location.stepCount, location.heading);
}

// Decay speed to zero when no steps detected recently
void updateSpeed() {
    if (lastStepTime == 0 || (millis() - lastStepTime) > 2000) {
        location.speed = 0.0f;
    }
}

// Main location tracking update (call this in main loop)
void updateLocationTracking(SensorData sensorData) {
    readImuOnly();
    if (!mpuHasFreshData) {
        // #region agent log
        static unsigned long lastNoImuLog = 0;
        if (locationTrackingEnabled && millis() - lastNoImuLog > 2000) {
            lastNoImuLog = millis();
            agentLog("H2", "location_tracking.h:updateLocationTracking", "no_imu_sample",
                     locationTrackingEnabled ? 1.0f : 0.0f, 0.0f, (float)activeImu, 0);
        }
        // #endregion
        return;
    }
    sensorData = getSensorData();

    unsigned long currentTime = millis();
    float deltaTime = (currentTime - lastUpdateTime) / 1000.0f;

    if (deltaTime <= 0.0f) return;
    if (deltaTime > 0.5f) deltaTime = 0.05f;

    location.heading = normalizeHeading(sensorData.headingDeg + HEADING_MOUNT_OFFSET);
    location.direction = headingToDirection(location.heading);

    if (!locationTrackingEnabled) {
        if (currentTime - lastTrackDebugMs > 4000) {
            lastTrackDebugMs = currentTime;
            Serial.println("Tracking OFF — press GPIO27 RESET to start map from (0,0)");
        }
        lastUpdateTime = currentTime;
        return;
    }

    // #region agent log
    static unsigned long lastAgentSample = 0;
    if (currentTime - lastAgentSample >= 500) {
        lastAgentSample = currentTime;
        float dyn = fabsf(smoothedAccelMag - stepMagBaseline);
        agentLog("H3", "location_tracking.h:updateLocationTracking", "track_sample",
                 sensorData.accelMagnitude, stepMagBaseline, dyn, location.stepCount);
        agentLog("H5", "location_tracking.h:updateLocationTracking", "imu_path",
                 (float)activeImu, sensorData.accelMagnitude, 0.0f, mpuInitialized ? 1 : 0);
    }
    // #endregion

    if (detectStep(sensorData.accelMagnitude)) {
        updatePosition();
    }
    
    if (currentTime - lastTrackDebugMs > 2000) {
        lastTrackDebugMs = currentTime;
        float dyn = fabsf(smoothedAccelMag - stepMagBaseline);
        Serial.printf("TrackON |mag|=%.2f dyn=%.2f base=%.2f steps=%d pos(%.2f,%.2f)m dist=%.2fm\n",
                      sensorData.accelMagnitude, dyn, stepMagBaseline, location.stepCount,
                      location.positionX, location.positionY, location.distanceTraveled);
    }
    
    updateSpeed();
    location.altitude = 0.0f;
    lastUpdateTime = currentTime;
}

// Get current location data
LocationData getLocationData() {
    return location;
}

// Print location data for debugging
void printLocationData() {
    Serial.println("=== Location Data ===");
    Serial.printf("Position: (%.2f, %.2f) meters\n", location.positionX, location.positionY);
    Serial.printf("Heading: %.1f° (%s)\n", location.heading, location.direction.c_str());
    Serial.printf("Speed: %.1f km/h\n", location.speed);
    Serial.printf("Altitude: %.1f meters\n", location.altitude);
    Serial.printf("Distance traveled: %.1f meters\n", location.distanceTraveled);
    Serial.printf("Steps: %d\n", location.stepCount);
    Serial.println("===================");
}

#endif // LOCATION_TRACKING_H
