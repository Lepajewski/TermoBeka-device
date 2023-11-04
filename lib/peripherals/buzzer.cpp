#include "global_config.h"

#include "buzzer.h"


Buzzer::Buzzer() :
    pin(PIN_BUZZER) {
    buzzer_init(this->pin);
};

Buzzer::Buzzer(gpio_num_t pin) {
    this->pin = pin;

    buzzer_init(this->pin);
}

Buzzer::~Buzzer() {
    buzzer_deinit(this->pin);
}

void Buzzer::on() {
    buzzer_on(this->pin);
}

void Buzzer::off() {
    buzzer_off(this->pin);
}

gpio_num_t Buzzer::getPin() {
    return this->pin;
}
