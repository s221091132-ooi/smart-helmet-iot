# 🔧 LM35 Temperature Sensor Fix - 0.0°C Problem

## Masalah
Temperature sensor menunjukkan 0.0°C walaupun wiring sudah betul.

## Punca Masalah
**GPIO 4 conflict dengan I2C!** 

ESP32 guna `Wire.begin()` untuk MPU9250, yang set I2C pins. GPIO 4 pada sesetengah ESP32 board ada conflict dengan I2C atau boot mode, sebab tu reading jadi 0.0°C.

## Penyelesaian ✅
**Tukar LM35 dari GPIO 4 ke GPIO 32**

GPIO 32 adalah ADC1_CH4 - pin analog yang stabil dan tiada conflict.

---

## 📋 Langkah-Langkah

### 1️⃣ Update Arduino Code
```bash
# Di Windows laptop:
cd C:\Users\jack_\Desktop\smart-helmet-iot
git pull origin main
```

Atau di MacBook:
```bash
cd ~/Desktop/smart-helmet-iot  # atau mana folder awak simpan
git pull origin main
```

### 2️⃣ Tukar Wiring LM35

**SEBELUM (SALAH):**
```
LM35 V_out  →  GPIO 4  ❌
LM35 +Vs    →  3.3V
LM35 GND    →  GND
```

**SEKARANG (BETUL):**
```
LM35 V_out  →  GPIO 32  ✅
LM35 +Vs    →  3.3V
LM35 GND    →  GND
```

### 3️⃣ Upload ke ESP32
1. Buka Arduino IDE
2. Buka file: `smart-helmet-iot/arduino/smart_helmet/smart_helmet.ino`
3. Click **Upload** button
4. Tunggu upload selesai

### 4️⃣ Test
1. Buka Serial Monitor (115200 baud)
2. Perhatikan mesej: "LM35 temperature sensor configured on GPIO 32"
3. Tunggu 5 saat untuk lihat sensor readings
4. Temperature sepatutnya tunjuk nilai sebenar (contoh: 25.3°C, 26.1°C)
5. Cuba sentuh LM35 dengan jari - temperature patut naik
6. Check website - temperature juga patut berubah

---

## 🧪 Optional: Debug Test

Jika masih ada masalah, cuba run test code ni:

1. Buka Arduino IDE
2. File → Open → `smart-helmet-iot/arduino/test_lm35_debug/test_lm35_debug.ino`
3. Upload ke ESP32
4. Buka Serial Monitor (115200 baud)
5. Tengok output:
   - **Raw ADC: 0** = Wiring salah! Check connection
   - **Raw ADC: 800-900** = OK! (sekitar 23-27°C)
   - **Raw ADC: < 100** = Connection lemah, check jumper wires

---

## 📊 Pin Summary (Updated)

| Sensor/Component | ESP32 Pin | Notes |
|-----------------|-----------|-------|
| **LM35 Temperature** | **GPIO 32** | ✅ **UPDATED** (was GPIO 4) |
| ACS712 Current | GPIO 35 | No change |
| Battery Voltage | GPIO 34 | No change |
| Buzzer | GPIO 25 | No change |
| LED | GPIO 26 | No change |
| Reset Button | GPIO 27 | No change |
| MPU9250 SDA | GPIO 21 | I2C (default) |
| MPU9250 SCL | GPIO 22 | I2C (default) |

---

## ❓ Troubleshooting

### Masih 0.0°C selepas tukar pin?
1. **Check wiring betul-betul**:
   - LM35 ada 3 kaki
   - Flat side menghadap awak:
     ```
     +Vs (left)   →  3.3V
     V_out (middle)  →  GPIO 32  
     GND (right)  →  GND
     ```

2. **Check jumper wires**:
   - Guna multimeter test connectivity
   - Cuba ganti jumper wires yang lain

3. **Check LM35 sensor**:
   - Guna standalone test code (test_lm35_debug.ino)
   - Jika Raw ADC = 0, sensor rosak atau wiring salah

4. **Check ESP32 board**:
   - Cuba pin lain: GPIO 33, GPIO 36, atau GPIO 39
   - Update `#define LM35_PIN 33` dalam sensors.h

### Nilai temperature terlalu tinggi/rendah?
- Check formula dalam readTemperature()
- LM35: 10mV per °C = 0.01V per °C
- Formula: `temperature = voltage * 100.0`

---

## 📝 Changes Made

**File modified:**
- `arduino/smart_helmet/sensors.h`
  - Line 12: Changed `#define LM35_PIN 4` to `#define LM35_PIN 32`
  - Line 97: Updated Serial message

**Files added:**
- `arduino/test_lm35_debug/test_lm35_debug.ino` - Debug test program

**GitHub:** ✅ Pushed
**Website:** No changes needed (website code OK)

---

## ⚡ Why GPIO 32 is Better

| Pin | ADC Channel | Safe for Analog? | Conflicts? |
|-----|-------------|------------------|------------|
| GPIO 4 | ADC2_CH0 | ❌ | I2C, Boot, WiFi conflict |
| GPIO 32 | ADC1_CH4 | ✅ | None! |
| GPIO 33 | ADC1_CH5 | ✅ | None! |
| GPIO 34 | ADC1_CH6 | ✅ | None! (Input only) |
| GPIO 35 | ADC1_CH7 | ✅ | None! (Input only) |

**GPIO 32 dan 33** adalah pilihan terbaik untuk sensor analog!

---

**Selepas tukar wiring dan upload, temperature sepatutnya OK! 👍**
