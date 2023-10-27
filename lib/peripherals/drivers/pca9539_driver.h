#ifndef LIB_PERIPHERALS_PCA9539_DRIVER_H_
#define LIB_PERIPHERALS_PCA9539_DRIVER_H_


#include "driver/gpio.h"
#include "driver/i2c.h"


#define PCA9539_I2C_ADDRESS_LL          0x74
#define PCA9539_I2C_ADDRESS_LH          0x75
#define PCA9539_I2C_ADDRESS_HL          0x76
#define PCA9539_I2C_ADDRESS_HH          0x77

#define PCA9539_REG_IP_0                0x00  // input port register
#define PCA9539_REG_OP                  0x02  // output port register
#define PCA9539_REG_PIP                 0x04  // polarity inversion register
#define PCA9539_REG_CP                  0x06  // configuration register

#define PCA9539_PORT_0                  0x00
#define PCA9539_PORT_1                  0x01

// #define PCA9539_IO_NUM_0                0x00
// #define PCA9539_IO_NUM_1                0x01
// #define PCA9539_IO_NUM_2                0x02
// #define PCA9539_IO_NUM_3                0x03
// #define PCA9539_IO_NUM_4                0x04
// #define PCA9539_IO_NUM_5                0x05
// #define PCA9539_IO_NUM_6                0x06
// #define PCA9539_IO_NUM_7                0x07

typedef enum {
    P0_0, P0_1, P0_2, P0_3, P0_4, P0_5, P0_6, P0_7,
    P1_0, P1_1, P1_2, P1_3, P1_4, P1_5, P1_6, P1_7
} pca9539_pin_num;

typedef enum {
    PIN_MODE_OUTPUT,
    PIN_MODE_INPUT
} pca9539_pin_mode;


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

esp_err_t pca9539_get_pin_port_cfg(pca9539_cfg_t *cfg, pca9539_pin_num pin, uint8_t *port_cfg);
// esp_err_t pca9539_set_pin_port_cfg(pca9539_cfg_t *cfg, pca9539_pin_num pin);

esp_err_t pca9539_get_pin_mode(pca9539_cfg_t *cfg, pca9539_pin_num pin, pca9539_pin_mode *pin_mode);
esp_err_t pca9539_set_pin_mode(pca9539_cfg_t *cfg, pca9539_pin_num pin, pca9539_pin_mode pin_mode);


#ifdef __cplusplus
}
#endif

#endif  // LIB_PERIPHERALS_PCA9539_DRIVER_H_
