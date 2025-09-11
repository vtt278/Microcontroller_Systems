//----------------------------------------------------------------
// ESP01_forwarder for STM32F401CCU6 Blackpill Environmental Monitoring System (Adafruit IO upload)
// Upload to ESP01 (ESP8266 core).
// Connect ESP01 TX/RX to STM32 Serial1 (A3/A2) crossed: ESP01 RX <- STM32 TX
//
// Vincent Tjoa
// September 11, 2025

#include <ESP8266WiFi.h>
#include <AdafruitIO_WiFi.h>

// WiFi / Adafruit IO credentials
#define WIFI_SSID "xxxxx" //change this to WiFi SSID
#define WIFI_PASS "xxxxx" //change this to WiFi password

#define IO_USERNAME "xxxxx" //change this to Adafruit IO username
#define IO_KEY "xxxxx" //change this to Adafruit IO key

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Feeds
AdafruitIO_Feed *temperature_feed = io.feed("SHTC3 Temperature");
AdafruitIO_Feed *humidity_feed    = io.feed("SHTC3 Humidity");
AdafruitIO_Feed *moisture1_feed   = io.feed("Resistive Soil Moisture Sensor 1");
AdafruitIO_Feed *moisture2_feed   = io.feed("Resistive Soil Moisture Sensor 2");

void setup() {
  Serial.begin(115200);   // RX=A2 (STM TX), TX=A3 (STM RX)
  io.connect();

  // Wait for connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    io.run();
    delay(500);
  }
  Serial.println("Connected to Adafruit IO!");
}

void loop() {
  io.run();

  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    float t, h;
    int m1, m2;

    if (sscanf(line.c_str(), "TEMP:%f,HUM:%f,SOIL1:%d,SOIL2:%d", &t, &h, &m1, &m2) == 4) {
      Serial.printf("Parsed -> T=%.1f H=%.1f M1=%d M2=%d\n", t, h, m1, m2);

      temperature_feed->save(t);
      humidity_feed->save(h);
      moisture1_feed->save(m1);
      moisture2_feed->save(m2);
    }
  }
}
