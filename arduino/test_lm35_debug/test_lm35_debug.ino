// LM35 Temperature Sensor Debug Test
// This will help us diagnose why you're getting 0.0°C

const int LM35_PIN = 4;              // Pin connected to LM35 V_out
const float V_REF = 3.3;             // ESP32 reference voltage
const float ADC_RESOLUTION = 4095.0; // ESP32 12-bit ADC

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n\n=================================");
  Serial.println("LM35 DEBUG TEST");
  Serial.println("=================================\n");
  
  // Configure pin as input
  pinMode(LM35_PIN, INPUT);
  
  Serial.println("Reading LM35 sensor every 1 second...");
  Serial.println("Format: Raw ADC | Voltage | Temperature\n");
}

void loop() {
  // Read raw ADC value (0 to 4095)
  int rawTemp = analogRead(LM35_PIN);
  
  // Convert to voltage
  float voltage = (rawTemp / ADC_RESOLUTION) * V_REF;
  
  // Convert to Celsius (LM35: 10mV per °C = 0.01V per °C)
  float temperatureC = voltage * 100.0;
  
  // Print detailed info
  Serial.print("Raw ADC: ");
  Serial.print(rawTemp);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 3);
  Serial.print(" V | Temperature: ");
  Serial.print(temperatureC, 1);
  Serial.println(" °C");
  
  // Additional debug info
  if (rawTemp == 0) {
    Serial.println("  ⚠️  WARNING: Raw ADC is 0! Check wiring:");
    Serial.println("     - LM35 V_out to GPIO 4");
    Serial.println("     - LM35 +Vs to 3.3V");
    Serial.println("     - LM35 GND to GND");
  } else if (rawTemp < 100) {
    Serial.println("  ⚠️  WARNING: Raw ADC very low! Possible poor connection.");
  } else if (temperatureC < 0 || temperatureC > 100) {
    Serial.println("  ⚠️  WARNING: Temperature out of normal range!");
  } else {
    Serial.println("  ✅ Reading looks good!");
  }
  
  Serial.println();
  delay(1000);
}
