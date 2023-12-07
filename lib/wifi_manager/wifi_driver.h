#ifndef LIB_WIFI_MANAGER_WIFI_DRIVER_H_
#define LIB_WIFI_MANAGER_WIFI_DRIVER_H_


#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "global_config.h"
#include "ntp_driver.h"


#define BIT_WIFI_RUNNING            BIT0
#define BIT_WIFI_STOPPED            BIT1
#define BIT_WIFI_DISCONNECTED       BIT2
#define BIT_WIFI_CONNECTED          BIT3
#define BIT_WIFI_NTP_START          BIT4
#define BIT_WIFI_NTP_GOT_TIME       BIT5
#define BIT_WIFI_SCAN_DONE          BIT6
#define BIT_WIFI_CONNECT_TIMEOUT    BIT7

#define BIT_WIFI_ALL    (     \
    BIT_WIFI_RUNNING        | \
    BIT_WIFI_STOPPED        | \
    BIT_WIFI_DISCONNECTED   | \
    BIT_WIFI_CONNECTED      | \
    BIT_WIFI_NTP_START      | \
    BIT_WIFI_NTP_GOT_TIME   | \
    BIT_WIFI_SCAN_DONE      | \
    BIT_WIFI_CONNECT_TIMEOUT)


#define SCAN_LIST_SIZE              10


#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    wifi_credentials credentials;
    ntp_driver_config_t ntp_config;
} wifi_driver_config_t;

typedef enum {
    WIFI_NOT_CONNECTED = 0,
    WIFI_SIGNAL_BAD = 1,
    WIFI_SIGNAL_WEAK = 2,
    WIFI_SIGNAL_GOOD = 3,
    WIFI_SIGNAL_STRONG = 4
} wifi_driver_rssi_strength_t;

void wifi_set_event_group(EventGroupHandle_t *event_group);
void wifi_setup_timers();
esp_err_t wifi_begin(wifi_driver_config_t *cfg);
esp_err_t wifi_end();
esp_err_t wifi_scan();
void wifi_ntp_connect();
wifi_driver_rssi_strength_t wifi_get_rssi_strength(); 


#ifdef __cplusplus
}
#endif


#endif  // LIB_WIFI_MANAGER_WIFI_DRIVER_H_
