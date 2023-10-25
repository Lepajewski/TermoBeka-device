#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "logger.h"

#include "ui_manager_task.h"


const char * const TAG = "UIMgrTask";


void uiManagerTask(void *pvParameters) {
    TB_LOGI(TAG, "start");

    // setup UI peripherals
    // setup display

    // setup BUZZER & LEDS

    // setup GPIO expander

    // setup buttons

    while (1) {
        TB_LOGI(TAG, "hello");

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
