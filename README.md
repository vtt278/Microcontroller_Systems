# MCU-Code
//
Adafruit ESP32 Feather V2 HC-SR04 Distance Sensor using an SSD1306 OLED Display (Adafruit IO IoT) notes:

This system use a HC-SR04 module to detect distance to an object in front of the sensor, and displays the data from the sensor on an SSD1306 0.96" OLED display
If the measured distance exceeds 100 cm, a 5V active buzzer is triggered. The buzzer is controlled through an S8050 NPN BJT transistor,
which is switched on by setting a GPIO pin HIGH to provide base current.

A pushbutton is connected to GPIO 4, allowing the user to toggle between "paused" and "active" measurement modes.
While paused, the display shows a "Paused!" message, and the buzzer is disabled regardless of the measured distance.

This code was created in Arduino IDE, and was directly flashed onto the ESP32 from the IDE.

SSD1306 is powered by 3.3V (can still be powered by 5V without voltage divider for SCL and SDA lines)
Active buzzer and HC-SR04 distance sensor are powered by 5V through Adafruit ESP32 Feather V2's USB pin

Connections:
 - HC-SR04 TRIG: GPIO 5
 - HC-SR04 ECHO: GPIO 36 (voltage divider from 5V powering the HC-SR04 to 3.3V, able to be tolerated by ESP32 GPIO)
 - OLED Display: I2C (SDA/SCL from ESP32)
 - Button: GPIO 4 (with internal pull-up resistor)
 - Buzzer (via NPN transistor): GPIO 25 (connect a 1k ohm resistor from ESP32 GPIO to the transistor's base pin. Buzzer's negative terminal is connected to S8050 collector, then BJT emitter leg to ground)


//
The Learning Farm ESP32 Automated Irrigation System with IoT Data Collection (Data Sender using ESP-NOW) notes:

This system uses 5V relays instead of MOSFETs for 12V DC Water pump and 12V DC Solenoid valve control
Operation: Uses a 5V 3A AC-to-DC adapter for the ESP32 and 5V relay system, and a 12V 15A AC-to-DC adapter for the water pump and solenoid valves

This system utilizes a DOIT ESP32 Devkit V1, DHT22 temp & humidity sensor, SSD1306 0.96" OLED display, 4 LEDs, 4 capacitive soil moisture sensors, and a 5V quad-relay module (for water pumps and solenoid valves). It operates an automated irrigation system by actuating the water pump and solenoid valves based off of each soil moisture sensor's readings

I used the ESP-NOW protocol to transmit data to a receiver ESP32, which then transmits the collected data to Adafruit IO cloud (I will post its code in another file)

Both the OLED display and MCP sensor is wired up to the ESP32’s I2C bus through each module’s SCL and SDA lines, displaying all the data collected by all the sensors in real-time. 
Each module connected to the ESP32 is powered with 3.3V, not 5V (+ terminal of each sensor to 3.3v rail on breadboard, and - terminal to ESP32 GND breadboard rail)

The LEDs blink or stays off depending on the soil moisture percentage. In this configuration, each LED is paired with a capacitive soil moisture sensor. 

When soil moisture percentage is 0-19%, it stays off.
When soil moisture percentage is 20-39%, it blinks once.
When soil moisture percentage is 40-59%, it blinks twice.
When soil moisture percentage is 60-79%, it blinks thrice.
When soil moisture percentage is 80-100%, it blinks four to five times.

Each blink consists of 200ms ON and 100ms OFF.
Each cycle runs totalBlinks × 2 (because each blink is 1 ON + 1 OFF).
Once blinking finishes, the LED stays off until a new blink count is triggered.

When calibrating the sensors, the capacitive soil moisture sensors transmits a raw value of around 3600 when dry, and around 1600 when wet. 

This code was created in Arduino IDE, and was directly flashed onto the ESP32 from the IDE.
I also may have had the help of AI in the creation of this code :)
