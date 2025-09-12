//----------------------------------------------------------------
// Arduino Uno R3 Air Quality Monitor
//
// Vincent Tjoa
// September 11, 2025

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <SDS011.h>
#include <MQUnifiedsensor.h>

// OLED Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // matches your test sketch

// SDS011 Setup
SoftwareSerial sdsSerial(8, 9); // RX=8, TX=9
SDS011 sds;

// MQ135 Setup
#define placa "Arduino UNO"
#define Voltage_Resolution 5
#define pin A0
#define type "MQ-135"
#define ADC_Bit_Resolution 10
#define RatioMQ135CleanAir 3.6

MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

// ESP-01 Forwarder
SoftwareSerial espSerial(2, 3); // RX=2, TX=3

// Data Variables
float PM25 = 0, PM10 = 0;
float CO = 0, CO2 = 0, Alcohol = 0;

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 15000; // 15s

// Setup
void setup() {
  Serial.begin(9600);

  // 1. OLED first
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed!");
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.println("Air Monitor");
  display.display();
  delay(1000);
  Serial.println("OLED initialized");

  // 2. SDS011 init
  sdsSerial.begin(9600);
  sds.begin(8, 9);
  Serial.println("SDS011 initialized");

  // 3. ESP-01 serial init
  espSerial.begin(9600);
  Serial.println("ESP-01 serial ready");

  // 4. MQ135 init + calibration
  MQ135.setRegressionMethod(1);
  MQ135.init();
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ135.update();
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    delay(500);
    Serial.print(".");
  }
  MQ135.setR0(calcR0 / 10.0);
  Serial.println(" MQ135 calibrated");

  Serial.println("Setup complete!");
}

// Loop
void loop() {
  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();

    // MQ135 Readings
    MQ135.update();
    MQ135.setA(110.47); MQ135.setB(-2.862); float co2 = MQ135.readSensor() + 400; // baseline offset
    MQ135.setA(605.18); MQ135.setB(-3.937); float co = MQ135.readSensor();
    MQ135.setA(77.255); MQ135.setB(-3.18); float alcohol = MQ135.readSensor();

    CO2 = co2; CO = co; Alcohol = alcohol;

    // SDS011 Readings
    int error;
    float p25, p10;
    error = sds.read(&p25, &p10);
    if (!error) {
      PM25 = p25;
      PM10 = p10;
    }

    // OLED Update
    display.clearDisplay();
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.print("PM2.5: ");
    display.print(PM25, 1);
    display.println(" ug/m3");

    display.setCursor(0, 12);
    display.print("PM10 : ");
    display.print(PM10, 1);
    display.println(" ug/m3");

    display.setCursor(0, 24);
    display.print("CO2  : ");
    display.print(CO2, 1);
    display.println(" ppm");

    display.setCursor(0, 36);
    display.print("CO   : ");
    display.print(CO, 1);
    display.println(" ppm");

    display.setCursor(0, 48);
    display.print("Alcohol: ");
    display.print(Alcohol, 1);
    display.println(" ppm");

    display.display();

    // Forward to ESP-01
    espSerial.print(PM25); espSerial.print(",");
    espSerial.print(PM10); espSerial.print(",");
    espSerial.print(CO2);  espSerial.print(",");
    espSerial.print(CO);   espSerial.print(",");
    espSerial.println(Alcohol);

    // Debug to Serial Monitor
    Serial.print("Data Sent: ");
    Serial.print(PM25); Serial.print(",");
    Serial.print(PM10); Serial.print(",");
    Serial.print(CO2);  Serial.print(",");
    Serial.print(CO);   Serial.print(",");
    Serial.println(Alcohol);
  }
}
