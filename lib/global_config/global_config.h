#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <inttypes.h>
#include "esp_log.h"
#include "driver/gpio.h"

#ifdef DEBUG
#define DEFAULT_LOG_LEVEL   ESP_LOG_DEBUG
#elif defined(RELEASE)
#define DEFAULT_LOG_LEVEL   ESP_LOG_ERROR
#endif


#define PIN_BUZZER      GPIO_NUM_1

#define LCD_CLK         38  // CLK - 5
#define LCD_DIN         39  // DIN - 4
#define LCD_DC          40  // DC - 3
#define LCD_CS          41  // CE - 2
#define LCD_RST         42  // RST - 1


typedef struct {
    char wifi_ssid[32];
    char wifi_pass[64];
    esp_log_level_t log_level;
} __attribute__ ((packed)) nvs_device_config_t;


#endif  // SRC_CONFIG_H_
