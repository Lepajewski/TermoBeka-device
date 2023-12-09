#ifndef LIB_REGULATOR_MANAGER_INTERNAL_TEMPERATURE_SENSOR_H_
#define LIB_REGULATOR_MANAGER_INTERNAL_TEMPERATURE_SENSOR_H_


#include "inttypes.h"
#include "esp_err.h"


#ifdef __cplusplus
extern "C" {
#endif


esp_err_t install_internal_temperature_sensor(int8_t range_min, int8_t range_max);
esp_err_t uninstall_internal_temperature_sensor();
esp_err_t get_internal_temperature(float *temperature);


#ifdef __cplusplus
}
#endif


#endif  // LIB_REGULATOR_MANAGER_INTERNAL_TEMPERATURE_SENSOR_H_
