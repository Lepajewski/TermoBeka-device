#include "logger.h"
#include "regulator_manager.h"
#include "regulator_manager_task.h"


const char * const TAG = "RegMgrTask";


void regulatorManagerTask(void *pvParameters) {
    TB_LOGI(TAG, "start");

    RegulatorManager manager;

    while (1) {
        manager.process_events();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
