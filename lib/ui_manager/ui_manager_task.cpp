#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "logger.h"

#include "ui_manager.h"
#include "ui_manager_task.h"
#include "lcd_controller.h"


const char * const TAG = "UIMgrTask";


void uiManagerTask(void *pvParameters) {
    TB_LOGI(TAG, "start");
    UIManager uiManager;

    uiManager.setup();

    while (1) {
        uiManager.process_events();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
