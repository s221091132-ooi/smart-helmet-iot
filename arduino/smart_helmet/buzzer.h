// buzzer.h - Buzzer control patterns for Smart Helmet
// Handles power-on alert, reset confirmation, and fall alert sounds

#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

// Buzzer states
enum BuzzerPattern {
    PATTERN_NONE,
    PATTERN_POWER_ON,
    PATTERN_RESET_CONFIRM,
    PATTERN_FALL_ALERT
};

// Buzzer state management
BuzzerPattern currentPattern = PATTERN_NONE;
unsigned long buzzerStateStartTime = 0;
int buzzerStateStep = 0;
bool buzzerOn = false;
bool locationResetReceived = false;

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
    locationResetReceived = false;
    
    Serial.println("Buzzer system initialized");
}

// Start power-on pattern (5 beeps, pause, repeat until reset)
void startPowerOnPattern() {
    currentPattern = PATTERN_POWER_ON;
    buzzerStateStartTime = millis();
    buzzerStateStep = 0;
    Serial.println("🔔 BUZZER: Starting power-on pattern");
    Serial.println("    Pattern: BEEP-BEEP-BEEP-BEEP-BEEP (repeat)");
    Serial.println("    Press RESET BUTTON to stop");
}

// Start reset confirmation pattern (beep-beep-beeeep)
void startResetConfirmPattern() {
    currentPattern = PATTERN_RESET_CONFIRM;
    buzzerStateStartTime = millis();
    buzzerStateStep = 0;
    locationResetReceived = true;
    Serial.println("🔔 BUZZER: Starting reset confirmation pattern");
    Serial.println("    Pattern: BEEP-BEEP-BEEEEP");
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
        Serial.println("⛔ BUZZER: Stopping...");
        Serial.printf("   Previous pattern: %s\n", getBuzzerPatternString());
        Serial.println("   Reason: stopBuzzer() called");
    }
    currentPattern = PATTERN_NONE;
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    buzzerOn = false;
}

// Check if location reset has been received
bool isLocationResetReceived() {
    return locationResetReceived;
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
            // Power-on pattern: 5 beeps (100ms on, 100ms off), then 2 second pause, repeat
            // Steps: 0=beep1_on, 1=beep1_off, 2=beep2_on, 3=beep2_off, 4=beep3_on, 5=beep3_off,
            //        6=beep4_on, 7=beep4_off, 8=beep5_on, 9=beep5_off, 10=pause
            
            if (buzzerStateStep < 10) {
                // Beeping phase
                if (buzzerStateStep % 2 == 0) {
                    // On phase (100ms)
                    if (elapsed < 100) {
                        if (!buzzerOn) {
                            digitalWrite(BUZZER_PIN, HIGH);
                            buzzerOn = true;
                        }
                    } else {
                        buzzerStateStep++;
                        buzzerStateStartTime = currentTime;
                    }
                } else {
                    // Off phase (100ms)
                    if (elapsed < 100) {
                        if (buzzerOn) {
                            digitalWrite(BUZZER_PIN, LOW);
                            buzzerOn = false;
                        }
                    } else {
                        buzzerStateStep++;
                        buzzerStateStartTime = currentTime;
                    }
                }
            } else {
                // Pause phase (2000ms)
                if (buzzerOn) {
                    digitalWrite(BUZZER_PIN, LOW);
                    buzzerOn = false;
                }
                
                if (elapsed >= 2000) {
                    // Restart pattern
                    buzzerStateStep = 0;
                    buzzerStateStartTime = currentTime;
                }
            }
            break;
            
        case PATTERN_RESET_CONFIRM:
            // Reset confirmation: beep (100ms), pause (100ms), beep (100ms), pause (100ms), beeeep (500ms)
            // Steps: 0=beep1_on, 1=pause1, 2=beep2_on, 3=pause2, 4=long_beep, 5=done
            
            switch (buzzerStateStep) {
                case 0:  // First beep (100ms)
                    if (!buzzerOn) {
                        digitalWrite(BUZZER_PIN, HIGH);
                        buzzerOn = true;
                    }
                    if (elapsed >= 100) {
                        digitalWrite(BUZZER_PIN, LOW);
                        buzzerOn = false;
                        buzzerStateStep++;
                        buzzerStateStartTime = currentTime;
                    }
                    break;
                    
                case 1:  // First pause (100ms)
                    if (elapsed >= 100) {
                        buzzerStateStep++;
                        buzzerStateStartTime = currentTime;
                    }
                    break;
                    
                case 2:  // Second beep (100ms)
                    if (!buzzerOn) {
                        digitalWrite(BUZZER_PIN, HIGH);
                        buzzerOn = true;
                    }
                    if (elapsed >= 100) {
                        digitalWrite(BUZZER_PIN, LOW);
                        buzzerOn = false;
                        buzzerStateStep++;
                        buzzerStateStartTime = currentTime;
                    }
                    break;
                    
                case 3:  // Second pause (100ms)
                    if (elapsed >= 100) {
                        buzzerStateStep++;
                        buzzerStateStartTime = currentTime;
                    }
                    break;
                    
                case 4:  // Long beep (500ms)
                    if (!buzzerOn) {
                        digitalWrite(BUZZER_PIN, HIGH);
                        buzzerOn = true;
                    }
                    if (elapsed >= 500) {
                        digitalWrite(BUZZER_PIN, LOW);
                        buzzerOn = false;
                        buzzerStateStep++;
                        buzzerStateStartTime = currentTime;
                    }
                    break;
                    
                case 5:  // Done
                    stopBuzzer();
                    break;
            }
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
        case PATTERN_RESET_CONFIRM: return "RESET_CONFIRM";
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
