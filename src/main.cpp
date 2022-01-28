#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 4
#define DHTTYPE DHT11 

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

const char *SSID = "WIFINAME";
const char *PWD = "WIFIPASS";

WebServer server(8080);

// JSON data buffer
StaticJsonDocument<250> jsonDocument;
char buffer[250];

// env variable
float temperature = 0.00;
float humidity = 0.00;

void connectToWiFi() {
  Serial.print("___________");
  Serial.print("Connecting to ");
  Serial.println(SSID);
  Serial.println(PWD);
  
  WiFi.begin(SSID, PWD);
  
  // Wait for wifi to be connected
  uint32_t notConnectedCounter = 0;
  while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.println("Wifi connecting...");
      notConnectedCounter++;
      if(notConnectedCounter > 150) { // Reset board if not connected after 5s
          Serial.println("Resetting due to Wifi not connecting...");
          ESP.restart();
      }
  }
  Serial.print("Wifi connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void create_json(char *tag, float value, char *unit) {  
  jsonDocument.clear();  
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}

void add_json_object(char *tag, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit; 
}


void getTemperature() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  temperature = event.temperature;
  Serial.println("Get temperature");
  create_json("temperature", temperature, "°C");
  server.send(200, "application/json", buffer);
}

void getHumidity() {
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  humidity = event.relative_humidity;
  Serial.println("Get humidity");
  create_json("humidity", humidity, "%");
  server.send(200, "application/json", buffer);
}

void getEnv() {
  Serial.println("Get env");
  jsonDocument.clear();
  add_json_object("temperature", temperature, "°C");
  add_json_object("humidity", humidity, "%");
  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}

void setup_routing() {	 	 
  server.on("/temperature", getTemperature);	 	 
  server.on("/humidity", getHumidity);	 	 
  server.on("/env", getEnv);	 	 
 	  	 
  // start server	 	 
  server.begin();	 	 
}

void handlePost() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  server.send(200, "application/json", "{}");
}


void setup() {
  Serial.begin(9600);
  dht.begin();
  sensor_t sensor;
  delayMS = sensor.min_delay / 1000;

  connectToWiFi();

  setup_routing();
}

void loop() {
server.handleClient();
}