#include <SPI.h>

#include "RF24.h"

 

RF24 radio(4,15);

const uint64_t address = 0x72646f4e31LL;

 

void setup(){

  Serial.begin(115200);

 

  radio.begin();

  radio.setChannel(115);

  radio.setPALevel(RF24_PA_MAX);

  radio.openReadingPipe(1,address);

  radio.startListening();

}

 

void loop(){

  if(radio.available()){

    char text[32] = "";

    radio.read(&text, sizeof(text));

    Serial.print("RX : ");

    Serial.println(text);

  }

}