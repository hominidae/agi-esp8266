// ESP8266 Demo webserver featuring ArduinoOTA
// Import ESP8266 related libraries
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Import MQTT Library
#include <PubSubClient.h>

// Import DHT sensor library and define variables
#include <DHT.h>
#define DHTTYPE DHT22
#define DHTPIN 13
// You need to pass a third parameter for the DHT threshold cycle.
// This is because the ESP8266 is faster than the 16Mhz Arduino.
DHT dht(DHTPIN, DHTTYPE, 11); // The "11" works with the ESP8266

// Import Adafruit TSL2561 sensor library
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

// Declare a global variables for ArduinoOTA
#define WLAN_SSID "***" // Put your wifi SSID here
#define WLAN_PASS "***" // Put your wifi password here
#define AGIhostname "agi-esp8266" // Change this to whatever you'd like
#define OTApass "***" // Change your AndroidOTA pass here

// Declare PubSub variables
const char* mqtt_user = "mosquitto"; // Change to your mqtt username
const char* mqtt_password = "mosquitto"; // Change to your mqtt password
const char* mqtt_server = "192.168.1.1"; // Change to your MQTT server
const char* humidity_topic = "sensor/humidity";
const char* temperature_topic = "sensor/temperature";
const char* luminosity_topic = "sensor/luminosity";
WiFiClient espClient;
PubSubClient client(espClient);
long lastReconnectAttempt = 0;

// Declare variables for sensors
float humidity, temp;                    // values read by DHT22
float lumen;                             // value read by TSL
unsigned long previousMillis = 0;        // Timing related for sensor reading
const long interval = 10000;              // Timing related for sensor reading

ESP8266WebServer server (80);             // Set the webserver port
const int led = 1;                        // Set the initial state for the LED GPIO pin
String webString="";                      // declaring an empty string for later

void setup_wifi() {                       // Connect to wifi when called
  delay(10);
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(WLAN_SSID);  
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);                         // wait 5 seconds
    ESP.restart();                       // Restart ESP if not connected
  }
  Serial.println("Connected!");
  Serial.print("Server IP address: ");
  Serial.println(WiFi.localIP());
}

// Function for handling temperature
void gettemphumid() {
  temp = dht.readTemperature();
  humidity = dht.readHumidity();
  if (isnan(temp) || isnan(humidity)){
    Serial.println("Failed to read from DHT sensor!");
  }
}

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
    // Once connected, publish a value
    client.publish("DEBUG","ESP8266 Connected");
  }
  return client.connected();
}

void postsensor() {
    gettemphumid();
    int luminosity = 0;
    luminosity = getlumen();
    client.publish(temperature_topic, String(temp).c_str(), true);
    client.publish(humidity_topic, String(humidity).c_str(), true);
    client.publish(luminosity_topic, String(luminosity).c_str(), true);
}

void displaySensorDetails() {            // Print TSL sensor details
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

void handleRoot() {                       // This is the web server root directory visit function
  digitalWrite(led, 1);                   // Here, we're switching the LED GPIO pin to off
  gettemphumid();
  lumen = getlumen();
  webString="Temperature: "+String((int)temp)+" C"+"\n"+"Humidity: "+String((int)humidity)+" %"+"\n"+"Luminosity: "+String((int)lumen)+" lux";
  server.send(200, "text/plain", webString);
  digitalWrite(led, 0);                   // Here, we turn the LED GPIO pin back on
}

void handleNotFound(){                    // web server 404 not found handling
  digitalWrite(led, 0);                   // Turn LED on
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 1);                   // Turn LED back off
}

void setup() {
  pinMode(led, OUTPUT);                   // Set led pin to output
  Serial.begin(9600);                     // Start serial connection
  Serial.println("");
  Serial.println("Booting...");
  ArduinoOTA.setHostname(AGIhostname);      // Set hostname
  ArduinoOTA.setPassword(OTApass);      // Set ArduinoOTA password
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();                     // Start AndroidOTA
  setup_wifi();
  Serial.print(AGIhostname);                 // print hostname
  Serial.println(" Weather Server");       // print text
  if (MDNS.begin("esp2866")) {
    Serial.println("MDNS responder started");
  }
  // Print off TSL2561 sensor details
  displaySensorDetails();
  configureSensor();
 
  server.on("/", handleRoot);            // Call rootdir function on visit to "/"
  server.onNotFound(handleNotFound);     // Call 404 not found function on any undeclared request
  server.begin();                        // Start web server
  Serial.println("HTTP server started"); // Print a debug message to serial
  client.setServer(mqtt_server, 1883);
  delay(1500);
  lastReconnectAttempt = 0;
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
    client.loop();
  }
  unsigned long currentMillis = millis(); // Wait 10 seconds between measurements
  if(currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
    postsensor();
  }
}
