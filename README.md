# How to use this?

 First, install the Arduino IDE and configure it to use the ESP8266 repository.

 Since other documentation explains how to do this better, here is a link to the
 official repository.

 https://github.com/esp8266/Arduino

 Second, once you have the ESP8266 board library installed you're going to need
 to download the following libraries and add them to the Arduino IDE library.

 (Usually, this is your documents directory under Arduino/libraries)

 You need these libraries installed to your Arduino IDE:

 Adafruit Unified Sensor library: https://github.com/adafruit/Adafruit_Sensor

 DHT11/DHT22: https://github.com/adafruit/DHT-sensor-library

 TSL2561: https://github.com/adafruit/Adafruit_TSL2561

 PubSubClient: https://github.com/knolleary/pubsubclient

 Third, you're going to need to flash your ESP8266. Personally, I did this using a
 serial connection from my Raspberry Pi and the esptool.py utility.
 
 Here are the steps I took to get that working:
  - Install Arduino IDE, install ESP8266 library.
  - Configured a sketch from the ESP8266 library called "ArduinoOTA/BasicOTA"
    This allows the ESP8266 to be updated over the air using the ESP8266's wireless adapter.
    You will need to configure the sketch with your desired wireless network SSID and password
    as well as an OTA password to update the ESP8266.
  - Be sure you select the right Board in the Arduino IDE environment. If you're using an ESP8266
    Huzzah like I am, it's an option in the "Tools->Board:" menu after the Arduino boards.
  - Export the compiled binary. You can use this with the esptool.py utility to flash the
    ESP8266 Huzzah when your press and hold down the GPIO 0 button, then pressing the reset
    button. Doing so, puts the ESP8266 Huzzah into programming mode.
  - You can find the esptool.py utility here:
    https://github.com/themadinventor/esptool
    Alternatively, you can use pip to install it.
    Simply run "sudo pip install esptool"
  - After transferring the compiled binary from the "Documents/arduino/basicOTA" to the Raspberry Pi
    you can then use the esptool.py utility to update the firmware with the ESP8266 Huzzah in
    programming mode.
  - The exact command is this:
     esptool.py -b 115200 -p /dev/ttyAMA0 write_flash -fm dio -fs 32m 0x00000 <file>.bin
  - This will upload the binary to the ESP8266 Huzzah. When done, press the reset button.
  - Within the Arduino IDE, you should now see the ESP8266 Huzzah in the "Tools->Port".
  - You can now make changes to agi-esp8266.ino and upload it to the ESP8266 Huzzah remotely.
