#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <inttypes.h>
#include "esp_log.h"
#include "driver/gpio.h"

#ifdef DEBUG
#define DEFAULT_LOG_LEVEL                   ESP_LOG_DEBUG
#elif defined(RELEASE)
#define DEFAULT_LOG_LEVEL                   ESP_LOG_ERROR
#endif

// Buzzer configuration
#define PIN_BUZZER                          GPIO_NUM_1

// GPIO Expander configuration
#define PIN_GPIO_EXPANDER_SDA               GPIO_NUM_9
#define PIN_GPIO_EXPANDER_SCL               GPIO_NUM_10
#define PIN_GPIO_EXPANDER_INTR              GPIO_NUM_3  // GPIO_NUM_46
#define GPIO_EXPANDER_FREQ_HZ               400000

#define PIN_LCD_SCLK                        GPIO_NUM_38  // CLK
#define PIN_LCD_DIN                         GPIO_NUM_39  // DIN/MOSI
#define PIN_LCD_DC                          GPIO_NUM_40  // DC - Data/Command
#define PIN_LCD_CS                          GPIO_NUM_41  // CE - Chip Select
#define PIN_LCD_RST                         GPIO_NUM_42  // RST - RESET


#define PIN_SD_SCLK                         GPIO_NUM_12
#define PIN_SD_MOSI                         GPIO_NUM_13
#define PIN_SD_MISO                         GPIO_NUM_11
#define PIN_SD_CS                           GPIO_NUM_14


// wifi & ntp configuration
#define WIFI_DEFAULT_SSID                   "Orange_Swiatlowod_3760"
#define WIFI_DEFAULT_PASS                   "2FT752946HF7"
#define NTP_SERVER_ADDR                     "pool.ntp.org"
#define NTP_TIME_ZONE                       "CET-1CEST,M3.5.0/2,M10.5.0/3"
#define NTP_CONNECT_TIMEOUT_MS              5000
#define NTP_RECONNECT_INTERVAL_MS           30000


typedef struct {
    char wifi_ssid[32];
    char wifi_pass[64];
    esp_log_level_t log_level;
} __attribute__ ((packed)) nvs_device_config_t;


#endif  // SRC_CONFIG_H_
