#ifndef LIB_NVS_MANAGER_H_
#define LIB_NVS_MANAGER_H_

#include "esp_err.h"
#include "global_config.h"


// class NVSManager {

//  public:
//     NVSManager();
//     ~NVSManager();

// esp_err_t nvs_erase();
esp_err_t nvs_check();

esp_err_t nvs_read_config(nvs_device_config_t *config, uint8_t *config_found);

// esp_err_t nvs_write_config(nvs_device_config_t *config, uint8_t *config_found);
// esp_err_t nvs_write_value(const char *key, void *value, size_t length);
// };




#endif  // LIB_NVS_MANAGER_H_
