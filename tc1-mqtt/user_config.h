#ifndef TC1_MQTT_USER_CONFIG_H_
#define TC1_MQTT_USER_CONFIG_H_

#define CONFIG_SSID "XinRui"                 
#define CONFIG_USER_KEY "emouse2010"      


/******************************************************
 *                    Constants
 ******************************************************/

#define MQTT_CLIENT_ID         "tc1_MQTT_Client"
#define MQTT_CLIENT_USERNAME    "hass"
#define MQTT_CLIENT_PASSWORD    "emousemqtt"
#define MQTT_CLIENT_KEEPALIVE   30

#define MQTT_CMD_TIMEOUT        5000  // 5s
#define MQTT_YIELD_TMIE         5000  // 5s
#define MQTT_CLIENT_PUB_MSG     "online"
//#define MQTT_CLIENT_SSL_ENABLE  // ssl
#define MQTT_CLIENT_PUB_DISCOVERY_TOPIC   "homeassistant/switch/tc1_slot%d/config" // 实际未使用
#define MQTT_CLIENT_SUB_TOPIC             "homeassistant/switch/tc1_slot%d/set"  
#define MQTT_CLIENT_PUB_TOPIC             "homeassistant/switch/tc1_slot%d/state"
#define MQTT_CLIENT_PUB_AVAILABLE_TOPIC   "homeassistant/switch/tc1_slot%d/available"

#define MQTT_CLIENT_DISCOVERY_DATA        "{\"name\": \"tc1_slot%d\", \"command_topic\": \"homeassistant/switch/tc1_slot%d/set\", \"state_topic\": \"homeassistant/switch/tc1_slot%d/state\"}"
 
#ifdef MQTT_CLIENT_SSL_ENABLE

#define MQTT_SERVER             "test.mosquitto.org"
#define MQTT_SERVER_PORT        8883
char* mqtt_server_ssl_cert_str =
"-----BEGIN CERTIFICATE-----\r\n\
MIIC8DCCAlmgAwIBAgIJAOD63PlXjJi8MA0GCSqGSIb3DQEBBQUAMIGQMQswCQYD\r\n\
VQQGEwJHQjEXMBUGA1UECAwOVW5pdGVkIEtpbmdkb20xDjAMBgNVBAcMBURlcmJ5\r\n\
MRIwEAYDVQQKDAlNb3NxdWl0dG8xCzAJBgNVBAsMAkNBMRYwFAYDVQQDDA1tb3Nx\r\n\
dWl0dG8ub3JnMR8wHQYJKoZIhvcNAQkBFhByb2dlckBhdGNob28ub3JnMB4XDTEy\r\n\
MDYyOTIyMTE1OVoXDTIyMDYyNzIyMTE1OVowgZAxCzAJBgNVBAYTAkdCMRcwFQYD\r\n\
VQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwGA1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1v\r\n\
c3F1aXR0bzELMAkGA1UECwwCQ0ExFjAUBgNVBAMMDW1vc3F1aXR0by5vcmcxHzAd\r\n\
BgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hvby5vcmcwgZ8wDQYJKoZIhvcNAQEBBQAD\r\n\
gY0AMIGJAoGBAMYkLmX7SqOT/jJCZoQ1NWdCrr/pq47m3xxyXcI+FLEmwbE3R9vM\r\n\
rE6sRbP2S89pfrCt7iuITXPKycpUcIU0mtcT1OqxGBV2lb6RaOT2gC5pxyGaFJ+h\r\n\
A+GIbdYKO3JprPxSBoRponZJvDGEZuM3N7p3S/lRoi7G5wG5mvUmaE5RAgMBAAGj\r\n\
UDBOMB0GA1UdDgQWBBTad2QneVztIPQzRRGj6ZHKqJTv5jAfBgNVHSMEGDAWgBTa\r\n\
d2QneVztIPQzRRGj6ZHKqJTv5jAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBBQUA\r\n\
A4GBAAqw1rK4NlRUCUBLhEFUQasjP7xfFqlVbE2cRy0Rs4o3KS0JwzQVBwG85xge\r\n\
REyPOFdGdhBY2P1FNRy0MDr6xr+D2ZOwxs63dG1nnAnWZg7qwoLgpZ4fESPD3PkA\r\n\
1ZgKJc2zbSQ9fCPxt2W3mdVav66c6fsb7els2W2Iz7gERJSX\r\n\
-----END CERTIFICATE-----";

#else  // ! MQTT_CLIENT_SSL_ENABLE

#define MQTT_SERVER             "192.168.1.101"
#define MQTT_SERVER_PORT        1883

#endif // MQTT_CLIENT_SSL_ENABLE

#define SLOT_NUM 6

#define USER_LED    MICO_GPIO_5   //高电平亮灯
#define USER_BUTTON MICO_GPIO_23
#define USER_POWER  ICO_GPIO_15

#define Relay_ON     1
#define Relay_OFF    0

// 6组继电器对应IO，高电平继电器吸合
#define Relay_0     MICO_GPIO_6
#define Relay_1     MICO_GPIO_8
#define Relay_2     MICO_GPIO_10
#define Relay_3     MICO_GPIO_7
#define Relay_4     MICO_GPIO_9
#define Relay_5     MICO_GPIO_18
#define Relay_NUM   SLOT_NUM
extern unsigned char slot_state[SLOT_NUM];
extern bool mqtt_ready;

#endif /* TC1_MQTT_USER_CONFIG_H_ */
