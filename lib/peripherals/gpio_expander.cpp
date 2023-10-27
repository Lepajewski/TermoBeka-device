#include "global_config.h"

#include "gpio_expander.h"


GPIOExpander::GPIOExpander() {
    i2c_config_t i2c_config;
    i2c_config.mode = I2C_MODE_MASTER;
    i2c_config.sda_io_num = PIN_GPIO_EXPANDER_SDA;
    i2c_config.scl_io_num = PIN_GPIO_EXPANDER_SCL;
    i2c_config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_config.master.clk_speed = GPIO_EXPANDER_FREQ_HZ;
    i2c_config.clk_flags = 0;

    this->pca9539_cfg = {
        .i2c_port = I2C_NUM_0,
        .i2c_config = i2c_config,
        .addr = PCA9539_I2C_ADDRESS_LL,
        .pin_interrupt = PIN_GPIO_EXPANDER_INT
    };
}

GPIOExpander::GPIOExpander(pca9539_cfg_t *pca9539_cfg) {
    this->pca9539_cfg = *pca9539_cfg;
}

GPIOExpander::~GPIOExpander() {
    end();
}

void GPIOExpander::begin() {
    init();

    set_polarity();
}

void GPIOExpander::end() {
    deinit();
}

void GPIOExpander::init() {
    ESP_ERROR_CHECK(pca9539_init(&this->pca9539_cfg));
}

void GPIOExpander::deinit() {
    ESP_ERROR_CHECK(pca9539_deinit(this->pca9539_cfg.i2c_port));
}

pca9539_pin_mode GPIOExpander::get_pin_mode(pca9539_pin_num pin) {
    pca9539_pin_mode mode;

    ESP_ERROR_CHECK(pca9539_get_pin_mode(&this->pca9539_cfg, pin, &mode));

    return mode;
}

void GPIOExpander::set_pin_mode(pca9539_pin_num pin, pca9539_pin_mode mode) {
    ESP_ERROR_CHECK(pca9539_set_pin_mode(&this->pca9539_cfg, pin, mode));
}

pca9539_pin_state GPIOExpander::get_input_pin_state(pca9539_pin_num pin) {
    pca9539_pin_state state;
    
    ESP_ERROR_CHECK(pca9539_get_input_pin_state(&this->pca9539_cfg, pin, &state));

    return state;
}

void GPIOExpander::set_polarity() {
    // buttons pins: 0 - 6 on port 0 have inverted polarity
    ESP_ERROR_CHECK(pca9539_set_port_polarity(&this->pca9539_cfg, PORT_0, 0b111111));
    // set normal polarity on port 1
    ESP_ERROR_CHECK(pca9539_set_port_polarity(&this->pca9539_cfg, PORT_1, 0));
}

void GPIOExpander::setup_buttons() {
    // ESP_ERROR_CHECK(pca9539_set_port_cfg(&this->pca9539_cfg, ))
}
