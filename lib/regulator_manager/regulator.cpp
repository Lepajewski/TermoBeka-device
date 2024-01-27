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

    this->get_external_temperature();

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
    // TB_LOGI(TAG, "avg chamber temperature: %" PRIi32, this->info.avg_chamber_temperature);


    float current_temperature = static_cast<float>(this->info.avg_chamber_temperature) / 100.0f;
    printf("\t\t\t\t\t\t\t\t\t%f, %f, %f, %f\n", current_temperature, this->set_temperature, this->config.hysteresis_down, this->config.hysteresis_up);
    if (current_temperature < (this->set_temperature - this->config.hysteresis_down)) {
        printf("\t\t\t\t\t\t\t\t\tHEATERS ON\n");
        this->heaters_on();
    } else if (current_temperature > (this->set_temperature + this->config.hysteresis_up)) {
        printf("\t\t\t\t\t\t\t\t\tHEATERS OFF\n");
        this->heaters_off();
    }

    int32_t preemption_error = (uint32_t)(get_time_since_startup_ms() - this->last_sample_time);
    this->last_sample_time = get_time_since_startup_ms();
    printf("\t\t\t\t\t\t\t\t\tPREEMPTION ERROR: %" PRIi32 "\n", preemption_error);

    int32_t delta_time = static_cast<int32_t>(this->config.sampling_rate) - preemption_error;
    printf("\t\t\t\t\t\t\t\t\tDELTA             %" PRIi32 "\n", delta_time);

    uint32_t next_sample_time = this->config.sampling_rate;
    if (delta_time < 0) {
        preemption_error -= this->config.sampling_rate;
        next_sample_time = this->config.sampling_rate - preemption_error;
    } else if (delta_time <= 0.1 * this->config.sampling_rate) {
        preemption_error = this->config.sampling_rate + delta_time;
        next_sample_time = preemption_error;
    } else if (delta_time > this->config.sampling_rate) {

    } else if (delta_time > 0.1 * this->config.sampling_rate) {
        preemption_error = this->config.sampling_rate - preemption_error;
        next_sample_time = preemption_error;
    }

    TB_LOGE(TAG, "setting timer to T+%" PRIu32 "ms %" PRIi32, next_sample_time, preemption_error);
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
    TB_LOGI(TAG, "Ambient temperature: %.2f", this->ambient_temperature);
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
    this->update_temperature(static_cast<int16_t>(this->set_temperature * 100));
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
    this->heaters_off();
    this->set_temperature = 0;

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
    TB_LOGI(TAG, "temperature to be set: %" PRIi16, temperature);

    if (temperature < this->ambient_temperature) {
        TB_LOGW(TAG, "set T is lower than ambient T, setting to ambient T");
        this->set_temperature = this->ambient_temperature;
    }

    if (temperature < this->config.min_temp) {
        TB_LOGW(TAG, "set T is lower than min T, setting to min T"); 
        this->set_temperature = this->config.min_temp / 100.0f;
    } else if (temperature > this->config.max_temp) {
        TB_LOGW(TAG, "set T is higher than max T, setting to max T");
        this->set_temperature = (float)this->config.max_temp / 100.0f;
    } else {
        this->set_temperature = static_cast<float>(temperature) / 100.0f;
    }

    TB_LOGI(TAG, "set temperature to %.2f*C", this->set_temperature);
}

EventGroupHandle_t *Regulator::get_regulator_event_group() {
    return &this->regulator_event_group;
}
