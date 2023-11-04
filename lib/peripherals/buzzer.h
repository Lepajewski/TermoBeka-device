#ifndef LIB_PERIPHERALS_BUZZER_H_
#define LIB_PERIPHERALS_BUZZER_H_


#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "hal/gpio_types.h"
#include "drivers/buzzer_driver.h"
#include "global_config.h"


#define DEFAULT_BEEP_PERIOD_MS      200


class Buzzer {
 private:
    gpio_num_t pin;
    TimerHandle_t timer;

    void initTimer();
 public:
    Buzzer(gpio_num_t pin=PIN_BUZZER);
    ~Buzzer();

    void on();
    void off();
    void beep(uint32_t timeout);

    gpio_num_t getPin();
};

#endif  // LIB_PERIPHERALS_BUZZER_WRAPPER_H_
