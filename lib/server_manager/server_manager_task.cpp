#include "logger.h"
#include "server_manager.h"
#include "server_manager_task.h"


const char * const TAG = "ServerMgrTask";


void serverManagerTask(void *pvParameters) {
    TB_LOGI(TAG, "start");

    ServerManager manager;

    while (1) {
        manager.process_events();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

