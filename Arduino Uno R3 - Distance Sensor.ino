//----------------------------------------------------------------
// Arduino Uno R3 Distance Sensor
//
// Vincent Tjoa
// September 11, 2025

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HCSR04.h>   // Gamegine library

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pins
#define TRIG_PIN 9
#define ECHO_PIN 8
#define BUZZER_PIN 10
#define THRESHOLD 120 // cm

HCSR04 hc(TRIG_PIN, ECHO_PIN);

unsigned long lastMillis = 0;
const unsigned long interval = 1000; // 1 sec

void setup() {
  Serial.begin(9600); // communication to ESP-01
  pinMode(BUZZER_PIN, OUTPUT);

  // OLED init
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    while(true);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,25);
  display.println("Door Security");
  display.setCursor(30,45);
  display.println("Module");
  display.display();
  delay(1000);
}

void loop() {
  if(millis() - lastMillis >= interval){
    lastMillis = millis();

    float distance = hc.dist(); // returns distance in cm
    if(distance == 0) distance = 400; // treat 0 as max distance

    String doorState = (distance < THRESHOLD) ? "OPEN" : "CLOSED";

    // Display on OLED
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("Door State:");
    display.setTextSize(3);
    display.setCursor(0,16);
    display.println(doorState);
    display.setTextSize(1);
    display.setCursor(0,55);
    display.print("Distance: ");
    display.print(distance);
    display.println(" cm");
    display.display();

    // Buzzer
    digitalWrite(BUZZER_PIN, (distance < THRESHOLD) ? HIGH : LOW);

    // Send distance to ESP-01
    Serial.println(distance);
  }
}
