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

    this->setup();
}

Regulator::~Regulator() {
    vEventGroupDelete(this->regulator_event_group);
}

void Regulator::setup_rtds() {
    this->controller.begin();
}

void Regulator::setup() {
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

    this->get_cpu_temperature();
    this->get_external_temperature();
    float chamber_temperature = this->get_avg_rtd_temperature();
    TB_LOGI(TAG, "avg chamber temperature: %f", chamber_temperature);

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

void Regulator::get_cpu_temperature() {
    if (get_internal_temperature(&this->info.uc_temperature) != ESP_OK) {
        this->info.uc_temperature = 0.0f;
    }

    // TB_LOGI(TAG, "uC temperature: %f", this->info.uc_temperature);
}

void Regulator::get_external_temperature() {
    external_temperature temperature = {};
    if (this->ds18b20.get_temperature(&temperature) != ESP_OK) {
        TB_LOGE(TAG, "fail to read ds18b20");
        temperature = {};
    }

    this->info.ssr_temperature_1 = temperature.temperature[0];
    this->info.ssr_temperature_2 = temperature.temperature[1];
    this->info.external_temperature = temperature.temperature[2];

    // TB_LOGI(TAG, "ssr_temperature_1: %f", temperature.temperature[0]);
    // TB_LOGI(TAG, "ssr_temperature_2: %f", temperature.temperature[1]);
    // TB_LOGI(TAG, "external_temperature: %f", temperature.temperature[2]);
}

float Regulator::get_avg_rtd_temperature() {
    float sum = 0.0f;
    uint8_t working_sensors = 0;

    rtd_temperature t = {};
    if (this->controller.get_temperatures(&t) != ESP_OK) {
        TB_LOGE(TAG, "fail to read RTD temperature");
    }

    if (t.fault[0] == Max31865Error::NoError) {
        this->info.temperature_0 = t.temperature[0];
        sum += this->info.temperature_0;
        working_sensors++;
    }

    if (t.fault[1] == Max31865Error::NoError) {
        this->info.temperature_1 = t.temperature[1];
        sum += this->info.temperature_1;
        working_sensors++;
    }

    if (t.fault[2] == Max31865Error::NoError) {
        this->info.temperature_2 = t.temperature[2];
        sum += this->info.temperature_2;
        working_sensors++;
    }

    if (t.fault[3] == Max31865Error::NoError) {
        this->info.temperature_3 = t.temperature[3];
        sum += this->info.temperature_3;
        working_sensors++;
    }

    if (t.fault[4] == Max31865Error::NoError) {
        this->info.temperature_4 = t.temperature[4];
        sum += this->info.temperature_4;
        working_sensors++;
    }

    // TB_LOGI(TAG, "RTD 0: %f", this->info.temperature_0);
    // TB_LOGI(TAG, "RTD 1: %f", this->info.temperature_1);
    // TB_LOGI(TAG, "RTD 2: %f", this->info.temperature_2);
    // TB_LOGI(TAG, "RTD 3: %f", this->info.temperature_3);
    // TB_LOGI(TAG, "RTD 4: %f", this->info.temperature_4);

    return sum / (float)working_sensors;
}

void Regulator::heaters_on() {
    this->expander.relay_on(RelayType::RELAY_HEATER_1);
    this->expander.relay_on(RelayType::RELAY_HEATER_2);
    this->info.relays_states |= (1U << static_cast<uint32_t>(RelayType::RELAY_HEATER_1));
    this->info.relays_states |= (1U << static_cast<uint32_t>(RelayType::RELAY_HEATER_2));
}

void Regulator::heaters_off() {
    this->expander.relay_off(RelayType::RELAY_HEATER_1);
    this->expander.relay_off(RelayType::RELAY_HEATER_2);
    this->info.relays_states &= ~(1U << static_cast<uint32_t>(RelayType::RELAY_HEATER_1));
    this->info.relays_states &= ~(1U << static_cast<uint32_t>(RelayType::RELAY_HEATER_2));
}

void Regulator::fans_on() {
    this->expander.relay_on(RelayType::RELAY_FAN_1);
    this->expander.relay_on(RelayType::RELAY_FAN_2);
    this->info.relays_states |= (1U << static_cast<uint32_t>(RelayType::RELAY_FAN_1));
    this->info.relays_states |= (1U << static_cast<uint32_t>(RelayType::RELAY_FAN_2));
}

void Regulator::fans_off() {
    this->expander.relay_off(RelayType::RELAY_FAN_1);
    this->expander.relay_off(RelayType::RELAY_FAN_2);
    this->info.relays_states &= ~(1U << (int)RelayType::RELAY_FAN_1);
    this->info.relays_states &= ~(1U << (int)RelayType::RELAY_FAN_2);
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

    this->info.uc_temperature = 0.0f;
    this->info.temperature_0 = 0.0f;
    this->info.temperature_1 = 0.0f;
    this->info.temperature_2 = 0.0f;
    this->info.temperature_3 = 0.0f;
    this->info.temperature_4 = 0.0f;
    this->info.relays_states = 0;
    this->info.ssr_temperature_1 = 0.0f;
    this->info.ssr_temperature_2 = 0.0f;
    this->info.external_temperature = 0.0f;

    this->last_sample_time = get_time_since_startup_ms();

    this->info.status = Status_RUNNING;

    this->fans_on();

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

    this->fans_off();

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
        this->get_external_temperature();
        this->set_temperature = this->info.external_temperature;
    } else if (temperature > this->config.max_temp) {
        TB_LOGW(TAG, "set T is too high");
        this->set_temperature = (float)this->config.max_temp;
    } else {
        this->set_temperature = (float)temperature;
    }

    TB_LOGI(TAG, "set temperature to %f*C", this->set_temperature);
}

EventGroupHandle_t *Regulator::get_regulator_event_group() {
    return &this->regulator_event_group;
}

void Regulator::print_info() {
    ;
}
