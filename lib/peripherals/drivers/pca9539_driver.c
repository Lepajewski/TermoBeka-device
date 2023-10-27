#include "i2c_wrapper.h"

#include "pca9539_driver.h"

static pca9539_port_num pca9539_get_port(pca9539_pin_num pin) {
    if (!(pin & 8)) {
        return PORT_0;
    }
    return PORT_1;
}

static uint8_t pca9539_get_pin(pca9539_pin_num pin) {
    return pin & 0b111;
}

static uint8_t pca9539_get_pin_value_from_port_mask(uint8_t port_mask, pca9539_pin_num pin) {
    uint8_t pin_bit = pca9539_get_pin(pin);
    return (port_mask & (1 << pin_bit)) >> pin_bit;
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

esp_err_t pca9539_get_port_cfg(pca9539_cfg_t *cfg, pca9539_port_num port, uint8_t *port_cfg) {
    return i2c_master_read_reg_8(cfg->i2c_port, cfg->addr, PCA9539_REG_CP | port, port_cfg);
}

esp_err_t pca9539_set_port_cfg(pca9539_cfg_t *cfg, pca9539_port_num port, uint8_t port_cfg) {
    esp_err_t err = ESP_OK;
    uint8_t current_port_cfg = 0;

    if ((err = pca9539_get_port_cfg(cfg, port, &current_port_cfg)) != ESP_OK) {
        return err;
    }

    // no change in port config
    if (current_port_cfg == port_cfg) {
        return err;
    }

    printf("Port cfg: %u -> %u\n", current_port_cfg, port_cfg);

    return i2c_master_write_reg_8(cfg->i2c_port, cfg->addr, PCA9539_REG_CP | port, port_cfg);
}

esp_err_t pca9539_get_port_polarity(pca9539_cfg_t *cfg, pca9539_port_num port, uint8_t *port_polarity) {
    return i2c_master_read_reg_8(cfg->i2c_port, cfg->addr, PCA9539_REG_PIP | port, port_polarity);
}

esp_err_t pca9539_set_port_polarity(pca9539_cfg_t *cfg, pca9539_port_num port, uint8_t port_polarity) {
    esp_err_t err = ESP_OK;
    uint8_t current_port_polarity = 0;

    if ((err = pca9539_get_port_polarity(cfg, port, &current_port_polarity)) != ESP_OK) {
        return err;
    }

    // no change
    if (current_port_polarity == port_polarity) {
        return err;
    }

    return i2c_master_write_reg_8(cfg->i2c_port, cfg->addr, PCA9539_REG_PIP | port, port_polarity);
}

esp_err_t pca9539_get_pin_polarity(pca9539_cfg_t *cfg, pca9539_pin_num pin, pca9539_polarity *polarity) {
    esp_err_t err = ESP_OK;
    uint8_t port_polarity = 0;

    if ((err = pca9539_get_port_polarity(cfg, pin, &port_polarity)) != ESP_OK) {
        return err;
    }

    *polarity = pca9539_get_pin_value_from_port_mask(port_polarity, pin);

    return err;
}

esp_err_t pca9539_set_pin_polarity(pca9539_cfg_t *cfg, pca9539_pin_num pin, pca9539_polarity polarity) {
    esp_err_t err = ESP_OK;
    uint8_t current_port_polarity = 0;
    uint8_t chosen_pin = pca9539_get_pin(pin);

    if ((err = pca9539_get_port_polarity(cfg, chosen_pin, &current_port_polarity)) != ESP_OK) {
        return err;
    }

    uint8_t port_polarity = (current_port_polarity & ~(1 << chosen_pin)) | (polarity << chosen_pin);

    // no change
    if (current_port_polarity == port_polarity) {
        return ESP_OK;
    }

    printf("Port polarity %u -> %u\n", current_port_polarity, port_polarity);
    return i2c_master_write_reg_8(cfg->i2c_port, cfg->addr, PCA9539_REG_CP | pca9539_get_port(chosen_pin), port_polarity);
}

esp_err_t pca9539_get_pin_mode(pca9539_cfg_t *cfg, pca9539_pin_num pin, pca9539_pin_mode *pin_mode) {
    esp_err_t err = ESP_OK;
    uint8_t port_config = 0;

    if ((err = pca9539_get_port_cfg(cfg, pin, &port_config)) != ESP_OK) {
        return err;
    }

    *pin_mode = pca9539_get_pin_value_from_port_mask(port_config, pin);

    return err;
}

esp_err_t pca9539_set_pin_mode(pca9539_cfg_t *cfg, pca9539_pin_num pin, pca9539_pin_mode pin_mode) {
    esp_err_t err = ESP_OK;
    uint8_t current_port_cfg = 0;
    uint8_t chosen_pin = pca9539_get_pin(pin);

    if ((err = pca9539_get_port_cfg(cfg, chosen_pin, &current_port_cfg)) != ESP_OK) {
        return err;
    }

    uint8_t port_cfg = (current_port_cfg & ~(1 << chosen_pin)) | (pin_mode << chosen_pin);

    // no change
    if (current_port_cfg == port_cfg) {
        return err;
    }

    return i2c_master_write_reg_8(cfg->i2c_port, cfg->addr, PCA9539_REG_CP | pca9539_get_port(chosen_pin), port_cfg);
}

esp_err_t pca9539_get_input_port_state(pca9539_cfg_t *cfg, pca9539_port_num port, uint8_t *state) {
    return i2c_master_read_reg_8(cfg->i2c_port, cfg->addr, PCA9539_REG_IP | port, state);
}

esp_err_t pca9539_get_output_port_state(pca9539_cfg_t *cfg, pca9539_port_num port, uint8_t *state) {
    return i2c_master_read_reg_8(cfg->i2c_port, cfg->addr, PCA9539_REG_OP | port, state);
}

esp_err_t pca9539_get_input_pin_state(pca9539_cfg_t *cfg, pca9539_pin_num pin, pca9539_pin_state *state) {
    esp_err_t err = ESP_OK;
    pca9539_port_num port = pca9539_get_port(pin);
    uint8_t port_state = 0;

    if ((err = pca9539_get_input_port_state(cfg, port, &port_state)) != ESP_OK) {
        return err;
    }

    printf("Port state: %u\n", port_state);

    *state = pca9539_get_pin_value_from_port_mask(port_state, pin) == 0 ? PIN_MODE_OUTPUT : PIN_MODE_INPUT;

    return err;
}

esp_err_t pca9539_get_output_pin_state(pca9539_cfg_t *cfg, pca9539_pin_num pin, pca9539_pin_state *state) {
    esp_err_t err = ESP_OK;
    pca9539_port_num port = pca9539_get_port(pin);
    uint8_t port_state = 0;

    if ((err = pca9539_get_output_port_state(cfg, port, &port_state)) != ESP_OK) {
        return err;
    }

    return pca9539_get_pin_value_from_port_mask(port_state, pin);
}
