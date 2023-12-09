#include "inttypes.h"


#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"


#include "internal_temperature_sensor.h"
#include "global_config.h"
#include "logger.h"
#include "tb_event.h"
#include "regulator_timer.h"
#include "regulator.h"


const char * const TAG = "Regulator";


Regulator::Regulator(regulator_config_t config) :
    config(config)
{
    this->info = {};
    this->info.status = Status_STOPPED;
    this->regulator_event_group = xEventGroupCreate();

    regulator_timer_set_event_group(&this->regulator_event_group);
    regulator_timer_setup(this->config.sampling_rate);
    regulator_update_timer_setup(this->config.update_interval);
}

Regulator::~Regulator() {
    vEventGroupDelete(this->regulator_event_group);
}

esp_err_t Regulator::process_next_sample() {
    esp_err_t err = ESP_OK;

    float uc_temp = 0.0f;
    if (get_internal_temperature(&uc_temp) != ESP_OK) {
        TB_LOGE(TAG, "fail to read uC temperature");
    }
    this->info.uc_temperature = (int32_t)(uc_temp * 100.0f);
    TB_LOGI(TAG, "uC temperature: %" PRIi32, this->info.uc_temperature);

    regulator_timer_run(this->config.sampling_rate);
    return err;
}

esp_err_t Regulator::start() {
    if (this->info.status == Status_RUNNING) {
        TB_LOGI(TAG, "already running");
        return ESP_FAIL;
    }

    if (install_internal_temperature_sensor(INTERNAL_TEMP_SENS_MIN_RANGE_C, INTERNAL_TEMP_SENS_MAX_RANGE_C) != ESP_OK) {
        TB_LOGE(TAG, "uC temp sens setup fail");
        return ESP_FAIL;
    }

    this->info.uc_temperature = 0;
    this->info.temperature_1 = 0;
    this->info.temperature_2 = 0;
    this->info.temperature_3 = 0;
    this->info.temperature_4 = 0;
    this->info.temperature_5 = 0;
    this->info.fans_flags = 0;
    this->info.heaters_flags = 0;
    this->info.ssr_temperature_1 = 0;
    this->info.ssr_temperature_2 = 0;

    this->info.status = Status_RUNNING;

    xEventGroupSetBits(this->regulator_event_group, BIT_REGULATOR_START);
    regulator_update_timer_run();

    return process_next_sample();
}

esp_err_t Regulator::stop() {
    if (this->info.status == Status_STOPPED) {
        TB_LOGI(TAG, "already stopped");
        return ESP_FAIL;
    }

    esp_err_t err = ESP_OK;
    if ((err = uninstall_internal_temperature_sensor()) != ESP_OK) {
        return err;
    }

    regulator_update_timer_stop();
    regulator_timer_stop();

    this->info.status = Status_STOPPED;
    return err;
}

void Regulator::process_regulator() {
    if (this->info.status != Status_RUNNING) {
        return;
    }

    EventBits_t bits = xEventGroupWaitBits(
        this->regulator_event_group,
        BIT_REGULATOR_TIMER_TIMEOUT,
        pdTRUE,
        pdFALSE,
        pdMS_TO_TICKS(1)
    );

    if ((bits & BIT_REGULATOR_TIMER_TIMEOUT) == BIT_REGULATOR_TIMER_TIMEOUT) {
        esp_err_t err = ESP_OK;
        if ((err = this->process_next_sample()) != ESP_OK) {
            TB_LOGE(TAG, "fail to process next sample: %d", err);
        }
    }
}

RegulatorStatusUpdate Regulator::get_regulator_run_info() {
    return this->info;
}

bool Regulator::is_running() {
    return (this->info.status == Status_RUNNING) ? true : false;
}

EventGroupHandle_t *Regulator::get_regulator_event_group() {
    return &this->regulator_event_group;
}

void Regulator::print_info() {
    ;
}
