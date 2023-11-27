#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "logger.h"

#include "ui_manager.h"
#include "ui_manager_task.h"
#include "lcd_controller.h"
#include "global_config.h"

#include <chrono>

const char * const TAG = "UIMgrTask";

void uiManagerTask(void *pvParameters) {
    TB_LOGI(TAG, "start");
    UIManager uiManager;

    // setup UI peripherals
    // setup display

    uiManager.setup();

    auto end = std::chrono::system_clock::now();

    while (1) {
        auto start = std::chrono::system_clock::now();
        std::chrono::duration<float> frame_duration = start - end;
        float d_time = frame_duration.count();
        end = start;

        uiManager.process_events();

        uiManager.update(d_time);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
