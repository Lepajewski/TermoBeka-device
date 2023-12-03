#include "logger.h"

#include "profile_manager.h"
#include "profile_manager_task.h"


const char * const TAG = "ProfMgrTask";


void profileManagerTask(void *pvParameters) {
    TB_LOGI(TAG, "start");

    ProfileManager manager;

    manager.begin();

    while (1) {
        manager.process_events();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
