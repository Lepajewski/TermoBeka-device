#ifndef LIB_SERVER_MANAGER_MQTT_DRIVER_H_
#define LIB_SERVER_MANAGER_MQTT_DRIVER_H_


#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "global_config.h"


#define BIT_MQTT_RUNNING            BIT0
#define BIT_MQTT_STOPPED            BIT1
#define BIT_MQTT_DISCONNECTED       BIT2
#define BIT_MQTT_CONNECTED          BIT3
#define BIT_MQTT_CONNECT_TIMEOUT    BIT4

#define BIT_MQTT_ALL                ( \
    BIT_MQTT_RUNNING                | \
    BIT_MQTT_STOPPED                | \
    BIT_MQTT_DISCONNECTED           | \
    BIT_MQTT_CONNECTED              | \
    BIT_MQTT_CONNECT_TIMEOUT        )


#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    mqtt_credentials credentials;
} mqtt_driver_config_t;


void mqtt_set_event_group(EventGroupHandle_t *event_group);
void mqtt_setup_timer();
esp_err_t mqtt_begin(mqtt_driver_config_t *cfg);
esp_err_t mqtt_end();


#ifdef __cplusplus
}
#endif


#endif  // LIB_SERVER_MANAGER_MQTT_DRIVER_H_