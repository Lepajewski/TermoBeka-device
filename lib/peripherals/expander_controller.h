#ifndef LIB_PERIPHERALS_EXPANDER_CONTROLLER_H_
#define LIB_PERIPHERALS_EXPANDER_CONTROLLER_H_


#include "drivers/pca9539_driver.h"


class ExpanderController {
 private:
    bool initialized;
    pca9539_cfg_t config;

 public:
    ExpanderController();
    ExpanderController(pca9539_cfg_t config);
    ~ExpanderController();

    void begin();
    void end();

    pca9539_pin_state get_input_pin_state(pca9539_pin_num pin);
    esp_err_t setup_pin(pca9539_polarity polarity, pca9539_pin_num pin, pca9539_pin_mode mode);
};


#endif  // LIB_PERIPHERALS_EXPANDER_CONTROLLER_H_
