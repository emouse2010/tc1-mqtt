#include "mico.h"
#include "mico_config.h"
#include "user_config.h"
#include "user_gpio.h"

#define app_log(M, ...) custom_log("APP", M, ##__VA_ARGS__)
static void micoNotify_ConnectFailedHandler(OSStatus err, void* inContext)
{
  app_log("join Wlan failed Err: %d", err);
}

static void micoNotify_WifiStatusHandler(WiFiEvent event,  void* inContext)
{
  switch (event)
  {
  case NOTIFY_STATION_UP:
    app_log("emouse Station up");
    mqtt_init();
    break;
  case NOTIFY_STATION_DOWN:
    app_log("emouse Station down");
    break;
  default:
    break;
  }
}

int user_wifi_init(void)
{
    OSStatus err = kNoErr;
    network_InitTypeDef_adv_st  wNetConfigAdv;

    MicoInit( );

    /* Register user function when wlan connection status is changed */
    err = mico_system_notify_register( mico_notify_WIFI_STATUS_CHANGED, (void *)micoNotify_WifiStatusHandler, NULL );
    require_noerr( err, exit );

    /* Register user function when wlan connection is faile in one attempt */
    err = mico_system_notify_register( mico_notify_WIFI_CONNECT_FAILED, (void *)micoNotify_ConnectFailedHandler, NULL );
    require_noerr( err, exit );

    /* Initialize wlan parameters */
    memset( &wNetConfigAdv, 0x0, sizeof(wNetConfigAdv) );
    strcpy((char*)wNetConfigAdv.ap_info.ssid, CONFIG_SSID);   /* wlan ssid string */
    strcpy((char*)wNetConfigAdv.key, CONFIG_USER_KEY);                /* wlan key string or hex data in WEP mode */
    wNetConfigAdv.key_len = strlen(CONFIG_USER_KEY);                  /* wlan key length */
    wNetConfigAdv.ap_info.security = SECURITY_TYPE_AUTO;          /* wlan security mode */
    wNetConfigAdv.ap_info.channel = 0;                            /* Select channel automatically */
    wNetConfigAdv.dhcpMode = DHCP_Client;                         /* Fetch Ip address from DHCP server */
    wNetConfigAdv.wifi_retry_interval = 100;                      /* Retry interval after a failure connection */

    /* Connect Now! */
    app_log("connecting to %s...", wNetConfigAdv.ap_info.ssid);
    micoWlanStartAdv(&wNetConfigAdv);
    exit:
        return err;
}

