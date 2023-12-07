#include "logger.h"
#include "wifi_manager.h"
#include "wifi_manager_task.h"


const char * const TAG = "WiFiMgrTask";


void wifiManagerTask(void *pvParameters) {
    TB_LOGI(TAG, "start");

    WiFiManager manager;

    while (1) {
        manager.process_events();
        
        manager.send_evt_wifi_strength();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
