#include "logger.h"
#include "system_manager.h"
#include "system_manager_task.h"


const char * const TAG = "SysMgrTask";


void systemManagerTask(void *pvParameters) {
    TB_LOGI(TAG, "start");
    SystemManager *sysMgr = get_system_manager();

    sysMgr->init_console();
    TB_LOGI(TAG, "init console");


    while (1) {
        TB_LOGI(TAG, "hello");
        sysMgr->poll_event();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
