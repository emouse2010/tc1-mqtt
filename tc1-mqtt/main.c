/**
 ******************************************************************************
 * @file    main.c
 * @author  emouse
 * @version V1.0.0
 * @date    2019.10.4
 * @brief   
 ******************************************************************************
 * 
 *
 ******************************************************************************
 */

#include "mico.h"
#include "mico_config.h"
#include "user_config.h"
#include "user_gpio.h"
#include "user_wifi.h"
#include "user_mqtt.h"

#define app_log(M, ...) custom_log("APP", M, ##__VA_ARGS__)
#define app_log_trace() custom_log_trace("APP")
bool mqtt_ready = false;
unsigned char slot_state[SLOT_NUM];

int application_start( void )
{
    // Add your code here
    app_log( "Start APP");
    /* Start MiCO system functions according to mico_config.h*/
    mico_system_init( mico_system_context_init( 0 ) );


    /* Output on debug serial port */
    app_log( "Hello world!" );
    gpio_init(); 
    user_wifi_init();
    //mqtt_init();  // init on wifi station up callback
    set_relay_all_on(); //contain mqtt send , init after mqtt init
}


