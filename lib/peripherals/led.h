#ifndef LIB_PERIPHERALS_LED_H_
#define LIB_PERIPHERALS_LED_H_


#include "drivers/pca9539_driver.h"
#include "expander_controller.h"


typedef uint8_t ColorMask;
enum class Color { NONE, R, G, RG, B, RB, GB, RGB };


typedef struct {
    pca9539_pin_num red;
    pca9539_pin_num green;
    pca9539_pin_num blue;
} led_pins_t;


class ExpanderController;


class LED {
 private:
    ExpanderController &controller;
    pca9539_polarity polarity;
    led_pins_t pins;
    Color current_color;

 public:
    LED(ExpanderController &controller, pca9539_polarity polarity, led_pins_t pins);
    ~LED();

    esp_err_t setup();
    void set_color(Color color);
};


#endif  // LIB_PERIPHERALS_LED_H_
