#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "logger.h"

#include "nvs_manager.h"


const char * const TAG = "NVS";


esp_err_t nvs_check() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    return err;
}

esp_err_t nvs_read_config(nvs_device_config_t *config, uint8_t *config_found) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    *config_found = 0;
    err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_get_u8(nvs_handle, "log_level", (uint8_t*) &config->log_level);
    switch (err) {
        case ESP_ERR_NVS_NOT_FOUND:
            {
                esp_err_t err = nvs_set_u8(nvs_handle, "log_level", DEFAULT_LOG_LEVEL);
                TB_LOGI(TAG, "log_level: could not read, set to: %" PRIu8, DEFAULT_LOG_LEVEL);
                if (err != ESP_OK) {
                    return err;
                }
                break;
            }
        case ESP_OK: 
        {
            *config_found = 1;
            break;
        } 
        default: return err;
    }

    return err;
}

