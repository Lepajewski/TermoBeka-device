#include "buzzer.h"


Buzzer::Buzzer(gpio_num_t pin) :
    pin(pin)
{
    initTimer();
    buzzer_init(this->pin);
    off();
}

Buzzer::~Buzzer() {
    buzzer_deinit(this->pin);
}

void Buzzer::initTimer() {
    auto onTimer = [](TimerHandle_t xTimer) {
        Buzzer *b = static_cast<Buzzer*>(pvTimerGetTimerID(xTimer));
        assert(b);
        b->off();
    };

    this->timer = xTimerCreate(
        "buzzerTimer",
        pdMS_TO_TICKS(DEFAULT_BEEP_PERIOD_MS),
        pdFALSE,
        static_cast<void*>(this), onTimer);
}

void Buzzer::on() {
    buzzer_on(this->pin);
}

void Buzzer::off() {
    buzzer_off(this->pin);
}

void Buzzer::beep(uint32_t timeout) {
    on();
    xTimerChangePeriod(this->timer, pdMS_TO_TICKS(timeout), 0);
}

gpio_num_t Buzzer::getPin() {
    return this->pin;
}
