#include "inttypes.h"
#include "logger.h"

#include "rtd_controller.h"


const char * const TAG = "RTDController";


RTDController::RTDController() {}

RTDController::~RTDController() {}

void RTDController::begin() {
    this->sysMgr = get_system_manager();
    this->spi_semaphore = this->sysMgr->get_spi_semaphore();

    while (this->spi_semaphore == NULL) {};

    xSemaphoreTake(*this->spi_semaphore, portMAX_DELAY);

    esp_err_t err = this->sensors.begin();
    if (err != ESP_OK) {
        TB_LOGE(TAG, "fail to begin RTDs");
        xSemaphoreGive(*this->spi_semaphore);
        return;
    }

    xSemaphoreGive(*this->spi_semaphore);

    TB_LOGI(TAG, "RTDs inited");
}

esp_err_t RTDController::get_avg_temperature(float *t) {
    esp_err_t err = ESP_OK;

    xSemaphoreTake(*this->spi_semaphore, portMAX_DELAY);

    err = this->sensors.getAvgTemperature(t);

    xSemaphoreGive(*this->spi_semaphore);

    return err;
}
