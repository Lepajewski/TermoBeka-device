#include "esp_timer.h"
#include "logger.h"

#include "pca9539_intr_driver.h"


static TaskHandle_t *pca9539_intr_task_handle = NULL;
static QueueHandle_t *pca9539_intr_evt_queue = NULL;

static const char * const TAG = "PCA9539Intr";


static void IRAM_ATTR pca9539_intr_handler(void *arg) {
    if (pca9539_intr_task_handle == NULL) {
        return;
    }

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    vTaskNotifyGiveIndexedFromISR(*pca9539_intr_task_handle, 0, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static esp_err_t pca9539_setup_intr(pca9539_cfg_t *cfg) {
    esp_err_t err = ESP_OK;
    gpio_config_t io_conf = {};

    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = (1ULL << cfg->intr_gpio_num);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

    if ((err = gpio_config(&io_conf)) != ESP_OK) {
        return err;
    }

    if ((err = gpio_install_isr_service(0)) != ESP_OK) {
        return err;
    }

    if ((err = gpio_isr_handler_add(cfg->intr_gpio_num, pca9539_intr_handler, NULL)) != ESP_OK) {
        return err;
    }

    return err;
}

static pca9539_pin_num pca9539_get_pin_from_port_change(pca9539_port_num port_num, uint8_t old_port_state, uint8_t new_port_state) {
    uint8_t pin_binary = old_port_state ^ new_port_state;
    uint8_t pos = 0;

    while (!(pin_binary & 1) && (pos < 8)) {
        pin_binary >>= 1;
        pos++;
    }

    return (port_num << 3) | pos;
}

static pin_change_type pca9539_get_pin_state_from_pin_change(pca9539_pin_num pin_num, uint8_t old_port_state, uint8_t new_port_state) {
    uint8_t pin = PCA9539_GET_PIN(pin_num);
    uint8_t pin_mask = 1 << pin;

    uint8_t old_pin_value = (old_port_state & pin_mask) >> pin;
    uint8_t new_pin_value = (new_port_state & pin_mask) >> pin;

    if (old_pin_value == 0 && new_pin_value == 1) {
        return PIN_CHANGE_0_TO_1;
    } else if (old_pin_value == 1 && new_pin_value == 0) {
        return PIN_CHANGE_1_TO_0;
    } else {
        return PIN_NO_CHANGE;
    }
}

esp_err_t pca9539_set_intr_task_handle(TaskHandle_t *task_handle) {
    if (task_handle == NULL) {
        return ESP_FAIL;
    }

    pca9539_intr_task_handle = task_handle;

    return ESP_OK;
}

esp_err_t pca9539_set_intr_evt_queue(QueueHandle_t *evt_queue) {
    if (evt_queue == NULL) {
        return ESP_FAIL;
    }

    pca9539_intr_evt_queue = evt_queue;

    return ESP_OK;
}

void PCA9539IntrTask(void *pvParameters) {
    pca9539_cfg_t *pca9539_cfg = (pca9539_cfg_t *) pvParameters;
    uint8_t port_state = 0;
    pca9539_intr_evt_t intr_evt;

    ESP_ERROR_CHECK(pca9539_setup_intr(pca9539_cfg));

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    TB_LOGI(TAG, "START, intrpin %u", pca9539_cfg->intr_gpio_num);

    ESP_ERROR_CHECK(pca9539_get_input_port_state(pca9539_cfg, PORT_0, &port_state));

    while (1) {
        uint8_t new_port_state = 0;

        ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);

        // it resets interrupt
        ESP_ERROR_CHECK(pca9539_get_input_port_state(pca9539_cfg, PORT_0, &new_port_state));

        intr_evt.port_num = PORT_0;
        intr_evt.pin_num = pca9539_get_pin_from_port_change(PORT_0, port_state, new_port_state);
        intr_evt.change_type = pca9539_get_pin_state_from_pin_change(intr_evt.pin_num, port_state, new_port_state);
        intr_evt.timestamp = esp_timer_get_time();

        // TB_LOGI(TAG, "INTERRUPT, PORT: %u, PIN: %u, change_type: %u, timestamp: %" PRIu64 "\n", intr_evt.port_num, intr_evt.pin_num, intr_evt.change_type, intr_evt.timestamp);

        // send event to intr queue
        if (pca9539_intr_evt_queue != NULL) {
            xQueueSend(*pca9539_intr_evt_queue, (void *) &intr_evt, pdMS_TO_TICKS(10));
        }

        port_state = new_port_state;
    }
}
