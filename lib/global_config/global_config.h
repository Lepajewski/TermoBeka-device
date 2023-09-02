#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <inttypes.h>
#include "esp_log.h"

#ifdef DEBUG
#define DEFAULT_LOG_LEVEL   ESP_LOG_DEBUG
#elif defined(RELEASE)
#define DEFAULT_LOG_LEVEL   ESP_LOG_ERROR
#endif


#define LCD_CLK         38  // CLK - 5
#define LCD_DIN         39  // DIN - 4
#define LCD_DC          40  // DC - 3
#define LCD_CS          41  // CE - 2
#define LCD_RST         42  // RST - 1


typedef struct {
    // const char *wifi_ssid;
    // const char *wifi_pass;
    esp_log_level_t log_level;
} nvs_device_config_t;

#endif  // SRC_CONFIG_H_
