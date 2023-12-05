#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "logger.h"

#include "ui_manager.h"
#include "ui_manager_task.h"
#include "lcd_controller.h"
#include "global_config.h"

const char * const TAG = "UIMgrTask";

void uiManagerTask(void *pvParameters) {
    TB_LOGI(TAG, "start");
    UIManager uiManager;

    uiManager.setup();

    uint64_t start = get_time_since_startup_ms();

    while (1) {
        uint64_t end = get_time_since_startup_ms();
        float d_time = (end - start) / 1000.0f;
        start = end;

        uiManager.process_events();

        uiManager.update(d_time);

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
