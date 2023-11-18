#include "logger.h"

#include "sd_manager.h"
#include "sd_manager_task.h"


const char * const TAG = "SDMgrTask";


void sdManagerTask(void *pvParameters) {
    TB_LOGI(TAG, "start");

    SDManager manager;

    manager.begin();

    while (1) {
        manager.process_events();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
