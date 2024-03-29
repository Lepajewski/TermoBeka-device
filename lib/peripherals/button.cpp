#include <cinttypes>

#include "button.h"


Button::Button(ExpanderController &controller, pca9539_polarity polarity, pca9539_pin_num num) :
    controller(controller),
    polarity(polarity),
    num(num),
    last_press_timestamp(0)
{}

Button::~Button() {}

pca9539_pin_num Button::get_pin_num() {
    return this->num;
}

pca9539_pin_state Button::get_pin_state() {
    return this->controller.get_input_pin_state(this->num);
}

ButtonType Button::get_button_type() {
    return static_cast<ButtonType>(get_pin_num());
}

esp_err_t Button::setup() {
    return this->controller.setup_pin(this->polarity, this->num, PIN_MODE_INPUT);
}

void Button::min_press_callback() {
}

void Button::press(uint64_t timestamp) {
    this->last_press_timestamp = timestamp;
}

void Button::release(uint64_t timestamp) {
    uint64_t hold_time = (uint64_t) ((timestamp - this->last_press_timestamp) / 1000);
    
    if (hold_time <= BUTTON_MIN_VALID_PRESS_TIME_MS) {
        this->callback(this, PressType::INVALID_PRESS);
    } else if (hold_time <= BUTTON_MAX_SHORT_PRESS_TIME_MS) {
        this->callback(this, PressType::SHORT_PRESS);
    } else if (hold_time <= BUTTON_MAX_LONG_PRESS_TIME_MS) {
        this->callback(this, PressType::LONG_PRESS);
    } else {
        this->callback(this, PressType::INVALID_PRESS);
    }
}

void Button::process_event(pin_change_type change, uint64_t timestamp) {
    switch (change) {
        case PIN_CHANGE_1_TO_0:
            release(timestamp);
            break;
        case PIN_CHANGE_0_TO_1:
            press(timestamp);
            break;
        case PIN_NO_CHANGE:
        default:
            break;
    }
}

void Button::set_callback(std::function<void(Button*, PressType)> cb) {
    this->callback = cb;
}
