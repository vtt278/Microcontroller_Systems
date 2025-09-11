//----------------------------------------------------------------
// STM32F401CCU6 Blackpill Environmental Monitoring System
//
// Vincent Tjoa
// September 11, 2025

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <U8x8lib.h>
#include <Adafruit_SHTC3.h>

// OLED Setup
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE, PB6, PB7);

// Soil Moisture Sensor Pins
const int moisture_pin = PB9;
const int moisture_pin2 = PB8;
int moisture_raw = 0, moisture_raw2 = 0;
int moisture_percent = 0, moisture_percent2 = 0;

// SHTC3 Setup
Adafruit_SHTC3 shtc3 = Adafruit_SHTC3();

void setup() {
  // USB Serial
  Serial.begin(115200);

  // Serial2 for ESP-01 (A2 = TX, A3 = RX)
  Serial2.begin(115200);

  // Start I2C bus BEFORE initializing OLED or SHTC3
  Wire.begin(PB7, PB6);   // (SDA, SCL)

  // SHTC3 init
  while (!Serial)
    delay(10);     // wait for serial console
  Serial.println("SHTC3 test");
  if (! shtc3.begin()) {
    Serial.println("Couldn't find SHTC3");
    while (1) delay(1);
  }
  Serial.println("Found SHTC3 sensor");

  pinMode(moisture_pin, INPUT);
  pinMode(moisture_pin2, INPUT);

  // OLED init
  display.begin();
  display.clearBuffer();
  display.setContrast(200);  // Try values 0–255
  display.setFont(u8g2_font_ncenB08_tr);
  display.drawStr(5, 30, "Starting...");
  display.sendBuffer();

  delay(1000);
  display.clearBuffer();
}

void loop() {
  sensors_event_t humidity, temp;
  shtc3.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data

  // Soil moisture readings
  moisture_raw = analogRead(moisture_pin);
  moisture_percent = ((4095 - moisture_raw) / (4095.0 - 1300.0) * 100);
  moisture_percent = constrain(moisture_percent, 0, 100);

  moisture_raw2 = analogRead(moisture_pin2);
  moisture_percent2 = ((4095 - moisture_raw2) / (4095.0 - 1300.0)) * 100;
  moisture_percent2 = constrain(moisture_percent2, 0, 100);

  // Serial Monitor
  Serial.print("SHTC3 Temp: ");
  Serial.print(temp.temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" %");

  Serial.print("Soil 1: ");
  Serial.print(moisture_percent);
  Serial.print("% (Raw: ");
  Serial.print(moisture_raw);
  Serial.print("), Soil 2: ");
  Serial.print(moisture_percent2);
  Serial.print("% (Raw: ");
  Serial.print(moisture_raw2);
  Serial.println(")");

  // Send to ESP-01 via Serial2
  Serial2.print("TEMP:");
  Serial2.print(temp.temperature, 1);
  Serial2.print(",HUM:");
  Serial2.print(humidity.relative_humidity, 1);
  Serial2.print(",SOIL1:");
  Serial2.print(moisture_percent);
  Serial2.print(",SOIL2:");
  Serial2.println(moisture_percent2);

  // OLED Display
  display.clearBuffer();
  display.setFont(u8g2_font_ncenB08_tr);
  display.drawStr(0, 12, ("SHTC3 Temp : " + String(temp.temperature, 1) + " C").c_str());
  display.drawStr(0, 24, ("Humidity : " + String(humidity.relative_humidity, 1) + " %").c_str());
  display.drawStr(0, 36, ("Soil Moisture 1: " + String(moisture_percent) + " %").c_str());
  display.drawStr(0, 48, ("Soil Moisture 2: " + String(moisture_percent2) + " %").c_str());
  display.sendBuffer();

  delay(1000);
}
