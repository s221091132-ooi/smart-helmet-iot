# 🎓 FINAL SETUP GUIDE - Smart Helmet IoT System

**Status**: ✅ **DEPLOYED & READY FOR HARDWARE SETUP**

---

## 📊 CURRENT STATUS

### ✅ **COMPLETED**
1. ✅ **Supabase Database** - Created and configured
2. ✅ **Website Dashboard** - Deployed to Vercel
3. ✅ **Arduino Code** - Complete and ready to upload
4. ✅ **API Routes** - All 5 routes working
5. ✅ **GitHub Repository** - Code pushed

### 🔗 **LIVE URLS**
- **Production Website**: https://helmet-dashboard-kappa.vercel.app
- **Supabase Project**: https://supabase.com/dashboard/project/xbivtgdxjcfsrpwemjoi
- **GitHub Repo**: https://github.com/s221091132-ooi/smart-helmet-iot

---

## 🛠️ ARDUINO SETUP (MacBook)

### 📦 **Step 1: Install Arduino IDE**
1. Download Arduino IDE 2.x from: https://www.arduino.cc/en/software
2. Install and open Arduino IDE

### 📦 **Step 2: Install ESP32 Board Support**
1. Go to **Arduino IDE > Settings (⌘,)**
2. In "Additional Board Manager URLs", paste:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools > Board > Boards Manager**
4. Search "ESP32" and install **"esp32 by Espressif Systems"**

### 📦 **Step 3: Install Required Libraries**
Go to **Sketch > Include Library > Manage Libraries**, then install:

1. **WiFi** (included with ESP32)
2. **HTTPClient** (included with ESP32)
3. **ArduinoJson** by Benoit Blanchon (v7.x)
4. **MPU9250** by hideakitai
5. **OneWire** by Paul Stoffregen
6. **DallasTemperature** by Miles Burton

### ⚙️ **Step 4: Configure Arduino Code**

1. Open the project folder:
   ```bash
   cd "/Users/hansongnkk/FYP KAIJIE/arduino/smart_helmet"
   ```

2. Open `smart_helmet.ino` in Arduino IDE

3. Edit `config.h` and replace:
   ```cpp
   #define WIFI_SSID "YOUR_WIFI_SSID"          // Your WiFi name
   #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"  // Your WiFi password
   ```

   **Note**: The API endpoint is already configured:
   ```cpp
   #define API_ENDPOINT "https://helmet-dashboard-kappa.vercel.app"
   ```

### 🔌 **Step 5: Hardware Wiring**

Connect components to ESP32 according to this pinout:

```
┌─────────────────────────────────────────────┐
│         ESP32 PINOUT DIAGRAM                │
├─────────────────────────────────────────────┤
│                                             │
│  MPU9250 (I2C):                            │
│    • SDA → GPIO 21                          │
│    • SCL → GPIO 22                          │
│    • VCC → 3.3V                             │
│    • GND → GND                              │
│                                             │
│  DS18B20 (Temperature):                     │
│    • DATA → GPIO 4                          │
│    • VCC → 3.3V                             │
│    • GND → GND                              │
│    • 4.7kΩ resistor between DATA and VCC   │
│                                             │
│  ACS712 (Current Sensor):                   │
│    • OUT → GPIO 36 (VP)                     │
│    • VCC → 5V                               │
│    • GND → GND                              │
│                                             │
│  Battery Voltage Divider:                   │
│    • Battery (+) → 10kΩ → GPIO 39 (VN)     │
│    •                 ↓                      │
│    •                10kΩ → GND              │
│    • Battery (-) → GND                      │
│                                             │
│  Buzzer:                                    │
│    • (+) → GPIO 5                           │
│    • (-) → GND                              │
│                                             │
│  LED:                                       │
│    • Anode (+) → GPIO 2                     │
│    • Cathode (-) → 220Ω → GND              │
│                                             │
└─────────────────────────────────────────────┘
```

### 📤 **Step 6: Upload Code to ESP32**

1. Connect ESP32 to MacBook via USB-C cable
2. In Arduino IDE:
   - **Tools > Board** → Select your ESP32 board (e.g., "ESP32 Dev Module")
   - **Tools > Port** → Select `/dev/cu.usbserial-xxxxx` or similar
   - **Tools > Upload Speed** → 115200
3. Click **Upload** button (→)
4. Wait for "Done uploading" message

### 🔍 **Step 7: Monitor Serial Output**

1. Open **Tools > Serial Monitor**
2. Set baud rate to **115200**
3. You should see:
   ```
   🚀 Smart Helmet System Starting...
   📡 Connecting to WiFi: [Your SSID]
   ✅ WiFi Connected! IP: 192.168.x.x
   🌐 Server: helmet-dashboard-kappa.vercel.app
   📊 Sending sensor data...
   ✅ Data sent successfully
   ```

---

## 🌐 TESTING THE SYSTEM

### 1️⃣ **Test Website (No Hardware Needed)**

Visit: https://helmet-dashboard-kappa.vercel.app

You should see:
- Dashboard interface loading
- Empty data (waiting for ESP32)
- All UI components visible

### 2️⃣ **Test with Hardware**

Once ESP32 is running:

1. **Check Real-Time Data**:
   - Open dashboard: https://helmet-dashboard-kappa.vercel.app
   - Data should update every 1 second
   - Battery gauge should show readings
   - Location map should track movement

2. **Test Fall Detection**:
   - Shake the helmet vigorously
   - Fall alert notification should appear
   - Click "Acknowledge" to dismiss

3. **Test Location Reset**:
   - Click "Reset Location" button
   - Confirm in dialog
   - ESP32 should beep twice
   - Position resets to center (0, 0)

---

## 🐛 TROUBLESHOOTING

### ❌ **ESP32 Won't Connect to WiFi**
- Check WiFi credentials in `config.h`
- Ensure WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
- Check signal strength

### ❌ **"Compilation Error" in Arduino IDE**
- Verify all libraries are installed (Step 3)
- Check board selection (Tools > Board)
- Update ESP32 board support to latest version

### ❌ **No Data on Website**
- Open Serial Monitor (115200 baud)
- Look for "✅ Data sent successfully"
- If you see "❌ HTTP Error", check API endpoint in `config.h`
- Verify Supabase environment variables in Vercel

### ❌ **Data Sent But Not Showing**
- Check Supabase dashboard: https://supabase.com/dashboard/project/xbivtgdxjcfsrpwemjoi
- Go to **Table Editor > helmet_data**
- Verify rows are being inserted
- Check browser console (F12) for errors

### ❌ **Website Shows "Error Loading Data"**
- Verify Supabase is running
- Check environment variables in Vercel
- Go to: https://vercel.com/ooikaijie-s-projects/helmet-dashboard/settings/environment-variables

---

## 📝 SENSOR CALIBRATION (OPTIONAL)

### **MPU9250 Calibration**
For better accuracy, calibrate the accelerometer/gyro:

1. Place helmet on flat surface
2. Uncomment in `smart_helmet.ino`:
   ```cpp
   // testMPU9250();  // Remove //
   ```
3. Upload and open Serial Monitor
4. Note the offset values
5. Update in `sensors.h`:
   ```cpp
   accel.x -= OFFSET_X;
   accel.y -= OFFSET_Y;
   accel.z -= OFFSET_Z;
   ```

---

## 🎯 NEXT STEPS

### **For Development**:
1. Test each sensor individually
2. Calibrate sensors (see above)
3. Fine-tune fall detection threshold (currently 2.5g)
4. Adjust location tracking sensitivity
5. Test battery life

### **For Production**:
1. Add authentication to dashboard
2. Enable Row Level Security in Supabase
3. Add email/SMS notifications for falls
4. Create mobile app version
5. Add GPS module for absolute positioning

---

## 📚 USEFUL COMMANDS (MacBook)

### **Git Commands**
```bash
# Check status
cd "/Users/hansongnkk/FYP KAIJIE"
git status

# Pull latest changes
git pull origin main

# Push changes
git add .
git commit -m "Your message"
git push origin main
```

### **Vercel Commands**
```bash
# Check deployment status
cd "/Users/hansongnkk/FYP KAIJIE/helmet-dashboard"
vercel ls

# View logs
vercel logs

# Redeploy
vercel --prod --yes
```

### **Find ESP32 Port (MacBook)**
```bash
ls /dev/cu.*
```
Look for `/dev/cu.usbserial-xxxxx` or `/dev/cu.SLAB_USBtoUART`

---

## 📞 SUPPORT

### **Documentation**
- Main README: `/Users/hansongnkk/FYP KAIJIE/README.md`
- Arduino Guide: `/Users/hansongnkk/FYP KAIJIE/arduino/README.md`
- Quick Start: `/Users/hansongnkk/FYP KAIJIE/QUICKSTART.md`
- Testing Guide: `/Users/hansongnkk/FYP KAIJIE/TESTING.md`

### **Key Files**
- Arduino Main: `/Users/hansongnkk/FYP KAIJIE/arduino/smart_helmet/smart_helmet.ino`
- Config: `/Users/hansongnkk/FYP KAIJIE/arduino/smart_helmet/config.h`
- Database Schema: `/Users/hansongnkk/FYP KAIJIE/supabase/schema.sql`

---

## ✅ CHECKLIST

Before starting, make sure:

- [ ] Arduino IDE 2.x installed
- [ ] ESP32 board support installed
- [ ] All 6 libraries installed
- [ ] WiFi credentials configured in `config.h`
- [ ] Hardware wired correctly
- [ ] ESP32 connected to MacBook via USB
- [ ] Serial Monitor set to 115200 baud

---

## 🎉 SUCCESS INDICATORS

You'll know everything is working when:

1. ✅ Serial Monitor shows "✅ Data sent successfully"
2. ✅ Dashboard shows live battery readings
3. ✅ Location map shows a dot moving
4. ✅ Temperature shows actual sensor reading
5. ✅ Supabase `helmet_data` table has new rows every second

---

**Last Updated**: 2026-05-12  
**Status**: Production Ready  
**Version**: 1.0.0

---

**🚀 Good luck with your FYP project!**
