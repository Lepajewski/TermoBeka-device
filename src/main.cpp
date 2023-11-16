#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_chip_info.h"
#include "sdkconfig.h"
#include "esp_flash.h"

#include "global_config.h"
#include "nvs_manager.h"
#include "logger.h"
#include "system_manager.h"
#include "system_manager_task.h"

#include "ui_manager_task.h"
#include "sd_manager_task.h"


static const char * const TAG = "MAIN";

SystemManager sysMgr;


// main must be C function
extern "C" {
    void app_main(void);
}



void app_main(void)
{
    // default log level at at startup
    esp_log_level_set("*", DEFAULT_LOG_LEVEL);

    // initialize Non-Volatile Storage
    ESP_ERROR_CHECK(nvs_check());

    // start System Manager Task
    xTaskCreatePinnedToCore(systemManagerTask, "SysMgr", 4096, NULL, 1, NULL, 1);

    // start UI Manager Task
    xTaskCreatePinnedToCore(uiManagerTask, "UIMgr", 4096, NULL, 1, NULL, 1);

    // start SD Manager Task
    xTaskCreatePinnedToCore(sdManagerTask, "SDMgr", 8192, NULL, 1, NULL, 0);

    // load configuration from NVS
    nvs_device_config_t config = {};
    uint8_t config_found = 0;
    ESP_ERROR_CHECK(nvs_read_config(&config, &config_found));

    // config log level
    esp_log_level_set("*", config.log_level);

    if (config_found) {
        TB_LOGI(TAG, "NVS config found");
    } else {
        TB_LOGI(TAG, "config not found, using default");
    }

    TB_LOGI(TAG, "log level: %" PRIu8, config.log_level);
}
