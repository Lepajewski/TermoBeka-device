#ifndef LIB_PERIPHERALS_DRIVERS_I2C_WRAPPER_H_
#define LIB_PERIPHERALS_DRIVERS_I2C_WRAPPER_H_


#include "driver/i2c.h"


#define I2C_MASTER_TIMEOUT_MS       100


#ifdef __cplusplus
extern "C" {
#endif

esp_err_t i2c_master_init(i2c_port_t port, i2c_config_t *cfg);
esp_err_t i2c_master_deinit(i2c_port_t port);
esp_err_t i2c_master_read_reg_8(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t *data);
esp_err_t i2c_master_write_reg_8(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t data);



#ifdef __cplusplus
}
#endif

#endif  // LIB_PERIPHERALS_DRIVERS_I2C_WRAPPER_H_
