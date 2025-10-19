# MCU Code
//----------------------------------------------------------------

### Adafruit ESP32 Feather V2 Air Quality Monitor (Adafruit IO IoT) notes:

This system continuously measures air quality parameters using two main sensors:
1. MQ-135 gas sensor for CO₂, CO, and alcohol concentration in ppm.
2. SDS011 particulate matter sensor for PM2.5 and PM10 levels in µg/m³.

All measured data is displayed on a 0.96” SSD1306 OLED and uploaded to Adafruit IO over Wi-Fi for cloud logging and visualization.

The MQ-135 is an analog gas sensor connected to the ESP32’s ADC pin. It requires calibration to determine its R₀ (resistance in clean air), which is done automatically at startup by sampling multiple readings. The sensor is then used with predefined regression equations for CO₂, CO, and alcohol gas detection.

The SDS011 particulate sensor communicates via UART (TX/RX) and reports PM2.5 and PM10 particle concentrations. The code uses asynchronous data acquisition to ensure smooth operation without blocking other processes. Valid readings are stored in buffers and filtered to produce stable averaged results.

All data is refreshed every 15 seconds, and valid readings are pushed to individual Adafruit IO feeds:
 - MQ135 CO, CO₂, Alcohol
 - SDS011 PM10, PM2.5

An OLED screen shows real-time sensor data locally. During operation, if no valid PM readings are yet available, the display shows “PM: waiting…” until SDS011 reports valid data.

The system connects automatically to Wi-Fi and Adafruit IO upon startup, displaying connection progress on the serial monitor. If Wi-Fi or Adafruit IO disconnects during runtime, the ESP32 automatically attempts reconnection to maintain cloud data transmission.

Connections:
 - MQ135 analog output → GPIO 32
 - SDS011 TX → GPIO 7
 - SDS011 RX → GPIO 8
 - OLED Display → I²C (SDA/SCL from ESP32)
 - All sensors powered from 5V pin (USB VBUS line), with shared ground to ESP32

Power and data flow summary:
 - MQ135 and SDS011 use 5V input (ESP32 tolerates analog input through onboard ADC pin).
 - OLED and logic communications run at 3.3V I²C.
 - Data from sensors → ESP32 → Adafruit IO cloud dashboard.

This program was written and uploaded via the Arduino IDE, using libraries for each module (Adafruit SSD1306, MQUnifiedsensor, esp_sds011, and AdafruitIO_WiFi).

//----------------------------------------------------------------

### Adafruit ESP32 Feather V2 HC-SR04 Distance Sensor (Adafruit IO IoT) notes:

This system uses a HC-SR04 module to detect distance to an object in front of the sensor, and displays the data from the sensor on an SSD1306 0.96" OLED display
If the measured distance exceeds 100 cm, a 5V active buzzer is triggered. The buzzer is controlled through an S8050 NPN BJT transistor,
which is switched on by setting a GPIO pin HIGH to provide base current.

A pushbutton is connected to GPIO 4, allowing the user to toggle between "paused" and "active" measurement modes.
While paused, the display shows a "Paused!" message, and the buzzer is disabled regardless of the measured distance.

SSD1306 is powered by 3.3V (can still be powered by 5V without voltage divider for SCL and SDA lines)
Active buzzer and HC-SR04 distance sensor are powered by 5V through Adafruit ESP32 Feather V2's USB pin

Connections:
 - HC-SR04 TRIG → GPIO 5
 - HC-SR04 ECHO → GPIO 36 (voltage divider from 5V powering the HC-SR04 to 3.3V, able to be tolerated by ESP32 GPIO)
 - OLED Display → I2C (SDA/SCL from ESP32)
 - Button → GPIO 4 (with internal pull-up resistor)
 - Buzzer (via NPN transistor) → GPIO 25 (connect a 1k ohm resistor from ESP32 GPIO to the transistor's base pin. Buzzer's negative terminal is connected to S8050 collector, then BJT emitter leg to ground)

Power and data flow summary:
 - The HC-SR04 sends ultrasonic pulses and receives echoes to measure distance.
 - The OLED displays the current distance or “Paused!” status for local monitoring.
 - All logic communication (trigger, echo processing, and display) runs at 3.3V.
 - Sensor and buzzer are powered through the 5V USB pin, with common ground shared with the ESP32.

This program was written and uploaded via the Arduino IDE, using libraries for each module (HC_SR04, Adafruit SSD1306, Adafruit GFX, and AdafruitIO_WiFi).

//----------------------------------------------------------------

### Adafruit ESP32 Feather V2 Environmental Monitoring System (Adafruit IO IoT) notes:

This system measures environmental parameters using an SHTC3 temperature and humidity sensor and two resistive soil moisture sensors, displaying data on an SSD1306 0.96” OLED while uploading readings to Adafruit IO over Wi-Fi for cloud monitoring.

The SHTC3 communicates via I2C and provides accurate digital readings for ambient temperature and relative humidity. Its data is refreshed every 15 seconds and transmitted to Adafruit IO for remote visualization.

Two resistive soil moisture sensors are connected to the ESP32’s analog input pins (GPIO33 and GPIO32). Their raw analog readings are converted into percentage values using a calibrated linear mapping: 1300 ADC (wet) equals 100%, and 4095 ADC (dry) equals 0%. The resulting moisture percentages represent soil hydration levels for two separate plants or soil areas.

The SSD1306 OLED display shows real-time temperature, humidity, and both soil moisture percentages on-screen. The display updates every 15 seconds with fresh data from the sensors.

Wi-Fi and Adafruit IO connectivity allow the system to send temperature, humidity, and both soil moisture values to individual Adafruit IO feeds. If the connection drops, the ESP32 automatically attempts to reconnect to both Wi-Fi and Adafruit IO.

Connections:
 - SHTC3 → I2C (SDA/SCL from ESP32)
 - Soil Moisture Sensor → GPIO33 (analog input)
 - Soil Moisture Sensor 2 → GPIO32 (analog input)
 - OLED Display → I2C (SDA/SCL from ESP32)
 - All sensors powered from 3.3V and GND

Power and data flow summary:
 - The SHTC3 sensor provides temperature and humidity data through I2C.
 - Two analog soil moisture sensors measure water content in soil via voltage readings.
 - The OLED displays all readings locally for user monitoring.
 - Data is uploaded to Adafruit IO for remote observation and analysis.

This program was written and uploaded via the Arduino IDE, using libraries for each module (Adafruit SHTC3, Adafruit SSD1306, Adafruit GFX, and AdafruitIO_WiFi).

//----------------------------------------------------------------

### Arduino Uno R3 Air Quality Monitor notes:

This system measures air quality using two sensors: an MQ-135 gas sensor for CO₂, CO, and alcohol concentration, and an SDS011 particulate matter sensor for PM2.5 and PM10 levels. Data is displayed locally on a 0.96” SSD1306 OLED and transmitted via an ESP-01 Wi-Fi module for wireless monitoring.

The MQ-135 is connected to the Arduino’s analog input (A0). At startup, the system calibrates the sensor to determine its R₀ (resistance in clean air), ensuring accurate gas concentration readings. The SDS011 communicates through UART (TX/RX on pins D8/D9) to provide particulate concentration data in µg/m³. Readings from both sensors are updated every 15 seconds.

An SSD1306 OLED connected through I²C shows live PM2.5, PM10, and gas concentration readings. The ESP-01 (TX: D2, RX: D3) receives data from the Arduino Uno for wireless transmission to Adafruit IO.

Connections:
 - MQ135 analog output → A0
 - SDS011 TX → D8
 - SDS011 RX → D9
 - ESP-01 TX → D2
 - ESP-01 RX → D3
 - OLED Display → I²C (SDA/SCL from Arduino)

Power and data flow summary:
 - MQ135 and SDS011 powered by 5V (common ground shared).
 - OLED runs on 3.3V I²C logic.
 - Data flows from sensors → Arduino Uno → OLED + ESP-01 (for wireless data transmission).

This program was written and uploaded via the Arduino IDE, using libraries for each module (MQUnifiedsensor, SDS011, Adafruit_GFX, Adafruit_SSD1306, SoftwareSerial).

//----------------------------------------------------------------

### Arduino Uno R3 Door Security Distance Sensor notes:

This system functions as a simple door monitoring module using an HC-SR04 ultrasonic sensor, SSD1306 OLED display, and a buzzer for alerts. It measures the distance between the sensor and a door surface to determine whether the door is open or closed, displaying the result locally and sending distance data to an ESP-01 via serial communication for IoT or remote monitoring.

The HC-SR04 ultrasonic sensor measures distance by sending and receiving ultrasonic pulses. The Arduino interprets the returned echo time to calculate distance in centimeters. If the measured distance is less than 120 cm, the system assumes the door is “OPEN” and activates the buzzer. Otherwise, it shows “CLOSED” and turns the buzzer off.

A 0.96” SSD1306 OLED display connected via I²C shows both the door status and the exact distance reading, refreshing once per second. The ESP-01 module connected via UART (Serial) receives the distance data for wireless transmission to Adafruit OP through WiFi.

Connections:
 - HC-SR04 TRIG → D9
 - HC-SR04 ECHO → D8
 - Buzzer → D10
 - OLED Display → I²C (SDA/SCL from Arduino)
 - ESP-01 TX/RX → Serial pins (via SoftwareSerial or hardware Serial)

Power and data flow summary:
 - HC-SR04 and buzzer powered by 5V.
 - OLED uses 3.3V I²C logic.
 - Distance readings flow from HC-SR04 → Arduino Uno → OLED + ESP-01.
 - The buzzer provides an audible alert when the door is detected as open.

This program was written and uploaded via the Arduino IDE, using libraries for each module (HCSR04, Adafruit_SSD1306, Adafruit_GFX).

//----------------------------------------------------------------

### Arduino Uno R3 Environmental Monitoring System notes:

This system monitors environmental parameters including temperature, humidity, and soil moisture using an SHTC3 sensor and two analog soil moisture probes. Data is displayed locally on a 0.96” SSD1306 OLED and transmitted via an ESP-01 module through serial communication for remote monitoring.

The SHTC3 temperature and humidity sensor communicates over I²C (address 0x70). It provides accurate digital readings that are refreshed every 15 seconds. The two soil moisture sensors are connected to analog inputs (A0 and A1) and mapped from raw ADC readings (1023 dry → 0%, 300 wet → 100%) to display soil moisture percentages.

All data is refreshed periodically and displayed on the OLED screen, showing temperature, humidity, and both soil moisture readings. Simultaneously, the same data is sent through the Serial interface to the ESP-01 for wireless data transmission to Adafruit IO.

Connections:
 - SHTC3: I²C (SDA/SCL from Arduino)
 - Soil Moisture Sensor 1: A0
 - Soil Moisture Sensor 2: A1
 - OLED Display: I²C (SDA/SCL from Arduino)
 - ESP-01 TX/RX → Serial pins

Power and data flow summary:
 - SHTC3 powered by 3.3V (I²C logic).
 - Soil moisture sensors powered by 5V with shared ground.
 - Data flows from sensors → Arduino Uno → OLED + ESP-01.
 - Temperature, humidity, and soil moisture values are displayed and transmitted every 15 seconds.

This program was written and uploaded via the Arduino IDE, using libraries for each module (Wire, Adafruit_SSD1306, Adafruit_GFX).

//----------------------------------------------------------------

### Arduino Uno R3 RCWL-0516 Security System notes:

This system detects motion using the RCWL-0516 microwave motion sensor and triggers a buzzer and OLED display alert when movement is detected. It can be used for indoor security applications such as motion alarms or presence detection systems.

The RCWL-0516 outputs a HIGH signal when motion is detected. The Arduino reads this signal from digital pin 11 and activates the buzzer (connected to pin 3) accordingly. Detection status is displayed on a 0.96” SSD1306 OLED screen and logged through the serial monitor for debugging or data collection.

Connections:
 - RCWL-0516 OUT → D11
 - Buzzer → D3
 - OLED → I²C (SDA/SCL)

Power and data flow summary:
 - RCWL-0516 and OLED powered by 5V.
 - Data flows from RCWL-0516 → Arduino Uno → OLED + buzzer.
 - Motion detection results are displayed and outputted via serial.

This program was written and uploaded via the Arduino IDE, using libraries for each module (Wire, Adafruit_SSD1306, Adafruit_GFX).

//----------------------------------------------------------------

### STM32F401CCU6 Blackpill Air Quality Monitor notes:

This system monitors air quality using an MQ-135 gas sensor and an SDS011 particulate matter sensor, displaying live readings on a 0.96” OLED screen and transmitting data to an ESP-01 for IoT monitoring. It measures PM2.5, PM10, CO₂, CO, and alcohol concentration in real time.

The MQ-135 detects multiple gases via analog input (A0), while the SDS011 sends PM data through UART (PA10=RX, PA9=TX). Both datasets are processed and updated every 15 seconds. The STM32 communicates with the ESP-01 through Serial2 (A2=TX, A3=RX), sending data as a formatted string for further use in cloud platforms or remote dashboards.

Connections:
 - MQ-135 → A0 (analog)
 - SDS011 → PA9 (TX), PA10 (RX)
 - OLED → I²C (PB6=SCL, PB7=SDA)
 - ESP-01 → A2 (TX), A3 (RX)

Power and data flow summary:
 - SHTC3 and OLED powered by 3.3V (I²C logic).
 - MQ-135 sensor powered by 5V with shared ground.
 - Data flows from sensors → STM32F401CCU6 → OLED + ESP-01.
 - Air quality, temperature, and humidity values are displayed and transmitted periodically.

This program was written and uploaded via the Arduino IDE, using libraries for each module (Wire, U8g2, U8x8, MQUnifiedsensor).

//----------------------------------------------------------------

### STM32F401CCU6 Blackpill Distance Sensor notes:

This system measures distance using an HC-SR04 ultrasonic sensor and displays the readings on a 0.96” OLED. It also determines door state (open or closed) and sends distance data to an ESP-01 module for wireless monitoring. A buzzer provides an alert when the door is closed.

The HC-SR04 sensor triggers a 10 µs pulse and measures the echo duration to calculate distance in centimeters. When the measured distance is below 120 cm, the door is considered “OPEN”; otherwise, it is “CLOSED.” The buzzer (connected to A5) activates when the distance exceeds 120 cm. The OLED displays both the door state and measured distance, updated every second.

Connections:
 - HC-SR04 → A0 (Trig), A1 (Echo)
 - OLED → I²C (PB6=SCL, PB7=SDA)
 - ESP-01 → Serial1 (PA2=TX, PA3=RX, crossed)
 - Buzzer → A5 (transistor-driven)

Power and data flow summary:
 - HC-SR04 powered by 5V with shared ground.
 - OLED powered by 3.3V (I²C logic).
 - Data flows from ultrasonic sensor → STM32F401CCU6 → OLED + ESP-01.
 - Distance values are displayed and transmitted every few seconds.

This program was written and uploaded via the Arduino IDE, using libraries for each module (Wire, U8g2, U8x8).

//----------------------------------------------------------------

### STM32F401CCU6 Blackpill Environmental Monitoring System notes:

This system monitors soil moisture, temperature, and humidity using two analog soil moisture sensors and an SHTC3 sensor. Data is displayed on a 0.96” OLED and transmitted to an ESP-01 via Serial2 for IoT monitoring.

The SHTC3 provides temperature and humidity readings over I²C, while two soil sensors (connected to PB8 and PB9) provide analog values that are converted into moisture percentages. The OLED shows live environmental data updated every 15 seconds.

Connections:
 - SHTC3 → I²C (PB6=SCL, PB7=SDA)
 - Soil 1 → PB9 (Analog)
 - Soil 2 → PB8 (Analog)
 - ESP-01 → Serial2 (A2=TX, A3=RX, crossed)

Power and data flow summary:
 - SHTC3 powered by 3.3V (I²C logic).
 - Soil moisture sensors powered by 5V with shared ground.
 - Data flows from sensors → STM32F401CCU6 → OLED + ESP-01.
 - Temperature, humidity, and soil moisture values are displayed and transmitted every 15 seconds.

This program was written and uploaded via the Arduino IDE, using libraries for each module (Wire, SPI, U8g2, U8x8, Adafruit_SHTC3).

//----------------------------------------------------------------

### ALL ESP-01 FORWARDER CODE NOTES FOR STM32F401CCU6 SYSTEMS:

An ESP-01 module uploads all the data received from the STM32 systems (using UART) to Adafruit IO via Wi-Fi.

Connections:
 - ESP-01 RX ← STM32 TX (A2)
 - ESP-01 TX → STM32 RX (A3)

Power and data flow summary:
 - ESP-01 powered by 3.3V (logic and power).
 - Data flows from Sensors → STM32 → ESP-01 → Wi-Fi → Adafruit IO.
 - The ESP-01 receives serial data, parses sensor values, and uploads them to multiple Adafruit IO feeds.

This program was written and uploaded via the Arduino IDE, using libraries for each module (ESP8266WiFi, AdafruitIO_WiFi).

//----------------------------------------------------------------

### ALL ESP-01 FORWARDER CODE NOTES FOR Arduino Uno R3 SYSTEMS:

An ESP-01 module uploads all the data received from the Arduino Uno R3 systems (also using UART like the STM32 setups) to Adafruit IO via Wi-Fi.

Connections:
 - ESP-01 RX ← Uno TX (pin 1)
 - ESP-01 TX → Uno RX (pin 0)

Power and data flow summary:
 - ESP-01 powered by 3.3V (logic and power).
 - Data flows from Sensors → Uno R3 → ESP-01 → Wi-Fi → Adafruit IO.
 - The ESP-01 receives serial data, parses sensor values, and uploads them to multiple Adafruit IO feeds.

This program was written and uploaded via the Arduino IDE, using libraries for each module (ESP8266WiFi, AdafruitIO_WiFi).
