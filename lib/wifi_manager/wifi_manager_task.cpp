#include "logger.h"
#include "wifi_manager.h"
#include "wifi_manager_task.h"

#define SEND_WIFI_STRENGTH_ITERATIONS_COOLDOWN 500

const char * const TAG = "WiFiMgrTask";


void wifiManagerTask(void *pvParameters) {
    TB_LOGI(TAG, "start");

    WiFiManager manager;

    int i = 0;
    while (1) {
        manager.process_events();
        
        if (i++ > SEND_WIFI_STRENGTH_ITERATIONS_COOLDOWN) {
            manager.send_evt_wifi_strength();
            i = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
