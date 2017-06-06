Project: AGI-ESP8266

Complete:
 - Test WiFi Connectivity
 - Test DHT22 Temp/Humidity
 - Test TSL2561
 - Test MQTT logging
 - Test Deepsleep function
 - Test DS18B20 Temperature Sensor
 - Test DHT22, TSL2561 and DS18B20 over multiple sleep cycles long term

Incomplete:
 - Test the Homie-esp8266 framework
  > Evaluate battery life efficiency
   . Deep sleep functionality?
   . Static WiFi settings to reduce 802.11 negotiation on wake from deep sleep
  > MQTT Over the Air updates sent via a retrieved flag from MQTT broker
  > Submit various sensor configurations to homie-esp8266 project
 - Test Soil Capacitance Measurement design
  > DFRobot Analog Soil Moisture Sensor (Corrosion resistant)
  > Generic DIY capacitive soil moisture sensor
 - Design eagle schematic for agi-esp8266
 - Order trial run of protoype PCBs online
 - Assemble the prototype evaluation candidates
  > Ship for evaluation alongside window garden kit
