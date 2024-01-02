#include "external_temperature_sensor.h"


ExternalTemperatureSensor::ExternalTemperatureSensor(uint8_t ow_pin) :
    ow_pin(ow_pin)
{
    this->ow = {};
}

ExternalTemperatureSensor::~ExternalTemperatureSensor() {}

esp_err_t ExternalTemperatureSensor::setup() {
    esp_err_t err = ESP_OK;

    err = ow_init(&this->ow, this->ow_pin) ? ESP_OK : ESP_FAIL;
    if (err != ESP_OK) {
        return err;
    }

    err = ow_reset(&this->ow) ? ESP_OK : ESP_FAIL;
    if (err != ESP_OK) {
        return err;
    }

    return err;
}

esp_err_t ExternalTemperatureSensor::get_temperature(external_temperature *temperature) {
    esp_err_t err = ESP_OK;

    err = ow_reset(&this->ow) ? ESP_OK : ESP_FAIL;
    if (err != ESP_OK) {
        return err;
    }

    ow_send(&this->ow, OW_SKIP_ROM);
    ow_send(&this->ow, DS18B20_CONVERT_T);

    while (ow_read(&ow) == 0) {};

    for (int i = 0; i < NUMBER_OF_EXTERNAL_TEMP_SENSORS; i++) {
        err = ow_reset(&this->ow) ? ESP_OK : ESP_FAIL;
        if (err != ESP_OK) {
            return err;
        }

        ow_send(&this->ow, OW_MATCH_ROM);

        for (int b = 0; b < 64; b += 8) {
            ow_send(&this->ow, this->address[i] >> b);
        }

        ow_send(&this->ow, DS18B20_READ_SCRATCHPAD);
        int16_t temp = 0;
        temp = ow_read(&this->ow) | (ow_read(&this->ow) << 8);

        temperature->temperature[i] = (float)temp / 16.0f;
    }
    
    return err;
}
