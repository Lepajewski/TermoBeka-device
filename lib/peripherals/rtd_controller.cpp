#include "inttypes.h"
#include "logger.h"

#include "rtd_controller.h"


#define PT100_R_REF             430.0f
#define PT100_R_NOM             100.0f
#define PT1000_R_REF            4300.0f
#define PT1000_R_NOM            1000.0f


const char * const TAG = "RTDController";


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

    TB_LOGI(TAG, "RTDs inited");
}

esp_err_t RTDController::get_temperatures(rtd_temperature *t) {
    esp_err_t err = ESP_OK;

    spi_bus_config_t busConfig = {};
    busConfig.miso_io_num = PIN_SPI_MISO;
    busConfig.mosi_io_num = PIN_SPI_MOSI;
    busConfig.sclk_io_num = PIN_SPI_CLK;
    busConfig.quadhd_io_num = -1;
    busConfig.quadwp_io_num = -1;
    xSemaphoreTake(*this->spi_semaphore, portMAX_DELAY);

    err = spi_bus_initialize(SPI3_HOST, &busConfig, 0);
    if (err == ESP_ERR_INVALID_STATE) {
        ESP_LOGD(TAG, "SPI bus already initialized");
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error initialising SPI bus: %s", esp_err_to_name(err));
    }

    if (err == ESP_OK) {
        for (int i = 0; i < NUMBER_OF_RTDS; i++) {
            uint16_t rtd = 0;
            Max31865Error fault = Max31865Error::NoError;

            if (this->sensors[i].get_rtd(&rtd, &fault) != ESP_OK) {
                TB_LOGE(TAG, "sensor get rtd fail");
            } else {
                t->temperature[i] = Max31865::RTDtoTemperature(rtd, this->config);
            }

            err = (fault == Max31865Error::NoError) ? ESP_OK : ESP_FAIL;
            t->fault[i] = fault;
        }
    }

    spi_bus_free(SPI3_HOST);
    xSemaphoreGive(*this->spi_semaphore);

    return err;
}
