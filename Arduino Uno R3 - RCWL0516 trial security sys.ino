//----------------------------------------------------------------
// Arduino_Uno_R3_RCWL0516_security_system
//
// Vincent Tjoa, 10 July 2025

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// === Pin Definitions ===
#define RCWL_PIN     11    // Motion sensor OUT
#define BUZZER_PIN    3    // Buzzer control pin

// === OLED Setup ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);

  pinMode(RCWL_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("RCWL Ready");
  display.display();
  delay(1500);
}

void loop() {
  bool motionDetected = digitalRead(RCWL_PIN) == HIGH;

  digitalWrite(BUZZER_PIN, motionDetected ? HIGH : LOW);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 10);
  if (motionDetected) {
    display.println("Motion");
    display.println("Detected!");
  } else {
    display.setTextSize(1);
    display.println("No motion...");
  }
  display.display();

  Serial.print("Motion: ");
  Serial.println(motionDetected ? "YES" : "NO");

  delay(100);  // debounce and reduce flickering
}
