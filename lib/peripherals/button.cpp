#include "button.h"


Button::Button(ExpanderController &controller, pca9539_polarity polarity, pca9539_pin_num num) :
    controller(controller),
    polarity(polarity),
    num(num)
{}

Button::~Button() {}

pca9539_pin_num Button::get_pin_num() {
    return this->num;
}

pca9539_pin_state Button::get_pin_state() {
    return this->controller.get_input_pin_state(this->num);
}

esp_err_t Button::setup() {
    return this->controller.setup_pin(this->polarity, this->num, PIN_MODE_INPUT);
}
