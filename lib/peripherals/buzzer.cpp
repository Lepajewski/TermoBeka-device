#include "buzzer.h"


Buzzer::Buzzer(ExpanderController &controller, pca9539_polarity polarity, pca9539_pin_num pin) :
    controller(controller),
    polarity(polarity),
    pin(pin)
{
    this->initTimer();
}

Buzzer::~Buzzer() {}

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
    this->controller.set_output_pin_state(this->pin, PIN_STATE_HIGH);
}

void Buzzer::off() {
    this->controller.set_output_pin_state(this->pin, PIN_STATE_LOW);
}

esp_err_t Buzzer::setup() {
    esp_err_t err = ESP_OK;

    if ((err = this->controller.setup_pin(this->polarity, this->pin, PIN_MODE_OUTPUT)) != ESP_OK) {
        return err;
    }

    this->off();

    return err;
}

void Buzzer::beep(uint32_t timeout) {
    this->on();
    xTimerChangePeriod(this->timer, pdMS_TO_TICKS(timeout), 0);
}
