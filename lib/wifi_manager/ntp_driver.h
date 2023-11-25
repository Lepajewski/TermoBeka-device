#ifndef LIB_WIFI_MANAGER_NTP_DRIVER_H_
#define LIB_WIFI_MANAGER_NTP_DRIVER_H_


#include "esp_err.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    const char *server_address;
    const char *timezone;
    uint32_t reconnect_timeout;
    uint32_t reconnect_interval;
} ntp_driver_config_t;

esp_err_t ntp_start();
void ntp_stop();
void get_timestamp(char *timestamp);


#ifdef __cplusplus
}
#endif


#endif  // LIB_WIFI_MANAGER_NTP_DRIVER_H_
