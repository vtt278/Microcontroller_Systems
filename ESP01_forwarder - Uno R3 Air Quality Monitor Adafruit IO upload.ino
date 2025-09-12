//----------------------------------------------------------------
// ESP01_forwarder for Arduino Uno R3 Air Quality Monitor (Adafruit IO upload)
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

// Feeds
AdafruitIO_Feed *co_feed = io.feed("MQ135 CO");
AdafruitIO_Feed *co2_feed = io.feed("MQ135 CO2");
AdafruitIO_Feed *alcohol_feed = io.feed("MQ135 Alcohol");
AdafruitIO_Feed *pm10_feed = io.feed("SDS011 PM10");
AdafruitIO_Feed *pm25_feed = io.feed("SDS011 PM2.5");

void setup() {
  Serial.begin(9600); // receiving from Uno
  io.connect();
  while (io.status() < AIO_CONNECTED) {
    io.run();
    delay(500);
  }
}

void loop() {
  io.run();

  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    // Expect "PM25,PM10,CO2,CO,Alcohol"
    float pm25, pm10, co2, co, alcohol;
    int count = sscanf(line.c_str(), "%f,%f,%f,%f,%f", &pm25, &pm10, &co2, &co, &alcohol);

    if (count == 5) {
      pm25_feed->save(pm25);
      pm10_feed->save(pm10);
      co2_feed->save(co2);
      co_feed->save(co);
      alcohol_feed->save(alcohol);
    }
  }
}
