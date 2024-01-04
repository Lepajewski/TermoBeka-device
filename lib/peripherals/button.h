#ifndef LIB_PERIPHERALS_BUTTON_H_
#define LIB_PERIPHERALS_BUTTON_H_


#include <functional>

#include "drivers/pca9539_driver.h"
#include "drivers/pca9539_intr_driver.h"
#include "expander_controller.h"


#define BUTTON_MIN_VALID_PRESS_TIME_MS       50
#define BUTTON_MAX_SHORT_PRESS_TIME_MS       350
#define BUTTON_MIN_LONG_PRESS_TIME_MS        351
#define BUTTON_MAX_LONG_PRESS_TIME_MS        5000


enum class PressType {
   SHORT_PRESS,
   LONG_PRESS,
   INVALID_PRESS
};

enum class ButtonType {
   L_UP = P1_4,
   L_MID = P1_5,
   L_BOT = P1_6,
   R_UP = P0_2,
   R_MID = P0_1,
   R_BOT = P0_0
};

class ExpanderController;
    
class Button {
 private:
    ExpanderController &controller;
    pca9539_polarity polarity;
    pca9539_pin_num num;

    uint64_t last_press_timestamp;

    void min_press_callback();

    void press(uint64_t timestamp);
    void release(uint64_t timestamp);

    std::function<void(Button*, PressType)> callback;
 public:
    Button(ExpanderController &controller, pca9539_polarity polarity, pca9539_pin_num num);
    ~Button();

    esp_err_t setup();

    pca9539_pin_num get_pin_num();
    pca9539_pin_state get_pin_state();

    ButtonType get_button_type();

    void process_event(pin_change_type change, uint64_t timestamp);
    void set_callback(std::function<void(Button*, PressType)> cb);
};

#endif  // LIB_PERIPHERALS_BUTTON_H_
