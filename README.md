# What is agi-esp8266?

  agi-esp8266 is intended to provide a simple, straight forward way for end users to measure environmental data
 affecting their plants. It's the instrumentation part of a planned system for growing plants on window sills
 and in greenhouses and cold frames.

  The software is currently fairly barebones, but some functionality is currently working. You can setup a simple
 MQTT broker on a Linux system which logs available data from several sensors being published to it.

# How do I use this?

 The src sub-directory contains three versions at the moment:
  nosleep - Install on an esp8266 to publish sensor data and host a web server on the esp8266 with the sensor values
  sleep - Install on an esp8266 to utilize deep sleep function to conserve battery power
  sleep-ds18b20 - Install on esp8266 to utilize deep sleep and connect to a ds18b20 i2c temperature sensor

 These are general instructions for if you have an FTDI cable. I don't, so I used my Raspberry Pi to set the
 ESP8266 Huzzah up. Skip to the bottom for instructions on doing that.

# Simple setup with an FTDI USB cable (esp8266 connected directly to PC)

 1. First, install the Arduino IDE and configure it to use the ESP8266 repository.

 Since other documentation explains how to do this better, here is a link to the
 official esp8266 arduino repository:

 https://github.com/esp8266/Arduino

 2. Next, you're going to need to install the Arduino IDE libraries used:

 (Note: Usually, this is your documents directory within "Arduino/libraries")

 These are the libraries you need to install to your Arduino IDE:

 - Adafruit Unified Sensor library: https://github.com/adafruit/Adafruit_Sensor

 - DHT11/DHT22: https://github.com/adafruit/DHT-sensor-library

 - TSL2561: https://github.com/adafruit/Adafruit_TSL2561

 - PubSubClient: https://github.com/knolleary/pubsubclient

 3. Third, you're going to need to flash your ESP8266. Personally, I did this using a
 serial connection from my Raspberry Pi and the esptool.py utility.

 However, if you are connected directly to your ESP8266 Huzzah (or other ESP8266) through
 an FTDI adapter all you need is to select your board and your port and upload as you normally
 would with an Arduino board.

 The exact connections for connecting to your esp8266 microcontroller will vary with which
 version of the esp8266 you've picked up. I have an Adafruit Huzzah and a Wemos D1 Mini, so most
 of my instructions will be directed at those.

# Alright, I have the Arduino IDE and libraries setup, what next?

 The current code, which is in the src/ directory has several versions of the code I've written and pieced together
 from material available online.

 There are currently versions which support ArduinoOTA which is a library to remotely flash the firmware on an esp8266
 over an existing wifi network.

 Pick your version, configure the code, compile and upload it using whichever method you've chosen.

 Note: If you're using ArduinoOTA, you'll need to do a bit of reading on how to use it. The "nosleep" code has variables
 which you will need to configure to get that working.

 Next up, we'll setup simple logging with MQTT.

# Logging with mosquitto and some basic command line stuff

 If you have a Raspberry Pi, then use the following command to install mosquitto, which is an MQTT
 broker/server.

 Simply run this command on your Raspbian or Ubuntu configuration:

 sudo apt-get install mosquitto

 Then edit /etc/mosquitto/mosquitto.conf to the configuration you want.

 Finally, run sudo mosquitto -v to run mosquitto with verbose output.

 I've yet to write any software to subscribe and parse data so I'm using these commands for logging in a terminal to flat text files:
 
 mosquitto_sub -v -t sensor/temperature | xargs -d$'\n' -L1 sh -c 'date "+%D %T.%3N $0"' > temperature &

 mosquitto_sub -v -t sensor/humidity | xargs -d$'\n' -L1 sh -c 'date "+%D %T.%3N $0"' > humidity &

 mosquitto_sub -v -t sensor/luminosity | xargs -d$'\n' -L1 sh -c 'date "+%D %T.%3N $0"' > luminosity &

 They subscribe to "sensor/<sensor>" events being sent to the MQTT server. They output the events to
 the temperature, humidity and luminosity files with an appended timestamp.

 Example output in the files looks like this:
 10/04/16 23:31:40.645 sensor/temperature 20.80 <--- 20 degree's celsius
 10/04/16 23:31:50.646 sensor/temperature 20.70
 10/04/16 23:32:00.650 sensor/temperature 20.70
 10/04/16 23:32:10.648 sensor/temperature 20.80
 10/04/16 23:32:20.647 sensor/temperature 20.70

 That's it! [More to come]
 
# How do I use a Raspberry Pi to do this?

 Here are the steps I took to get that working:
  - Install Arduino IDE, install ESP8266 library.
  - Configured a sketch from the ESP8266 library called "ArduinoOTA/BasicOTA"
    This allows the ESP8266 to be updated over the air using the ESP8266's wireless adapter.
    You will need to configure the sketch with your desired wireless network SSID and password
    as well as an OTA password to update the ESP8266.
  - Be sure you select the right Board in the Arduino IDE environment. If you're using an ESP8266
    Huzzah like I am, it's an option in the "Tools->Board:" menu after the Arduino boards.
  - Export the compiled binary. You can use this with the esptool.py utility to flash the
    ESP8266 Huzzah. Copy the exported binary to your Raspberry Pi using the sftp protocol.
  - I used esptool.py to flash my ESP8266. You can find the esptool.py utility here:
    https://github.com/themadinventor/esptool
    Alternatively, you can use pip to install it.
    Simply run "sudo pip install esptool"
  - After transferring the compiled binary from the "Documents/arduino/BasicOTA" to the Raspberry Pi
    you can then use the esptool.py utility to update the firmware with the ESP8266 Huzzah in
    programming mode. To put the ESP8266 into programming mode, you need to press the GPIO 0
    button and then the reset button. The red LED should be dimly lit indicating you're in programming
    mode.
  - Next, you need to connect the Raspberry Pi to your ESP8266. The GPIO on the Raspberry Pi has four
    pins you're going to use. UART_TXD, UART_RXD, GND and 5V. These connect to your Raspberry Pi in the
    follow way:
    Pin 10 on the RPi to the TXD pin on the Huzzah.
    Pin 8 on the RPi to the RXD pin on the Huzzah.
    Pin 6 to the GND pin on the Huzzah.
    Pin 4 or Pin 2 to the V+ pin on the Huzzah.
    Congrats! Your Pi is ready to do serial communications.
  - Just a couple more steps. You need to disable the Raspberry Pi's default behaviour for the serial port.
  - On Raspbian Jessie, the method normally used to disable a serial tty is different.
    Raspbian Jessie switched from using /etc/inittab to using systemd.
    Use the following two commands to turn off and disable the serial terminal from running by default.
    sudo systemctl stop serial-getty@ttyAMA0.service
    sudo systemctl disable serial-getty@ttyAMA0.service
    Next up, you can use the esptool.py utility to flash any firmware you'd like.    
  - The exact command to update the firmware on the ESP8266 (once you've put it into programming mode) is this:
     esptool.py -b 115200 -p /dev/ttyAMA0 write_flash -fm dio -fs 32m 0x00000 <file>.bin
    Obviously, replace <file>.bin with the file you're using. In this case, the BasicOTA.ino.bin you've copied
    from your Arduino IDE project directory.
  - This will upload the binary to the ESP8266 Huzzah. When done, press the reset button.
  - Within the Arduino IDE, you should now see the ESP8266 Huzzah in the "Tools->Port".
  - You can now make changes to agi-esp8266.ino and upload it to the ESP8266 Huzzah remotely.
  - You can also install minicom, which will allow you to listen to the serial output from the ESP8266.
    To install that, run sudo apt-get install minicom
  - The command to listen to the serial output is as follows:
    minicom -b 9600 -D /dev/ttyAMA0
    I used a value of 9600 baud, but you can change that to whatever you'd like.
