#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "logger.h"

#include "ui_manager_task.h"

#include "buzzer.h"
#include "gpio_expander.h"


const char * const TAG = "UIMgrTask";


void uiManagerTask(void *pvParameters) {
    TB_LOGI(TAG, "start");

    // setup UI peripherals
    // setup display

    // setup BUZZER & LEDS
    Buzzer buzzer;

    // setup GPIO expander
    GPIOExpander expander;

    expander.begin();

    // setup buttons

    while (1) {
        expander.poll_intr_events();

        // Test Buzzer
        // buzzer.on();

        // vTaskDelay(pdMS_TO_TICKS(200));

        // buzzer.off();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
