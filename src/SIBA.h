/*
 * SIBA.h
 * Created by GyoJun Ahn @ DCU ICS Lab. on 05/08/2019
 *  
 */

#ifndef SIBA_h
#define SIBA_h

#define EVENT_COUNT 30

/* topic name managements */
#define DEV_CTRL "dev/control"
#define DEV_CTRL_END "dev/control/end"
#define DEV_REG "dev/register"
#define MQTT_SERVER "192.168.2.1"
#define MQTT_PORT 1883
#define REGISTER_EVENT_CODE 0

#define SB_ACTION void (*sb_action)()

#include "includes/PubSubClient/PubSubClient.h"
#include <ESP8266WiFi.h>

typedef struct sb_event{
    size_t sb_code;
    SB_ACTION;
}sb_event;

typedef struct sb_keypair{
  String key;
  String value;
}siba_keypair;


class SIBA{

    private:
        char* ssid;
        char* pwd;
        char* mqtt_server;
        uint16_t mqtt_port;
        char* dev_type;
        static String mac_address;
        String cur_ip;
        static size_t is_reg;

        WiFiClient espClient;
        PubSubClient client;
        static size_t action_cnt;
        static sb_event action_store[EVENT_COUNT]; //이벤트를 담는 배열

        static SIBA context;

        void (*grep_event(size_t code))();
        size_t exec_event(SB_ACTION);
        size_t pub_result(size_t action_res);
        void regist_dev();
        void subscribe_topic(char* topic);
        void publish_topic(char* topic, sb_keypair sets[], uint16_t len);
        void init_wifi(char* ssid, char* pwd);
        void mqtt_reconnect();
        static void mqtt_callback(char *topic, uint8_t *payload, unsigned int length);

        static void register_event(); //0번 코드에 대응되는 이벤트

    public:
        SIBA();

        size_t init(const char* ssid, const char* pwd, const char* dev_type); 
        size_t add_event(size_t code, SB_ACTION);
        void verify_connection();
};

#endif