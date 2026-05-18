/*
 * MPU6050 / I2C DEBUG — upload this sketch alone to test wiring
 *
 * Wiring (ESP32):
 *   VCC -> 3.3V
 *   GND -> GND
 *   SDA -> GPIO 21
 *   SCL -> GPIO 22
 *
 * Open Serial Monitor @ 115200 baud
 * Do NOT upload smart_helmet.ino until this test passes.
 */

#include <Wire.h>

#define SDA_PIN 21
#define SCL_PIN 22

#define MPU6050_ADDR_A 0x68
#define MPU6050_ADDR_B 0x69
#define MPU6050_WHO_AM_I 0x75
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B

uint8_t mpuAddr = 0;

bool writeByte(uint8_t addr, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

bool readBytes(uint8_t addr, uint8_t reg, uint8_t* buf, uint8_t len) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(addr, len) != len) return false;
  for (uint8_t i = 0; i < len; i++) {
    if (!Wire.available()) return false;
    buf[i] = Wire.read();
  }
  return true;
}

int16_t read16(uint8_t addr, uint8_t reg) {
  uint8_t b[2];
  if (!readBytes(addr, reg, b, 2)) return 0;
  return (int16_t)((b[0] << 8) | b[1]);
}

void scanI2C() {
  Serial.println("\n========== I2C SCAN (SDA=21, SCL=22) ==========");
  int count = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  Found device at 0x%02X", addr);
      if (addr == 0x68 || addr == 0x69) Serial.print("  <-- MPU6050/9250?");
      if (addr == 0x0C) Serial.print("  <-- AK8963 mag (MPU9250)?");
      Serial.println();
      count++;
    }
  }
  if (count == 0) {
    Serial.println("  *** NO DEVICES FOUND ***");
    Serial.println("  Check: 3.3V (not 5V), GND, SDA->21, SCL->22, loose wires");
  } else {
    Serial.printf("  Total: %d device(s)\n", count);
  }
  Serial.println("================================================\n");
}

uint8_t readWhoAmI(uint8_t addr) {
  uint8_t who = 0xFF;
  if (!readBytes(addr, MPU6050_WHO_AM_I, &who, 1)) return 0xFF;
  return who;
}

bool tryInitMPU6050(uint8_t addr) {
  Serial.printf("Trying MPU6050 at 0x%02X ...\n", addr);

  if (!writeByte(addr, MPU6050_PWR_MGMT_1, 0x00)) {
    Serial.println("  FAIL: cannot write PWR_MGMT (no ACK?)");
    return false;
  }
  delay(100);

  uint8_t who = readWhoAmI(addr);
  Serial.printf("  WHO_AM_I = 0x%02X (expect 0x68 for MPU6050)\n", who);

  if (who == 0xFF) {
    Serial.println("  FAIL: cannot read WHO_AM_I");
    return false;
  }
  if (who != 0x68 && who != 0x71) {
    Serial.printf("  WARN: unexpected ID (chip may be MPU9250=0x71 or fake)\n");
  }

  mpuAddr = addr;
  Serial.println("  OK — MPU responding!\n");
  return true;
}

void printRawSensors() {
  int16_t ax = read16(mpuAddr, MPU6050_ACCEL_XOUT_H);
  int16_t ay = read16(mpuAddr, MPU6050_ACCEL_XOUT_H + 2);
  int16_t az = read16(mpuAddr, MPU6050_ACCEL_XOUT_H + 4);
  int16_t gx = read16(mpuAddr, 0x43);
  int16_t gy = read16(mpuAddr, 0x45);
  int16_t gz = read16(mpuAddr, 0x47);

  float axG = ax / 16384.0f;
  float ayG = ay / 16384.0f;
  float azG = az / 16384.0f;
  float magMs = sqrt(axG * axG + ayG * ayG + azG * azG) * 9.81f;

  Serial.println("--- Live readings (move/tilt sensor) ---");
  Serial.printf("Accel raw: X=%6d  Y=%6d  Z=%6d\n", ax, ay, az);
  Serial.printf("Accel (g): X=%.2f  Y=%.2f  Z=%.2f  |mag|=%.2f m/s2 (~9.8 at rest)\n",
                axG, ayG, azG, magMs);
  Serial.printf("Gyro raw:  X=%6d  Y=%6d  Z=%6d  (rotate Z = yaw)\n", gx, gy, gz);
  Serial.printf("Gyro dps:  X=%.1f  Y=%.1f  Z=%.1f\n", gx / 131.0f, gy / 131.0f, gz / 131.0f);

  if (abs(ax) < 100 && abs(ay) < 100 && abs(az) < 100) {
    Serial.println("WARN: accel near zero — bad read or wrong chip?");
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n########################################");
  Serial.println("#  MPU6050 / I2C DEBUG SKETCH");
  Serial.println("#  Baud: 115200");
  Serial.println("########################################\n");

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);  // slow 100kHz for unreliable wiring
  Serial.println("I2C started: SDA=GPIO21, SCL=GPIO22, clock=100kHz\n");

  scanI2C();

  bool ok = tryInitMPU6050(MPU6050_ADDR_A);
  if (!ok) ok = tryInitMPU6050(MPU6050_ADDR_B);

  if (!ok) {
    Serial.println("========================================");
    Serial.println("MPU6050 NOT FOUND — fix wiring first:");
    Serial.println("  1) VCC -> 3.3V ONLY");
    Serial.println("  2) GND -> GND");
    Serial.println("  3) SDA -> GPIO 21");
    Serial.println("  4) SCL -> GPIO 22");
    Serial.println("  5) Optional: 4.7k pull-up SDA & SCL to 3.3V");
    Serial.println("  6) If AD0 pin tied to 3.3V, address is 0x69");
    Serial.println("========================================\n");
    Serial.println("Will keep scanning I2C every 5 sec...\n");
  } else {
    Serial.printf("SUCCESS! Using address 0x%02X\n", mpuAddr);
    Serial.println("Shake/rotate module — values must change below.\n");
  }
}

void loop() {
  if (mpuAddr == 0) {
  static unsigned long lastScan = 0;
    if (millis() - lastScan > 5000) {
      lastScan = millis();
      scanI2C();
    }
    delay(500);
    return;
  }

  printRawSensors();
  delay(500);
}
