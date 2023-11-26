#include "inttypes.h"

#include "global_config.h"
#include "logger.h"
#include "profile_timer.h"
#include "profile.h"


const char * const TAG = "Profile";


Profile::Profile(profile_config_t config) :
    running(false),
    start_time(0),
    vertices(0),
    duration(0),
    config(config)
{
    this->profile_event_group = xEventGroupCreate();

    profile_timer_set_event_group(&this->profile_event_group);
    profile_timer_setup(this->config.step_time);
}

Profile::~Profile() {
    vEventGroupDelete(this->profile_event_group);
}

esp_err_t Profile::process_raw_profile() {
    int32_t previous_time = -1;
    this->profile.clear();

    for (uint8_t i = 0; i < PROFILE_MAX_VERTICES; i++) {
        float temperature = static_cast<float>(this->config.profile.points[i].temperature) / 100;
        int32_t time = this->config.profile.points[i].time_ms;

        if (this->config.profile.points[i].temperature <= -1 || time < 0) {
            break;
        } else if ((int16_t) temperature >= this->config.min_temp &&
            (int16_t) temperature <= this->config.max_temp &&
            time > previous_time && time >= 0) {
            
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
        this->vertices = size;
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t Profile::calculate_duration() {
    uint32_t duration = this->profile.back().time_ms / 1000;
    if (duration >= this->config.min_duration &&
        duration <= this->config.max_duration) {
        this->duration = duration;
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t Profile::calculate_next_step() {
    if (this->vertices < 2) {
        this->end();
        return ESP_OK;
    }

    profile_point p1 = this->profile.front();
    TB_LOGI(TAG, "current time: %" PRIi64, (uint64_t) p1.time_ms + this->start_time);

    this->profile.pop_front();
    this->vertices--;
    profile_point p2 = this->profile.front();
    
    int32_t next_step_time = 0;
    if ((p2.temperature == p1.temperature) && (p2.time_ms != p1.time_ms)) {
        next_step_time = p2.time_ms - p1.time_ms;
    } else if (p2.time_ms != p1.time_ms) {
        float a = 0.0f;
        float b = 0.0f;
        if (p2.time_ms != p1.time_ms) {
            a = ((float)(p2.temperature - p1.temperature) / (float)(p2.time_ms - p1.time_ms));
        }
        b = (float)(p1.temperature - a * p1.time_ms);
        // TB_LOGI(TAG, "direction: temp = %lf*t + %lf", a, b);
        if (a < 1e-3 && a > -1e-1) {
            next_step_time = p2.time_ms - p1.time_ms;
        } else {
            next_step_time = p1.time_ms + this->config.step_time;
            int16_t next_step_temperature = a * next_step_time + b;
            // TB_LOGI(TAG, "next: %" PRIi16 ", %" PRIi32, next_step_temperature, next_step_time);
            if (next_step_time < p2.time_ms) {
                this->profile.push_front({next_step_temperature, next_step_time});
                this->vertices++;
                next_step_time = this->config.step_time;
            } else {
                next_step_time = p2.time_ms - p1.time_ms;
            }
        }
    } else {
        return ESP_FAIL;
    }

    this->print_profile();

    TB_LOGI(TAG, "set timer to: T+%" PRIi32 "ms", next_step_time);
    profile_timer_run(next_step_time);
    return ESP_OK;
}

esp_err_t Profile::prepare() {
    esp_err_t err = ESP_OK;

    if ((err = this->process_raw_profile()) != ESP_OK) {
        ESP_LOGE(TAG, "invalid defined profile");
        return err;
    }

    if ((err = this->calculate_vertices()) != ESP_OK) {
        return err;
    }

    if ((err = this->calculate_duration()) != ESP_OK) {
        return err;
    }

    TB_LOGI(TAG, "vertices: %" PRIu8 ", duration: %" PRIu32 "s", this->vertices, this->duration);

    return err;
}

esp_err_t Profile::start() {
    esp_err_t err = ESP_OK;

    if (this->running) {
        TB_LOGW(TAG, "profile already running");
        return ESP_FAIL;
    }

    if ((err = this->prepare()) != ESP_OK) {
        TB_LOGE(TAG, "fail to prepare profile");
        return err;
    }
    this->print_profile();

    uint64_t time = get_time_since_startup_ms();
    this->set_absolute_start_time(time);
    this->running = true;

    xEventGroupSetBits(this->profile_event_group, BIT_PROFILE_START);

    return calculate_next_step();
}

void Profile::process_next_step() {
    EventBits_t bits = xEventGroupWaitBits(
        this->profile_event_group,
        BIT_PROFILE_TIMER_TIMEOUT,
        pdTRUE,
        pdFALSE,
        pdMS_TO_TICKS(5)
    );

    if ((bits & BIT_PROFILE_TIMER_TIMEOUT) == BIT_PROFILE_TIMER_TIMEOUT) {
        TB_LOGI(TAG, "next step");
        this->calculate_next_step();
    }
}

esp_err_t Profile::end() {
    if (this->running) {
        TB_LOGI(TAG, "ending current profile");
        profile_timer_stop();
        xEventGroupSetBits(this->profile_event_group, BIT_PROFILE_END);
        this->running = false;
        return ESP_OK;
    }
    TB_LOGW(TAG, "no profile is running");
    return ESP_FAIL;
}

uint8_t Profile::get_vertices() {
    return this->vertices;
}

bool Profile::is_running() {
    return this->running;
}

void Profile::set_absolute_start_time(uint64_t time) {
    TB_LOGI(TAG, "set time to: %" PRIi64, time);
    this->start_time = time;
}

uint64_t Profile::get_absolute_start_time() {
    return this->start_time;
}

uint64_t Profile::get_absolute_end_time() {
    return this->start_time + this->duration;
}

uint32_t Profile::get_duration_total() {
    return this->duration;
}

uint32_t Profile::get_duration_so_far() {
    return 0;
}

uint16_t Profile::get_min_temp() {
    return this->config.min_temp;
}

uint16_t Profile::get_max_temp() {
    return this->config.max_temp;
}

EventGroupHandle_t *Profile::get_profile_event_group() {
    return &this->profile_event_group;
}

void Profile::print_raw_profile() {
    for (uint8_t i = 0; i < PROFILE_MAX_VERTICES; i++) {
        int16_t temperature = this->config.profile.points[i].temperature;
        int32_t time = this->config.profile.points[i].time_ms;
        TB_LOGI(TAG, "%" PRIu8 ": %.2lf*C, %.3lfs", i, (float) temperature/100, (float) time/1000);
    }
}

void Profile::print_profile() {
    uint8_t i = 0;
    for (auto p : this->profile) {
        TB_LOGI(TAG, "%" PRIu8 ": %.2lf*C, %.3lfs", i++, (float) p.temperature/100, (float) p.time_ms/ 1000);
    }
}
