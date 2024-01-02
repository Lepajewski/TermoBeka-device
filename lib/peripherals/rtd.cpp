#include "rtd.h"


#define RTD_MIN_TRESHOLD    0
#define RTD_MAX_TRESHOLD    32767


static const max31865_config_t max31865_config = {
    .vbias = true,
    .autoConversion = true,
    .nWires = Max31865NWires::Four,
    .faultDetection = Max31865FaultDetection::NoAction,
    .filter = Max31865Filter::Hz50,
};


RTD::RTD(int cs) :
    cs(cs)
{
    this->sensor = new Max31865(this->cs);
}

RTD::~RTD() {
    delete this->sensor;
}

esp_err_t RTD::get_rtd(uint16_t *rtd, Max31865Error *fault) {
    this->sensor->begin(max31865_config);
    this->sensor->setRTDThresholds(RTD_MIN_TRESHOLD, RTD_MAX_TRESHOLD);
    this->sensor->getRTD(rtd, fault);

    return this->sensor->end();
}
