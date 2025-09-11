//----------------------------------------------------------------
// ESP01_forwarder for STM32F401CCU6 Blackpill Distance Sensor (Adafruit IO upload)
// Upload to ESP01 (ESP8266 core).
// Connect ESP01 TX/RX to STM32 Serial1 (A3/A2) crossed: ESP01 RX <- STM32 TX
//
// Vincent Tjoa
// September 11, 2025
// ESP01_forwarder for STM32 Blackpill Distance Sensor system
// Upload to ESP01 (ESP8266 core).
// Connect ESP01 TX/RX to STM32 Serial1 (A3/A2) crossed: ESP01 RX <- STM32 TX

#include <ESP8266WiFi.h>
#include <AdafruitIO_WiFi.h>

// WiFi / Adafruit IO credentials
#define WIFI_SSID "xxxxx" //change this to WiFi SSID
#define WIFI_PASS "xxxxx" //change this to WiFi password

#define IO_USERNAME "xxxxx" //change this to Adafruit IO username
#define IO_KEY "xxxxx" //change this to Adafruit IO key

// Feed name (must exist or will be created on Adafruit IO)
#define DIST_FEED_NAME "distance"

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
AdafruitIO_Feed *distanceFeed = nullptr;

String incomingLine = "";

void setup() {
  Serial.begin(115200); // Hardware UART0 on ESP01 (connects to STM32 RX/TX)

  // start Adafruit IO
  io.connect();

  // wait for connection
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    io.run();
    delay(500);
  }
  Serial.println("\nConnected to Adafruit IO!");
  distanceFeed = io.feed(DIST_FEED_NAME);
}

void loop() {
  io.run(); // keep Adafruit IO connection alive

  // read serial from STM32
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n') {
      incomingLine.trim();
      if (incomingLine.length() > 0) {
        // expected format: D:33.73
        if (incomingLine.startsWith("D:")) {
          String valStr = incomingLine.substring(2);
          float val = valStr.toFloat();
          Serial.printf("Got distance: %.2f\n", val);

          if (distanceFeed) {
            distanceFeed->save(val);
            Serial.println("Sent to Adafruit IO");
          }
        } else {
          Serial.print("Unknown line: ");
          Serial.println(incomingLine);
        }
      }
      incomingLine = "";
    } else {
      incomingLine += c;
      // protect from runaway
      if (incomingLine.length() > 64) incomingLine = "";
    }
  }
  delay(10);
}
