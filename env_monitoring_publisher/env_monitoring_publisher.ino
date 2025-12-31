#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11
#define MQ2PIN 34

const char* ssid = "TEST123";
const char* password = "12345678";

const char* mqtt_server = "10.49.201.155"; 
const int mqtt_port = 1883;
const char* topic = "environment/data";

WiFiClient espClient;
PubSubClient client(espClient);
HTTPClient http;

DHT dht(DHTPIN, DHTTYPE);

void setup_wifi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Publisher")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  analogReadResolution(12); //?
  analogSetAttenuation(ADC_11db); //?
}

void loop() {

  if (!client.connected())
    reconnect();

  client.loop();

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int mq2 = analogRead(MQ2PIN);

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("DHT read failed!");
    delay(2000);
    return;
  }

  int gasStatus = 0;
  if (mq2 > 300 && mq2 <= 600) gasStatus = 1;
  else if (mq2 > 600) gasStatus = 2;

  int climate = 0;
  if (temperature > 35) climate = 1;
  else if (temperature < 15) climate = 2;
  if (humidity > 70) climate += 10;

  Serial.println("-----------------------");
  Serial.println("Temperature: " + String(temperature));
  Serial.println("Humidity: " + String(humidity));
  Serial.println("MQ2: " + String(mq2));
  Serial.println("Gas Status: " + String(gasStatus));
  Serial.println("Climate Code: " + String(climate));

  String payload = "{";
  payload += "\"temperature\":" + String(temperature) + ",";
  payload += "\"humidity\":" + String(humidity) + ",";
  payload += "\"gas\":" + String(mq2) + ",";
  payload += "\"gasStatus\":" + String(gasStatus) + ",";
  payload += "\"climate\":" + String(climate);
  payload += "}";

  client.publish(topic, payload.c_str());

  Serial.println("MQTT Published:");
  Serial.println(payload);

  delay(20000); 
}
