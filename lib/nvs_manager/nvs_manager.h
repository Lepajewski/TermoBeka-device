#ifndef LIB_NVS_MANAGER_H_
#define LIB_NVS_MANAGER_H_

#include "esp_err.h"
#include "nvs_config.h"


class NVSManager {
 private:
    nvs_device_config_t default_config;
    nvs_device_config_t config;

    esp_err_t nvs_check();
 public:
    NVSManager();
    ~NVSManager();

    void begin();
    esp_err_t save_default_config();
    esp_err_t read_default_config();

    nvs_device_config_t *get_default_config();
    nvs_device_config_t *get_config();
};


#endif  // LIB_NVS_MANAGER_H_
