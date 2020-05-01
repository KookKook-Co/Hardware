#include "WiFi.h"
#include <PubSubClient.h>

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); 

#define RL 47  //The value of resistor RL is 47K
#define m -0.263 //Enter calculated Slope 
#define b 0.42 //Enter calculated intercept
#define Ro -10.5 //Enter found Ro value
#define MQ_sensor 34 //Sensor is connected to A4

const char* topicA ="/1/ammonia";

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
  float VRL; //Voltage drop across the MQ sensor
  float Sensor_R; //Sensor resistance at gas concentration 
  float ratio; //Define variable for ratio

  Serial.println(client.connected());
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  
  if (now - lastMsg > 10000) {
    lastMsg = now;
    VRL = analogRead(MQ_sensor)*(5.0/1023.0);
    
    if(isnan(VRL)){
      Serial.println("Failed to read from Ammonia sensor!");
      return;
    }
  
    Sensor_R = ((5.0*RL)/VRL)-RL; //Use formula to get Rs value
    ratio = Sensor_R/Ro;  // find ratio Rs/Ro
 
    float ppm = pow(10, ((log10(ratio)-b)/m)); //use formula to calculate ppm

    Serial.print("NH3 (ppm) = "); //Display a ammonia in ppm
    Serial.print(ppm);
    Serial.print("     Voltage = "); //Display a intro message 
    Serial.println(VRL);

    double percentage = ppm / 10000; //Convert to percentage
    Serial.println(percentage);

    if(isnan(ppm)){
      ppm = 0.0;
    }

    static char ammonia[7];
    dtostrf(ppm, 6, 2, ammonia);
    client.publish(topicA, ammonia);
    Serial.print("Sent");
    lcd.setCursor(0, 0);
    lcd.print("ppm= ");
    lcd.print(ppm);
    //lcd.setCursor(0,1);
    //lcd.print("Voltage = ");
    //lcd.print(VRL);
    delay(2000);
 
  }
   
}