// LM35 Temperature Calibration Test
// Use this to find the correct calibration factor for your LM35

const int LM35_PIN = 32;             // Pin connected to LM35 V_out
const float V_REF = 3.3;             // ESP32 reference voltage
const float ADC_RESOLUTION = 4095.0; // ESP32 12-bit ADC

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n\n=================================");
  Serial.println("LM35 CALIBRATION TEST");
  Serial.println("=================================\n");
  
  pinMode(LM35_PIN, INPUT);
  
  Serial.println("Instructions:");
  Serial.println("1. Use a thermometer to measure actual room temperature");
  Serial.println("2. Compare with ESP32 reading below");
  Serial.println("3. Calculate calibration offset\n");
  Serial.println("Reading LM35 every 2 seconds...\n");
}

void loop() {
  // Take 10 samples for better accuracy
  long totalRaw = 0;
  for (int i = 0; i < 10; i++) {
    totalRaw += analogRead(LM35_PIN);
    delay(10);
  }
  float avgRaw = totalRaw / 10.0;
  
  // Convert to voltage
  float voltage = (avgRaw / ADC_RESOLUTION) * V_REF;
  
  // LM35 formula: 10mV per °C = 0.01V per °C
  float tempBasic = voltage * 100.0;
  
  // Try different calibration offsets
  float tempOffset3 = tempBasic + 3.0;
  float tempOffset6 = tempBasic + 6.0;
  float tempOffset9 = tempBasic + 9.0;
  
  // Print all readings
  Serial.println("──────────────────────────────────");
  Serial.print("Raw ADC (avg): ");
  Serial.println(avgRaw, 2);
  Serial.print("Voltage: ");
  Serial.print(voltage, 3);
  Serial.println(" V");
  Serial.println();
  
  Serial.println("📊 Temperature Readings:");
  Serial.print("  Basic (no offset):  ");
  Serial.print(tempBasic, 1);
  Serial.println(" °C");
  
  Serial.print("  +3°C offset:        ");
  Serial.print(tempOffset3, 1);
  Serial.println(" °C");
  
  Serial.print("  +6°C offset:        ");
  Serial.print(tempOffset6, 1);
  Serial.println(" °C  ← YOU ARE HERE (18°C + 6 = 24°C)");
  
  Serial.print("  +9°C offset:        ");
  Serial.print(tempOffset9, 1);
  Serial.println(" °C");
  
  Serial.println();
  Serial.println("💡 RECOMMENDATION:");
  Serial.println("   If your actual room temp is 24°C,");
  Serial.println("   and ESP32 shows 18°C, then:");
  Serial.println("   USE +6°C OFFSET in your main code!");
  Serial.println("──────────────────────────────────\n");
  
  delay(2000);
}
