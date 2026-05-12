# Smart Helmet IoT System

A comprehensive IoT monitoring system for smart helmets featuring real-time fall detection, location tracking, battery monitoring, and a web-based dashboard.

## Overview

This system combines ESP32-based hardware sensors with a modern web dashboard to provide:
- **Fall Detection**: Automatic detection using MPU9250 accelerometer with impact and stillness validation
- **Location Tracking**: Hybrid pedestrian dead reckoning using step detection and gyroscope heading
- **Battery Monitoring**: Real-time voltage, percentage, capacity, and solar charging status
- **Real-time Dashboard**: Web interface with live updates, 2D map visualization, and alerts
- **Alert System**: Buzzer patterns and LED indicators for different events

## System Architecture

```
ESP32 Helmet → WiFi → Vercel API → Supabase Database → Next.js Dashboard → User
```

## Hardware Requirements

### Components
- **ESP32 Development Board** (with WiFi capability)
- **MPU9250** - 9-axis motion sensor (accelerometer, gyroscope, magnetometer)
- **DS18B20** - Digital temperature sensor
- **ACS712** - Current sensor (5A model recommended)
- **B25 Fuel Gauge** - Battery monitoring IC
- **Buzzer** - Active or passive buzzer
- **LED** - Standard 5mm LED
- **Resistors** - 220Ω for LED, pull-up resistors for I2C
- **LiPo Battery** - 3000mAh recommended
- **Solar Panel** (optional) - For charging

### Wiring Diagram

```
ESP32 Pin Connections:
┌─────────────────────────────────────────┐
│ Component    │ ESP32 Pin │ Notes       │
├─────────────────────────────────────────┤
│ MPU9250 SDA  │ GPIO 21   │ I2C Data    │
│ MPU9250 SCL  │ GPIO 22   │ I2C Clock   │
│ MPU9250 VCC  │ 3.3V      │             │
│ MPU9250 GND  │ GND       │             │
├─────────────────────────────────────────┤
│ DS18B20 Data │ GPIO 4    │ + 4.7kΩ PU  │
│ DS18B20 VCC  │ 3.3V      │             │
│ DS18B20 GND  │ GND       │             │
├─────────────────────────────────────────┤
│ ACS712 OUT   │ GPIO 34   │ ADC Input   │
│ ACS712 VCC   │ 5V        │             │
│ ACS712 GND   │ GND       │             │
├─────────────────────────────────────────┤
│ Battery+     │ GPIO 35   │ Via divider │
│ Battery-     │ GND       │             │
├─────────────────────────────────────────┤
│ Buzzer+      │ GPIO 25   │             │
│ Buzzer-      │ GND       │             │
├─────────────────────────────────────────┤
│ LED Anode    │ GPIO 26   │ + 220Ω res  │
│ LED Cathode  │ GND       │             │
└─────────────────────────────────────────┘

Voltage Divider for Battery Monitoring:
Battery+ ─── R1 (10kΩ) ─── GPIO 35 ─── R2 (10kΩ) ─── GND
```

## Software Setup

### 1. Supabase Setup

1. Create a new project at [supabase.com](https://supabase.com)
2. Navigate to the SQL Editor
3. Run the schema creation script:
   ```bash
   supabase/schema.sql
   ```
4. (Optional) Run seed data for testing:
   ```bash
   supabase/seed.sql
   ```
5. Get your credentials:
   - Go to Project Settings → API
   - Copy `Project URL` and `anon/public` key

### 2. Arduino/ESP32 Setup

#### Install Arduino IDE
1. Download from [arduino.cc](https://www.arduino.cc/en/software)
2. Install ESP32 board support:
   - Open Arduino IDE
   - Go to File → Preferences
   - Add to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to Tools → Board → Boards Manager
   - Search for "ESP32" and install

#### Install Required Libraries
In Arduino IDE, go to Tools → Manage Libraries and install:
- `MPU9250` by hideakitai or bolderflight
- `OneWire` by Paul Stoffregen
- `DallasTemperature` by Miles Burton
- `ArduinoJson` by Benoit Blanchon

#### Configure the Helmet Code
1. Open `arduino/smart_helmet/smart_helmet.ino`
2. Edit `arduino/smart_helmet/config.h`:
   ```cpp
   #define WIFI_SSID "YourWiFiName"
   #define WIFI_PASSWORD "YourWiFiPassword"
   #define API_ENDPOINT "https://your-project.vercel.app"
   ```
3. Upload to ESP32:
   - Select Board: "ESP32 Dev Module"
   - Select Port: Your ESP32's COM port
   - Click Upload

### 3. Web Dashboard Setup

#### Prerequisites
- Node.js 18+ installed
- npm or yarn package manager

#### Installation
```bash
cd helmet-dashboard

# Install dependencies
npm install

# Copy environment template
cp .env.local.example .env.local

# Edit .env.local with your Supabase credentials
nano .env.local
```

#### Configure Environment Variables
Edit `helmet-dashboard/.env.local`:
```env
NEXT_PUBLIC_SUPABASE_URL=https://your-project.supabase.co
NEXT_PUBLIC_SUPABASE_ANON_KEY=your_anon_key_here
```

#### Run Development Server
```bash
npm run dev
```
Open [http://localhost:3000](http://localhost:3000)

#### Deploy to Vercel
```bash
# Install Vercel CLI
npm install -g vercel

# Login to Vercel
vercel login

# Deploy
vercel

# Set environment variables in Vercel dashboard:
# Settings → Environment Variables
# Add NEXT_PUBLIC_SUPABASE_URL and NEXT_PUBLIC_SUPABASE_ANON_KEY

# Deploy to production
vercel --prod
```

After deployment, update the `API_ENDPOINT` in your Arduino `config.h` with your Vercel URL.

## Usage Instructions

### First Time Setup

1. **Power on the ESP32**
   - The buzzer will beep 5 times rapidly, pause 2 seconds, and repeat
   - This indicates waiting for location reset

2. **Open the Dashboard**
   - Navigate to your Vercel URL
   - You should see "Waiting for helmet data..."

3. **Reset Location Origin**
   - Click the "Reset Location" button on the dashboard
   - Confirm the reset
   - The helmet will play the confirmation sound: beep-beep-beeeep
   - Location tracking is now active from origin (0, 0)

### Normal Operation

- **Data Updates**: Dashboard updates every 1 second
- **Battery Monitoring**: 
  - Green indicator: >30% battery
  - Yellow indicator: 10-30% battery
  - Red flashing: <10% battery (critical)
- **Location Tracking**: Walk around to see your path on the map
- **Fall Detection**: 
  - If a fall is detected, buzzer sounds continuously and LED turns on
  - Red alert appears on dashboard
  - Click "Acknowledge Alert" to dismiss

### Testing Features

The Arduino code includes test functions you can uncomment in `setup()`:

```cpp
// Test buzzer patterns
testBuzzerPatterns();

// Test sensor readings
testSensors();

// Test location tracking (walk for 30 seconds)
testLocationTracking();

// Test API connection
testAPIConnection();
```

## Features Explained

### Fall Detection Algorithm
1. Monitors acceleration magnitude continuously
2. Detects sudden spike >2.5g (impact threshold)
3. Waits 500ms then checks for stillness
4. If acceleration remains <0.5g for 2 seconds, fall is confirmed
5. Triggers buzzer, LED, and sends alert to dashboard
6. 3-second debounce prevents false positives

### Location Tracking (Hybrid Approach)
1. **Step Detection**: Analyzes vertical acceleration peaks to count steps
2. **Heading Calculation**: Uses gyroscope integration with magnetometer correction
3. **Position Update**: Calculates displacement using step length (0.7m) and heading
4. **Dead Reckoning**: Accumulates position changes from origin
5. **Direction Mapping**: Converts heading to cardinal directions (N, NE, E, etc.)

**Accuracy**: Expect ~5-15m error over 100m distance (typical for pedestrian dead reckoning)

### Battery Monitoring
- **Voltage**: Measured via voltage divider on ADC
- **Percentage**: Calculated based on voltage curve (3.0V=0%, 4.2V=100%)
- **Capacity**: Computed from percentage × battery capacity
- **Solar Current**: Measured via ACS712 current sensor

## API Endpoints

### POST `/api/helmet/data`
Send sensor data from ESP32
```json
{
  "battery_voltage": 4.15,
  "battery_percentage": 85,
  "remaining_capacity": 2550,
  "solar_current": 0.25,
  "temperature": 28.5,
  "accel": {"x": 150, "y": -20, "z": 16380},
  "gyro": {"x": 5, "y": -2, "z": 10},
  "position": {"x": 25.3, "y": -12.7},
  "direction": "NE",
  "speed": 4.2,
  "altitude": 102.5,
  "distance_traveled": 185.3
}
```

### POST `/api/helmet/fall`
Report fall detection event
```json
{
  "position_x": 10.5,
  "position_y": 8.3,
  "accel_magnitude": 28.7
}
```

### GET `/api/helmet/status`
Get current helmet status and unacknowledged falls

### GET `/api/helmet/history`
Query historical data with pagination
- Query params: `limit`, `offset`, `start_date`, `end_date`

### POST `/api/helmet/reset-location`
Reset location tracking origin

## Troubleshooting

### ESP32 Won't Connect to WiFi
- Check SSID and password in `config.h`
- Ensure 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Check WiFi signal strength
- View Serial Monitor (115200 baud) for error messages

### No Data on Dashboard
- Verify Supabase credentials in `.env.local`
- Check ESP32 Serial Monitor for HTTP errors
- Ensure API_ENDPOINT in Arduino matches Vercel URL
- Check browser console for errors

### Inaccurate Location Tracking
- Calibrate MPU9250: Keep helmet still for 2 seconds on startup
- Adjust step threshold in `location_tracking.h`
- Check magnetometer interference (avoid metal objects)
- Reset location regularly for better accuracy

### Fall Detection Too Sensitive/Insensitive
- Adjust thresholds in `fall_detection.h`:
  - `IMPACT_THRESHOLD` (default 24.5 m/s²)
  - `STILLNESS_THRESHOLD` (default 4.9 m/s²)
  - `STILLNESS_DURATION` (default 2000ms)

### Battery Reading Incorrect
- Check voltage divider resistor values
- Verify `VOLTAGE_DIVIDER_RATIO` in `sensors.h`
- Calibrate using a multimeter

## Performance Considerations

### ESP32
- **Update Rate**: 1 second (configurable in `config.h`)
- **Power Consumption**: ~100-200mA during WiFi transmission
- **WiFi Range**: ~50m indoor, ~100m outdoor (depends on environment)
- **Battery Life**: ~15-20 hours with 3000mAh battery

### Dashboard
- **Data Storage**: ~86,400 records/day at 1-second intervals
- **Supabase Free Tier**: 500MB database, 2GB bandwidth/month
- **Recommended Cleanup**: Delete records >30 days old

### Database Optimization
Add automatic cleanup (Supabase SQL Editor):
```sql
-- Delete helmet_data older than 30 days
DELETE FROM helmet_data 
WHERE timestamp < NOW() - INTERVAL '30 days';

-- Schedule this to run daily
```

## Security Notes

- **Never commit** `config.h` or `.env.local` to version control
- Add to `.gitignore`:
  ```
  arduino/smart_helmet/config.h
  helmet-dashboard/.env.local
  ```
- Use Supabase Row Level Security (RLS) policies (already configured in schema)
- For production, add API key authentication to prevent unauthorized access

## Future Enhancements

- [ ] GPS integration for absolute positioning
- [ ] LoRa module for long-range communication
- [ ] Email/SMS notifications for fall alerts
- [ ] Heart rate monitoring integration
- [ ] Mobile app (React Native)
- [ ] SD card logging for offline operation
- [ ] WebSocket for real-time updates (currently using polling)
- [ ] User authentication and multi-helmet support

## License

MIT License - Feel free to use and modify for your projects.

## Support

For issues, questions, or contributions:
1. Check existing documentation
2. Review Serial Monitor output for errors
3. Check browser console for web errors
4. Verify all connections and configurations

## Credits

Developed for IoT Final Year Project
- Hardware: ESP32, MPU9250, DS18B20, ACS712
- Backend: Vercel, Supabase
- Frontend: Next.js, Tailwind CSS, TypeScript
