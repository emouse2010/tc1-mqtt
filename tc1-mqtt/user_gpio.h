#ifndef TC1_MQTT_USER_GPIO_H_
#define TC1_MQTT_USER_GPIO_H_

extern void gpio_init(void);
extern void set_relay_all_on(void);
extern void set_relay_all_off(void);
extern void set_relay(unsigned char slot_id, unsigned char state);

#endif /* TC1_MQTT_USER_GPIO_H_ */

