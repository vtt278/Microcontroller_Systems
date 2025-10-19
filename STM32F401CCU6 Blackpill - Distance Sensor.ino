//----------------------------------------------------------------
// STM32F401CCU6 Blackpill Distance Sensor
//
// Vincent Tjoa
// September 11, 2025

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <U8x8lib.h>

// OLED: hardware I2C on PB6 (SCL), PB7 (SDA)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE, PB6, PB7);

// HC-SR04 pins
const uint8_t TRIG_PIN = A0; // PA0
const uint8_t ECHO_PIN = A1; // PA1

// Buzzer
const uint8_t BUZZER_PIN = A5; // PA5 (S8050 base via resistor)

// Timing
const unsigned long INTERVAL_MS = 1000; //1s delay
unsigned long lastMillis = 0;

void setup() {
  Serial.begin(115200);      // debug via ST-Link/USB if available
  Serial1.begin(115200);     // comms to ESP01 (ensure ESP01 serial uses same baud)

  // initialize I2C (explicit)
  Wire.begin(PB7, PB6);

  // init OLED
  display.begin();
  display.clearBuffer();
  display.setFont(u8g2_font_6x12_tr);
  display.drawStr(5, 30, "Starting...");
  display.sendBuffer();
  delay(800);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

float measureDistanceCm() {
  // HC-SR04: trigger 10us pulse, measure echo
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // pulseIn timeout in microseconds (e.g., 30000 us = ~5 m)
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) return 0.0; // no echo / out of range

  float distance_cm = (duration / 58.0); // typical conversion
  return distance_cm;
}

void showOnOLED(float distance, const char *doorState) {
  display.clearBuffer();
  display.setFont(u8g2_font_6x12_tr);
  display.drawStr(0, 10, "Door State:");

  display.setFont(u8g2_font_logisoso24_tr); // large font for OPEN/CLOSED
  display.drawStr(0, 42, doorState);

  display.setFont(u8g2_font_6x12_tr);
  char buf[32];
  snprintf(buf, sizeof(buf), "Distance: %.2f cm", distance);
  display.drawStr(0, 62, buf);

  display.sendBuffer();
}

void sendToESP01(float distance) {
  // example format: D:33.73\n
  char out[32];
  int n = snprintf(out, sizeof(out), "D:%.2f\n", distance);
  Serial1.write((const uint8_t*)out, n);
}

void loop() {
  unsigned long now = millis();
  if (now - lastMillis < INTERVAL_MS) return;
  lastMillis = now;

  float distance = measureDistanceCm();
  if (distance == 0.0) distance = 400.0; // treat as out-of-range

  // door logic: <120 => OPEN
  const char *doorState = (distance < 120.0) ? "OPEN" : "CLOSED";
  // buzzer logic: HIGH when >120
  digitalWrite(BUZZER_PIN, (distance > 120.0) ? HIGH : LOW);

  // display
  showOnOLED(distance, doorState);
  Serial.printf("Distance: %.2f cm, Door: %s\n", distance, doorState);
  sendToESP01(distance);
}
