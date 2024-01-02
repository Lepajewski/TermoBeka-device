#ifndef LIB_PERIPHERALS_BUZZER_H_
#define LIB_PERIPHERALS_BUZZER_H_


#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "drivers/pca9539_driver.h"
#include "expander_controller.h"


#define DEFAULT_BEEP_PERIOD_MS      200


class ExpanderController;


class Buzzer {
 private:
    ExpanderController &controller;
    pca9539_polarity polarity;
    pca9539_pin_num pin;

    TimerHandle_t timer;

    void initTimer();
    void on();
    void off();
 public:
    Buzzer(ExpanderController &controller, pca9539_polarity polarity, pca9539_pin_num pin);
    ~Buzzer();

    esp_err_t setup();
    void beep(uint32_t timeout);
};

#endif  // LIB_PERIPHERALS_BUZZER_WRAPPER_H_
