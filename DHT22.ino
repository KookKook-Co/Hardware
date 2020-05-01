#include "WiFi.h"
#include <PubSubClient.h>

#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); 

#define DHTPIN 17     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 11
DHT dht(DHTPIN, DHTTYPE);


int ledState = LOW;
long now=millis();
long lastMeasure=0;

const char* topicT ="/1/temperature";
const char* topicH ="/1/humidity";



// Update these with values suitable for your network.

const char* ssid =        "WiFi Name";
const char* password =    "WiFi Password";
const char* mqtt_server = "your mqtt server";   /// example 192.168.0.19

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  lcd.init();
  lcd.backlight();

  dht.begin();
    
}

void setup_wifi() {
  delay(10);// We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");// Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);// Wait 5 seconds before retrying
    }
  }
}
void loop() {
//  Serial.println(client.connected());
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) ) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    static char temperatureTemp[7];
    dtostrf(t, 6, 2, temperatureTemp);
    static char humidityTemp[7];
    dtostrf(h, 6, 2, humidityTemp);
    client.publish(topicT, temperatureTemp);
    client.publish(topicH, humidityTemp);
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t Temperature: ");
    Serial.print(t);
    Serial.println(" *C ");

    lcd.setCursor(0, 0);
    lcd.print("Humidity: ");
    lcd.print(h);
    lcd.setCursor(0,1);
    lcd.print("Temp: ");
    lcd.print(t);

  }
}