/*
MQTT based Smart Meter for Home Assistant by Steven (2018)
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <dsmr.h>

#define DEBUG

void sendMQTTMessage(String topic, String message);

using MyData = ParsedData<
  /* String */ identification,
  /* String */ p1_version,
  /* String */ timestamp,
  /* String */ equipment_id,
  /* FixedValue */ energy_delivered_tariff1,
  /* FixedValue */ energy_delivered_tariff2,
  /* FixedValue */ energy_returned_tariff1,
  /* FixedValue */ energy_returned_tariff2,
  /* String */ electricity_tariff,
  /* FixedValue */ power_delivered,
  /* FixedValue */ power_returned,
  /* FixedValue */ electricity_threshold,
  /* uint8_t */ electricity_switch_position,
  /* uint32_t */ electricity_failures,
  /* uint32_t */ electricity_long_failures,
  /* String */ electricity_failure_log,
  /* uint32_t */ electricity_sags_l1,
  /* uint32_t */ electricity_sags_l2,
  /* uint32_t */ electricity_sags_l3,
  /* uint32_t */ electricity_swells_l1,
  /* uint32_t */ electricity_swells_l2,
  /* uint32_t */ electricity_swells_l3,
  /* String */ message_short,
  /* String */ message_long,
  /* FixedValue */ voltage_l1,
  /* FixedValue */ voltage_l2,
  /* FixedValue */ voltage_l3,
  /* FixedValue */ current_l1,
  /* FixedValue */ current_l2,
  /* FixedValue */ current_l3,
  /* FixedValue */ power_delivered_l1,
  /* FixedValue */ power_delivered_l2,
  /* FixedValue */ power_delivered_l3,
  /* FixedValue */ power_returned_l1,
  /* FixedValue */ power_returned_l2,
  /* FixedValue */ power_returned_l3,
  /* uint16_t */ gas_device_type,
  /* String */ gas_equipment_id,
  /* uint8_t */ gas_valve_position,
  /* TimestampedFixedValue */ gas_delivered,
  /* uint16_t */ thermal_device_type,
  /* String */ thermal_equipment_id,
  /* uint8_t */ thermal_valve_position,
  /* TimestampedFixedValue */ thermal_delivered,
  /* uint16_t */ water_device_type,
  /* String */ water_equipment_id,
  /* uint8_t */ water_valve_position,
  /* TimestampedFixedValue */ water_delivered,
  /* uint16_t */ slave_device_type,
  /* String */ slave_equipment_id,
  /* uint8_t */ slave_valve_position,
  /* TimestampedFixedValue */ slave_delivered
>;

/**
 * This illustrates looping over all parsed fields using the
 * ParsedData::applyEach method.
 *
 * When passed an instance of this Printer object, applyEach will loop
 * over each field and call Printer::apply, passing a reference to each
 * field in turn. This passes the actual field object, not the field
 * value, so each call to Printer::apply will have a differently typed
 * parameter.
 *
 * For this reason, Printer::apply is a template, resulting in one
 * distinct apply method for each field used. This allows looking up
 * things like Item::name, which is different for every field type,
 * without having to resort to virtual method calls (which result in
 * extra storage usage). The tradeoff is here that there is more code
 * generated (but due to compiler inlining, it's pretty much the same as
 * if you just manually printed all field names and values (with no
 * cost at all if you don't use the Printer).
 */
 
//struct Printer {
//  template<typename Item>
//  void apply(Item &i) {
//    if (i.present()) {
//      Serial1.print(Item::name);
//      Serial1.print(F(": "));
//      Serial1.print(i.val());
//      Serial1.print(Item::unit());
//      Serial1.println();
//    }
//  }
//};

// Update these with values suitable for your network.
const char* ssid = "De Jong Airport";
const char* password = "Appeltaart";
const char* mqtt_server = "raspi";
const char* mqtt_debug_topic = "debug/smartmeter/debug";
const char* mqtt_topic_prefix = "home/smartmeter/";

struct Printer {
  template<typename Item>
  void apply(Item &i) {
    if (i.present()) {
        sendMQTTMessage((mqtt_topic_prefix + String(Item::name)), String(i.val()));
      Serial1.print(Item::name);
      Serial1.print(F(": "));
      Serial1.print(i.val());
      Serial1.print(Item::unit());
      Serial1.println();
    }
  }
};

P1Reader reader(&Serial, 2); //RTS connected to D2?

WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];
unsigned long last; //used by P1Reader

unsigned long reconnect_at_time = 0;

void print_to_mqtt(String message) {
#ifdef DEBUG
  sendMQTTMessage(mqtt_debug_topic, message);
  Serial1.println("debug: " + message);
#endif
}

void setup_wifi() {
   delay(100);
  // We start by connecting to a WiFi network
    Serial1.println("Connecting to ");
    Serial1.println(ssid);
    WiFi.mode(WIFI_STA);  //station only, no accesspoint
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      //Serial1.println(".");
    }
  randomSeed(micros());
  Serial1.println("");
  Serial1.println("WiFi connected");
  Serial1.println("IP address: ");
  Serial1.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial1.println("Command received at: ");
  Serial1.println(topic);
  String mqttTopic = "";
  String message = "";
  //if(strcmp(topic, mqtt_ring_sub) == 0) {
  //}
} //end callback

void reconnect() {
  // dont Loop until we're reconnected
  Serial1.println("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  // Attempt to connect
  //if you MQTT broker has clientID,username and password
  //please change following line to    if (client.connect(clientId,userName,passWord))
  if (client.connect(clientId.c_str()))
  {
    Serial1.println("connected");
   //once connected to MQTT broker, subscribe command if any
    //client.subscribe(mqtt_ring_sub);
  } else {
    Serial1.println("failed, rc=");
    //Serial1.println(client.state());
  }
} //end reconnect()

void sendMQTTMessage(String topic, String message) {
    char msg[50];
    char tpc[50];
    topic.toCharArray(tpc, 50);
    message.toCharArray(msg,50);
    //Serial1.println(tpc);
    //Serial1.println(msg);
    //publish sensor data to MQTT broker
    client.publish(tpc, msg);
    Serial1.println(" ..sent");
}

void setup() {
  Serial.begin(115200); //used for communication
#ifdef DEBUG
  Serial1.begin(115200);  // cannot receive data - used for debug output
#endif
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // start a read right away
  reader.enable(true);
  last = millis();
}

void loop() {
  if (!client.connected() && millis() > reconnect_at_time) {
    reconnect();
    //failed to reconnect?
    if (!client.connected()) {
      //try again 6seconds later
      Serial1.println("Reconnect failed, trying again in 6 seconds");
      reconnect_at_time = millis()+6000;
    }
  }
  client.loop();
  reader.loop();

  // Every minute, fire off a one-off reading
  unsigned long now = millis();
  if (now - last > 60000) {
    reader.enable(true);
    last = now;
  }

  if (reader.available()) {
    MyData data;
    String err;
    if (reader.parse(&data, &err)) {
      // Parse succesful, print result
      data.applyEach(Printer());
    } else {
      // Parser error, print error
      Serial1.println(err);
    }
  }

}
