#include "driver/temperature_sensor.h"
#include "internal_temperature_sensor.h"


static temperature_sensor_handle_t temp_handle = NULL;


esp_err_t install_internal_temperature_sensor(int8_t range_min, int8_t range_max) {
    temperature_sensor_config_t temp_sensor = {
        .range_min = range_min,
        .range_max = range_max,
    };

    return temperature_sensor_install(&temp_sensor, &temp_handle);
}

esp_err_t uninstall_internal_temperature_sensor() {
    return temperature_sensor_uninstall(temp_handle);
}

esp_err_t get_internal_temperature(float *temperature) {
    esp_err_t err = ESP_OK;

    if ((err = temperature_sensor_enable(temp_handle)) != ESP_OK) {
        return err;
    }

    if ((err = temperature_sensor_get_celsius(temp_handle, temperature)) != ESP_OK) {
        return err;
    }

    return temperature_sensor_disable(temp_handle);
}
