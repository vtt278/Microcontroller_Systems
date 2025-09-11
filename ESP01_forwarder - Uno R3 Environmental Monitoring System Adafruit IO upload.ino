//----------------------------------------------------------------
// ESP01_forwarder for Arduino Uno R3 Environmental Monitoring System (Adafruit IO upload)
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

// Adafruit IO instance
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Feeds
AdafruitIO_Feed *temperature_feed = io.feed("SHTC3 Temperature");
AdafruitIO_Feed *humidity_feed    = io.feed("SHTC3 Humidity");
AdafruitIO_Feed *moisture1_feed   = io.feed("Resistive Soil Moisture Sensor 1");
AdafruitIO_Feed *moisture2_feed   = io.feed("Resistive Soil Moisture Sensor 2");

void setup() {
  Serial.begin(9600); // receive data from Uno

  io.connect();
  while(io.status() < AIO_CONNECTED) {
    delay(500);
  }
}

void loop() {
  io.run(); // keep connection alive

  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.length() > 0) {
      // Parse CSV: temp,humidity,moisture1,moisture2
      float temp = 0, hum = 0;
      int soil1 = 0, soil2 = 0;

      int firstComma  = line.indexOf(',');
      int secondComma = line.indexOf(',', firstComma + 1);
      int thirdComma  = line.indexOf(',', secondComma + 1);

      if (firstComma > 0 && secondComma > 0 && thirdComma > 0) {
        temp  = line.substring(0, firstComma).toFloat();
        hum   = line.substring(firstComma + 1, secondComma).toFloat();
        soil1 = line.substring(secondComma + 1, thirdComma).toInt();
        soil2 = line.substring(thirdComma + 1).toInt();

        // Publish to Adafruit IO
        temperature_feed->save(temp);
        humidity_feed->save(hum);
        moisture1_feed->save(soil1);
        moisture2_feed->save(soil2);
      }
    }
  }
}
