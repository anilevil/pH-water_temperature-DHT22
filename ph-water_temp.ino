#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "ArduinoJson.h"
#include <ThingsBoard.h>
#include <SPI.h> 


// Update these with values suitable for your network.
const char* ssid ="";                           //wifi ssid
const char* password = "";                      //wifi password 
const char* mqtt_server = "";                   //mqtt server ip
#define mqtt_port                               //mqtt port number
#define MQTT_USER ""                            //mqtt user name
#define MQTT_PASSWORD ""
#define MQTT_SERIAL_PUBLISH_CH ""               //mqtt publish path
#define MQTT_SERIAL_RECEIVER_CH ""              //mqtt recieve path

#define ONE_WIRE_BUS 34                         //water temperature data pin to esp pin
#define DHTTYPE DHT22
#define DHTPIN 14                               //DHT22 data pin to esp pin
#define PH_PIN 35                               //pH data pin to esp pin
float voltage;
DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float Celcius=0;
float Fahrenheit=0;
DynamicJsonDocument doc(1024) ;
DynamicJsonDocument ht(512);
char charBuf[150] ;
int i=0,j=0;
int sensorValue = 0; 
unsigned long int avgValue; 
float b;
float phValue;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      Serial.println("Parsing start: ");
 
       //char JSONMessage[] = " {\"temp1\": \"111\", }"; //Original message
         //StaticJsonDocument<256> doc;
        //JsonObject& parsed = doc.parseObject(JSONMessage); //Parse message
        ///JsonObject root;
        //root["co"]=111;
        //Serial.println(root["co"]);
//        JsonObject& parsed = deserializeJson(doc, JSONMessage);


 
      client.publish("v1/devices/me/telemetry","{\"co\":111}");
      // ... and resubscribe
      client.subscribe(MQTT_SERIAL_RECEIVER_CH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte *payload, unsigned int length) {
    Serial.println("-------new message from broker-----");
    Serial.print("channel:");
    Serial.println(topic);
    Serial.print("data:");  
    Serial.write(payload, length);
    Serial.println();
}

void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
  Serial.setTimeout(500);// Set time out for 
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
      Serial.println(F("DHTxx test!"));
  //pH_value.begin();
  dht.begin();
   sensors.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
client.loop();
 

   doc["humiditya"]=dht22().substring(0,5);
   doc["tempa"] = dht22().substring(6,11);
   doc["watertemp"] =  water_temp();//== NULL ?float(0):water_temp() ;
   doc["ph"] = pH_value();//== NULL ?float(0):pH_value() ; 
     serializeJson(doc,charBuf);
    serializeJson(doc,Serial);
  client.publish("v1/devices/me/telemetry",charBuf);
  Serial.println(charBuf);
 // client.publish("v1/devices/me/telemetry",charBuf);
  
  delay(1000);
  Serial.println("Sent");
}

String dht22()
{
 
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) )
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return "None";
  }

  Serial.print(F("Humidity: "));
 Serial.println(h);
 Serial.print(F("Temperature: "));
 Serial.println(t);
 

 return String(String(h)+"-"+String(t)+"-");
}

float water_temp()
{
   sensors.requestTemperatures();
  Celcius=sensors.getTempCByIndex(0);
  Fahrenheit=sensors.toFahrenheit(Celcius);
  return float(Celcius);
}

float pH_value()
{
  voltage = analogRead(PH_PIN);
  delay(10);
  phValue = 0.01875*voltage - 0.55625;
  Serial.println(phValue);
  return float(phValue);
}
