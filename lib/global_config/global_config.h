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

// Shared SPI configuration
#define PIN_SPI_MOSI                            GPIO_NUM_35
#define PIN_SPI_CLK                             GPIO_NUM_36
#define PIN_SPI_MISO                            GPIO_NUM_37

// I2C configuration
#define PIN_I2C_SDA                             GPIO_NUM_16
#define PIN_I2C_SCL                             GPIO_NUM_17

// GPIO Expander configuration
#define PIN_GPIO_EXPANDER_SDA                   PIN_I2C_SDA
#define PIN_GPIO_EXPANDER_SCL                   PIN_I2C_SCL
#define PIN_GPIO_EXPANDER_1_INTR                GPIO_NUM_3
#define PIN_GPIO_EXPANDER_1_RESET               GPIO_NUM_20
#define PIN_GPIO_EXPANDER_2_RESET               GPIO_NUM_15
#define GPIO_EXPANDER_FREQ_HZ                   400000

// LCD configuration
#define PIN_LCD_SCLK                            PIN_SPI_CLK  // CLK
#define PIN_LCD_DIN                             PIN_SPI_MOSI // DIN/MOSI
#define PIN_LCD_DC                              GPIO_NUM_19  // DC - Data/Command
#define PIN_LCD_CS                              GPIO_NUM_18  // CE - Chip Select
#define PIN_LCD_RST                             GPIO_NUM_8   // RST - RESET

// SD configuration
#define PIN_SD_SCLK                             GPIO_NUM_12
#define PIN_SD_MOSI                             GPIO_NUM_11
#define PIN_SD_MISO                             GPIO_NUM_13
#define PIN_SD_CS                               GPIO_NUM_10
#define SD_CARD_MOUNT_POINT                     "/sd"
#define SD_CARD_DATA_RING_BUFFER_LEN            8192

// PATHS
#define CONFIG_INI_PATH                         "config.ini"
#define PROFILE_FOLDER_PATH                     "profiles"

// wifi & ntp configuration
#define WIFI_MAX_SSID_LEN                       32
#define WIFI_DEFAULT_SSID                       "default"
#define WIFI_MAX_PASS_LEN                       64
#define WIFI_DEFAULT_PASS                       "default"
#define WIFI_RECONNECT_RETRY                    10
#define WIFI_RECONNECT_INTERVAL_MS              60000
#define NTP_DEFAULT_SERVER_ADDR                 "pool.ntp.org"
#define NTP_TIME_ZONE                           "CET-1CEST,M3.5.0/2,M10.5.0/3"
#define NTP_CONNECT_TIMEOUT_MS                  5000
#define NTP_RECONNECT_INTERVAL_MS               15000


// mqtt configuration
#define MQTT_MAX_BROKER_URI_LEN                 32
#define MQTT_DEFAULT_BROKER_URI                 "localhost"
#define MQTT_AUTO_RECONNECT_TIMEOUT_MS          3000
#define MQTT_RECONNECT_RETRY                    3
#define MQTT_RECONNECT_INTERVAL_MS              30000
#define MQTT_MAX_USERNAME_LEN                   32
#define MQTT_DEFAULT_USERNAME                   "default"
#define MQTT_MAX_PASSWORD_LEN                   32
#define MQTT_DEFAULT_PASSWORD                   "default"

#define MQTT_CA_CERT_MAX_LEN                    2048
#define MQTT_TLS_CA_SD_CARD_PATH                "certificates/ca.crt"

#define MQTT_TOPIC_PREFIX                       "termobeka/"
#define MQTT_TOPIC_FROM_DEVICE                  "/from-device"
#define MQTT_TOPIC_TO_DEVICE                    "/to-device"


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
#define PROFILE_UPDATE_TIMER_INTERVAL_MS        10000


// Regulator configuration
#define INTERNAL_TEMP_SENS_MIN_RANGE_C          -10
#define INTERNAL_TEMP_SENS_MAX_RANGE_C          80

#define REGULATOR_SAMPLING_RATE_MS              1000
#define REGULATOR_UPDATE_TIMER_INTERVAL_MS      1000
#define REGULATOR_HYSTERESIS_UP_C               0.2f
#define REGULATOR_HYSTERESIS_DOWN_C             0.2f



#define NUMBER_OF_EXTERNAL_TEMP_SENSORS         3
#define NUMBER_OF_RTDS                          5

#define DS18B20_ADDRESS_1                       0x6a01191ee2ef3428
#define DS18B20_ADDRESS_2                       0x701191f0a97c228
#define DS18B20_ADDRESS_3                       0x8e01191b0c925e28

#define PIN_EXTERNAL_TEMP_SENSORS               GPIO_NUM_7
#define PIN_RTD_MOSI                            PIN_SPI_MOSI
#define PIN_RTD_MISO                            PIN_SPI_MISO
#define PIN_RTD_CLK                             PIN_SPI_CLK
#define PIN_RTD_CS_0                            GPIO_NUM_14
#define PIN_RTD_CS_1                            GPIO_NUM_47
#define PIN_RTD_CS_2                            GPIO_NUM_38
#define PIN_RTD_CS_3                            GPIO_NUM_40
#define PIN_RTD_CS_4                            GPIO_NUM_42
#define PIN_RTD_DRDY_0                          GPIO_NUM_9
#define PIN_RTD_DRDY_1                          GPIO_NUM_21
#define PIN_RTD_DRDY_2                          GPIO_NUM_48
#define PIN_RTD_DRDY_3                          GPIO_NUM_39
#define PIN_RTD_DRDY_4                          GPIO_NUM_41



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
