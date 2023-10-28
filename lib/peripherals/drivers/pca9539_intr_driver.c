#include "pca9539_intr_driver.h"
#include "logger.h"


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
    uint32_t counter = 0;

    ESP_ERROR_CHECK(pca9539_setup_intr(pca9539_cfg));

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // while (pca9539_intr_evt_queue == NULL) {
    //     vTaskDelay(pdMS_TO_TICKS(1));
    // }

    TB_LOGI(TAG, "START, cfgpin %u", pca9539_cfg->intr_gpio_num);

    ESP_ERROR_CHECK(pca9539_get_input_port_state(pca9539_cfg, PORT_0, &port_state));
    ESP_ERROR_CHECK(pca9539_get_input_port_state(pca9539_cfg, PORT_1, &port_state));

    while (1) {
        uint8_t new_port_state = 0;

        ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);

        // it resets interrupt
        ESP_ERROR_CHECK(pca9539_get_input_port_state(pca9539_cfg, PORT_0, &new_port_state));

        TB_LOGI(TAG, "INTERRUPT %" PRIu32 ", PORT_0 %u -> %u\n", counter, port_state, new_port_state);

        port_state = new_port_state;
        counter++;
    }
}
