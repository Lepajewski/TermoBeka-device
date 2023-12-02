#ifndef LIB_NVS_MANAGER_NVS_CONFIG_H_
#define LIB_NVS_MANAGER_NVS_CONFIG_H_


#include "global_config.h"


typedef struct {
    char wifi_ssid[WIFI_MAX_SSID_LEN];
    char wifi_pass[WIFI_MAX_PASS_LEN];
    char mqtt_broker_uri[MQTT_MAX_BROKER_URI_LEN];
    char mqtt_username[MQTT_MAX_USERNAME_LEN];
    char mqtt_password[MQTT_MAX_PASSWORD_LEN];
    esp_log_level_t log_level;
} __attribute__ ((packed)) nvs_device_config_t;


#endif  // LIB_NVS_MANAGER_NVS_CONFIG_H_
