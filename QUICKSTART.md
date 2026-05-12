# Smart Helmet IoT System - Quick Start Guide

Get your Smart Helmet system up and running in 30 minutes.

## What You'll Build

A complete IoT monitoring system with:
- Real-time fall detection
- Location tracking (2D map)
- Battery monitoring
- Live web dashboard
- Buzzer & LED alerts

## Prerequisites Checklist

### Hardware (must have before starting)
- [ ] ESP32 development board
- [ ] MPU9250 sensor module
- [ ] DS18B20 temperature sensor
- [ ] ACS712 current sensor
- [ ] Buzzer and LED
- [ ] LiPo battery (3000mAh recommended)
- [ ] Breadboard and jumper wires
- [ ] USB cable for ESP32

### Software (install first)
- [ ] Arduino IDE installed
- [ ] Node.js 18+ installed
- [ ] Git installed
- [ ] Code editor (VS Code recommended)

### Accounts (sign up for free)
- [ ] GitHub account
- [ ] Vercel account
- [ ] Supabase account

## 5-Step Setup Process

### Step 1: Hardware Assembly (10 minutes)

1. Connect sensors to ESP32 following this wiring:

**Critical Connections:**
```
MPU9250:  SDA→GPIO21, SCL→GPIO22, VCC→3.3V, GND→GND
DS18B20:  Data→GPIO4 (with 4.7kΩ pullup), VCC→3.3V, GND→GND
ACS712:   OUT→GPIO34, VCC→5V, GND→GND
Battery:  Via voltage divider to GPIO35 and GND
Buzzer:   +→GPIO25, -→GND
LED:      Anode→GPIO26 (with 220Ω), Cathode→GND
```

2. Double-check all connections
3. Connect ESP32 to computer via USB

### Step 2: Database Setup (5 minutes)

1. Go to https://supabase.com and create account
2. Click "New Project"
3. Choose name, password, region
4. Wait for provisioning (2-3 minutes)
5. Go to SQL Editor
6. Copy contents of `supabase/schema.sql`
7. Paste and click "Run"
8. Go to Settings → API → Copy:
   - Project URL
   - anon/public key

### Step 3: Arduino Code (5 minutes)

1. Open Arduino IDE
2. Install ESP32 board support (see arduino/README.md)
3. Install libraries:
   - MPU9250
   - OneWire
   - DallasTemperature
   - ArduinoJson

4. Open `arduino/smart_helmet/smart_helmet.ino`
5. Edit `config.h`:
   ```cpp
   #define WIFI_SSID "YourWiFiName"
   #define WIFI_PASSWORD "YourPassword"
   #define API_ENDPOINT "https://temp-url.vercel.app"  // Update after Step 4
   ```

6. Select Board: "ESP32 Dev Module"
7. Select Port: Your ESP32's port
8. Click Upload
9. Open Serial Monitor (115200 baud)
10. Verify "SYSTEM READY" appears

### Step 4: Web Dashboard (5 minutes)

1. Open terminal in project folder
2. Navigate to dashboard:
   ```bash
   cd helmet-dashboard
   ```

3. Install dependencies:
   ```bash
   npm install
   ```

4. Create `.env.local`:
   ```bash
   cp .env.local.example .env.local
   ```

5. Edit `.env.local` with your Supabase credentials:
   ```env
   NEXT_PUBLIC_SUPABASE_URL=https://xxx.supabase.co
   NEXT_PUBLIC_SUPABASE_ANON_KEY=your_key_here
   ```

6. Deploy to Vercel:
   ```bash
   npm install -g vercel
   vercel login
   vercel --prod
   ```

7. Copy your Vercel URL (e.g., `https://smart-helmet-xxx.vercel.app`)

### Step 5: Final Configuration (5 minutes)

1. Update ESP32 with Vercel URL:
   - Edit `arduino/smart_helmet/config.h`
   - Change `API_ENDPOINT` to your Vercel URL
   - Re-upload to ESP32

2. Test the system:
   - Power on ESP32
   - Wait for buzzer (5 beeps pattern)
   - Open dashboard in browser
   - Click "Reset Location"
   - Listen for confirmation beep: beep-beep-beeeep

3. Verify data flow:
   - Dashboard shows "Connected" status
   - Battery percentage visible
   - Temperature reading present
   - Map shows origin (0, 0)

## First Use Test

### Test 1: Basic Connectivity (2 minutes)
1. Check dashboard updates every 1 second
2. Verify "Last update" timestamp changes
3. Check all sensor values are reasonable

**Expected Results:**
- Battery: 3.0-4.2V
- Temperature: 20-30°C
- Position: (0.0, 0.0)

### Test 2: Location Tracking (2 minutes)
1. Walk forward 10 steps
2. Watch dashboard map
3. Check position changes

**Expected Results:**
- Position moves away from origin
- Direction shows on compass
- Speed >0 when walking

### Test 3: Fall Detection (1 minute)
1. Hold helmet
2. Quick downward motion + stop
3. Keep still for 2 seconds

**Expected Results:**
- Buzzer sounds continuously
- LED turns on
- Red alert on dashboard
- Can acknowledge alert

## Troubleshooting

### ESP32 won't connect to WiFi
→ Check SSID/password spelling
→ Ensure 2.4GHz network (not 5GHz)
→ Check Serial Monitor for errors

### Dashboard shows "Disconnected"
→ Verify Supabase credentials
→ Check API endpoint URL
→ Look at browser console (F12)

### No data appearing
→ Check ESP32 Serial Monitor for HTTP errors
→ Verify Vercel deployment succeeded
→ Test API manually: `https://your-url.vercel.app/api/helmet/status`

### Fall detection not working
→ Check MPU9250 wiring
→ Verify sensor initialization in Serial Monitor
→ Adjust thresholds in `fall_detection.h`

## Next Steps

Once everything works:
1. ✅ Review full documentation in README.md
2. ✅ Run complete test suite (TESTING.md)
3. ✅ Customize thresholds for your use case
4. ✅ Add custom features
5. ✅ Share your Vercel URL with others

## Support Resources

- **Main README**: Comprehensive system documentation
- **Arduino README**: Detailed hardware setup
- **TESTING.md**: Complete testing checklist
- **DEPLOYMENT.md**: Vercel deployment guide

## Estimated Costs

All free tier for personal/testing use:
- Supabase: Free (500MB database)
- Vercel: Free (100GB bandwidth)
- Hardware: ~$50-80 USD

## Safety Notes

- Never exceed 3.3V on ESP32 GPIO pins
- Use voltage divider for battery monitoring
- Ensure proper polarity on LED and buzzer
- Don't short power pins

---

**Total Setup Time**: ~30 minutes
**Difficulty Level**: Intermediate
**Skills Required**: Basic electronics, command line, web development

**Questions?** Check the troubleshooting section or review detailed documentation in README.md.

Good luck with your Smart Helmet project! 🚀
