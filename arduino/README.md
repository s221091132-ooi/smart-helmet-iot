# Smart Helmet Arduino Setup Guide

This guide covers the Arduino/ESP32 firmware setup for the Smart Helmet IoT system.

## Hardware Connections

### MPU9250 (9-Axis IMU)
```
MPU9250 → ESP32
VCC     → 3.3V
GND     → GND
SDA     → GPIO 21
SCL     → GPIO 22
```

### DS18B20 (Temperature Sensor)
```
DS18B20 → ESP32
VCC     → 3.3V
GND     → GND
Data    → GPIO 4 (with 4.7kΩ pull-up resistor to 3.3V)
```

### ACS712 (Current Sensor)
```
ACS712  → ESP32
VCC     → 5V
GND     → GND
OUT     → GPIO 34 (ADC1)
```

### Battery Voltage Monitoring
```
Battery Voltage Divider:
Battery+ ─── R1 (10kΩ) ─── GPIO 35 (ADC1) ─── R2 (10kΩ) ─── GND

This creates a 2:1 voltage divider to safely read battery voltage
(LiPo batteries can be 4.2V max, ESP32 ADC max is 3.3V)
```

### Buzzer
```
Buzzer  → ESP32
+       → GPIO 25
-       → GND
```

### LED
```
LED     → ESP32
Anode   → GPIO 26 (through 220Ω resistor)
Cathode → GND
```

## Software Setup

### 1. Install Arduino IDE

1. Download from https://www.arduino.cc/en/software
2. Install for your operating system

### 2. Add ESP32 Board Support

1. Open Arduino IDE
2. Go to **File → Preferences**
3. In "Additional Board Manager URLs", add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Click OK
5. Go to **Tools → Board → Boards Manager**
6. Search for "esp32"
7. Install "esp32 by Espressif Systems"

### 3. Install Required Libraries

Go to **Tools → Manage Libraries** and install:

#### MPU9250 Library
- Search: "MPU9250"
- Install: "MPU9250" by hideakitai OR "Bolder Flight Systems MPU9250"
- Version: Latest

#### Temperature Sensor Libraries
- Search: "OneWire"
- Install: "OneWire" by Paul Stoffregen
- Search: "DallasTemperature"
- Install: "DallasTemperature" by Miles Burton

#### JSON Library
- Search: "ArduinoJson"
- Install: "ArduinoJson" by Benoit Blanchon
- Version: 6.x or later

### 4. Configure WiFi and API Settings

1. Navigate to `arduino/smart_helmet/`
2. Open `config.h`
3. Update the following values:

```cpp
#define WIFI_SSID "YourWiFiNetworkName"
#define WIFI_PASSWORD "YourWiFiPassword"
#define API_ENDPOINT "https://your-vercel-deployment.vercel.app"
```

**Important**: 
- ESP32 only supports 2.4GHz WiFi networks
- The API_ENDPOINT should be your Vercel deployment URL (no trailing slash)

### 5. Upload the Code

1. Connect ESP32 to your computer via USB
2. In Arduino IDE, select:
   - **Board**: "ESP32 Dev Module" (Tools → Board → ESP32 Arduino)
   - **Port**: Your ESP32's COM port (Tools → Port)
   - **Upload Speed**: 921600 (Tools → Upload Speed)
3. Open `smart_helmet.ino`
4. Click the **Upload** button (right arrow icon)
5. Wait for compilation and upload to complete

### 6. Monitor Serial Output

1. Open Serial Monitor: **Tools → Serial Monitor**
2. Set baud rate to **115200**
3. You should see initialization messages:
   ```
   =================================
   SMART HELMET IoT SYSTEM
   Version 1.0
   =================================
   
   Initializing sensors...
   MPU9250 initialized successfully
   DS18B20 found: 1 device(s)
   Analog sensors configured
   
   Connecting to WiFi...
   WiFi connected successfully!
   IP Address: 192.168.1.xxx
   
   SYSTEM READY
   =================================
   ```

## Testing and Calibration

### 1. Sensor Test

The system performs automatic sensor initialization on startup. Check Serial Monitor for:
- MPU9250 initialization status
- Temperature sensor detection
- WiFi connection status

### 2. Fall Detection Test

To test fall detection:
1. Hold the helmet
2. Make a quick downward motion (simulate dropping)
3. Keep it still for 2 seconds
4. You should hear the fall alert buzzer pattern

Or use the built-in simulator:
```cpp
// In setup(), add:
simulateFall();
```

### 3. Location Tracking Test

To test location tracking:
1. Press "Reset Location" on the dashboard
2. Wait for confirmation beep (beep-beep-beeeep)
3. Walk around for 30 seconds
4. Check the dashboard map for your path

### 4. Buzzer Pattern Test

To test all buzzer patterns:
```cpp
// In setup(), add:
testBuzzerPatterns();
```

This will play:
- Power-on pattern (5 beeps)
- Reset confirmation (beep-beep-beeeep)
- Fall alert (continuous beeping)

## Calibration

### MPU9250 Calibration

The MPU9250 auto-calibrates on startup:
1. Place helmet on flat surface
2. Keep it completely still
3. Power on and wait 5 seconds
4. Do not move until "MPU9250 calibration complete" appears

For better accuracy:
- Recalibrate before each use
- Keep away from magnetic interference (speakers, motors)

### Battery Voltage Calibration

If battery readings are inaccurate:

1. Measure actual battery voltage with multimeter
2. Read ESP32 reported voltage from Serial Monitor
3. Calculate correction factor:
   ```
   Correction = Actual Voltage / Reported Voltage
   ```
4. Update in `sensors.h`:
   ```cpp
   #define VOLTAGE_DIVIDER_RATIO (2.0 * correction_factor)
   ```

### Step Detection Tuning

If step detection is too sensitive or missing steps:

Edit `location_tracking.h`:
```cpp
#define STEP_THRESHOLD 11.8  // Increase to reduce sensitivity
#define STEP_LENGTH 0.7      // Adjust based on your stride (meters)
```

## Troubleshooting

### WiFi Connection Fails

**Symptoms**: "WiFi connection failed" in Serial Monitor

**Solutions**:
1. Check SSID and password spelling in `config.h`
2. Ensure using 2.4GHz network (not 5GHz)
3. Check WiFi signal strength
4. Try moving closer to router
5. Restart ESP32

### MPU9250 Not Detected

**Symptoms**: "MPU9250 initialization failed"

**Solutions**:
1. Check I2C connections (SDA to GPIO 21, SCL to GPIO 22)
2. Verify 3.3V power supply
3. Check for loose connections
4. Try different MPU9250 I2C address (0x68 or 0x69)
5. Test I2C scanner code to detect address

### Temperature Sensor Not Found

**Symptoms**: "DS18B20 not found"

**Solutions**:
1. Check data pin connection to GPIO 4
2. Verify 4.7kΩ pull-up resistor is installed
3. Check 3.3V power supply
4. Try replacing sensor (may be faulty)

### HTTP POST Fails

**Symptoms**: "HTTP POST failed" in Serial Monitor

**Solutions**:
1. Verify API_ENDPOINT URL is correct
2. Check WiFi connection is active
3. Verify Vercel API is deployed and running
4. Check Supabase credentials in dashboard `.env.local`
5. Look for error codes in Serial Monitor

### Compilation Errors

**Symptoms**: Red error messages during upload

**Solutions**:
1. Verify all libraries are installed
2. Check library versions (ArduinoJson needs v6+)
3. Restart Arduino IDE
4. Clear cache: Delete Arduino build folder
5. Reinstall ESP32 board support

### Upload Fails

**Symptoms**: "Failed to connect to ESP32"

**Solutions**:
1. Hold BOOT button while clicking upload
2. Try different USB cable
3. Check COM port selection
4. Install CP210x or CH340 USB drivers
5. Reduce upload speed to 115200

## Power Management

### Battery Life Optimization

Default configuration sends data every 1 second. To extend battery life:

1. Increase update interval in `config.h`:
   ```cpp
   #define DATA_SEND_INTERVAL 5000  // 5 seconds instead of 1
   ```

2. Enable deep sleep when stationary:
   ```cpp
   if (speed == 0.0) {
     esp_sleep_enable_timer_wakeup(5000000);  // 5 seconds
     esp_light_sleep_start();
   }
   ```

### Solar Charging

If using solar panel:
1. Connect solar panel through charge controller
2. Monitor charging current via ACS712
3. Check dashboard for solar current reading

## Advanced Configuration

### Changing Pin Assignments

If you need different GPIO pins, update in `sensors.h`:
```cpp
#define ONE_WIRE_BUS 4           // DS18B20
#define ACS712_PIN 34            // Current sensor
#define BATTERY_VOLTAGE_PIN 35   // Battery ADC
#define BUZZER_PIN 25            // Buzzer
#define LED_PIN 26               // LED
```

### Adjusting Fall Detection

Edit thresholds in `fall_detection.h`:
```cpp
#define IMPACT_THRESHOLD 24.5    // m/s² (2.5g)
#define STILLNESS_THRESHOLD 4.9  // m/s² (0.5g)
#define STILLNESS_DURATION 2000  // milliseconds
#define DEBOUNCE_DURATION 3000   // milliseconds
```

### Custom Buzzer Patterns

Edit patterns in `buzzer.h` within the `updateBuzzer()` function.

## File Structure

```
arduino/smart_helmet/
├── smart_helmet.ino      # Main sketch
├── config.h              # WiFi and API configuration
├── sensors.h             # Sensor reading functions
├── fall_detection.h      # Fall detection algorithm
├── location_tracking.h   # Location tracking logic
├── buzzer.h             # Buzzer control patterns
└── http_client.h        # HTTP API communication
```

## Next Steps

After successful upload:
1. Power on the helmet
2. Open the web dashboard
3. Click "Reset Location" button
4. Start testing features

For web dashboard setup, see the main README.md file.
