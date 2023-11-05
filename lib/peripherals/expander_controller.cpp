#include "global_config.h"

#include "expander_controller.h"


ExpanderController::ExpanderController() :
    initialized(false) 
{
    i2c_config_t i2c_config;
    i2c_config.mode = I2C_MODE_MASTER;
    i2c_config.sda_io_num = PIN_GPIO_EXPANDER_SDA;
    i2c_config.scl_io_num = PIN_GPIO_EXPANDER_SCL;
    i2c_config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_config.master.clk_speed = GPIO_EXPANDER_FREQ_HZ;
    i2c_config.clk_flags = 0;

    this->config = {
        .i2c_port = I2C_NUM_0,
        .i2c_config = i2c_config,
        .addr = PCA9539_I2C_ADDRESS_LL,
        .intr_gpio_num = PIN_GPIO_EXPANDER_INTR
    };
}

ExpanderController::ExpanderController(pca9539_cfg_t config) :
    initialized(false)
{
    this->config = config;
}

ExpanderController::~ExpanderController() {
    end();
}

void ExpanderController::begin() {
    if (!this->initialized) {
        // start i2c bus
        ESP_ERROR_CHECK(pca9539_init(&this->config));

        // set expander registers to default value since there is not RST option
        ESP_ERROR_CHECK(pca9539_set_default_config(&this->config));

        this->initialized = true;
    }
}

void ExpanderController::end() {
    if (this->initialized) {
        ESP_ERROR_CHECK(pca9539_deinit(this->config.i2c_port));
        this->initialized = false;
    }
}

pca9539_pin_state ExpanderController::get_input_pin_state(pca9539_pin_num pin) {
    pca9539_pin_state state;
    
    ESP_ERROR_CHECK(pca9539_get_input_pin_state(&this->config, pin, &state));

    return state;
}

esp_err_t ExpanderController::setup_pin(pca9539_polarity polarity, pca9539_pin_num pin, pca9539_pin_mode mode) {
    esp_err_t err = ESP_OK;

    if ((err = pca9539_set_pin_polarity(&this->config, pin, polarity)) != ESP_OK) {
        return err;
    }

    if ((err = pca9539_set_pin_mode(&this->config, pin, mode)) != ESP_OK) {
        return err;
    }

    return err;
}

esp_err_t ExpanderController::set_output_pin_state(pca9539_pin_num pin, pca9539_pin_state state) {
    return pca9539_set_output_pin_state(&this->config, pin, state);
}
