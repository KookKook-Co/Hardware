#include "WiFi.h"
#include <PubSubClient.h>

const int windPin = 34; //windPin refers to ESP32 GPIO 25

 //lcd import
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); 

const char* topicW ="/1/windspeed";

const char* ssid =        "WiFi Name";
const char* password =    "WiFi Password";
const char* mqtt_server = "your mqtt server"; 

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


void setup() {
  // wind speed sensor code starts here!!!
  pinMode(windPin, INPUT); //initialize digital pin windPin as an INPUT
  Serial.begin(9600);


  

  int sensorValue = analogRead(windPin);
  Serial.println(sensorValue);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //lcd code starts here!!!
  lcd.init(); // initialize LCD                      
  lcd.backlight(); // turn on LCD backlight
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
  long now = millis();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (now - lastMsg > 10000){
    lastMsg = now;
    int sensorValue = analogRead(windPin);
    float outVoltage = sensorValue * (5.0/1023.0);
    float level = 2*outVoltage;
    Serial.println(sensorValue);

    if(isnan(level)){
      Serial.println("Failed to read from Ammonia sensor!");
      return;
    }

    static char windSensor[7];
    dtostrf(level, 6, 2, windSensor);
    client.publish(topicW, windSensor);

    lcd.setCursor(0, 0);
    lcd.print("Speed: ");
    lcd.print(level);
    lcd.print("m/s");
  
    lcd.setCursor(0, 1);
    lcd.print("Voltage: ");
    lcd.print(outVoltage);
    lcd.print("V");
    delay(5000);
  }