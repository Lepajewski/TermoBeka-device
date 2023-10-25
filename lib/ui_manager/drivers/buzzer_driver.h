#ifndef LIB_UI_MANAGER_DRIVERS_BUZZER_DRIVER_H_
#define LIB_UI_MANAGER_DRIVERS_BUZZER_DRIVER_H_


#include "driver/gpio.h"


#ifdef __cplusplus
extern "C" {
#endif

esp_err_t buzzer_init(gpio_num_t pin);
esp_err_t buzzer_deinit(gpio_num_t pin);

esp_err_t buzzer_on(gpio_num_t pin);
esp_err_t buzzer_off(gpio_num_t pin);

#ifdef __cplusplus
}
#endif

#endif  // LIB_UI_MANAGER_DRIVERS_BUZZER_DRIVER_H_
