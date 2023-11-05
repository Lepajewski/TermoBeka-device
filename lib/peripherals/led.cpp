#include "led.h"

LED::LED(ExpanderController &controller, pca9539_polarity polarity, led_pins_t pins) :
    controller(controller),
    polarity(polarity),
    pins(pins),
    current_color(Color::NONE)
{}

LED::~LED() {};

esp_err_t LED::setup() {
    esp_err_t err = ESP_OK;

    if ((err = this->controller.setup_pin(this->polarity, this->pins.red, PIN_MODE_OUTPUT))) {
        return err;
    }
    if ((err = this->controller.setup_pin(this->polarity, this->pins.green, PIN_MODE_OUTPUT))) {
        return err;
    }
    if ((err = this->controller.setup_pin(this->polarity, this->pins.blue, PIN_MODE_OUTPUT))) {
        return err;
    }
    return err;
}

void LED::set_color(Color color) {
    this->current_color = color;
    uint8_t c = (uint8_t)this->current_color;

    if ((c & (uint8_t)Color::R) == (uint8_t)Color::R) {
        this->controller.set_output_pin_state(this->pins.red, PIN_STATE_HIGH);
    } else {
        this->controller.set_output_pin_state(this->pins.red, PIN_STATE_LOW);
    }

    if ((c & (uint8_t)Color::G) == (uint8_t)Color::G) {
        this->controller.set_output_pin_state(this->pins.green, PIN_STATE_HIGH);
    } else {
        this->controller.set_output_pin_state(this->pins.green, PIN_STATE_LOW);
    }

    if ((c & (uint8_t)Color::B) == (uint8_t)Color::B) {
        this->controller.set_output_pin_state(this->pins.blue, PIN_STATE_HIGH);
    } else {
        this->controller.set_output_pin_state(this->pins.blue, PIN_STATE_LOW);
    }
}
