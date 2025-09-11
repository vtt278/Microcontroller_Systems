//----------------------------------------------------------------
// ESP01_forwarder for Arduino Uno R3 Distance Sensor (Adafruit IO upload)
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

#define RX 2 // connect to Uno TX
#define TX 3 // connect to Uno RX
#define SERIAL_BAUD 9600

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
AdafruitIO_Feed *distance_feed = io.feed("distance");

void setup() {
  Serial.begin(SERIAL_BAUD); // Serial to Uno
  io.connect();

  // Wait for connection
  while(io.status() < AIO_CONNECTED){
    delay(500);
  }
  Serial.println("Connected to Adafruit IO!");
}

void loop() {
  io.run(); // keep connection alive

  if(Serial.available()){
    String distance = Serial.readStringUntil('\n');
    distance.trim();
    if(distance.length() > 0){
      float d = distance.toFloat();
      distance_feed->save(d);
      Serial.print("Sent distance: ");
      Serial.println(d);
    }
  }
}
