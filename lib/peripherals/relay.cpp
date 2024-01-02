#include "relay.h"


Relay::Relay(ExpanderController &controller, pca9539_polarity polarity, pca9539_pin_num pin, RelayType type) :
    controller(controller),
    polarity(polarity),
    pin(pin),
    type(type),
    toggled(false)
{}

Relay::~Relay() {}

RelayType Relay::get_type() {
    return this->type;
}

esp_err_t Relay::setup() {
    return this->controller.setup_pin(this->polarity, this->pin, PIN_MODE_OUTPUT);
}

void Relay::toggle_on() {
    this->controller.set_output_pin_state(this->pin, PIN_STATE_HIGH);
}

void Relay::toggle_off() {
    this->controller.set_output_pin_state(this->pin, PIN_STATE_LOW);
}
