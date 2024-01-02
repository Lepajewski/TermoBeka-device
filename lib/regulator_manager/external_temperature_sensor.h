#ifndef LIB_REGULATOR_MANAGER_EXTERNAL_TEMPERATURE_SENSOR_H_
#define LIB_REGULATOR_MANAGER_EXTERNAL_TEMPERATURE_SENSOR_H_


#include "inttypes.h"

#include "esp_err.h"
#include "drivers/onewire/onewire.h"

#include "global_config.h"

#define OW_READ_ROM                 0x33
#define OW_MATCH_ROM                0x55
#define OW_SKIP_ROM                 0xCC
#define OW_ALARM_SEARCH             0xEC
#define OW_SEARCH_ROM               0xF0

#define DS18B20_CONVERT_T           0x44
#define DS18B20_WRITE_SCRATCHPAD    0x4e
#define DS18B20_READ_SCRATCHPAD     0xbe
#define DS18B20_COPY_SCRATCHPAD     0x48
#define DS18B20_RECALL_EE           0xb8
#define DS18B20_READ_POWER_SUPPLY   0xb4


typedef struct {
   float temperature[NUMBER_OF_EXTERNAL_TEMP_SENSORS];
} external_temperature;


class ExternalTemperatureSensor {
 private:
    uint8_t ow_pin;
    OW ow;
    uint64_t address[NUMBER_OF_EXTERNAL_TEMP_SENSORS] = {
      DS18B20_ADDRESS_1,
      DS18B20_ADDRESS_2,
      DS18B20_ADDRESS_3
    };


 public:
    ExternalTemperatureSensor(uint8_t ow_pin);
    ~ExternalTemperatureSensor();

    esp_err_t setup();

    esp_err_t get_temperature(external_temperature *temperature);

};


#endif  // LIB_REGULATOR_MANAGER_EXTERNAL_TEMPERATURE_SENSOR_H_
