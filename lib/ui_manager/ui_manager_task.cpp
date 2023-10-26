#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "logger.h"

#include "ui_manager_task.h"

#include "buzzer.h"


const char * const TAG = "UIMgrTask";


void uiManagerTask(void *pvParameters) {
    TB_LOGI(TAG, "start");

    // setup UI peripherals
    // setup display

    // setup BUZZER & LEDS
    Buzzer buzzer;

    // setup GPIO expander

    // setup buttons

    while (1) {
        TB_LOGI(TAG, "hello");

        // Test Buzzer
        buzzer.on();

        vTaskDelay(pdMS_TO_TICKS(200));

        buzzer.off();

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
