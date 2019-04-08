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
#define SW 5

RF24 radio(CE,CSN); //nrf24l01 resource

const uint64_t address = 0x72646f4e31LL;
char full_packet[60] = "";
byte packet_cnt = 0;
String ssid = "";
String password = "";
byte init_flag=0;
volatile byte isr_lock=0;
int press_btn;
int press_time;

unsigned long compare_time_ms = 0;

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

void check_state_sw(void) {
  unsigned long current_time_ms = millis();
  press_btn = digitalRead(SW);

  if(current_time_ms - compare_time_ms >= 5){
    for(int i = 0; i < 1; i++){ // i < ㅁ <- setting current time checking
      if(press_btn == 0) press_time+=5;
      else if(press_btn == 1){
        if(press_time > 1000) isr_lock = 1;
        press_time = 0;
      }
    }
    compare_time_ms = current_time_ms;
  }
}

void setup(){
  Serial.begin(BAUDRATE);
  radio.begin();//radio resource init
  radio.setChannel(CHANNEL);
  radio.setPALevel(RF24_PA_MAX);
  radio.powerDown();//power down
  Serial.println("*** RF24 INITIALIZE FINISH...");
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
  check_state_sw();
  
  if(init_flag){
    //esp12 connected
  }
  if(isr_lock){
    rf24_read_packet();
    isr_lock=0;
  }
}
