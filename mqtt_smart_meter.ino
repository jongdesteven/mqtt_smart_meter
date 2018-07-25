/*
MQTT based Smart Meter for Home Assistant by Steven (2018)
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MQTT_DEBUG

// Update these with values suitable for your network.
const char* ssid = "SSID";
const char* password = "PASSOWORD";
const char* mqtt_server = "MQTT-SERVER-IP";
const char* mqtt_debug_topic = "debug/smartmeter/debug"

WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];

unsigned long reconnect_at_time = 0;

void print_to_mqtt(String message) {
#ifdef MQTT_DEBUG
  sendMQTTMessage(mqtt_debug_topic, message);
#endif
}

void setup_wifi() {
   delay(100);
  // We start by connecting to a WiFi network
    print_to_mqtt("Connecting to ");
    print_to_mqtt(ssid);
    WiFi.mode(WIFI_STA);  //station only, no accesspoint
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      print_to_mqtt(".");
    }
  randomSeed(micros());
  print_to_mqtt("");
  print_to_mqtt("WiFi connected");
  print_to_mqtt("IP address: ");
  print_to_mqtt(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  print_to_mqtt("Command received at: ");
  print_to_mqtt(topic);
  String mqttTopic = "";
  String message = "";
  if(strcmp(topic, mqtt_ring_sub) == 0) {

  }
} //end callback

void reconnect() {
  // dont Loop until we're reconnected
  print_to_mqtt("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  // Attempt to connect
  //if you MQTT broker has clientID,username and password
  //please change following line to    if (client.connect(clientId,userName,passWord))
  if (client.connect(clientId.c_str()))
  {
    print_to_mqtt("connected");
   //once connected to MQTT broker, subscribe command if any
    //client.subscribe(mqtt_ring_sub);
  } else {
    print_to_mqtt("failed, rc=");
    print_to_mqtt(client.state());
  }
} //end reconnect()

void sendMQTTMessage(String topic, String message) {
    char msg[50];
    char tpc[50];
    topic.toCharArray(tpc, 50);
    message.toCharArray(msg,50);
    print_to_mqtt(tpc);
    print_to_mqtt(msg);
    //publish sensor data to MQTT broker
    client.publish(tpc, msg);
    print_to_mqtt(" ..sent");
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  //print_to_mqtt("reading:"+reading);

  if (!client.connected() && millis() > reconnect_at_time) {
    reconnect();
    //failed to reconnect?
    if (!client.connected()) {
      //try again 6seconds later
      print_to_mqtt("Reconnect failed, trying again in 6 seconds");
      reconnect_at_time = millis()+6000;
    }
  }
  client.loop();

}
