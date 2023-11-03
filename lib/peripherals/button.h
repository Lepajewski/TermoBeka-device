#ifndef LIB_PERIPHERALS_BUTTON_H_
#define LIB_PERIPHERALS_BUTTON_H_


// #include "freertos/timers.h"

#include "drivers/pca9539_driver.h"
#include "expander_controller.h"


#define BUTTON_MIN_VALID_PRESS_TIME_MS       10
#define BUTTON_MAX_SINGLE_PRESS_TIME_MS      1000
#define BUTTON_MIN_LONG_PRESS_TIME_MS        1001
#define BUTTON_MAX_LONG_PRESS_TIME_MS        5000
// maximum time between single presses to determine double press
#define BUTTON_MAX_TIME_BETWEEN_PRESS_MS     500  


class ExpanderController;

    
class Button {
 private:
    ExpanderController &controller;
    pca9539_polarity polarity;
    pca9539_pin_num num;

    uint64_t last_press_timestamp;
    uint64_t last_release_timestamp;

   //  TimerHandle_t min_valid_press_timer;
   //  TimerHandle_t max_single_press_timer;
   //  TimerHandle_t min_long_press_timer;
   //  TimerHandle_t max_long_press_timer;

 public:
    Button(ExpanderController &controller, pca9539_polarity polarity, pca9539_pin_num num);
    ~Button();

    esp_err_t setup();

    pca9539_pin_num get_pin_num();
    pca9539_pin_state get_pin_state();
};

#endif  // LIB_PERIPHERALS_BUTTON_H_
