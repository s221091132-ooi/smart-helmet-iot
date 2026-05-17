// buzzer.h - Buzzer control patterns for Smart Helmet
// Handles power-on alert, reset confirmation, and fall alert sounds

#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

// Buzzer states
enum BuzzerPattern {
    PATTERN_NONE,
    PATTERN_POWER_ON,
    PATTERN_FALL_ALERT
};

// Buzzer state management
BuzzerPattern currentPattern = PATTERN_NONE;
unsigned long buzzerStateStartTime = 0;
int buzzerStateStep = 0;
bool buzzerOn = false;

// Forward declarations
const char* getBuzzerPatternString();

// Initialize buzzer system
void initializeBuzzer() {
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    
    currentPattern = PATTERN_NONE;
    buzzerStateStartTime = 0;
    buzzerStateStep = 0;
    buzzerOn = false;
    
    Serial.println("Buzzer system initialized");
}

// Start power-on pattern (continuous long beep until button pressed)
void startPowerOnPattern() {
    currentPattern = PATTERN_POWER_ON;
    buzzerStateStartTime = millis();
    buzzerStateStep = 0;
    Serial.println("🔔 BUZZER: Starting power-on pattern");
    Serial.println("    Pattern: TIIIIIIIIT (continuous long beep)");
    Serial.println("    Note: Will not stop until reset button (GPIO 27) is pressed");
}

// Start fall alert pattern (continuous beeping)
void startFallAlertPattern() {
    currentPattern = PATTERN_FALL_ALERT;
    buzzerStateStartTime = millis();
    buzzerStateStep = 0;
    digitalWrite(LED_PIN, HIGH);  // Turn on LED
    Serial.println("BUZZER: Starting fall alert pattern");
}

// Stop any buzzer pattern
void stopBuzzer() {
    if (currentPattern != PATTERN_NONE) {
        Serial.println("\n════════════════════════════════════════");
        Serial.println("⛔ BUZZER: STOPPING...");
        Serial.println("════════════════════════════════════════");
        Serial.printf("   Previous pattern: %s\n", getBuzzerPatternString());
        Serial.printf("   Millis: %lu ms\n", millis());
        Serial.println("   Reason: stopBuzzer() function called");
        Serial.println("   ⚠️  CHECK CALL STACK TO SEE WHO CALLED THIS!");
        Serial.println("════════════════════════════════════════\n");
    }
    currentPattern = PATTERN_NONE;
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    buzzerOn = false;
}

// Update buzzer state machine (call this in main loop)
void updateBuzzer() {
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - buzzerStateStartTime;
    
    switch (currentPattern) {
        case PATTERN_NONE:
            // No pattern active
            break;
            
        case PATTERN_POWER_ON:
            // Power-on pattern: continuous long beep (does not stop automatically)
            // Only stops when reset button is pressed
            
            if (!buzzerOn) {
                digitalWrite(BUZZER_PIN, HIGH);
                digitalWrite(LED_PIN, HIGH);
                buzzerOn = true;
            }
            // Keep buzzer on continuously - no automatic stop
            break;
            
        case PATTERN_FALL_ALERT:
            // Fall alert: continuous beeping (500ms on, 500ms off)
            
            if (buzzerStateStep == 0) {
                // On phase (500ms)
                if (!buzzerOn) {
                    digitalWrite(BUZZER_PIN, HIGH);
                    digitalWrite(LED_PIN, HIGH);
                    buzzerOn = true;
                }
                
                if (elapsed >= 500) {
                    digitalWrite(BUZZER_PIN, LOW);
                    buzzerOn = false;
                    buzzerStateStep = 1;
                    buzzerStateStartTime = currentTime;
                }
            } else {
                // Off phase (500ms)
                if (buzzerOn) {
                    digitalWrite(BUZZER_PIN, LOW);
                    buzzerOn = false;
                }
                
                if (elapsed >= 500) {
                    buzzerStateStep = 0;
                    buzzerStateStartTime = currentTime;
                }
            }
            break;
    }
}

// Get current buzzer pattern (for debugging)
const char* getBuzzerPatternString() {
    switch (currentPattern) {
        case PATTERN_NONE: return "NONE";
        case PATTERN_POWER_ON: return "POWER_ON";
        case PATTERN_FALL_ALERT: return "FALL_ALERT";
        default: return "UNKNOWN";
    }
}

// Check if a pattern is currently playing
bool isBuzzerActive() {
    return currentPattern != PATTERN_NONE;
}

// Check if power-on pattern is active (waiting for reset)
bool isWaitingForReset() {
    return currentPattern == PATTERN_POWER_ON;
}

#endif // BUZZER_H
