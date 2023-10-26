#include "buzzer_driver.h"

esp_err_t buzzer_init(gpio_num_t pin) {
    esp_err_t err = ESP_OK;
    
    if ((err = buzzer_deinit(pin)) != ESP_OK) {
        return err;
    }

    if ((err = gpio_set_direction(pin, GPIO_MODE_OUTPUT)) != ESP_OK) {
        return err;
    }

    return err;
}

esp_err_t buzzer_deinit(gpio_num_t pin) {
    return gpio_reset_pin(pin);
}

esp_err_t buzzer_on(gpio_num_t pin) {
    return gpio_set_level(pin, 1);
}

esp_err_t buzzer_off(gpio_num_t pin) {
    return gpio_set_level(pin, 0);
}
