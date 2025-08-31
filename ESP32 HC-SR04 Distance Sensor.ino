//----------------------------------------------------------------
// Adafruit ESP32 Feather V2 HC-SR04 Distance Sensor using an SSD1306 OLED Display
//
// Vincent Tjoa, July 3, 2025

#include <HCSR04.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  // Reset pin not used with I2C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //for screen

#define BUTTON_PIN 4  // GPIO pin connected to button
#define BUZZER_PIN 25

unsigned long pauseStartTime = 0; //for button
bool isPaused = false;
bool lastButtonState = HIGH;
bool showResumedMessage = false;

HCSR04 hc(5, 36); //initialisation class HCSR04 (trig pin , echo pin)


void setup()
{
  Serial.begin(115200);
  delay(50); 

  pinMode(BUTTON_PIN, INPUT_PULLUP); // use internal pull-up resistor
  pinMode(BUZZER_PIN, OUTPUT); // Setup buzzer pin as output

  Serial.println("\n NEW CODE BEGINS\n");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C is the most common I2C address
    Serial.println(F("SSD1306 allocation failed"));
    while (true); // Halt
  }

  display.clearDisplay();
  display.setTextSize(1);      // Normal text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("OLED + HC-SR04 + BUTTON");
  display.println("justin tjitra is the goat");
  display.println("i love mr 1580 :)");
  display.display();
  delay(2000);
}

void loop()
{
  float distance = hc.dist();

    // Read the current state of the button
  bool currentButtonState = digitalRead(BUTTON_PIN);

  // Detect button press (falling edge)
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    isPaused = !isPaused; // Toggle pause state
    Serial.print("\n");
    Serial.println(isPaused ? "Paused!" : "Resumed!");
    delay(200); // debounce delay
  }

  if (!isPaused) {
    showResumedMessage = true; // trigger "Resumed" message once
  }
  delay(200);

  lastButtonState = currentButtonState; // update button state for next loop

  if (!isPaused) {

    showResumedMessage = true;

    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("Distance:");
    display.setTextSize(2);
    display.setCursor(0, 16);
    display.print(distance, 3); // 2 decimal places
    display.print(" cm");
    display.display();
    Serial.print("Distance: ");
    Serial.print(distance, 3);
    Serial.println();

    // Buzzer logic
    if (distance > 100.0) {
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }


   } else {
    // Show "Paused" on the screen
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(20, 24);
    display.println("Paused!");
    display.display();

    digitalWrite(BUZZER_PIN, LOW);
  }
  delay(500); // we suggest to use over 60ms measurement cycle, in order to prevent trigger signal to the echo signal.
}

