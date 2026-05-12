// location_tracking.h - Hybrid Location Tracking for Smart Helmet
// Uses step detection + gyroscope heading for pedestrian dead reckoning

#ifndef LOCATION_TRACKING_H
#define LOCATION_TRACKING_H

#include "sensors.h"
#include <math.h>

// Step detection configuration
#define STEP_THRESHOLD 11.8         // 1.2g in m/s² (vertical acceleration for step)
#define STEP_LENGTH 0.7             // Average step length in meters
#define STEP_MIN_INTERVAL 300       // Minimum time between steps (ms) - prevents double counting
#define SMOOTHING_ALPHA 0.3         // Low-pass filter coefficient

// Heading configuration
#define GYRO_SAMPLE_TIME 0.05       // Sample time in seconds (50ms = 20Hz)
#define COMPLEMENTARY_FILTER_ALPHA 0.98  // Weight for gyro vs magnetometer
#define MAG_DECLINATION 0.0         // Magnetic declination in degrees (adjust for location)

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
    float altitude;           // Altitude in meters
    float distanceTraveled;   // Total distance traveled in meters
    int stepCount;            // Total steps counted
    String direction;         // Cardinal direction (N, NE, E, SE, S, SW, W, NW)
};

LocationData location;

// Step detection state
float lastVerticalAccel = 0;
float smoothedVerticalAccel = 0;
bool stepDetected = false;
unsigned long lastStepTime = 0;
unsigned long lastUpdateTime = 0;

// Heading calculation state
float headingFromMag = 0;
float headingFromGyro = 0;

// Initialize location tracking
void initializeLocationTracking() {
    location.positionX = 0.0;
    location.positionY = 0.0;
    location.heading = 0.0;
    location.speed = 0.0;
    location.altitude = 100.0;  // Default altitude
    location.distanceTraveled = 0.0;
    location.stepCount = 0;
    location.direction = "N";
    
    lastVerticalAccel = 0;
    smoothedVerticalAccel = 0;
    lastStepTime = millis();
    lastUpdateTime = millis();
    
    Serial.println("Location tracking initialized at origin (0, 0)");
}

// Reset location to origin (0, 0)
void resetLocation() {
    location.positionX = 0.0;
    location.positionY = 0.0;
    location.distanceTraveled = 0.0;
    location.stepCount = 0;
    
    Serial.println("Location reset to origin (0, 0)");
}

// Low-pass filter for smoothing vertical acceleration
float smoothVerticalAcceleration(float newAccel) {
    smoothedVerticalAccel = SMOOTHING_ALPHA * newAccel + (1 - SMOOTHING_ALPHA) * smoothedVerticalAccel;
    return smoothedVerticalAccel;
}

// Detect steps from vertical acceleration
bool detectStep(float verticalAccel) {
    unsigned long currentTime = millis();
    
    // Smooth the acceleration
    float smoothed = smoothVerticalAcceleration(verticalAccel);
    
    // Detect peak (step) when crossing threshold and sufficient time has passed
    bool isPeak = (smoothed > STEP_THRESHOLD) && (lastVerticalAccel <= STEP_THRESHOLD);
    bool enoughTimePassed = (currentTime - lastStepTime) >= STEP_MIN_INTERVAL;
    
    if (isPeak && enoughTimePassed) {
        lastStepTime = currentTime;
        lastVerticalAccel = smoothed;
        return true;
    }
    
    lastVerticalAccel = smoothed;
    return false;
}

// Calculate heading from magnetometer
float calculateMagnetometerHeading(int16_t magX, int16_t magY) {
    // Calculate heading using atan2
    float heading = atan2(magY, magX) * 180.0 / PI;
    
    // Apply magnetic declination
    heading += MAG_DECLINATION;
    
    // Normalize to 0-360 degrees
    if (heading < 0) heading += 360.0;
    if (heading >= 360.0) heading -= 360.0;
    
    return heading;
}

// Update heading using complementary filter (gyro + magnetometer)
void updateHeading(int16_t gyroZ, int16_t magX, int16_t magY, float deltaTime) {
    // Convert gyro raw value to degrees per second
    float gyroRate = gyroZ / 131.0;  // For ±250°/s range
    
    // Integrate gyroscope (dead reckoning)
    headingFromGyro += gyroRate * deltaTime;
    
    // Normalize gyro heading
    if (headingFromGyro < 0) headingFromGyro += 360.0;
    if (headingFromGyro >= 360.0) headingFromGyro -= 360.0;
    
    // Get heading from magnetometer
    headingFromMag = calculateMagnetometerHeading(magX, magY);
    
    // Complementary filter: combine gyro (short-term) with magnetometer (long-term)
    location.heading = COMPLEMENTARY_FILTER_ALPHA * headingFromGyro + 
                       (1 - COMPLEMENTARY_FILTER_ALPHA) * headingFromMag;
    
    // Update gyro heading with filtered result to prevent drift
    headingFromGyro = location.heading;
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
    
    // Update step count
    location.stepCount++;
}

// Calculate speed based on step frequency
void updateSpeed() {
    unsigned long currentTime = millis();
    unsigned long timeSinceLastStep = currentTime - lastStepTime;
    
    if (timeSinceLastStep < 2000) {  // If stepped within last 2 seconds
        // Speed = distance / time
        float stepsPerSecond = 1000.0 / timeSinceLastStep;
        float metersPerSecond = stepsPerSecond * STEP_LENGTH;
        location.speed = metersPerSecond * 3.6;  // Convert m/s to km/h
    } else {
        // No recent steps, speed is zero
        location.speed = 0.0;
    }
}

// Update altitude (simplified - using vertical acceleration integration)
void updateAltitude(float accelZ, float deltaTime) {
    // Convert acceleration to m/s²
    float verticalAccel = (accelZ / 16384.0) * 9.81;
    
    // Remove gravity offset
    verticalAccel -= 9.81;
    
    // Simple altitude estimation (accumulates error over time)
    // In practice, would need barometer for accuracy
    static float verticalVelocity = 0;
    verticalVelocity += verticalAccel * deltaTime;
    location.altitude += verticalVelocity * deltaTime;
    
    // Add damping to prevent drift
    verticalVelocity *= 0.98;
}

// Main location tracking update (call this in main loop)
void updateLocationTracking(SensorData sensorData) {
    unsigned long currentTime = millis();
    float deltaTime = (currentTime - lastUpdateTime) / 1000.0;  // Convert to seconds
    
    if (deltaTime <= 0) return;  // Skip if no time has passed
    
    // Update heading using gyro and magnetometer
    updateHeading(sensorData.gyroZ, sensorData.magX, sensorData.magY, deltaTime);
    
    // Update direction string
    location.direction = headingToDirection(location.heading);
    
    // Detect steps from vertical acceleration
    float verticalAccel = sqrt(sensorData.accelZ * sensorData.accelZ) / 16384.0 * 9.81;
    if (detectStep(verticalAccel)) {
        Serial.println("LOCATION: Step detected!");
        updatePosition();
    }
    
    // Update speed
    updateSpeed();
    
    // Update altitude
    updateAltitude(sensorData.accelZ, deltaTime);
    
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
