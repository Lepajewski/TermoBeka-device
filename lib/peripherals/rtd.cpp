#include "rtd.h"


static const max31865_config_t max31865_config = {
    .vbias = true,
    .autoConversion = true,
    .nWires = Max31865NWires::Four,
    .faultDetection = Max31865FaultDetection::NoAction,
    .filter = Max31865Filter::Hz50,
};


RTD::RTD(int cs) :
    cs(cs),
    running(false)
{
    this->sensor = new Max31865(this->cs);
}

RTD::~RTD() {
    if (this->running) {
        this->sensor->end();
        this->running = false;
    }
    delete this->sensor;
}

esp_err_t RTD::setup() {
    if (this->running) {
        return ESP_FAIL;
    }

    esp_err_t err = this->sensor->begin(max31865_config);
    if (err != ESP_OK) {
        return err;
    }
    
    err = this->sensor->setRTDThresholds(RTD_MIN_TRESHOLD, RTD_MAX_TRESHOLD);
    if (err != ESP_OK) {
        return err;
    }

    this->running = true;
    return err;
}

esp_err_t RTD::get_rtd(uint16_t *rtd, Max31865Error *fault) {
    return this->running ? this->sensor->getRTD(rtd, fault) : ESP_FAIL;
}
