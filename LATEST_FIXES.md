# Latest Fixes Applied - Smart Helmet IoT System

## Date: May 13, 2026

## Issues Fixed

### 1. Current Sensor Display (ACS712)
**Problem**: Website showed "Not charging" instead of exact values like "0.00 A"

**Solution**: 
- Modified `BatteryGauge.tsx` to always display the exact numerical value
- Changed threshold from `> 0` to `> 0.01` to show green icon only when actually charging
- Now displays "0.00 A" in gray when not charging, instead of text "Not charging"

**File Changed**: `helmet-dashboard/components/BatteryGauge.tsx`

### 2. Temperature Sensor (LM35)
**Problem**: Website showed static 25°C instead of real-time changing values from LM35 sensor

**Solution**:
- Replaced DS18B20 digital temperature sensor with LM35 analog sensor
- Removed OneWire and DallasTemperature library dependencies
- Changed pin 4 from digital (DS18B20) to analog (LM35) reading
- LM35 formula: `temperatureC = (analogRead(pin) / 4095.0) * 3.3 * 100.0`
- Now reads real-time analog voltage from LM35 and converts to Celsius

**Files Changed**: 
- `arduino/smart_helmet/sensors.h`

**Wiring**: LM35 connected to GPIO 4 (analog pin)

### 3. Buzzer and Physical Reset Button Logic
**Problem**: 
- Buzzer was stopping automatically without button press
- Physical button (GPIO 27) was not working correctly
- Flow was incorrect

**Solution**:
- **New Flow**:
  1. **Power On**: Buzzer plays continuous "TIT-TIT-TIT-TIT-TIT" pattern (5 beeps, 2-second pause, repeat)
  2. **User presses physical button (GPIO 27)**: Buzzer stops immediately
  3. **Confirmation sound**: Buzzer plays "TIT-TIT-TIT-TIIIIT" (3 fast beeps + 1 long beep)
  4. **Location resets**: Map on website shows (0, 0, 0)
  5. **Buzzer silences**: Stays quiet until next power-on

- **Key Changes**:
  - Power-on buzzer now only stops when physical button (GPIO 27) is pressed
  - Server-side reset (from website) no longer stops the buzzer
  - Updated reset confirmation pattern to 3 fast beeps + 1 long beep
  - Added clear debug messages to track button presses

**Files Changed**:
- `arduino/smart_helmet/smart_helmet.ino`
- `arduino/smart_helmet/buzzer.h`

## How to Update Your Hardware

### For MacBook (you already have the files):
1. Open Terminal
2. Navigate to project folder:
   ```bash
   cd ~/Desktop/smart-helmet-iot
   ```
3. Pull latest changes:
   ```bash
   git pull origin main
   ```

### For Windows Laptop:
1. Open Command Prompt or Git Bash
2. Navigate to project folder:
   ```cmd
   cd C:\Users\jack_\Desktop\smart-helmet-iot
   ```
3. Pull latest changes:
   ```cmd
   git pull origin main
   ```
4. Open Arduino IDE
5. Open `C:\Users\jack_\Desktop\smart-helmet-iot\arduino\smart_helmet\smart_helmet.ino`
6. Click **Upload** button

## Testing Checklist

### Test 1: Current Sensor Display
- [ ] Power on ESP32
- [ ] Check website - should show exact value like "0.00 A" in gray
- [ ] Connect solar panel (if available) - should show green icon with value like "0.15 A"

### Test 2: Temperature Sensor (LM35)
- [ ] Power on ESP32
- [ ] Check website - should show changing temperature values (e.g., 23.4°C, 23.0°C, 22.0°C)
- [ ] Touch LM35 sensor with finger - temperature should increase on website
- [ ] Remove finger - temperature should decrease

### Test 3: Buzzer and Reset Button
- [ ] Power on ESP32
- [ ] Buzzer should play "TIT-TIT-TIT-TIT-TIT" pattern continuously
- [ ] Press physical button on GPIO 27
- [ ] Buzzer should stop immediately
- [ ] Buzzer should play "TIT-TIT-TIT-TIIIIT" (3 fast + 1 long beep)
- [ ] Buzzer should stay silent
- [ ] Website map should show location (0, 0)

## Pin Connections Summary

| Component | ESP32 Pin | Type | Notes |
|-----------|-----------|------|-------|
| LM35 Temperature | GPIO 4 | Analog | V_out to GPIO 4, +Vs to 3.3V, GND to GND |
| ACS712 Current | GPIO 35 | Analog | V_out to GPIO 35 |
| Battery Voltage | GPIO 34 | Analog | Via voltage divider (6:1 ratio) |
| Buzzer | GPIO 25 | Digital | Positive to GPIO 25, Negative to GND |
| LED | GPIO 26 | Digital | Positive to GPIO 26 via 220Ω resistor |
| Reset Button | GPIO 27 | Digital | One pin to GPIO 27, other pin to GND |
| MPU9250 | I2C | I2C | SDA to GPIO 21, SCL to GPIO 22 |

## Website Deployment
✅ Successfully deployed to Vercel: https://helmet-dashboard-kappa.vercel.app

## Files Modified in This Update
1. `arduino/smart_helmet/sensors.h` - LM35 temperature sensor support
2. `arduino/smart_helmet/smart_helmet.ino` - Physical button logic
3. `arduino/smart_helmet/buzzer.h` - Reset confirmation pattern (3 fast + 1 long)
4. `helmet-dashboard/components/BatteryGauge.tsx` - Exact current value display

## GitHub Repository
All changes pushed to: https://github.com/s221091132-ooi/smart-helmet-iot

## Need Help?
If you encounter any issues:
1. Check Serial Monitor in Arduino IDE (115200 baud rate)
2. Check browser console for website errors
3. Verify all pin connections match the table above
4. Make sure LM35 is connected to GPIO 4 (not DS18B20)
