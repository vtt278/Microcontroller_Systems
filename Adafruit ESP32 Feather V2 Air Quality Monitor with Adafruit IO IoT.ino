//----------------------------------------------------------------
// Adafruit ESP32 Feather V2 Air Quality Monitor (Adafruit IO IoT)
//
// Vincent Tjoa
// September 11, 2025

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MQUnifiedsensor.h>
#include <SoftwareSerial.h>
#include <esp_sds011.h>
#include <WiFi.h>
#include <AdafruitIO_WiFi.h>

// Wi-Fi credentials
#define WIFI_SSID "xxxxx" //change this to WiFi SSID
#define WIFI_PASS "xxxxx" //change this to WiFi password

// Adafruit IO credentials
#define IO_USERNAME "xxxxx" //change this to Adafruit IO username
#define IO_KEY "xxxxx" //change this to Adafruit IO username

// Adafruit IO instance
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Feeds
AdafruitIO_Feed *co_feed = io.feed("MQ135 CO");
AdafruitIO_Feed *co2_feed = io.feed("MQ135 CO2");
AdafruitIO_Feed *alcohol_feed = io.feed("MQ135 Alcohol");
AdafruitIO_Feed *pm10_feed = io.feed("SDS011 PM10");
AdafruitIO_Feed *pm25_feed = io.feed("SDS011 PM2.5");

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  
// SDS011 setup
#define SDS_RX 7
#define SDS_TX 8

HardwareSerial serialSDS(1);
Sds011Async<HardwareSerial> sds011(serialSDS);

// Global PM buffers (must persist in memory)
constexpr int PM_TABLE_SIZE = 10;
int pm25_table[PM_TABLE_SIZE];
int pm10_table[PM_TABLE_SIZE];

float PM10, PM25;
bool hasPM = false;

// MQ135 setup
#define placa "ESP-32"
#define Voltage_Resolution 3.3
#define pin 32
#define type "MQ-135"
#define ADC_Bit_Resolution 12
#define RatioMQ135CleanAir 3.6

MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 15000; // 15s


void setup() {
  Serial.begin(115200);

  // OLED init
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(50, 0);
  display.println("Air");
  display.setCursor(22, 22);
  display.println("Quality");
  display.setCursor(25, 48);
  display.println("Monitor");
  display.display();
  delay(1000);
  display.clearDisplay();

  // MQ135 init
  MQ135.setRegressionMethod(1);
  MQ135.init();

  Serial.print("Calibrating MQ135...");
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ135.update();
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
    delay(500);
  }
  MQ135.setR0(calcR0 / 10.0);
  Serial.println(" done!");

  if (isinf(calcR0) || calcR0 == 0) {
    Serial.println("Error: invalid R0 for MQ135");
    while (1);
  }

  // SDS011 init
  serialSDS.begin(9600, SERIAL_8N1, SDS_RX, SDS_TX);
  delay(100);

  Serial.println(F("Init SDS011..."));
  sds011.set_sleep(false);
  sds011.set_data_reporting_mode(Sds011::REPORT_ACTIVE);

  // Async callback for SDS011
  sds011.on_query_data_auto_completed([](int n) {
    int pm25_raw, pm10_raw;
    if (sds011.filter_data(n, pm25_table, pm10_table, pm25_raw, pm10_raw)) {
      PM25 = float(pm25_raw) / 10.0;
      PM10 = float(pm10_raw) / 10.0;
      hasPM = true;
      Serial.printf("PM2.5: %.1f µg/m³ | PM10: %.1f µg/m³\n", PM25, PM10);
    }
  });

  // start async measurement capture
  if (!sds011.query_data_auto_async(PM_TABLE_SIZE, pm25_table, pm10_table)) {
    Serial.println(F("SDS011 measurement capture start failed"));
  }

  // Connect to Adafruit IO
  io.connect();
  unsigned long startAttempt = millis();
  while(io.status() < AIO_CONNECTED && millis() - startAttempt < 15000) {
    Serial.print(".");
    io.run();
    delay(500);
  }
  if (io.status() == AIO_CONNECTED) {
    Serial.println("\n✅ Connected to Adafruit IO");
  } else {
    Serial.println("⚠ Adafruit IO not connected, continuing without it.");
  }

  Serial.println("Setup complete.");
}


void loop() {
  io.run(); // keep Adafruit IO connection alive
  sds011.perform_work();
  MQ135.update();

  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();

    // MQ135 readings
    MQ135.setA(110.47); MQ135.setB(-2.862);
    float CO2 = MQ135.readSensor();

    MQ135.setA(605.18); MQ135.setB(-3.937);
    float CO = MQ135.readSensor();

    MQ135.setA(77.255); MQ135.setB(-3.18);
    float Alcohol = MQ135.readSensor();

    Serial.printf("CO2: %.1f ppm | CO: %.1f ppm | Alcohol: %.1f ppm\n",
                  CO2 + 400, CO, Alcohol);

    // OLED update
    display.clearDisplay();
    display.setTextSize(1);
    int y = 0;
    if (hasPM) {
      display.setCursor(0, y+5); display.printf("PM2.5: %.1f ug/m3", PM25); y += 18;
      display.setCursor(0, y); display.printf("PM10 : %.1f ug/m3", PM10); y += 12;
    } else {
      display.setCursor(0, y); display.println("PM: waiting..."); y += 22;
    }
    display.setCursor(0, y); display.printf("CO2  : %.1f ppm", CO2 + 400); y += 12;
    display.setCursor(0, y); display.printf("CO   : %.1f ppm", CO); y += 12;
    display.setCursor(0, y); display.printf("Alcohol: %.1f ppm", Alcohol);
    display.display();

    // Send to Adafruit IO
    co_feed->save(CO);
    co2_feed->save(CO2+400);
    alcohol_feed->save(Alcohol);
    if (hasPM) {         // only send if valid
      pm10_feed->save(PM10);
      pm25_feed->save(PM25);
    }
  }

  // WiFi reconnect check
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠ WiFi dropped, reconnecting...");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  }
  if (io.status() < AIO_CONNECTED) {
    Serial.println("⚠ Adafruit IO disconnected, reconnecting...");
    io.connect();
  }
}
