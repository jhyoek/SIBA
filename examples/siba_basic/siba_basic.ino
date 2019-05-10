/*
 *  SIBA basic use example source 
 * 
 * 
 */

#include <SIBA.h>

SIBA siba;

const char* ssid  = "your nat router ip";
const char* password  = "your nat router password";
const char* hardware_auth_key  = "your H/W auth key in here";

void event_test(){
    Serial.println("some message is arrived");
}

void setup() {
    Serial.begin(115200);
    siba.init(ssid, password, hardware_auth_key);
    /* 
    *  put your setup code here, to run once:
    *  and add your event function.
    *  event function format is void func()
    */ 

    siba.add_event(10, event_test);
}

void loop() {
    siba.verify_connection();

    // put your main code here, to run repeatedly:
}