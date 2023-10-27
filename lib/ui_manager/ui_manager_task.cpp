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

    pca9539_pin_mode mode;

    expander.set_pin_mode(P0_0, PIN_MODE_INPUT);
    expander.set_pin_mode(P0_1, PIN_MODE_INPUT);
    expander.set_pin_mode(P0_2, PIN_MODE_INPUT);
    expander.set_pin_mode(P0_3, PIN_MODE_INPUT);
    expander.set_pin_mode(P0_4, PIN_MODE_INPUT);
    expander.set_pin_mode(P0_5, PIN_MODE_INPUT);

    // setup buttons

    while (1) {
        // Test Buzzer
        buzzer.on();

        vTaskDelay(pdMS_TO_TICKS(200));

        buzzer.off();

        pca9539_pin_state state = expander.get_input_pin_state(P0_0);
        TB_LOGI(TAG, "P0_0 state: %u", state);

        // expander.set_pin_mode(P0_6, PIN_MODE_OUTPUT);
        // mode = expander.get_pin_mode(P0_6);
        // TB_LOGI(TAG, "Pin %s mode: %u", pca9539_pin_num_to_s(P0_6), mode);


        vTaskDelay(pdMS_TO_TICKS(2000));

        // expander.set_pin_mode(P0_6, PIN_MODE_INPUT);
        // mode = expander.get_pin_mode(P0_6);
        // TB_LOGI(TAG, "Pin %s mode: %u", pca9539_pin_num_to_s(P0_6), mode);

    }
}
