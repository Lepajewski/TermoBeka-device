#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <inttypes.h>
#include "esp_log.h"
#include "driver/gpio.h"

#ifdef DEBUG
#define DEFAULT_LOG_LEVEL                       ESP_LOG_DEBUG
#elif defined(RELEASE)
#define DEFAULT_LOG_LEVEL                       ESP_LOG_ERROR
#endif

// Buzzer configuration
#define PIN_BUZZER                              GPIO_NUM_1

// GPIO Expander configuration
#define PIN_GPIO_EXPANDER_SDA                   GPIO_NUM_9
#define PIN_GPIO_EXPANDER_SCL                   GPIO_NUM_10
#define PIN_GPIO_EXPANDER_INTR                  GPIO_NUM_3  // GPIO_NUM_46
#define GPIO_EXPANDER_FREQ_HZ                   400000

// LCD configuration
#define PIN_LCD_SCLK                            GPIO_NUM_38  // CLK
#define PIN_LCD_DIN                             GPIO_NUM_39  // DIN/MOSI
#define PIN_LCD_DC                              GPIO_NUM_40  // DC - Data/Command
#define PIN_LCD_CS                              GPIO_NUM_41  // CE - Chip Select
#define PIN_LCD_RST                             GPIO_NUM_42  // RST - RESET

// SD configuration
#define PIN_SD_SCLK                             GPIO_NUM_12
#define PIN_SD_MOSI                             GPIO_NUM_13
#define PIN_SD_MISO                             GPIO_NUM_11
#define PIN_SD_CS                               GPIO_NUM_14
#define SD_CARD_MOUNT_POINT                     "/sd"
#define SD_CARD_DATA_RING_BUFFER_LEN            8192


// wifi & ntp configuration
#define WIFI_MAX_SSID_LEN                       32
#define WIFI_DEFAULT_SSID                       "Orange_Swiatlowod_3760"
// #define WIFI_DEFAULT_SSID                       "Net_gora"
#define WIFI_MAX_PASS_LEN                       64
#define WIFI_DEFAULT_PASS                       "2FT752946HF7"
// #define WIFI_DEFAULT_PASS                       "haslotookon"
#define WIFI_RECONNECT_RETRY                    10
#define WIFI_RECONNECT_INTERVAL_MS              60000
#define NTP_DEFAULT_SERVER_ADDR                 "pool.ntp.org"
#define NTP_TIME_ZONE                           "CET-1CEST,M3.5.0/2,M10.5.0/3"
#define NTP_CONNECT_TIMEOUT_MS                  5000
#define NTP_RECONNECT_INTERVAL_MS               15000


// mqtt configuration
#define MQTT_MAX_BROKER_URI_LEN                 32
// #define MQTT_DEFAULT_BROKER_URI                 "mqtts://192.168.1.105:8883"
#define MQTT_DEFAULT_BROKER_URI                 "mqtts://192.168.1.16:8883"
#define MQTT_AUTO_RECONNECT_TIMEOUT_MS          3000
#define MQTT_RECONNECT_RETRY                    3
#define MQTT_RECONNECT_INTERVAL_MS              30000
#define MQTT_MAX_USERNAME_LEN                   32
// #define MQTT_DEFAULT_USERNAME                   "default"
#define MQTT_DEFAULT_USERNAME                   "termobeka"
#define MQTT_MAX_PASSWORD_LEN                   32
// #define MQTT_DEFAULT_PASSWORD                   "default"
#define MQTT_DEFAULT_PASSWORD                   "qwerty"

#define MQTT_CA_CERT_MAX_LEN                    2048
#define MQTT_TLS_CA_SD_CARD_PATH                "certificates/ca.crt"

#define MQTT_TOPIC_PREFIX                       "termobeka/"
#define MQTT_TOPIC_PROFILE_UPDATE               "/profile-update"
#define MQTT_TOPIC_REGULATOR_UPDATE             "/regulator-update"
#define MQTT_TOPIC_SERVER_COMMANDS              "/server-commands"


// Profile configuration
#define PROFILE_MAX_VERTICES                    40
#define PROFILE_MIN_TEMPERATURE                 2000        // = 20*C
#define PROFILE_MAX_TEMPERATURE                 25000       // = 250*C
#define PROFILE_STEP_TIME_MS                    1000
#define PROFILE_MIN_DURATION_MS                 60000
#define PROFILE_MAX_DURATION_MS                 86400000
#define PROFILE_STEP_MIN_ABS_SLOPE_TRESHOLD     1e-3
#define PROFILE_STOPPED_CONST_TIMER_TIMEOUT_MS  UINT32_MAX
#define PROFILE_LONG_TIMEOUT_INTERVAL_MS        1200000
#define PROFILE_UPDATE_TIMER_INTERVAL_MS        5000


typedef struct {
    char ssid[WIFI_MAX_SSID_LEN];
    char pass[WIFI_MAX_PASS_LEN];
} wifi_credentials;

typedef struct {
    char uri[MQTT_MAX_BROKER_URI_LEN];
    char username[MQTT_MAX_USERNAME_LEN];
    char password[MQTT_MAX_PASSWORD_LEN];
} mqtt_credentials;


#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))


#define TIMESTAMP_SIZE                          27
void get_timestamp(char *timestamp);
uint64_t get_time_ms();
uint64_t get_time_since_startup_ms();


#endif  // SRC_CONFIG_H_
