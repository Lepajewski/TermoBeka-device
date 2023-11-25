#ifndef LIB_NVS_MANAGER_H_
#define LIB_NVS_MANAGER_H_

#include "esp_err.h"
#include "nvs_config.h"


class NVSManager {
 private:
    nvs_device_config_t default_config;
    nvs_device_config_t config;

    esp_err_t nvs_check();

    esp_err_t save(const char* key, nvs_device_config_t *config);
    esp_err_t load(const char* key, nvs_device_config_t *config);
 public:
    NVSManager();
    ~NVSManager();

    void begin();
    esp_err_t save_default_config();
    esp_err_t load_default_config();
    esp_err_t save_config(nvs_device_config_t *config);
    esp_err_t load_config();

    nvs_device_config_t *get_default_config();
    nvs_device_config_t *get_config();
};


#endif  // LIB_NVS_MANAGER_H_
