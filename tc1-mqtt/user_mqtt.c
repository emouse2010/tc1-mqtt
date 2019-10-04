/**
 ******************************************************************************
 * @file    mqtt_client.c
 * @author  Eshen Wang
 * @version V1.0.0
 * @date    16-Nov-2015
 * @brief   MiCO application demonstrate a MQTT client.
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, MXCHIP Inc. SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2014 MXCHIP Inc.</center></h2>
 ******************************************************************************
 */

#include "mico.h"
#include "MQTTClient.h"
#include "user_mqtt.h"
#include "user_config.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define app_log(M, ...) custom_log("APP", M, ##__VA_ARGS__)
#define mqtt_log(M, ...) custom_log("MQTT", M, ##__VA_ARGS__)

extern bool mqtt_ready;

/******************************************************
 *               Function Declarations
 ******************************************************/

static void mqtt_client_thread( mico_thread_arg_t arg );
static void messageArrived( MessageData* md );
static OSStatus mqtt_msg_publish( Client *c, const char* topic, char qos, char retained,
                                  const unsigned char* msg,
                                  uint32_t msg_len );

OSStatus user_send_handler( void *arg );
OSStatus user_recv_handler( void *arg );

/******************************************************
 *               Variables Definitions
 ******************************************************/

mico_queue_t   mqtt_msg_send_queue = NULL;

Client c;  // mqtt client object
Network n;  // socket network for mqtt client

static mico_worker_thread_t  mqtt_client_worker_thread; /* Worker thread to manage send/recv events */
static mico_timed_event_t   mqtt_client_send_event;

/******************************************************
 *               Function Definitions
 ******************************************************/

/* Application entrance */
int mqtt_init( void  )
{
    OSStatus err = kNoErr;

#ifdef MQTT_CLIENT_SSL_ENABLE
    int mqtt_thread_stack_size = 0x3000;
#else
    int mqtt_thread_stack_size = 0x800;
#endif
    uint32_t mqtt_lib_version = MQTTClientLibVersion( );
    app_log( "MQTT client version: [%ld.%ld.%ld]",
             0xFF & (mqtt_lib_version >> 16), 0xFF & (mqtt_lib_version >> 8), 0xFF & mqtt_lib_version);
    /* create mqtt msg send queue */
    err = mico_rtos_init_queue( &mqtt_msg_send_queue, "mqtt_msg_send_queue", sizeof(p_mqtt_send_msg_t),
                                MAX_MQTT_SEND_QUEUE_SIZE );
    require_noerr_action( err, exit, app_log("ERROR: create mqtt msg send queue err=%d.", err) );

    /* start mqtt client */
    err = mico_rtos_create_thread( NULL, MICO_APPLICATION_PRIORITY, "mqtt_client",
                                   (mico_thread_function_t)mqtt_client_thread, mqtt_thread_stack_size, 0 );
    require_noerr_string( err, exit, "ERROR: Unable to start the mqtt client thread." );

    /* Create a worker thread for user handling MQTT data event  */
    err = mico_rtos_create_worker_thread( &mqtt_client_worker_thread, MICO_APPLICATION_PRIORITY, 0x800, 5 );
    require_noerr_string( err, exit, "ERROR: Unable to start the mqtt client worker thread." );

    mqtt_ready = true;
    /* Trigger a period send event */
    //mico_rtos_register_timed_event( &mqtt_client_send_event, &mqtt_client_worker_thread, user_send_handler, 2000, NULL );
    
exit:
    if ( kNoErr != err )  app_log("ERROR, app thread exit err: %d", err);
    return err;
}

static OSStatus mqtt_client_release( Client *c, Network *n)
{
    OSStatus err = kNoErr;

    if ( c->isconnected ) MQTTDisconnect( c );

    n->disconnect( n );  // close connection

    if ( MQTT_SUCCESS != MQTTClientDeinit( c ) ) {
        app_log("MQTTClientDeinit failed!");
        err = kDeletedErr;
    }
    return err;
}

// publish msg to mqtt server
static OSStatus mqtt_msg_publish( Client *c, const char* topic, char qos, char retained,
                                  const unsigned char* msg,
                                  uint32_t msg_len )
{
    OSStatus err = kUnknownErr;
    int ret = 0;
    MQTTMessage publishData = MQTTMessage_publishData_initializer;

    require( topic && msg_len && msg, exit);

    // upload data qos0
    publishData.qos = (enum QoS) qos;
    publishData.retained = retained;
    publishData.payload = (void*) msg;
    publishData.payloadlen = msg_len;

    ret = MQTTPublish( c, topic, &publishData );

    if ( MQTT_SUCCESS == ret ) {
        err = kNoErr;
    } else if ( MQTT_SOCKET_ERR == ret ) {
        err = kConnectionErr;
    } else {
        err = kUnknownErr;
    }

exit:
    return err;
}

void mqtt_client_thread( mico_thread_arg_t arg )
{
    OSStatus err = kUnknownErr;

    int rc = -1;
    fd_set readfds;
    struct timeval t = { 0, MQTT_YIELD_TMIE * 1000 };

    ssl_opts ssl_settings;
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    p_mqtt_send_msg_t p_send_msg = NULL;
    int msg_send_event_fd = -1;
    bool no_mqtt_msg_exchange = true;

    mqtt_log("MQTT client thread started...");

    memset( &c, 0, sizeof(c) );
    memset( &n, 0, sizeof(n) );

    /* create msg send queue event fd */
    msg_send_event_fd = mico_create_event_fd( mqtt_msg_send_queue );
    require_action( msg_send_event_fd >= 0, exit, mqtt_log("ERROR: create msg send queue event fd failed!!!") );

MQTT_start:
    /* 1. create network connection */
#ifdef MQTT_CLIENT_SSL_ENABLE
    ssl_settings.ssl_enable = true;
    ssl_settings.ssl_debug_enable = false;  // ssl debug log
    ssl_settings.ssl_version = TLS_V1_2_MODE;
    ssl_settings.ca_str_len = strlen(mqtt_server_ssl_cert_str);
    ssl_settings.ca_str = mqtt_server_ssl_cert_str;
#else
    ssl_settings.ssl_enable = false;
#endif

    while( 1 ){
        rc = NewNetwork( &n, MQTT_SERVER, MQTT_SERVER_PORT, ssl_settings );
        if( rc == MQTT_SUCCESS ) break;
        mqtt_log("ERROR: MQTT network connection err=%d, reconnect after 3s...", rc);
        mico_rtos_thread_sleep( 3 );
    }

    mqtt_log("MQTT network connection success!");

    /* 2. init mqtt client */
    //c.heartbeat_retry_max = 2;
    rc = MQTTClientInit( &c, &n, MQTT_CMD_TIMEOUT );
    require_noerr_string(rc, MQTT_reconnect, "ERROR: MQTT client init err.");

    mqtt_log("MQTT client init success!");

    /* 3. create mqtt client connection */
    connectData.willFlag = 0;
    connectData.MQTTVersion = 4;  // 3: 3.1, 4: v3.1.1
    connectData.clientID.cstring = MQTT_CLIENT_ID;
    connectData.username.cstring = MQTT_CLIENT_USERNAME;
    connectData.password.cstring = MQTT_CLIENT_PASSWORD;
    connectData.keepAliveInterval = MQTT_CLIENT_KEEPALIVE;
    connectData.cleansession = 1;

    rc = MQTTConnect( &c, &connectData );
    require_noerr_string(rc, MQTT_reconnect, "ERROR: MQTT client connect err.");

    mqtt_log("MQTT client connect success!");

    /* 4. mqtt client subscribe */
    rc = MQTTSubscribe( &c, MQTT_CLIENT_SUB_TOPIC, QOS0, messageArrived );
    require_noerr_string(rc, MQTT_reconnect, "ERROR: MQTT client subscribe err.");

    mqtt_log("MQTT client subscribe success! recv_topic=[%s].", MQTT_CLIENT_SUB_TOPIC);

    /* 5. client loop for recv msg && keepalive */
    while ( 1 )
    {
        no_mqtt_msg_exchange = true;
        FD_ZERO( &readfds );
        FD_SET( c.ipstack->my_socket, &readfds );
        FD_SET( msg_send_event_fd, &readfds );
        select( msg_send_event_fd + 1, &readfds, NULL, NULL, &t );

        /* recv msg from server */
        if ( FD_ISSET( c.ipstack->my_socket, &readfds ) )
        {
            rc = MQTTYield( &c, (int) MQTT_YIELD_TMIE );
            require_noerr( rc, MQTT_reconnect );
            no_mqtt_msg_exchange = false;
        }

        /* recv msg from user worker thread to be sent to server */
        if ( FD_ISSET( msg_send_event_fd, &readfds ) )
        {
            while ( mico_rtos_is_queue_empty( &mqtt_msg_send_queue ) == false )
            {
                // get msg from send queue
                mico_rtos_pop_from_queue( &mqtt_msg_send_queue, &p_send_msg, 0 );
                require_string( p_send_msg, exit, "Wrong data point");

                // send message to server
                err = mqtt_msg_publish( &c, p_send_msg->topic, p_send_msg->qos, p_send_msg->retained,
                                        p_send_msg->data,
                                        p_send_msg->datalen );

                require_noerr_string( err, MQTT_reconnect, "ERROR: MQTT publish data err" );

                mqtt_log("MQTT publish data success! send_topic=[%s], msg=[%ld][%s].\r\n", p_send_msg->topic, p_send_msg->datalen, p_send_msg->data);
                no_mqtt_msg_exchange = false;
                free( p_send_msg );
                p_send_msg = NULL;
            }
        }

        /* if no msg exchange, we need to check ping msg to keep alive. */
        if ( no_mqtt_msg_exchange )
        {
            rc = keepalive( &c );
            require_noerr_string( rc, MQTT_reconnect, "ERROR: keepalive err" );
        }
    }

MQTT_reconnect:
    mqtt_log("Disconnect MQTT client, and reconnect after 5s, reason: mqtt_rc = %d, err = %d", rc, err );
    mqtt_client_release( &c, &n );
    mico_rtos_thread_sleep( 5 );
    goto MQTT_start;

exit:
    mqtt_log("EXIT: MQTT client exit with err = %d.", err);
    mqtt_client_release( &c, &n);
    mico_rtos_delete_thread( NULL );
}

// callback, msg received from mqtt server
static void messageArrived( MessageData* md )
{
    OSStatus err = kUnknownErr;
    p_mqtt_recv_msg_t p_recv_msg = NULL;
    MQTTMessage* message = md->message;
    /*
     app_log("MQTT messageArrived callback: topiclen=[%d][%s],\t payloadlen=[%d][%s]",
     md->topicName->lenstring.len,
     md->topicName->lenstring.data,
     (int)message->payloadlen,
     (char*)message->payload);
     */
    mqtt_log("\t\t\t\t\t======MQTT received callback ![%d]======", MicoGetMemoryInfo()->free_memory );

    p_recv_msg = (p_mqtt_recv_msg_t) calloc( 1, sizeof(mqtt_recv_msg_t) );
    require_action( p_recv_msg, exit, err = kNoMemoryErr );

    p_recv_msg->datalen = message->payloadlen;
    p_recv_msg->qos = (char) (message->qos);
    p_recv_msg->retained = message->retained;
    strncpy( p_recv_msg->topic, md->topicName->lenstring.data, md->topicName->lenstring.len );
    memcpy( p_recv_msg->data, message->payload, message->payloadlen );

    err = mico_rtos_send_asynchronous_event( &mqtt_client_worker_thread, user_recv_handler, p_recv_msg );
    require_noerr( err, exit );

exit:
    if ( err != kNoErr ) {
        app_log("ERROR: Recv data err = %d", err);
        if( p_recv_msg ) free( p_recv_msg );
    }
    return;
}


/* Application process MQTT received data */
OSStatus user_recv_handler( void *arg )
{
    OSStatus err = kUnknownErr;
    p_mqtt_recv_msg_t p_recv_msg = arg;
    require( p_recv_msg, exit );

    app_log("\t\t\t\t\tuser get data success! from_topic=[%s], msg=[%ld][%s].\r\n", p_recv_msg->topic, p_recv_msg->datalen, p_recv_msg->data);
    unsigned char i;
    for ( i = 0; i < SLOT_NUM; i++ )
    {
        char rec_cmp[10];
        sprintf(rec_cmp, "S%d_ON", i+1);
        if(strcmp(p_recv_msg->data,rec_cmp) == 0)
        {
            set_relay(i,1);
        }
        sprintf(rec_cmp, "S%d_OFF", i+1);
        if(strcmp(p_recv_msg->data,rec_cmp) == 0)
        {
            set_relay(i,0);
        }
    }
    
    free( p_recv_msg );

exit:
    return err;
}

/* Application collect data and seng them to MQTT send queue */
OSStatus user_send_handler( void *arg )
{
    OSStatus err = kUnknownErr;
    p_mqtt_send_msg_t p_send_msg = NULL;

    app_log("======App prepare to send ![%d]======", MicoGetMemoryInfo()->free_memory);

    /* Send queue is full, pop the oldest */
    if ( mico_rtos_is_queue_full( &mqtt_msg_send_queue ) == true ){
        mico_rtos_pop_from_queue( &mqtt_msg_send_queue, &p_send_msg, 0 );
        free( p_send_msg );
        p_send_msg = NULL;
    }

    /* Push the latest data into send queue*/
    p_send_msg = (p_mqtt_send_msg_t) calloc( 1, sizeof(mqtt_send_msg_t) );
    require_action( p_send_msg, exit, err = kNoMemoryErr );

    p_send_msg->qos = 0;
    p_send_msg->retained = 0;
    p_send_msg->datalen = strlen( MQTT_CLIENT_PUB_MSG );
    memcpy( p_send_msg->data, MQTT_CLIENT_PUB_MSG, p_send_msg->datalen );
    strncpy( p_send_msg->topic, MQTT_CLIENT_PUB_TOPIC, MAX_MQTT_TOPIC_SIZE );

    err = mico_rtos_push_to_queue( &mqtt_msg_send_queue, &p_send_msg, 0 );
    require_noerr( err, exit );

    app_log("Push user msg into send queue success!");

exit:
    if( err != kNoErr && p_send_msg ) free( p_send_msg );
    return err;

}

// Send slot state, slot_id = 0-5
OSStatus mqtt_send_slot_state( unsigned char slot_id, unsigned char slot_state)
{
    OSStatus err = kUnknownErr;
    p_mqtt_send_msg_t p_send_msg = NULL;

    char send_buff[10];
    if(slot_state == 0)
    {
        sprintf(send_buff, "S%d_OFF", slot_id+1);
    }
    else
    {
        sprintf(send_buff, "S%d_ON", slot_id+1);
    }
    

    app_log("======Slot state prepare to send ![%d]======", MicoGetMemoryInfo()->free_memory);

    /* Send queue is full, pop the oldest */
    if ( mico_rtos_is_queue_full( &mqtt_msg_send_queue ) == true ){
        mico_rtos_pop_from_queue( &mqtt_msg_send_queue, &p_send_msg, 0 );
        free( p_send_msg );
        p_send_msg = NULL;
    }

    /* Push the latest data into send queue*/
    p_send_msg = (p_mqtt_send_msg_t) calloc( 1, sizeof(mqtt_send_msg_t) );
    require_action( p_send_msg, exit, err = kNoMemoryErr );

    p_send_msg->qos = 0;
    p_send_msg->retained = 0;
    p_send_msg->datalen = strlen( send_buff );
    memcpy( p_send_msg->data, send_buff, p_send_msg->datalen );
    strncpy( p_send_msg->topic, MQTT_CLIENT_PUB_TOPIC, MAX_MQTT_TOPIC_SIZE );

    err = mico_rtos_push_to_queue( &mqtt_msg_send_queue, &p_send_msg, 0 );
    require_noerr( err, exit );

    app_log("Push slot state msg into send queue success!");

exit:
    if( err != kNoErr && p_send_msg ) free( p_send_msg );
    return err;

}



