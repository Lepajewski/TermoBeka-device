#ifndef LIB_NVS_MANAGER_H_
#define LIB_NVS_MANAGER_H_

#include "esp_err.h"
#include "global_config.h"


esp_err_t nvs_check();
esp_err_t read_nvs_config(nvs_device_config_t *config, uint8_t *config_found);


#endif  // LIB_NVS_MANAGER_H_
