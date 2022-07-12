# kitchentimer
This is the "kitchentimer" sketch, written for ESP32 and Arduino-IDE. It works with Adafruit LED 7-segment backpacks with I2C interface and was tested with ESP32-ST and the wiring for a wroom eboxmaker himalaya board.

Like the well-known devices, the timer should serve as a kitchen clock to have the cooking times under control.

You are in need of the "Adafruit LEDBackpack" and "Adafruit GFX"-Libraries. "Wire.h" and "WiFi.h" have to be included.

Just install the Libraries and load up the sketch to a ESP32-WiFi-Kit compatible board. For the right functionality, additional hardware is required.

The ESP interacts with display (I2C interface), switch and buzzer, and provide a little HTTP server. So we are able to control the kitchen timer over web.

Have fun!
