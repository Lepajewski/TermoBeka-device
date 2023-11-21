#include "logger.h"
#include "wifi_manager.h"
#include "wifi_manager_task.h"


const char * const TAG = "WiFiMgrTask";


void wifiManagerTask(void *pvParameters) {
    TB_LOGI(TAG, "start");

    WiFiManager manager;

    //manager.begin();

    while (1) {
        manager.process_events();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
