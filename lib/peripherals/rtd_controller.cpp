#include "inttypes.h"
#include "logger.h"

#include "rtd_controller.h"


#define PT100_R_REF             430.0f
#define PT100_R_NOM             100.0f
#define PT1000_R_REF            4300.0f
#define PT1000_R_NOM            1000.0f


#define RTD_CONVERT_SLOPE       30.90411507
#define RTD_CONVERT_INTERCEPT   8234.256681893283


const char * const TAG = "RTDController";


static float rtd_to_temperature(uint16_t rtd) {
    return (rtd - RTD_CONVERT_INTERCEPT) / RTD_CONVERT_SLOPE;
}


RTDController::RTDController() {
    this->config = {
        .ref = PT1000_R_REF,
        .nominal = PT1000_R_NOM
    };
}

RTDController::RTDController(max31865_rtd_config_t config) :
    config(config)
{}

RTDController::~RTDController() {}

void RTDController::begin() {
    this->sysMgr = get_system_manager();
    this->spi_semaphore = this->sysMgr->get_spi_semaphore();

    while (this->spi_semaphore == NULL) {};

    xSemaphoreTake(*this->spi_semaphore, portMAX_DELAY);

    for (auto &s : this->sensors) {
        esp_err_t err = s.setup();
        if (err != ESP_OK) {
            TB_LOGE(TAG, "fail to setup RTDs");
            xSemaphoreGive(*this->spi_semaphore);
            return;
        }
    }

    xSemaphoreGive(*this->spi_semaphore);

    TB_LOGI(TAG, "RTDs inited");
}

esp_err_t RTDController::get_temperatures(rtd_temperature *t) {
    esp_err_t err = ESP_OK;
    uint16_t rtd[NUMBER_OF_RTDS] = {};

    xSemaphoreTake(*this->spi_semaphore, portMAX_DELAY);

    for (int i = 0; i < NUMBER_OF_RTDS; i++) {
        if (this->sensors[i].get_rtd(&rtd[i], &t->fault[i]) != ESP_OK) {
            TB_LOGE(TAG, "sensor %d: get rtd fail", i);
        }
    }

    xSemaphoreGive(*this->spi_semaphore);

    for (int i = 0; i < NUMBER_OF_RTDS; i++) {
        if (t->fault[i] == Max31865Error::NoError && rtd[i] >= RTD_MIN_TRESHOLD && rtd[i] <= RTD_MAX_TRESHOLD) {
            t->temperature[i] = rtd_to_temperature(rtd[i]);
        } else {
            t->fault[i] = Max31865Error::RTDHigh;
            TB_LOGE(TAG, "sensor %d: invalid RTD: %" PRIu16, i, rtd[i]);
        }
    }

    return err;
}
