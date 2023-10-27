#include "i2c_wrapper.h"

esp_err_t i2c_master_init(i2c_port_t port, i2c_config_t *cfg) {
    esp_err_t err;
    if ((err = i2c_param_config(port, cfg)) != ESP_OK) {
        return err;
    }

    return i2c_driver_install(port, cfg->mode, 0, 0, 0);
}

esp_err_t i2c_master_deinit(i2c_port_t port) {
    return i2c_driver_delete(port);
}

esp_err_t i2c_master_read_reg_8(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t *data) {
    return i2c_master_write_read_device(port, addr, &reg, sizeof(uint8_t), data, sizeof(uint8_t), pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
}

esp_err_t i2c_master_write_reg_8(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t data) {
    uint8_t write_buf[2] = {reg, data};

    return i2c_master_write_to_device(port, addr, write_buf, sizeof(write_buf), pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
}
