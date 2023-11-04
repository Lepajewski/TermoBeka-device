#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "logger.h"

#include "system_manager_task.h"


const char * const TAG = "SysMgrTask";


void systemManagerTask(void *pvParameters) {
    TB_LOGI(TAG, "start");

    // TB_LOGI(TAG, "configure UART");
    // sysMgr.configure_uart();

    TB_LOGI(TAG, "init console");
    sysMgr.init_console();


    while (1) {
        TB_LOGI(TAG, "hello");
        sysMgr.poll_event();

        // sysMgr.process_command();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
