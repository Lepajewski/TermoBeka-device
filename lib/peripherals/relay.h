#ifndef LIB_PERIPHERALS_RELAY_H_
#define LIB_PERIPHERALS_RELAY_H_


#include "relay_type.h"
#include "drivers/pca9539_driver.h"
#include "expander_controller.h"


class Relay {
 private:
    ExpanderController &controller;
    pca9539_polarity polarity;
    pca9539_pin_num pin;
    RelayType type;
    bool toggled;

 public:
    Relay(ExpanderController &controller, pca9539_polarity polarity, pca9539_pin_num pin, RelayType type);
    ~Relay();

    RelayType get_type();

    esp_err_t setup();
    void toggle_on();
    void toggle_off();
};


#endif  // LIB_PERIPHERALS_RELAY_H_
