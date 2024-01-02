#ifndef LIB_PERIPHERALS_GPIO_EXPANDER_H_
#define LIB_PERIPHERALS_GPIO_EXPANDER_H_


#include "drivers/pca9539_driver.h"
#include "drivers/pca9539_intr_driver.h"
#include "expander_controller.h"
#include "button.h"
#include "buzzer.h"
#include "led.h"


#define EXPANDER_EVT_INTR_QUEUE_SIZE      10
#define USED_BUTTONS                      6
#define USED_LEDS                         3


class LED;


class GPIOExpander {
 private:   
    pca9539_cfg_t config;
    TaskHandle_t intr_task_handle;
    QueueHandle_t intr_evt_queue;

    ExpanderController controller;
    Button buttons[USED_BUTTONS] = {
        Button(controller, PIN_POLARITY_INVERSE, P1_4),
        Button(controller, PIN_POLARITY_INVERSE, P0_2),
        Button(controller, PIN_POLARITY_INVERSE, P1_5),
        Button(controller, PIN_POLARITY_INVERSE, P0_1),
        Button(controller, PIN_POLARITY_INVERSE, P1_6),
        Button(controller, PIN_POLARITY_INVERSE, P0_0),
    };
    Buzzer buzzer = Buzzer(controller, PIN_POLARITY_NORMAL, P1_7);
    LED *leds;

    esp_err_t init_evt_intr_queue();
    void deinit_evt_intr_queue();
    void start_evt_intr_task();
    void process_intr_event(pca9539_intr_evt_t *intr_evt);
    void setup_buttons();
    void setup_buzzer();
    void setup_leds();
    Button *lookup_button(pca9539_pin_num num);

   std::function<void(Button*, PressType)> button_callback;
 public:
    GPIOExpander();
    GPIOExpander(pca9539_cfg_t *config);
    ~GPIOExpander();

    void begin();
    void end();

    void poll_intr_events();
    void set_callback(std::function<void(Button*, PressType)> cb);
    void set_backlight_color(Color color);
    void buzzer_beep(uint32_t timeout);
};

#endif  // LIB_PERIPHERALS_GPIO_EXPANDER_H_
