#include "i2c_wrapper.h"

#include "pca9539_driver.h"

static uint8_t pca9539_get_port(pca9539_pin_num pin) {
    if (!(pin & 8)) {
        return PCA9539_PORT_0;
    }
    return PCA9539_PORT_1;
}

static uint8_t pca9539_get_pin(pca9539_pin_num pin) {
    return pin & 0b111;
}

static uint8_t pca9539_get_pin_mode_from_port_cfg(uint8_t port_cfg, pca9539_pin_num pin) {
    uint8_t pin_bit = pca9539_get_pin(pin);
    return (port_cfg & (1 << pin_bit)) >> pin_bit;
}

const char *pca9539_pin_num_to_s(pca9539_pin_num pin) {
    switch (pin) {
        case P0_0: return "P0_0";
        case P0_1: return "P0_1";
        case P0_2: return "P0_2";
        case P0_3: return "P0_3";
        case P0_4: return "P0_4";
        case P0_5: return "P0_5";
        case P0_6: return "P0_6";
        case P0_7: return "P0_7";
        case P1_0: return "P1_0";
        case P1_1: return "P1_1";
        case P1_2: return "P1_2";
        case P1_3: return "P1_3";
        case P1_4: return "P1_4";
        case P1_5: return "P1_5";
        case P1_6: return "P1_6";
        case P1_7: return "P1_7";
        default: return "Invalid Pin";
    }
}

esp_err_t pca9539_init(pca9539_cfg_t *cfg) {
    esp_err_t err = ESP_OK;

    if ((err = i2c_master_init(cfg->i2c_port, &cfg->i2c_config)) != ESP_OK) {
        return err;
    }

    return err;
}

esp_err_t pca9539_deinit(i2c_port_t i2c_port) {
    esp_err_t err = ESP_OK;

    if ((err = i2c_master_deinit(i2c_port)) != ESP_OK) {
        return err;
    }

    return err;
}

esp_err_t pca9539_get_pin_port_cfg(pca9539_cfg_t *cfg, pca9539_pin_num pin, uint8_t *port_cfg) {
    uint8_t port_reg = PCA9539_REG_CP | pca9539_get_port(pin);

    i2c_master_read_reg_8(cfg->i2c_port, cfg->addr, port_reg, port_cfg);

    printf("Current port %u cfg: %u\n", port_reg, *port_cfg);

    return ESP_OK;
}

esp_err_t pca9539_get_pin_mode(pca9539_cfg_t *cfg, pca9539_pin_num pin, pca9539_pin_mode *pin_mode) {
    esp_err_t err = ESP_OK;
    uint8_t data = 0;

    if ((err = pca9539_get_pin_port_cfg(cfg, pin, &data)) != ESP_OK) {
        return err;
    }

    *pin_mode = pca9539_get_pin_mode_from_port_cfg(data, pin);

    // printf("Pin %u %u, mode: %u\n", pca9539_get_pin(pin), (1 << pca9539_get_pin(pin)), *pin_mode);

    return err;
}

esp_err_t pca9539_set_pin_mode(pca9539_cfg_t *cfg, pca9539_pin_num pin, pca9539_pin_mode pin_mode) {
    esp_err_t err = ESP_OK;
    uint8_t current_port_cfg = 0;
    uint8_t chosen_pin = pca9539_get_pin(pin);

    if ((err = pca9539_get_pin_port_cfg(cfg, chosen_pin, &current_port_cfg)) != ESP_OK) {
        return err;
    }

    uint8_t new_port_cfg = (current_port_cfg & ~(1 << chosen_pin)) | (pin_mode << chosen_pin);

    // pin is already in desired mode
    if (pca9539_get_pin_mode_from_port_cfg(current_port_cfg, pin) == pca9539_get_pin_mode_from_port_cfg(new_port_cfg, pin)) {
        // printf("Pin already in mode %u\n", pin_mode);
        return ESP_OK;
    }

    // printf("New port cfg: %u\n", new_port_cfg);

    return i2c_master_write_reg_8(cfg->i2c_port, cfg->addr, PCA9539_REG_CP | pca9539_get_port(chosen_pin), new_port_cfg);
}
