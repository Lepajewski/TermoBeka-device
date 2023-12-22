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
    config(config),
    set_temperature(0),
    last_sample_time(0)
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

    this->info.temperature_1 = this->set_temperature;
    this->info.temperature_2 = this->set_temperature;
    this->info.temperature_3 = this->set_temperature;
    this->info.temperature_4 = this->set_temperature;
    this->info.temperature_5 = this->set_temperature;

    uint32_t preemption_error = (uint32_t)(get_time_since_startup_ms() - this->last_sample_time);
    this->last_sample_time = get_time_since_startup_ms();
    if (preemption_error > this->config.sampling_rate) {
        preemption_error -= this->config.sampling_rate;
    }
    uint32_t next_sample_time = this->config.sampling_rate - preemption_error;

    TB_LOGI(TAG, "setting timer to T+%" PRIu32 "ms %" PRIu32, next_sample_time, preemption_error);
    regulator_timer_run(next_sample_time);
    return err;
}

int16_t Regulator::get_ambient_temperature() {
    float temperature = 0.0f;
    if (get_internal_temperature(&temperature) != ESP_OK) {
        return this->config.min_temp;
    }
    return (int16_t)(temperature * 100.0f);
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

    this->last_sample_time = get_time_since_startup_ms();

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

int16_t Regulator::get_min_temperature() {
    return this->config.min_temp;
}

int16_t Regulator::get_max_temperature() {
    return this->config.max_temp;
}


void Regulator::update_temperature(int16_t temperature) {
    if (temperature < this->config.min_temp) {
        TB_LOGW(TAG, "set T is lower than ambient T");
        this->set_temperature = this->get_ambient_temperature();
    } else if (temperature > this->config.max_temp) {
        TB_LOGW(TAG, "set T is too high");
        this->set_temperature = this->config.max_temp;
    } else {
        this->set_temperature = temperature;
    }
}

EventGroupHandle_t *Regulator::get_regulator_event_group() {
    return &this->regulator_event_group;
}

void Regulator::print_info() {
    ;
}
