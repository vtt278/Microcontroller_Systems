//----------------------------------------------------------------
// ESP01_forwarder for STM32F401CCU6 Blackpill ESP01 Air Quality System (Adafruit IO upload)
// Upload to ESP01 (ESP8266 core).
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

// Feeds (names match your original ESP32 baseline)
AdafruitIO_Feed *co_feed     = io.feed("MQ135 CO");
AdafruitIO_Feed *co2_feed    = io.feed("MQ135 CO2");
AdafruitIO_Feed *alcohol_feed= io.feed("MQ135 Alcohol");
AdafruitIO_Feed *pm10_feed   = io.feed("SDS011 PM10");
AdafruitIO_Feed *pm25_feed   = io.feed("SDS011 PM2.5");

void setup() {
  Serial.begin(115200);   // RX=A2 (STM TX), TX=A3 (STM RX) on the STM32 side
  delay(50);

  io.connect();
  // attempt connection (non-blocking but we’ll wait a bit for UX)
  unsigned long t0 = millis();
  while (io.status() < AIO_CONNECTED && millis() - t0 < 15000) {
    io.run();
    delay(250);
    Serial.print(".");
  }
  if (io.status() >= AIO_CONNECTED) {
    Serial.println("\n✅ Connected to Adafruit IO");
  } else {
    Serial.println("\n⚠ Could not connect to Adafruit IO (will keep retrying in loop)");
  }
}

void loop() {
  io.run();

  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) return;

    float pm25, pm10, co2, co, alc;

    // Expect: PM25:%f,PM10:%f,CO2:%f,CO:%f,ALC:%f
    int parsed = sscanf(line.c_str(), "PM25:%f,PM10:%f,CO2:%f,CO:%f,ALC:%f",
                        &pm25, &pm10, &co2, &co, &alc);

    if (parsed == 5) {
      Serial.printf("Parsed -> PM2.5=%.1f PM10=%.1f CO2=%.1f CO=%.1f ALC=%.1f\n",
                    pm25, pm10, co2, co, alc);

      // Publish gases
      co2_feed->save(co2);
      co_feed->save(co);
      alcohol_feed->save(alc);

      // Publish PM only if valid (STM32 sends -1.0 when not ready)
      if (pm25 >= 0.0f) pm25_feed->save(pm25);
      if (pm10 >= 0.0f) pm10_feed->save(pm10);
    } else {
      Serial.print("Ignoring line (bad format): ");
      Serial.println(line);
    }
  }

  // lightweight keepalive if Wi-Fi drops
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  }
  if (io.status() < AIO_CONNECTED) {
    io.connect(); // non-blocking, io.run() handles the rest
  }
}
