#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_chip_info.h"
#include "sdkconfig.h"
#include "esp_flash.h"

#include "global_config.h"
#include "nvs_manager.h"
#include "logger.h"

#include "system_manager_task.h"
#include "ui_manager_task.h"
#include "sd_manager_task.h"
#include "wifi_manager_task.h"


static const char * const TAG = "MAIN";


// main must be C function
#ifdef __cplusplus
extern "C" {
    void app_main(void);
}
#endif



void app_main(void)
{
    // default log level at at startup
    esp_log_level_set("*", DEFAULT_LOG_LEVEL);

    // start System Manager Task
    xTaskCreatePinnedToCore(systemManagerTask, "SysMgr", 4096, NULL, 1, NULL, 1);

    // start UI Manager Task
    xTaskCreatePinnedToCore(uiManagerTask, "UIMgr", 4096, NULL, 1, NULL, 1);

    // start SD Manager Task
    xTaskCreatePinnedToCore(sdManagerTask, "SDMgr", 8192, NULL, 1, NULL, 0);

    // start WiFi Manager Task
    xTaskCreatePinnedToCore(wifiManagerTask, "WiFiMgr", 4096, NULL, 1, NULL, 0);
}
