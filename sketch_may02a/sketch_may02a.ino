#include <ArduinoJson.h>

/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define CTRL_TOPIC_LEN 30
#define ACTION_COUNT 30
#define EN_LED 2

// Update these with values suitable for your network.

const char* ssid = "IoT-Hub";
const char* password = "raspberry";
const char* mqtt_server = "192.168.2.1";
String local_ip;
String mac_addr = "";

typedef void (*action)();

typedef struct siba_keypair{
  String key;
  String value;
}siba_keypair;

typedef struct siba_action{
  byte code;
  action func;
}siba_action;

siba_action action_store[ACTION_COUNT] = {0}; // 액션 스토어
byte action_index = 0;

WiFiClient espClient;
PubSubClient client(espClient);

//사용자 설정 정보
const String dev_type ="adlplX123eQ!wd";

//액션을 추가하는 함수
void siba_add_action(byte code, void (*action)()){
  action_store[action_index++] = {code, action};
}

//코드로 액션을 찾는 함수
action siba_action_find(byte code){
  //sequential search
  byte i = action_index;
  byte res = 0;
  action ret = NULL;
  while(i--) if(res=action_store[i].code==code) break;
  if(res) ret=action_store[i].func;
  return ret;
}

//액션을 실행하는 함수
void siba_action_call(action func){
  if(func){
    func();
    siba_action_res();
  }
  else Serial.println(F("code is not added."));
}

void siba_action_res(){

  siba_keypair sets[] = {
    {"status", "true"}
  };
  
  siba_mqtt_pub("dev/control/end", sets, sizeof(sets)/sizeof(sets[0]));
}

void siba_led_ctrl(byte st){
  digitalWrite(EN_LED, st);
  digitalWrite(LED_BUILTIN, !st);
}

void setup_wifi() {
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(EN_LED, OUTPUT);

  siba_led_ctrl(0);

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
  Serial.print("IP address: ");
  local_ip = WiFi.localIP().toString();
  Serial.println(local_ip);
  Serial.print("MAC address: ");
  mac_addr = WiFi.macAddress();
  Serial.println(mac_addr);
}

void callback(char* topic, byte* payload, unsigned int length) {
  
  String msg="";
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  for(int i=0; i<length; i++){
    msg+=(char)payload[i];
  }

  //받아온 json문자열 파싱
  StaticJsonDocument<256> doc;
  auto error = deserializeJson(doc, msg);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
  }

  Serial.println(msg);
  int code = doc[F("code")];
  Serial.println(code);
  siba_led_ctrl(1);
  siba_action_call(siba_action_find(code));
}

String siba_regist_dev(){
  String DEV_MAC = "dev_mac";
  String CUR_IP = "cur_ip";
  String DEV_TYPE = "dev_type";

  siba_keypair sets[] = {
    {DEV_MAC, mac_addr},
    {CUR_IP, local_ip},
    {DEV_TYPE, dev_type}
  };
  
  siba_mqtt_pub("dev/register", sets, sizeof(sets)/sizeof(sets[0]));
}

void siba_mqtt_pub(char* topic, siba_keypair sets[], int len){
  int row = len;
  StaticJsonDocument<256> doc;
  char buffer[256];
  while(len--){
    doc[sets[len].key] = sets[len].value;
  }
  size_t n = serializeJson(doc, buffer);
  client.publish(topic, buffer, n);
}

void siba_topic_sub(){
  String control_channel = String("dev/control/")+mac_addr;
  char buf[CTRL_TOPIC_LEN] = {0};
  control_channel.toCharArray(buf,CTRL_TOPIC_LEN);

  Serial.println(F("subscribe control topic."));
  client.subscribe(buf);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      siba_regist_dev();
      // ... and resubscribe
      siba_topic_sub();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void test(){
  Serial.println("add action test");
}

void setup() {
  //pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //prev=millis();
  siba_add_action(11,test);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  /*long now = millis();
  if(now-prev>1000){
    Serial.println("data publish");
    client.publish("dev/heartbeat", "Yes");
    prev = now;
  }*/
}
