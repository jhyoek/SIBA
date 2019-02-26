#include "ArduinoJson.h"

const byte ledPin = D8;
const byte interruptPin = D2;
volatile byte state = LOW;
volatile byte lock = 0x00;

char* userName = "";
char* ssid = ""; //AP's ssid
char* password = ""; // AP's password



void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), on, CHANGE);

 
}

void on() {

  char json[] = "{\"ssid\":\"test\",\"password\":\"1351824120\"}";
  StaticJsonBuffer<200> jsonBuffer;
  //DynamicJsonDocument doc(1024);
  //deserializeJson(doc, json); //데이터 역직렬화
  JsonObject& root = jsonBuffer.parseObject(json);
  //JsonObjectRef root = doc.as<JsonObject>();
  
  ssid = root["ssid"];//AP이름
  password = root["password"];//AP패스워드
  
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
