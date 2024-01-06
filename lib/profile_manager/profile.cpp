#include "inttypes.h"

#include "global_config.h"
#include "logger.h"
#include "tb_event.h"
#include "profile_timer.h"
#include "profile.h"


const char * const TAG = "Profile";


Profile::Profile(profile_config_t config) :
    config(config)
{
    this->info = {};
    this->info.running = false;
    this->profile_event_group = xEventGroupCreate();

    profile_timer_set_event_group(&this->profile_event_group);
    profile_timer_setup(this->config.step_time);
}

Profile::~Profile() {
    vEventGroupDelete(this->profile_event_group);
}

esp_err_t Profile::parse_raw_profile() {
    uint32_t previous_time = 0;
    this->profile.clear();

    for (uint8_t i = 0; i < PROFILE_MAX_VERTICES; i++) {
        int16_t temperature = this->config.profile.points[i].temperature;
        uint32_t time = this->config.profile.points[i].time_ms;
        if ((this->config.profile.points[i].temperature <= -1) || time == UINT32_MAX) {
            break;
        } else if (temperature >= this->config.min_temp &&
            temperature <= this->config.max_temp &&
            (time > previous_time || previous_time == 0)) {
            previous_time = time;
            this->profile.push_back({this->config.profile.points[i].temperature, time});
        } else {
            return ESP_FAIL;
        }
    }
    return ESP_OK;
}

esp_err_t Profile::calculate_vertices() {
    uint8_t size = this->profile.size();
    if (size > 0) {
        this->info.total_vertices = size;
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t Profile::calculate_duration() {
    uint32_t duration = this->profile.back().time_ms;
    if (duration >= this->config.min_duration &&
        duration <= this->config.max_duration) {
        this->info.total_duration = duration;
        return ESP_OK;
    }
    return ESP_FAIL;
}


esp_err_t Profile::prepare() {
    esp_err_t err = ESP_OK;

    if ((err = this->parse_raw_profile()) != ESP_OK) {
        TB_LOGE(TAG, "invalid defined profile");
        return err;
    }

    if ((err = this->calculate_vertices()) != ESP_OK) {
        TB_LOGE(TAG, "invalid #vertices");
        return err;
    }

    if ((err = this->calculate_duration()) != ESP_OK) {
        TB_LOGE(TAG, "invalid duration");
        return err;
    }

    TB_LOGI(TAG, "vertices: %" PRIu8 ", total duration: %" PRIu32 "s", this->info.current_vertices, this->info.total_duration);

    return err;
}

esp_err_t Profile::process_next_step() {
    if (this->info.current_vertices < 2) {
        this->end();
        return ESP_OK;
    }

    profile_point p1 = this->profile.front();
    this->profile.pop_front();
    profile_point p2 = this->profile.front();

    if (p1.time_ms == 0) {
        this->info.absolute_start_time = get_time_since_startup_ms();
    }

    this->info.current_duration = (uint32_t)(get_time_since_startup_ms() - this->info.absolute_start_time);
    this->info.profile_time_left = this->info.total_duration - this->info.current_duration;
    this->info.step_start_time = p1.time_ms;
    this->info.step_end_time = p2.time_ms;
    this->info.current_vertices--;

    // invalid time
    if (p1.time_ms >= p2.time_ms) {
        TB_LOGE(TAG, "invalid step time");
        return ESP_FAIL;
    }

    uint32_t preemption_error = this->info.current_duration - this->info.step_start_time;

    uint32_t next_step_timeout = this->info.step_end_time - this->info.step_start_time - preemption_error;

    if ((p1.temperature == p2.temperature) &&
        (p1.time_ms < p2.time_ms) &&
        (p2.time_ms - p1.time_ms >= PROFILE_LONG_TIMEOUT_INTERVAL_MS)) {
        next_step_timeout = PROFILE_LONG_TIMEOUT_INTERVAL_MS - preemption_error;
        this->info.step_end_time = this->info.step_start_time + next_step_timeout + preemption_error;
        if (this->info.step_end_time < p2.time_ms) {
            this->profile.push_front({p1.temperature, this->info.step_end_time});
            this->info.current_vertices++;
        }
    }

    // temperature change and step is longer than minimum step time
    if ((p1.temperature != p2.temperature) && (next_step_timeout > this->config.step_time)) {
        // calculate direction
        float a = 0.0f;
        float b = 0.0f;
        // slope
        a = ((float)(p2.temperature - p1.temperature) / (float)(p2.time_ms - p1.time_ms));
        // intercept
        b = (float)(p1.temperature - a * p1.time_ms);

        // direction treshold range
        if (a > PROFILE_STEP_MIN_ABS_SLOPE_TRESHOLD || a < -PROFILE_STEP_MIN_ABS_SLOPE_TRESHOLD) {
            if (preemption_error > this->config.step_time) {
                preemption_error -= this->config.step_time;
            }
            next_step_timeout = this->config.step_time - preemption_error;
            this->info.step_end_time = this->info.step_start_time + next_step_timeout + preemption_error;
            
            if (this->info.step_end_time < p2.time_ms) {
                int16_t next_step_temperature = a * (this->info.step_start_time + this->config.step_time) + b;
                this->profile.push_front({next_step_temperature, this->info.step_end_time});
                this->info.current_vertices++;
            }
        }
    }

    this->info.current_temperature = (int32_t) p1.temperature;
    this->info.step_time_left = next_step_timeout;

    if (next_step_timeout == 0) {
        TB_LOGE(TAG, "invalid next step timeout");
        return ESP_FAIL;
    }

    send_evt_regulator_update((int16_t) this->info.current_temperature);
    profile_timer_run(next_step_timeout);
    return ESP_OK;
}

void Profile::send_evt_regulator(RegulatorEvent *evt) {
    if (this->config.regulator_queue_handle == NULL) {
        TB_LOGE(TAG, "queue not set");
        return;
    }

    evt->origin = EventOrigin::PROFILE;
    if (xQueueSend(*this->config.regulator_queue_handle, &*evt, portMAX_DELAY) != pdTRUE) {
        TB_LOGE(TAG, "event send fail");
    }
}

void Profile::send_evt_regulator_start() {
    RegulatorEvent evt = {};
    evt.type = RegulatorEventType::START;
    this->send_evt_regulator(&evt);
}

void Profile::send_evt_regulator_stop() {
    RegulatorEvent evt = {};
    evt.type = RegulatorEventType::STOP;
    this->send_evt_regulator(&evt);
}

void Profile::send_evt_regulator_update(int16_t temperature) {
    RegulatorEvent evt = {};
    evt.type = RegulatorEventType::TEMPERATURE_UPDATE;
    RegulatorEventTemperatureUpdate payload = {};
    payload.temperature = temperature;
    memcpy(&evt.payload, &payload.buffer, sizeof(RegulatorEventTemperatureUpdate));
    this->send_evt_regulator(&evt);
}

esp_err_t Profile::start() {
    esp_err_t err = ESP_OK;

    if (this->info.running) {
        TB_LOGW(TAG, "profile already running");
        return ESP_FAIL;
    }

    if ((err = this->prepare()) != ESP_OK) {
        TB_LOGE(TAG, "fail to prepare profile");
        return err;
    }

    this->info.absolute_start_time = get_time_since_startup_ms();
    this->info.current_duration = 0;

    this->info.step_start_time = 0;
    this->info.step_end_time = 0;

    this->info.step_time_left = 0;
    this->info.profile_time_left = 0;

    this->info.absolute_ended_time = 0;

    this->info.current_temperature = 0;
    this->info.current_vertices = this->info.total_vertices;

    this->info.progress_percent = 0.0f;

    this->info.running = true;
    
    if ((err = process_next_step()) != ESP_OK) {
        TB_LOGE(TAG, "fail to start profile");
        this->info.running = false;
        return err;
    }

    xEventGroupSetBits(this->profile_event_group, BIT_PROFILE_START);
    this->send_evt_regulator_start();
    // this->send_evt_regulator_update((int16_t) this->info.current_temperature);

    return err;
}

esp_err_t Profile::end() {
    if (!this->info.running) {
        TB_LOGW(TAG, "no profile is running");
        return ESP_FAIL;
    }

    TB_LOGI(TAG, "ending current profile");
    profile_timer_stop();
    this->info.running = false;

    this->info.absolute_ended_time = get_time_since_startup_ms();

    this->info.current_duration = this->info.total_duration;
    this->info.progress_percent = (float)this->info.current_duration / (float)this->info.total_duration * 100.0f;

    this->print_info();

    this->send_evt_regulator_stop();

    xEventGroupSetBits(this->profile_event_group, BIT_PROFILE_END);
    return ESP_OK;
}

void Profile::process_profile() {
    EventBits_t bits = xEventGroupWaitBits(
        this->profile_event_group,
        BITS_PROFILE_TIMERS,
        pdTRUE,
        pdFALSE,
        pdMS_TO_TICKS(1)
    );

    if ((bits & BIT_PROFILE_TIMER_TIMEOUT) == BIT_PROFILE_TIMER_TIMEOUT) {
        esp_err_t err = ESP_OK;
        if (this->info.running) {
            TB_LOGI(TAG, "next step");
            if ((err = this->process_next_step()) != ESP_OK) {
                TB_LOGE(TAG, "fail to calculate next step %d", err);
            }
        }
    }
}

bool Profile::is_running() {
    return this->info.running;
}

EventGroupHandle_t *Profile::get_profile_event_group() {
    return &this->profile_event_group;
}

void Profile::print_raw_profile() {
    for (uint8_t i = 0; i < PROFILE_MAX_VERTICES; i++) {
        int16_t temperature = this->config.profile.points[i].temperature;
        uint32_t time = this->config.profile.points[i].time_ms;
        TB_LOGI(TAG, "%" PRIu8 ": %.2lf*C, %.3lfs", i, (float) temperature/100.0f, (float) time/1000.0f);
    }
}

void Profile::print_profile() {
    uint8_t i = 0;
    for (auto p : this->profile) {
        printf("%" PRIu8 ": %.2lf*C, %.3lfs\n", i++, (float) p.temperature/100.0f, (float) p.time_ms/ 1000.0f);
    }
}

void Profile::print_info() {
    printf("Absolute Start Time: %llu ms\n", this->info.absolute_start_time);
    printf("Current Duration: %u ms\n", this->info.current_duration);
    printf("Total Duration: %u ms\n", this->info.total_duration);
    printf("Step Start Time: %u ms\n", this->info.step_start_time);
    printf("Step End Time: %u ms\n", this->info.step_end_time);

    printf("Step Time Left: %u ms\n", this->info.step_time_left);
    printf("Profile Time Left: %u ms\n", this->info.profile_time_left);

    printf("Absolute Ended Time: %" PRIu64 " ms\n", this->info.absolute_ended_time);

    printf("Current Temperature: %d\n", this->info.current_temperature);
    printf("Current Vertices: %u\n", this->info.current_vertices);
    printf("Total Vertices: %u\n", this->info.total_vertices);

    printf("Progress: %.2lf%\n", this->info.progress_percent);

    printf("Running: %d\n", this->info.running);
}
