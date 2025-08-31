# MCU-Code

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
