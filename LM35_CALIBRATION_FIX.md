# 🌡️ LM35 Temperature Calibration Fix

## Masalah
ESP32 menunjukkan **18°C** tapi sebenarnya **24°C** (6°C terlalu rendah)

## Penyelesaian
Tambah **calibration offset +6°C** dalam kod.

---

## 🔧 Apa yang Saya Dah Buat

### 1. Tambah Calibration Constant
```cpp
#define LM35_CALIBRATION_OFFSET 6.0  // +6°C offset
```

### 2. Update `readTemperature()` Function
```cpp
// SEBELUM:
float temperatureC = tempVoltage * 100.0;

// SEKARANG:
float temperatureC = (tempVoltage * 100.0) + LM35_CALIBRATION_OFFSET;
```

### 3. Tambah Averaging (10 samples)
Untuk reading yang lebih stabil dan accurate.

---

## 📋 Cara Update

### Windows Laptop:
```cmd
cd C:\Users\jack_\Desktop\smart-helmet-iot
git pull origin main
```

### MacBook:
```bash
cd ~/Desktop/smart-helmet-iot
git pull origin main
```

### Upload ke ESP32:
1. Buka Arduino IDE
2. Open: `smart-helmet-iot/arduino/smart_helmet/smart_helmet.ino`
3. Click **Upload**
4. Tunggu selesai

---

## ✅ Test

1. **Buka Serial Monitor** (115200 baud)
2. **Tunggu 5 saat** untuk sensor readings
3. **Check temperature** - sepatutnya nampak nilai yang betul sekarang (~24°C)
4. **Test dengan sentuh LM35** - temperature patut naik 2-3°C
5. **Check website** - temperature patut sama dengan Serial Monitor

---

## 🎯 Kenapa Perlu Calibration?

LM35 adalah sensor analog yang bergantung kepada:
1. **ESP32 ADC accuracy** (±3-5% error)
2. **Reference voltage** (3.3V mungkin sebenarnya 3.25V atau 3.35V)
3. **LM35 tolerance** (±0.5°C)
4. **Wiring resistance** (jumper wires ada resistance sikit)

Semua ini boleh cause offset. Calibration offset adalah **normal** dan **best practice**!

---

## 🔍 Jika Masih Tidak Tepat

### Kalau temperature masih salah (contoh: 22°C instead of 24°C):

1. **Guna calibration test code** saya dah buat:
   ```
   arduino/test_lm35_calibration/test_lm35_calibration.ino
   ```

2. **Upload dan buka Serial Monitor** - dia akan tunjuk berbagai offset:
   ```
   Basic (no offset):  18.0 °C
   +3°C offset:        21.0 °C
   +6°C offset:        24.0 °C  ← YOU ARE HERE
   +9°C offset:        27.0 °C
   ```

3. **Adjust `LM35_CALIBRATION_OFFSET`** dalam `sensors.h`:
   - Jika terlalu tinggi 2°C: change dari `6.0` ke `4.0`
   - Jika terlalu rendah 2°C: change dari `6.0` ke `8.0`

4. **Upload semula dan test**

---

## 📊 Expected Behavior

| Actual Temp | ESP32 Reading (Before) | ESP32 Reading (After) |
|-------------|------------------------|------------------------|
| 23°C | 17°C ❌ | 23°C ✅ |
| 24°C | 18°C ❌ | 24°C ✅ |
| 25°C | 19°C ❌ | 25°C ✅ |
| 30°C | 24°C ❌ | 30°C ✅ |

---

## 🔧 Manual Adjustment (If Needed)

Edit file: `arduino/smart_helmet/sensors.h`

Find line:
```cpp
#define LM35_CALIBRATION_OFFSET 6.0
```

Change value based on your testing:
- **Too high by 2°C?** → Change to `4.0`
- **Too low by 2°C?** → Change to `8.0`
- **Perfect!** → Leave as `6.0`

Then upload to ESP32 again.

---

## 📝 Changes Summary

**Files Modified:**
- `arduino/smart_helmet/sensors.h`
  - Added: `#define LM35_CALIBRATION_OFFSET 6.0`
  - Updated: `readTemperature()` function
  - Added: 10-sample averaging for stability

**Files Added:**
- `arduino/test_lm35_calibration/test_lm35_calibration.ino`
  - Calibration test tool
  - Shows multiple offset options

**Status:**
- ✅ Pushed to GitHub
- ✅ Ready to test

---

## 💡 Pro Tips

1. **Wait 2-3 minutes** after power on - LM35 needs warm-up time
2. **Don't touch LM35 metal body** - body heat will affect reading
3. **Keep LM35 away from ESP32** - ESP32 generates heat
4. **Room temp fluctuates** - 23-25°C is normal variation
5. **Website updates every 1 second** - give it a few seconds

---

**Sekarang temperature sepatutnya accurate! 🎉**

Pull code terbaru, upload, dan test!
