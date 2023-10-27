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


    // setup buttons

    while (1) {
        TB_LOGI(TAG, "hello");

        // Test Buzzer
        buzzer.on();

        vTaskDelay(pdMS_TO_TICKS(200));

        buzzer.off();

        pca9539_pin_mode mode = expander.get_pin_mode(P1_5);
        TB_LOGI(TAG, "Pin %s mode: %u", pca9539_pin_num_to_s(P1_5), mode);

        expander.set_pin_mode(P0_6, PIN_MODE_OUTPUT);
        mode = expander.get_pin_mode(P0_6);
        TB_LOGI(TAG, "Pin %s mode: %u", pca9539_pin_num_to_s(P0_6), mode);

        vTaskDelay(pdMS_TO_TICKS(1));

        expander.set_pin_mode(P0_6, PIN_MODE_OUTPUT);
        mode = expander.get_pin_mode(P0_6);
        TB_LOGI(TAG, "Pin %s mode: %u", pca9539_pin_num_to_s(P0_6), mode);

        vTaskDelay(pdMS_TO_TICKS(2000));

        expander.set_pin_mode(P0_6, PIN_MODE_INPUT);
        mode = expander.get_pin_mode(P0_6);
        TB_LOGI(TAG, "Pin %s mode: %u", pca9539_pin_num_to_s(P0_6), mode);

    }
}
