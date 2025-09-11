//----------------------------------------------------------------
// STM32F401CCU6 Blackpill Air Quality Monitor
//
// Vincent Tjoa
// September 11, 2025

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <U8x8lib.h>
#include <MQUnifiedsensor.h>

// SSD1306 Display (I2C on PB6=SCL, PB7=SDA)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE, PB6, PB7);

// MQ135 Setup
#define MQ_PIN A0
#define MQ_BOARD "STM32"
#define MQ_VRES 3.3
#define MQ_ADC_BITS 12
#define MQ_CLEAN_AIR_RATIO 3.6
MQUnifiedsensor MQ135(MQ_BOARD, MQ_VRES, MQ_ADC_BITS, MQ_PIN, "MQ-135");

// SDS011 (particles) on Serial1 (PA10=RX, PA9=TX on many STM32 boards)
HardwareSerial& SerialPM = Serial1;

// Link to ESP-01 (STM32<->ESP-01) on Serial2 (A2=TX, A3=RX)
HardwareSerial& Link = Serial2;

// Timing
unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 15000; // 15 s

// Live values
float pm25 = NAN, pm10 = NAN;
bool  hasPM = false;
float co_ppm = NAN, co2_ppm = NAN, alc_ppm = NAN;

// Minimal SDS011 frame parser
bool readSDS011(Stream& s, float& out_pm25, float& out_pm10) {
  while (s.available() >= 10) {
    if (s.peek() != 0xAA) { s.read(); continue; }
    uint8_t frame[10];
    s.readBytes(frame, 10);
    if (frame[1] != 0xC0 || frame[9] != 0xAB) continue;
    // checksum = sum(frame[2]..frame[7]) & 0xFF
    uint8_t sum = 0;
    for (int i = 2; i <= 7; i++) sum += frame[i];
    if (sum != frame[8]) continue;

    uint16_t pm25raw = (uint16_t)frame[2] | ((uint16_t)frame[3] << 8);
    uint16_t pm10raw = (uint16_t)frame[4] | ((uint16_t)frame[5] << 8);
    out_pm25 = pm25raw / 10.0f;
    out_pm10 = pm10raw / 10.0f;
    return true;
  }
  return false;
}

void drawSplash() {
  display.clearBuffer();
  display.setFont(u8g2_font_logisoso16_tr);
  display.drawStr(30, 18, "Air");
  display.drawStr(10, 40, "Quality");
  display.drawStr(8, 62, "Monitor");
  display.sendBuffer();
}

void drawValues() {
  display.clearBuffer();
  display.setFont(u8g2_font_6x13_tr);
  int y = 12;
  if (hasPM) {
    char line[32];
    snprintf(line, sizeof(line), "PM2.5: %.1f ug/m3", pm25);
    display.drawStr(0, y, line); y += 12;
    snprintf(line, sizeof(line), "PM10 : %.1f ug/m3", pm10);
    display.drawStr(0, y, line); y += 12;
  } else {
    display.drawStr(0, y, "PM  : waiting..."); y += 12;
  }
  {
    char line[32];
    snprintf(line, sizeof(line), "CO2 : %.1f ppm", co2_ppm);
    display.drawStr(0, y, line); y += 12;
    snprintf(line, sizeof(line), "CO  : %.1f ppm", co_ppm);
    display.drawStr(0, y, line); y += 12;
    snprintf(line, sizeof(line), "Alc : %.1f ppm", alc_ppm);
    display.drawStr(0, y, line);
  }
  display.sendBuffer();
}

void setup() {
  // USB serial for debug
  Serial.begin(115200);
  delay(50);
  Serial.println("\nSTM32 Air Quality Node booting...");

  // Start I2C BEFORE display begin
  Wire.begin(PB7, PB6);

  // Display splash
  display.begin();
  display.setContrast(200);
  drawSplash();
  delay(800);

  // MQ135 init
  MQ135.setRegressionMethod(1);
  MQ135.init();
  Serial.print("Calibrating MQ135 (R0)...");
  float r0_sum = 0;
  for (int i = 0; i < 10; i++) {
    MQ135.update();
    r0_sum += MQ135.calibrate(MQ_CLEAN_AIR_RATIO);
    Serial.print(".");
    delay(500);
  }
  float R0 = r0_sum / 10.0f;
  MQ135.setR0(R0);
  Serial.printf("\nMQ135 R0=%.3f\n", R0);
  if (isinf(R0) || R0 == 0) {
    Serial.println("ERROR: Invalid R0. Check sensor wiring/supply.");
    while (1) { delay(1000); }
  }

  // SDS011 @ 9600
  SerialPM.begin(9600);
  Serial.println("SDS011 serial started @9600");

  // Link to ESP-01
  Link.begin(115200);
  Serial.println("Link to ESP-01 @115200 ready");

  lastUpdate = millis();
}

void loop() {
  // Update SDS011 if a full frame arrives
  float p25, p10;
  if (readSDS011(SerialPM, p25, p10)) {
    pm25 = p25;
    pm10 = p10;
    hasPM = true;
    Serial.printf("SDS011 -> PM2.5=%.1f PM10=%.1f\n", pm25, pm10);
  }

  // MQ135 continuous update (needed by library)
  MQ135.update();

  // every 15 s, compute gases, refresh OLED, and push to ESP-01
  if (millis() - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = millis();

    // MQ135 curves (same as your ESP32 baseline)
    MQ135.setA(110.47); MQ135.setB(-2.862);  float co2_raw = MQ135.readSensor();   // "CO2-like" curve
    MQ135.setA(605.18); MQ135.setB(-3.937);  float co_raw  = MQ135.readSensor();
    MQ135.setA(77.255); MQ135.setB(-3.18);   float alc_raw = MQ135.readSensor();

    co2_ppm = co2_raw + 400.0f;  // add ~400ppm baseline
    co_ppm  = co_raw;
    alc_ppm = alc_raw;

    // Debug
    Serial.printf("MQ135 -> CO2: %.1f ppm | CO: %.1f ppm | Alcohol: %.1f ppm\n",
                  co2_ppm, co_ppm, alc_ppm);

    // OLED
    drawValues();

    // Send framed line to ESP-01
    float pm25_out = hasPM ? pm25 : -1.0f;
    float pm10_out = hasPM ? pm10 : -1.0f;

    // Format: PM25:xx.x,PM10:xx.x,CO2:xx.x,CO:xx.x,ALC:xx.x
    Link.print("PM25:");
    Link.print(pm25_out, 1);
    Link.print(",PM10:");
    Link.print(pm10_out, 1);
    Link.print(",CO2:");
    Link.print(co2_ppm, 1);
    Link.print(",CO:");
    Link.print(co_ppm, 1);
    Link.print(",ALC:");
    Link.println(alc_ppm, 1);
  }
}
