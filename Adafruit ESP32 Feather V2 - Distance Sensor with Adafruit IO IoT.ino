//----------------------------------------------------------------
// Adafruit ESP32 Feather V2 HC-SR04 Distance Sensor (Adafruit IO IoT)
//
// Vincent Tjoa
// September 11, 2025

#include <WiFi.h>
#include <AdafruitIO_WiFi.h>
#include <HCSR04.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// Wi-Fi credentials
#define WIFI_SSID "xxxxx" //WiFi SSID
#define WIFI_PASS "xxxxx" //WiFi Password

// Adafruit IO credentials
#define IO_USERNAME "xxxxx" //change this to Adafruit IO username
#define IO_KEY "xxxxx" //change this to Adafruit IO key

// Adafruit IO instance
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Feeds
AdafruitIO_Feed *distance_feed = io.feed("distance");

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin setup
#define BUTTON_PIN 4
#define BUZZER_PIN 25
HCSR04 hc(5, 36); // trig, echo

unsigned long pauseStartTime = 0;
bool isPaused = false;
bool lastButtonState = HIGH;

unsigned long lastMillis = 0;
const unsigned long interval = 1000; // 1 sec

void setup() {
  Serial.begin(115200);
  delay(100);

  // Pin modes
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  // Initial screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(30, 25);
  display.println("Door Security");
  display.setCursor(50, 35);
  display.println("Module");
  display.display();
  delay(1000);

  // Connect to Adafruit IO
  io.connect();
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    io.run();
    delay(500);
  }
  Serial.println("\nConnected to Adafruit IO!");
}

void loop() {
  io.run(); // keep Adafruit IO connection alive

  // Button state
  bool currentButtonState = digitalRead(BUTTON_PIN);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    isPaused = !isPaused;
    Serial.println(isPaused ? "Paused!" : "Resumed!");
    delay(200);
  }
  lastButtonState = currentButtonState;

  if (millis() - lastMillis >= interval) {
    lastMillis = millis();

    if (!isPaused) {
      float distance = hc.dist();
      if(distance == 0) distance = 400;

      String doorState = (distance < 120.0) ? "OPEN" : "CLOSED";

      // OLED display
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(1);
      display.println("Door State:");
      display.setTextSize(3);
      display.setCursor(0, 16);
      display.print(doorState);
      display.setTextSize(1);
      display.setCursor(0, 55);
      display.print("Distance: ");
      display.print(distance, 2);
      display.print(" cm");
      display.display();

      // Serial output
      Serial.printf("Distance: %.2f cm\n", distance);
      Serial.println("Door: " + doorState);

      // Buzzer
      digitalWrite(BUZZER_PIN, (distance > 120.0) ? HIGH : LOW);

      // Send to Adafruit IO
      distance_feed->save(distance);
    } else {
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(20, 24);
      display.println("Paused!");
      display.display();
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}
