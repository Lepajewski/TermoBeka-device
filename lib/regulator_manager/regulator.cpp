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
    running(false),
    set_temperature(0),
    last_sample_time(0),
    start_time(0)
{
    this->info = {};
    this->uc_temperature = 0.0f;
    this->ambient_temperature = 0.0f;
    this->relays_states = 0;

    this->regulator_event_group = xEventGroupCreate();

    regulator_timer_set_event_group(&this->regulator_event_group);
    regulator_timer_setup(this->config.sampling_rate);
    regulator_update_timer_setup(this->config.update_interval);

    this->setup();
}

Regulator::~Regulator() {
    vEventGroupDelete(this->regulator_event_group);
}

void Regulator::setup_rtds() {
    this->controller.begin();
}

void Regulator::setup() {
    if (install_internal_temperature_sensor(INTERNAL_TEMP_SENS_MIN_RANGE_C, INTERNAL_TEMP_SENS_MAX_RANGE_C) != ESP_OK) {
        TB_LOGE(TAG, "uC temp sens setup fail");
    }


    this->expander.begin();

    if (this->ds18b20.setup() != ESP_OK) {
        TB_LOGE(TAG, "fail to setup onewire");
    } else {
        TB_LOGI(TAG, "onewire init");
    }

    this->heaters_off();
    this->fans_off();
    this->expander.relay_off(RelayType::RELAY_4);
    this->expander.relay_off(RelayType::RELAY_5);
    this->expander.relay_off(RelayType::RELAY_6);
    this->expander.relay_off(RelayType::RELAY_7);

    this->setup_rtds();
}

esp_err_t Regulator::process_next_sample() {
    esp_err_t err = ESP_OK;

    this->get_avg_chamber_temperature();
    TB_LOGI(TAG, "avg chamber temperature: %" PRIi32, this->info.avg_chamber_temperature);

    uint32_t preemption_error = (uint32_t)(get_time_since_startup_ms() - this->last_sample_time);
    this->last_sample_time = get_time_since_startup_ms();
    if (preemption_error > this->config.sampling_rate) {
        preemption_error -= this->config.sampling_rate;
    } else if (preemption_error == this->config.sampling_rate) {
        preemption_error = 0;
    }


    uint32_t next_sample_time = this->config.sampling_rate - preemption_error;

    TB_LOGI(TAG, "setting timer to T+%" PRIu32 "ms %" PRIu32, next_sample_time, preemption_error);
    regulator_timer_run(next_sample_time);
    return err;
}

void Regulator::get_cpu_temperature() {
    if (get_internal_temperature(&this->uc_temperature) != ESP_OK) {
        this->uc_temperature = 0.0f;
    }

    // TB_LOGI(TAG, "uC temperature: %f", this->uc_temperature);
}

void Regulator::get_external_temperature() {
    external_temperature temperature = {};
    if (this->ds18b20.get_temperature(&temperature) != ESP_OK) {
        TB_LOGE(TAG, "fail to read ds18b20");
        temperature = {};
    }

    this->ambient_temperature = temperature.temperature[0];
}

void Regulator::get_avg_chamber_temperature() {
    float t = 0.0f;
    
    if (this->controller.get_avg_temperature(&t) == ESP_OK) {
        this->info.avg_chamber_temperature = (int32_t) (t * 100.0f);
    } else {
        TB_LOGE(TAG, "fail to read AVG chamber temperature");
    }
}

void Regulator::heaters_on() {
    this->expander.relay_on(RelayType::RELAY_HEATER_1);
    this->expander.relay_on(RelayType::RELAY_HEATER_2);
}

void Regulator::heaters_off() {
    this->expander.relay_off(RelayType::RELAY_HEATER_1);
    this->expander.relay_off(RelayType::RELAY_HEATER_2);
}

void Regulator::fans_on() {
    this->expander.relay_on(RelayType::RELAY_FAN_1);
    this->expander.relay_on(RelayType::RELAY_FAN_2);
}

void Regulator::fans_off() {
    this->expander.relay_off(RelayType::RELAY_FAN_1);
    this->expander.relay_off(RelayType::RELAY_FAN_2);
}

esp_err_t Regulator::start() {
    if (this->running) {
        TB_LOGI(TAG, "already running");
        return ESP_FAIL;
    }

    this->get_avg_chamber_temperature();
    this->update_temperature(0);
    this->info.time = 0;

    this->last_sample_time = get_time_since_startup_ms();
    this->start_time = get_time_since_startup_ms();

    this->fans_on();

    this->running = true;

    xEventGroupSetBits(this->regulator_event_group, BIT_REGULATOR_START | BIT_REGULATOR_UPDATE);
    regulator_update_timer_run();

    return process_next_sample();
}

esp_err_t Regulator::stop() {
    if (!this->running) {
        TB_LOGI(TAG, "already stopped");
        return ESP_FAIL;
    }

    regulator_update_timer_stop();
    regulator_timer_stop();

    this->fans_off();

    this->running = false;
    return ESP_OK;
}

void Regulator::process_regulator() {
    if (!this->running) {
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
    this->info.time = (uint32_t)(get_time_since_startup_ms() - this->start_time);
    return this->info;
}

bool Regulator::is_running() {
    return this->running;
}

int16_t Regulator::get_min_temperature() {
    return this->config.min_temp;
}

int16_t Regulator::get_max_temperature() {
    return this->config.max_temp;
}


void Regulator::update_temperature(int16_t temperature) {
    this->get_external_temperature();

    if (temperature < this->ambient_temperature) {
        TB_LOGW(TAG, "set T is lower than ambient T, setting to ambient T");
        this->set_temperature = this->ambient_temperature;
    }

    if (temperature < this->config.min_temp) {
        TB_LOGW(TAG, "set T is lower than min T, setting to min T"); 
        this->set_temperature = this->config.min_temp;
    } else if (temperature > this->config.max_temp) {
        TB_LOGW(TAG, "set T is highet than max T, setting to max T");
        this->set_temperature = (float)this->config.max_temp;
        this->set_temperature = this->config.max_temp;
    } else {
        this->set_temperature = (float)temperature;
    }    

    TB_LOGI(TAG, "set temperature to %f*C", this->set_temperature / 100.0f);
}

EventGroupHandle_t *Regulator::get_regulator_event_group() {
    return &this->regulator_event_group;
}
