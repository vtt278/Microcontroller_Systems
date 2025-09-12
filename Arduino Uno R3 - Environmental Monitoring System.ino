// Arduino Uno R3 Environmental Monitoring System
//
// Vincent Tjoa
// September 11, 2025

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED setup (128x64, default I2C address 0x3C)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Soil moisture pins
#define MOISTURE_PIN1 A0
#define MOISTURE_PIN2 A1

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 15000; // 15s

// SHTC3 constants
#define SHTC3_ADDR 0x70

void shtc3_sendCmd(uint16_t cmd) {
  Wire.beginTransmission(SHTC3_ADDR);
  Wire.write(cmd >> 8);
  Wire.write(cmd & 0xFF);
  Wire.endTransmission();
}

bool shtc3_read(float &temperature, float &humidity) {
  // Wake up
  shtc3_sendCmd(0x3517);
  delay(1);

  // Trigger measurement (T first, normal mode)
  shtc3_sendCmd(0x7866);
  delay(15);

  Wire.requestFrom(SHTC3_ADDR, 6);
  if (Wire.available() < 6) return false;

  uint16_t t_raw = (Wire.read() << 8) | Wire.read();
  Wire.read(); // skip CRC
  uint16_t rh_raw = (Wire.read() << 8) | Wire.read();
  Wire.read(); // skip CRC

  shtc3_sendCmd(0xB098); // sleep

  temperature = -45.0 + 175.0 * (float)t_raw / 65535.0;
  humidity    = 100.0 * (float)rh_raw / 65535.0;

  return true;
}

void setup() {
  Serial.begin(9600); // match ESP01 baud rate
  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("❌ SSD1306 not found!"));
    for (;;); // Halt
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 25);
  display.println(F("Enviro Monitor"));
  display.display();
  delay(1500);

  Serial.println(F("✅ Setup complete, starting measurements..."));
}

void loop() {
  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();

    // Read sensors
    float temp = 0, hum = 0;
    bool ok = shtc3_read(temp, hum);

    int raw1 = analogRead(MOISTURE_PIN1);
    int raw2 = analogRead(MOISTURE_PIN2);

    int soil1 = map(raw1, 1023, 300, 0, 100);
    int soil2 = map(raw2, 1023, 300, 0, 100);
    soil1 = constrain(soil1, 0, 100);
    soil2 = constrain(soil2, 0, 100);

    // OLED Display
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print("Temp: ");
    display.print(temp, 1);
    display.print(" C");

    display.setCursor(0, 16);
    display.print("Humidity: ");
    display.print(hum, 1);
    display.print(" %");

    display.setCursor(0, 32);
    display.print("Soil 1: ");
    display.print(soil1);
    display.print(" %");

    display.setCursor(0, 48);
    display.print("Soil 2: ");
    display.print(soil2);
    display.print(" %");

    display.display();

    // Debug output
    if (ok) {
      Serial.print("Temp (C): "); Serial.println(temp, 1);
      Serial.print("Humidity (%): "); Serial.println(hum, 1);
    } else {
      Serial.println("❌ Failed to read SHTC3");
    }
    Serial.print("Soil1 raw: "); Serial.print(raw1);
    Serial.print(" -> "); Serial.print(soil1); Serial.println(" %");
    Serial.print("Soil2 raw: "); Serial.print(raw2);
    Serial.print(" -> "); Serial.print(soil2); Serial.println(" %");

    // CSV output for ESP01
    Serial.print(temp, 1); Serial.print(",");
    Serial.print(hum, 1);  Serial.print(",");
    Serial.print(soil1);   Serial.print(",");
    Serial.println(soil2);

    Serial.println();
  }
}
