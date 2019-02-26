#include <ArduinoJson.h>

const byte ledPin = D8;
const byte interruptPin = D2;
volatile byte state = LOW;
volatile byte lock = 0x00;

char* userName = "";
char* ssid = ""; //AP's ssid
char* password = ""; // AP's password



void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), on, CHANGE);
}

void on() {

  char json[] = "{\"ssid\":\"test\",\"password\":\"1351824120\"}";
  StaticJsonDocument<256> doc;
  deserializeJson(doc, json);
  JsonObject root = doc.as<JsonObject>();

  String ssid = root["ssid"];
  String password = root["password"];

  Serial.println(ssid);
  Serial.println(password);
  
  if(lock==0x00){
    lock=0xff;
    Serial.println("hello");
    lock=0x00;
    long prev=millis();
    while(millis()-prev<1000){}  
  }
}

void loop() {
  digitalWrite(ledPin, state);
  
  }
