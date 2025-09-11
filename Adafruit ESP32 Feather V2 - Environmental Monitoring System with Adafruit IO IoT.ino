//----------------------------------------------------------------
// Adafruit ESP32 Feather V2 Environmental Monitoring System (Adafruit IO IoT)
//
// Vincent Tjoa
// September 11, 2025

#include <Adafruit_SHTC3.h>
#include <WiFi.h>
#include <AdafruitIO_WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// Wi-Fi credentials
#define WIFI_SSID "xxxxx" //change this to WiFi SSID
#define WIFI_PASS "xxxxx" //change this to WiFI password

// Adafruit IO credentials
#define IO_USERNAME "xxxxx" //change this to Adafruit IO username
#define IO_KEY "xxxxx" //change this to Adafruit IO key

// Adafruit IO instance
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Feeds
AdafruitIO_Feed *temperature_feed = io.feed("SHTC3 Temperature");
AdafruitIO_Feed *humidity_feed = io.feed("SHTC3 Humidity");
AdafruitIO_Feed *moisture1_feed = io.feed("Resistive Soil Moisture Sensor 1");
AdafruitIO_Feed *moisture2_feed = io.feed("Resistive Soil Moisture Sensor 2");

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// SHTC3 setup
Adafruit_SHTC3 shtc3 = Adafruit_SHTC3();

// Soil Moisture Sensor 1 Setup
const int moisture_pin = 33;  // Analog pin GPIO33
int moisture_raw = 0;
int moisture_percent = 0;

// Soil Moisture Sensor 2 Setup
const int moisture_pin2 = 32; // Analog pin GPIO32
int moisture_raw2 = 0;
int moisture_percent2 = 0;

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 15000; // 15s

void setup() {
  Serial.begin(115200);

  // SHTC3 init
  while (!Serial)
    delay(10);
  Serial.println("SHTC3 test");
  if (! shtc3.begin()) {
    Serial.println("Couldn't find SHTC3");
    while (1) delay(1);
  }
  Serial.println("Found SHTC3 sensor");
  
  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  // Initial screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(30, 15);
  display.println("Environmental");
  display.setCursor(35,25);
  display.println("Monitoring");
  display.setCursor(50, 35);
  display.println("Device");
  display.display();
  delay(500);

  // Connect to Adafruit IO
  io.connect();
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    io.run();
    delay(500);
  }

  unsigned long startAttempt = millis();
  while(io.status() < AIO_CONNECTED && millis() - startAttempt < 15000) {
    Serial.print(".");
    io.run();
    delay(500);
  }
  if (io.status() < AIO_CONNECTED) {
    Serial.println("⚠ Skipping Adafruit IO, not connected.");
  }

  Serial.println("\nConnected to Adafruit IO!");

  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP()); // Print local IP address
}


void loop() {
  io.run(); // keep Adafruit IO connection alive

  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();

    sensors_event_t humidity, temp;
    shtc3.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data

    moisture_raw = analogRead(moisture_pin);
    // Updated mapping for resistive soil_moisture_1: 1300 (wet) -> 100%, 4095 (dry) -> 0%
    moisture_percent = ((4095 - moisture_raw) / (4095.0 - 1300.0) * 100);
    moisture_percent = constrain(moisture_percent, 0, 100); // Clamp between 0–100

    moisture_raw2 = analogRead(moisture_pin2);
    // Updated mapping for resistive soil_moisture_2: 1300 (wet) -> 100%, 4095 (dry) -> 0%
    moisture_percent2 = ((4095 - moisture_raw2) / (4095.0 - 1300.0)) * 100;
    moisture_percent2 = constrain(moisture_percent2, 0, 100); // Clamp between 0–100

    // Serial Debug
    Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
    Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
    Serial.printf("Soil Moisture 1: %d%% (raw: %d)\n", moisture_percent, moisture_raw);
    Serial.printf("Soil Moisture 2: %d%% (raw: %d)\n", moisture_percent2, moisture_raw2);
    Serial.println("");

    // Display Output
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.printf("SHTC3 Temp : %.1f C\n", temp.temperature);
    display.setCursor(0, 20);
    display.printf("SHTC3 Humidity : %.1f C\n", humidity.relative_humidity);
    display.setCursor(0, 30);
    display.printf("Soil Moisture 1: %d %%", moisture_percent);  // Display unchanged
    display.setCursor(0, 40);
    display.printf("Soil Moisture 2: %d %%", moisture_percent2);
    display.display();

    // Send to Adafruit IO
    temperature_feed->save(temp.temperature);
    humidity_feed->save(humidity.relative_humidity);
    moisture1_feed->save(moisture_percent);
    moisture2_feed->save(moisture_percent2);

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("⚠ WiFi disconnected, reconnecting...");
      WiFi.begin(WIFI_SSID, WIFI_PASS);
    }

    if (io.status() < AIO_CONNECTED) {
      Serial.println("⚠ Adafruit IO disconnected, reconnecting...");
      io.connect();
    }
  }
}
