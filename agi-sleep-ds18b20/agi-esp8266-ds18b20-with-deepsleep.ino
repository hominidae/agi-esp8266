// agi-esp8266 Deep Sleep prototype
// Import esp8266 related libraries
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Import MQTT Library
#include <PubSubClient.h>

// Import DHT sensor library and define variables
#include <DHT.h>
#include <DHT_U.h>
#define DHTTYPE DHT22
#define DHTPIN 13
// You need to pass a third parameter for the DHT threshold cycle.
// This is because the ESP8266 is faster than the 16Mhz Arduino.
DHT dht(DHTPIN, DHTTYPE, 11); // The "11" works with the ESP8266

// Import Adafruit TSL2561 library
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

// Import Dallas DS18B20 libraries
#include <OneWire.h> // Import onewire library
#include <DallasTemperature.h> // Import Dallas library
#define ONE_WIRE_BUS 14 // Set GPIO 14 to OneWire bus
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance
DallasTemperature sensors(&oneWire); // Pass oneWire reference to Dallas
DeviceAddress insideThermometer; // Arrays to hold device address

// Declare a few wifi variables
#define WLAN_SSID "**" // Put WiFi SSID here
#define WLAN_PASS "***" // Put WiFi Password here

// Declare a few PunSub MQTT variables
const char* mqtt_user = "mosquitto"; // Username
const char* mqtt_password = "mosquitto"; // Password
const char* mqtt_server = "192.168.1.1"; // MQTT Server
const char* humidity_topic = "sensor/humidity";
const char* temperature_topic = "sensor/temperature";
const char* luminosity_topic = "sensor/luminosity";
WiFiClient espClient;
PubSubClient client(espClient);
long lastReconnectAttempt = 0;

// Declare variables for sensors
float humidity, temp; // Values read by DHT22
float lumen; // Values read by TSL2561
float justtemp; // Value read by Dallas
const int led = 1; // Set the initial value for the GPIO LED pin
unsigned long previousMillis = 0; // Set default timing related variable

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {\
  	Serial.println("Connected failed! Rebooting...");
  	delay(5000); // Wait 5 seconds
  	ESP.restart(); // Restart ESP8266 if not connected
  }
  Serial.println("Connected!");
  Serial.print("ESP8266 IP Address: ");
  Serial.println(WiFi.localIP());
}

// Function for handling temperature humidity
void gettemphumid() {
  delay(1000);
  temp = dht.readTemperature();
  humidity = dht.readHumidity();
}

// Function for dallas temperature library
float justgettemp(DeviceAddress deviceAddress) {
	float justtemp = sensors.getTempC(deviceAddress);
	if (isnan(justtemp)) {
		Serial.println("Failed to read DS18B20 temperature!");
	}
  return justtemp;
}

// Configuration for TSL2561
void configureSensor() {
  tsl.enableAutoRange(true);
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);
}

int getlumen() {
  int lumenvalue = 0;
  sensors_event_t event;
  tsl.getEvent(&event);
  lumenvalue = event.light;
  return lumenvalue;
}


boolean reconnect() {
  if (client.connect("AGI-ESP8266", mqtt_user, mqtt_password)) {
  	// Once connected, publish a debug value
  	client.publish("DEBUG", "ESP8266 Connected!");
  }
  return client.connected();
}

void postsensor() {
  gettemphumid();
  justgettemp(insideThermometer);
  int luminosity = 0;
  luminosity = getlumen();
  client.publish(temperature_topic, String(temp).c_str(), true);
  client.publish(temperature_topic, String(justtemp).c_str(), true);
  client.publish(humidity_topic, String(humidity).c_str(), true);
  client.publish(luminosity_topic, String(luminosity).c_str(), true);
}

// Function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress) {
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
}

void setup() {
  pinMode(led, OUTPUT);	// Set LED pin to output
  Serial.begin(9600);	// Start serial connection
  Serial.println("");	// Print empty space
  Serial.println("Booting...");
  setup_wifi();		// Setup the wifi connection
  Serial.println(ESP.getResetReason()); // Get the reason for reset
  Serial.println("agi-esp8266 started!");
  Serial.println("Initializing sensors!");
  configureSensor();	// Call the configureSensor function
  digitalWrite(14, HIGH);
  pinMode(14, OUTPUT);
  sensors.begin(); // Start oneWire sensors search
  Serial.print("OneWire Devices: ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" found.");
  if (!sensors.getAddress(insideThermometer, 0)) {
	Serial.println("Unable to find address for device 0");
  }
  sensors.requestTemperatures(); // Send command to request temperatures
  client.setServer(mqtt_server, 1883);
  lastReconnectAttempt = 0;
}

void loop() {
  if (!client.connected()) {
  	long now = millis();
	if (now - lastReconnectAttempt > 5000) {
		lastReconnectAttempt = now; // Attempt to reconnect
		if (reconnect()) {
		lastReconnectAttempt = 0;
	}
  }
  } else {
	// The client is connected!
	Serial.println("Connected to MQTT server!");
 	client.loop();
	}
  delay(1000);
  Serial.println("Posting Sensors to MQTT server!");
  postsensor(); // Post sensors via MQTT
  // Test Dallas OneWire Temperature Sensor!
  printTemperature(insideThermometer);
  delay(5000); // Wait 5 seconds for sensors to post before entering deepsleep mode
  WiFi.disconnect(); // Disconnect wifi so it will reconnect on start
  ESP.deepSleep(300000000, WAKE_RF_DEFAULT); // sleep for 5 minutes
}
