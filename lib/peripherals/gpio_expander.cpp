#include "global_config.h"
#include "logger.h"

#include "gpio_expander.h"


static const char * const TAG = "GPIOExpander";


GPIOExpander::GPIOExpander() {
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
        .intr_gpio_num = PIN_GPIO_EXPANDER_1_INTR,
        .rst_gpio_num = PIN_GPIO_EXPANDER_1_RESET
    };

    this->intr_task_handle = NULL;
    this->intr_evt_queue = NULL;

    this->controller = ExpanderController(this->config);

    led_pins_t led_pins = {
        .red = P1_2,
        .green = P1_0,
        .blue = P0_7
    };
    this->leds = new LED(this->controller, PIN_POLARITY_NORMAL, led_pins);
}

GPIOExpander::GPIOExpander(pca9539_cfg_t *config) {
    this->config = *config;
    this->intr_task_handle = NULL;
    this->intr_evt_queue = NULL;
    this->controller = ExpanderController(this->config);
    led_pins_t led_pins = {
        .red = P1_2,
        .green = P1_0,
        .blue = P0_7
    };
    this->leds = new LED(this->controller, PIN_POLARITY_NORMAL, led_pins);
}

GPIOExpander::~GPIOExpander() {
    end();
}

void GPIOExpander::begin() {
    // init queue for button press events
    ESP_ERROR_CHECK(init_evt_intr_queue());
    ESP_ERROR_CHECK(pca9539_set_intr_evt_queue(&this->intr_evt_queue));

    // start intr task to handle interrupts
    start_evt_intr_task();

    this->controller.begin();

    setup_buttons();

    this->setup_buzzer();

    setup_leds();

    // notify intr task that i2c is configured
    xTaskNotifyGive(this->intr_task_handle);
}

void GPIOExpander::end() {
    delete leds;
    controller.end();
    deinit_evt_intr_queue();
}

esp_err_t GPIOExpander::init_evt_intr_queue() {
    this->intr_evt_queue = xQueueCreate(EXPANDER_EVT_INTR_QUEUE_SIZE, sizeof(pca9539_intr_evt_t));
    return this->intr_evt_queue == NULL ? ESP_FAIL : ESP_OK;
}

void GPIOExpander::deinit_evt_intr_queue() {
    vQueueDelete(this->intr_evt_queue);
    this->intr_evt_queue = NULL;
}

void GPIOExpander::start_evt_intr_task() {
    xTaskCreatePinnedToCore(PCA9539IntrTask, "PCA9539Intr", 4096, &this->config, 1, &this->intr_task_handle, 1);
    ESP_ERROR_CHECK(pca9539_set_intr_task_handle(&this->intr_task_handle));
}

void GPIOExpander::process_intr_event(pca9539_intr_evt_t *intr_evt) {

}

void GPIOExpander::setup_buttons() {
    for (auto &b : this->buttons) {
        esp_err_t err = ESP_OK;
        if ((err = b.setup()) != ESP_OK) {
            TB_LOGE(TAG, "button setup error: %d", err);
        } else {
            b.set_callback(this->button_callback);
        }
    }
}

void GPIOExpander::setup_buzzer() {
    esp_err_t err = this->buzzer.setup();
    if (err != ESP_OK) {
        TB_LOGE(TAG, "buzzer setup error: %d", err);
    }
}

void GPIOExpander::setup_leds() {
    esp_err_t err = this->leds->setup();
    if (err != ESP_OK) {
        TB_LOGE(TAG, "leds setup error: %d", err);
    }
}

Button *GPIOExpander::lookup_button(pca9539_pin_num num){
    for (auto &b : this->buttons) {
        if (num == b.get_pin_num()) {
            return &b;
        }
    }
    return nullptr;
}

void GPIOExpander::poll_intr_events() {
    pca9539_intr_evt_t intr_evt;

    while (uxQueueMessagesWaiting(this->intr_evt_queue)) {
        if (xQueueReceive(this->intr_evt_queue, &intr_evt, pdMS_TO_TICKS(10) == pdPASS)) {
            TB_LOGI(TAG, "INTR EVENT, PORT: %u, PIN: %u, change_type: %u, timestamp: %" PRIu64, intr_evt.port_num, intr_evt.pin_num, intr_evt.change_type, intr_evt.timestamp);
            Button *b = lookup_button(intr_evt.pin_num);
            if (b != nullptr) {
                b->process_event(intr_evt.change_type, intr_evt.timestamp);
            }
        } else {
            TB_LOGE(TAG, "FAIL to receive from intr queue");
        }
    }
}

void GPIOExpander::set_callback(std::function<void(Button*, PressType)> cb) {
    this->button_callback = cb;
}

void GPIOExpander::set_backlight_color(Color color) {
    this->leds->set_color(color);
}

void GPIOExpander::buzzer_beep(uint32_t timeout) {
    this->buzzer.beep(timeout);
}
