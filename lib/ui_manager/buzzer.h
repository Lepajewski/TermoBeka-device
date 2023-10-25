#ifndef LIB_UI_MANAGER_BUZZER_H_
#define LIB_UI_MANAGER_BUZZER_H_

#include "hal/gpio_types.h"
#include "drivers/buzzer_driver.h"


class Buzzer {
 private:
    gpio_num_t pin;

 public:
    Buzzer();
    Buzzer(gpio_num_t pin);
    ~Buzzer();

    void beep(uint32_t timeout);
    void on();
    void off();

    gpio_num_t getPin();

};

#endif  // LIB_UI_MANAGER_BUZZER_WRAPPER_H_
