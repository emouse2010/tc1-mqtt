#ifndef TC1_MQTT_USER_MQTT_H_
#define TC1_MQTT_USER_MQTT_H_

/******************************************************
 *                    Structures
 ******************************************************/

#define MAX_MQTT_TOPIC_SIZE         (256)
#define MAX_MQTT_DATA_SIZE          (1024)
// 6 gpio,If the queue is too small, the previous message will be lost.
#define MAX_MQTT_SEND_QUEUE_SIZE    (12)   


typedef struct
{
    char topic[MAX_MQTT_TOPIC_SIZE];
    char qos;
    char retained;

    uint8_t data[MAX_MQTT_DATA_SIZE];
    uint32_t datalen;
} mqtt_recv_msg_t, *p_mqtt_recv_msg_t, mqtt_send_msg_t, *p_mqtt_send_msg_t;

extern int mqtt_init(void);
extern OSStatus mqtt_send_slot_state( unsigned char slot_id, unsigned char slot_state);

#endif /* TC1_MQTT_USER_MQTT_H_ */
