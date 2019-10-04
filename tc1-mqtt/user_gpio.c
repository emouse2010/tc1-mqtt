#include "mico.h"
#include "user_config.h"
#include "user_gpio.h"
#include "mico_platform.h"
#include "user_mqtt.h"

#define gpio_log(M, ...) custom_log("GPIO", M, ##__VA_ARGS__)

mico_gpio_t relay[Relay_NUM] = { Relay_0, Relay_1, Relay_2, Relay_3, Relay_4, Relay_5 };

mico_timer_t user_button_timer;
uint16_t key_time = 0;
#define BUTTON_LONG_PRESS_TIME    10     //100ms*10=1s

bool relay_flag = false;

extern bool mqtt_ready;
extern unsigned char slot_state[SLOT_NUM]; // define in main

bool get_relay_state();
void user_button_press(void);
void user_led_display();
static void button_timeout_handler( void* arg );
static void button_falling_irq_handler( void* arg );

void gpio_init(void)
{
    MicoGpioInitialize(USER_LED, OUTPUT_PUSH_PULL);
    int i;
    for ( i = 0; i < Relay_NUM; i++ )
    {
        MicoGpioInitialize( relay[i], OUTPUT_PUSH_PULL);
        MicoGpioOutputLow( relay[i] );
    }
    MicoGpioInitialize(USER_BUTTON , INPUT_PULL_UP);
    mico_init_timer( &user_button_timer, 100, button_timeout_handler, NULL );

    //External interrupts are used for keystroke detection
    MicoGpioEnableIRQ(USER_BUTTON, IRQ_TRIGGER_FALLING_EDGE, button_falling_irq_handler, NULL );

}

void set_relay_all_on(void)
{
    unsigned char i;
    for ( i = 0; i < Relay_NUM; i++ )
    {
        MicoGpioInitialize( relay[i], OUTPUT_PUSH_PULL);
        MicoGpioOutputHigh( relay[i] );
        slot_state[i] = 1;
         if(mqtt_ready)
        {
            mqtt_send_slot_state(i, 1);
        }
        else
        {
            gpio_log("mqtt not ready");
        }
    }
    MicoGpioOutputHigh(USER_LED);
    relay_flag = true;
    gpio_log("relay ON");

}

void set_relay_all_off(void)
{
    unsigned char i;
    for ( i = 0; i < Relay_NUM; i++ )
    {
        MicoGpioInitialize( relay[i], OUTPUT_PUSH_PULL);
        MicoGpioOutputLow( relay[i] );
        slot_state[i] = 0;
        if(mqtt_ready)
        {
            mqtt_send_slot_state(i, 0);
        }
        else
        {
            gpio_log("mqtt not ready");
        }
        
    }
    MicoGpioOutputLow(USER_LED);
    relay_flag = false;
    gpio_log("relay OFF");
}

void set_relay(unsigned char slot_id, unsigned char state)
{
    if(state == 0)
    {
        MicoGpioOutputLow( relay[slot_id] );
        slot_state[slot_id] = 0;
        user_led_display();
        mqtt_send_slot_state(slot_id, 0);
    }
    else
    {
        MicoGpioOutputHigh( relay[slot_id] );
        slot_state[slot_id] = 1;
        user_led_display();
        mqtt_send_slot_state(slot_id, 1);
    }
    
}

void user_button_press(void)
{
    if(relay_flag)
    {
        set_relay_all_off();
    }
    else
    {
        set_relay_all_on();
    }

}

// if any slot ON retrun true
bool get_relay_state()
{
    unsigned char i;
    for ( i = 0; i < SLOT_NUM; i++ )
    {
        if (slot_state[i]== 1 )
        {
            return true;
        }
    }
    return false;
}

void user_led_display()
{
    if(get_relay_state())
    {
        MicoGpioOutputHigh(USER_LED);
    }
    else
    {
        MicoGpioOutputLow(USER_LED);
    }
    
}

static void button_falling_irq_handler( void* arg )
{
    mico_start_timer(&user_button_timer);
}


static void button_timeout_handler( void* arg )
{
    static uint8_t key_trigger, key_continue;
    static uint8_t key_last;
    uint8_t tmp = ~(0xfe | MicoGpioInputGet(USER_BUTTON));
    key_trigger = tmp & (tmp ^ key_continue);
    key_continue = tmp;
    if ( key_trigger != 0 ) key_time = 0; 
    if ( key_continue != 0 )
    {
        //any button pressed
        key_time++;
        if ( key_time < BUTTON_LONG_PRESS_TIME )
            key_last = key_continue;
        else
        {
            if ( key_time == 30 )
            {
                //key_long_press( );
                gpio_log("button short pressed:%d",key_time);
            }
            else if ( key_time == 100 )
            {
                // long long press
                gpio_log("button long pressed:%d",key_time);
            }
        }
    }
    else
    {
        //button released
        if ( key_time < BUTTON_LONG_PRESS_TIME & key_time>1)
        {
            //100ms*10=1s 
            key_time = 0;
            gpio_log("button short pressed:%d",key_time);
            user_button_press( );
        } else if ( key_time > 100 )
        {
            MicoSystemReboot( );
        }
        mico_stop_timer( &user_button_timer );
    }

}
