#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include "RF24.h"

#define RADIO_CNT 2
#define CE 4
#define CSN 15
#define BAUDRATE 115200
#define CHANNEL 115
#define INTTERUPT_PIN 5

RF24 radio(CE,CSN); //nrf24l01 resource

const uint64_t address = 0x72646f4e31LL;
char full_packet[60] = "";
byte packet_cnt = 0;
String ssid = "";
String password = "";
byte init_flag=0;
volatile byte isr_lock=0;

byte json_parse(){
   Serial.println("*** JSON PARSING...");
   StaticJsonDocument<200> doc;

   //json parse
   DeserializationError error = deserializeJson(doc,full_packet);
   if(error){ //when error occured
    Serial.println("*** JSON PARSING FAILED");
    return 0;
   }
   
   ssid = (const char*)doc["ssid"];
   password = (const char*)doc["psw"];

   if(ssid == "null" || password == "null"){ //null validation
    Serial.println("*** JSON PARSING FAILED: NULL VALUE");
    return 0;
   }

   Serial.println("*** JSON PARSING SUCCESS");
   Serial.println("*** SSID: "+ssid+", PASSWORD: "+password);
   return 1;
}

byte establish_esp(){
  Serial.println("*** TRY 802.11 CONNECTION");
  WiFi.begin(ssid, password);
  long prev_time = millis();
  //wait iot hub connect finish but 10 seconds limitation
  while (WiFi.status() != WL_CONNECTED && millis()-prev_time<10000){ 
    delay(500);
  }//AVR spin

  //if iot hub connection success
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("*** WIFI CONNECT SUCCESS");  
    Serial.println(WiFi.localIP());
    return 1;
  }
  else{
    Serial.println("*** WIFI CONNECT FAILED");
    return 0;
  }
}

void setup(){
  Serial.begin(BAUDRATE);
  radio.begin();//radio resource init
  radio.setChannel(CHANNEL);
  radio.setPALevel(RF24_PA_MAX);
  pinMode(INTTERUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTTERUPT_PIN), isr, CHANGE);
  radio.powerDown();//power down
  Serial.println("*** RF24 INITIALIZE FINISH...");
}

//interrupt function
void isr(){
  //must solve chattering
  if(!isr_lock){
    Serial.println("*** ENTER INTERRUPT ROUTINE");
    isr_lock=1;
  }
}


void rf24_read_packet(){
  Serial.println("*** ENTER RF24 READ FUNCTION");
  radio.powerUp();//power up
  radio.openReadingPipe(1,address);
  radio.startListening();
  Serial.println("*** RF24 RELOAD");
  long prev_time = millis();
  while(millis()-prev_time<30000){ //rf24 limitation is 30 sec
    if(radio.available()) {
      radio.read((full_packet+(packet_cnt*30)), sizeof(full_packet)/RADIO_CNT);
      if(++packet_cnt == RADIO_CNT) {
        if(json_parse()){// if parse success
          radio.powerDown();//power down              
          if(establish_esp()){//esp12E connect iot hub
            //when iot hub connection success
            //send MAC address to IoT Hub
            //WiFi.macAddress();
            init_flag=1;
          }
        }
        packet_cnt=0;
        break;
      }
    }
    delay(100); // if no delay, wdt error occured
  }
}

void loop(){
  if(init_flag){
    //esp12 connected
  }
  if(isr_lock){
    rf24_read_packet();
    isr_lock=0;
  }
}
