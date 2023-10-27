#ifndef LIB_PERIPHERALS_PCA9539_DRIVER_H_
#define LIB_PERIPHERALS_PCA9539_DRIVER_H_


#include "driver/gpio.h"
#include "driver/i2c.h"


#define PCA9539_I2C_ADDRESS_LL          0x74
#define PCA9539_I2C_ADDRESS_LH          0x75
#define PCA9539_I2C_ADDRESS_HL          0x76
#define PCA9539_I2C_ADDRESS_HH          0x77

#define PCA9539_REG_IP                  0x00  // input port register
#define PCA9539_REG_OP                  0x02  // output port register
#define PCA9539_REG_PIP                 0x04  // polarity inversion register
#define PCA9539_REG_CP                  0x06  // configuration register

typedef enum {
    PORT_0,
    PORT_1
} pca9539_port_num;

typedef enum {
    P0_0, P0_1, P0_2, P0_3, P0_4, P0_5, P0_6, P0_7,
    P1_0, P1_1, P1_2, P1_3, P1_4, P1_5, P1_6, P1_7
} pca9539_pin_num;

typedef enum {
    PIN_POLARITY_NORMAL,
    PIN_POLARITY_INVERSE
} pca9539_polarity;

typedef enum {
    PIN_MODE_OUTPUT,
    PIN_MODE_INPUT
} pca9539_pin_mode;

typedef enum {
    PIN_STATE_LOW,
    PIN_STATE_HIGH
} pca9539_pin_state;


typedef struct {
    i2c_port_t i2c_port;
    i2c_config_t i2c_config;
    uint8_t addr;
    gpio_num_t pin_interrupt;
} pca9539_cfg_t;


#ifdef __cplusplus
extern "C" {
#endif

const char *pca9539_pin_num_to_s(pca9539_pin_num pin);

esp_err_t pca9539_init(pca9539_cfg_t *cfg);
esp_err_t pca9539_deinit(i2c_port_t i2c_port);

esp_err_t pca9539_get_port_cfg(pca9539_cfg_t *cfg, pca9539_port_num port, uint8_t *port_cfg);
esp_err_t pca9539_set_port_cfg(pca9539_cfg_t *cfg, pca9539_port_num port, uint8_t port_cfg);

esp_err_t pca9539_get_port_polarity(pca9539_cfg_t *cfg, pca9539_port_num port, uint8_t *port_polarity);
esp_err_t pca9539_set_port_polarity(pca9539_cfg_t *cfg, pca9539_port_num port, uint8_t port_polarity);

esp_err_t pca9539_get_pin_polarity(pca9539_cfg_t *cfg, pca9539_pin_num pin, pca9539_polarity *polarity);
esp_err_t pca9539_set_pin_polarity(pca9539_cfg_t *cfg, pca9539_pin_num pin, pca9539_polarity polarity);

esp_err_t pca9539_get_pin_mode(pca9539_cfg_t *cfg, pca9539_pin_num pin, pca9539_pin_mode *pin_mode);
esp_err_t pca9539_set_pin_mode(pca9539_cfg_t *cfg, pca9539_pin_num pin, pca9539_pin_mode pin_mode);

esp_err_t pca9539_get_input_port_state(pca9539_cfg_t *cfg, pca9539_port_num port, uint8_t *state);
esp_err_t pca9539_get_output_port_state(pca9539_cfg_t *cfg, pca9539_port_num port, uint8_t *state);

esp_err_t pca9539_get_input_pin_state(pca9539_cfg_t *cfg, pca9539_pin_num pin, pca9539_pin_state *state);
esp_err_t pca9539_get_output_pin_state(pca9539_cfg_t *cfg, pca9539_pin_num pin, pca9539_pin_state *state);


#ifdef __cplusplus
}
#endif

#endif  // LIB_PERIPHERALS_PCA9539_DRIVER_H_
