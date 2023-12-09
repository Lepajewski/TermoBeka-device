#include <time.h>

#include "logger.h"

#include "profile_timer.h"
#include "profile_manager.h"


const char * const TAG = "ProfMgr";


ProfileManager::ProfileManager() {
    this->config = {};
}

ProfileManager::~ProfileManager() {
    this->end();
}

void ProfileManager::begin() {
    this->sysMgr = get_system_manager();
    this->event_queue_handle = this->sysMgr->get_event_queue();
    this->profile_queue_handle = this->sysMgr->get_profile_queue();

    profile_t profile = {};
    for (uint8_t i = 0; i < PROFILE_MAX_VERTICES; i++) {
        profile.points[i] = {-1, UINT32_MAX};
    }

    // profile.points[0] = {2000,  0};
    // profile.points[1] = {2000, 20000};
    // profile.points[2] = {2000, 10000000};
    // profile.points[3] = {2000, 40000000};

    profile.points[0] = {2000,  0};
    profile.points[1] = {12000, 2000000};
    profile.points[2] = {12000, 12800000};
    profile.points[3] = {4000,  14420000};

    this->config.profile = profile;
    this->config.min_temp = PROFILE_MIN_TEMPERATURE;
    this->config.max_temp = PROFILE_MAX_TEMPERATURE;
    this->config.step_time = PROFILE_STEP_TIME_MS;
    this->config.min_duration = PROFILE_MIN_DURATION_MS;
    this->config.max_duration = PROFILE_MAX_DURATION_MS;
    this->config.update_interval = PROFILE_UPDATE_TIMER_INTERVAL_MS;
    this->config.regulator_queue_handle = this->sysMgr->get_regulator_queue();

    this->profile = new Profile(this->config);
    TB_LOGI(TAG, "begin");
}

void ProfileManager::end() {
    delete this->profile;
}

void ProfileManager::process_profile_queue_event(ProfileEvent *evt) {
    TB_LOGI(TAG, "%s", profile_event_type_to_s(evt->type));
    switch (evt->type) {
        case ProfileEventType::NEW_PROFILE:
        {
            ProfileEventNewProfile *payload = reinterpret_cast<ProfileEventNewProfile*>(evt->payload);
            this->send_evt_response(this->process_new_profile(&payload->profile));
            break;
        }
        case ProfileEventType::START:
        {
            this->send_evt_response(this->start_profile());
            break;
        }
        case ProfileEventType::STOP:
        {
            this->send_evt_response(this->stop_profile());
            break;
        }
        case ProfileEventType::RESUME:
        {
            this->send_evt_response(this->resume_profile());
            break;
        }
        case ProfileEventType::END:
        {
            this->send_evt_response(this->end_profile());
            break;
        }
        case ProfileEventType::INFO:
        {
            this->print_profile_info();
            break;
        }
        case ProfileEventType::NONE:
        default:
            break;
    }
}

void ProfileManager::poll_profile_queue_events() {
    ProfileEvent evt;

    while (uxQueueMessagesWaiting(*this->profile_queue_handle)) {
        if (xQueueReceive(*this->profile_queue_handle, &evt, pdMS_TO_TICKS(1)) == pdPASS) {
            TB_LOGI(TAG, "new event, type: %d", evt.type);
            process_profile_queue_event(&evt);
        }
    }
}

void ProfileManager::poll_running_profile_events() {
    EventBits_t bits = xEventGroupWaitBits(
        *this->profile->get_profile_event_group(),
        BITS_PROFILE_CONTROL,
        pdTRUE,
        pdFALSE,
        pdMS_TO_TICKS(1)
    );

    if ((bits & BIT_PROFILE_START) == BIT_PROFILE_START) {
        TB_LOGI(TAG, "profile started");
        this->send_evt_start();
    } else if ((bits & BIT_PROFILE_STOP) == BIT_PROFILE_STOP) {
        TB_LOGI(TAG, "profile stopped");
        this->send_evt_stop();
    } else if ((bits & BIT_PROFILE_RESUME) == BIT_PROFILE_RESUME) {
        TB_LOGI(TAG, "profile resumed");
        this->send_evt_resume();
    } else if ((bits & BIT_PROFILE_END) == BIT_PROFILE_END) {
        TB_LOGI(TAG, "profile ended");
        this->send_evt_end();
    } else if ((bits & BIT_PROFILE_UPDATE) == BIT_PROFILE_UPDATE) {
        TB_LOGI(TAG, "profile update");
    }

    if (bits) {
        this->send_evt_update();
    }
}

void ProfileManager::process_events() {
    this->poll_profile_queue_events();
    this->poll_running_profile_events();
    this->profile->process_profile();
}

profile_event_response ProfileManager::process_new_profile(profile_t *profile) {
    if (this->profile->is_running()) {
        TB_LOGE(TAG, "another profile is running");
        return PROFILE_LOAD_FAIL;
    }

    delete this->profile;
    memcpy(&this->config.profile, profile, sizeof(profile_t));
    this->profile = new Profile(this->config);
    this->profile->print_raw_profile();
    return PROFILE_LOAD_SUCCESS;
}

profile_event_response ProfileManager::start_profile() {
    esp_err_t err = this->profile->start();
    if (err != ESP_OK) {
        TB_LOGE(TAG, "fail to start profile");
        return PROFILE_START_FAIL;
    }

    TB_LOGI(TAG, "started profile");
    return PROFILE_START_SUCCESS;
}

profile_event_response ProfileManager::stop_profile() {
    esp_err_t err = this->profile->stop();
    if (err != ESP_OK) {
        TB_LOGE(TAG, "fail to stop profile");
        return PROFILE_STOP_FAIL;
    }

    TB_LOGI(TAG, "stopped profile");
    return PROFILE_STOP_SUCCESS;
}

profile_event_response ProfileManager::resume_profile() {
    esp_err_t err = this->profile->resume();
    if (err != ESP_OK) {
        TB_LOGE(TAG, "fail to resume profile");
        return PROFILE_RESUME_FAIL;
    }

    TB_LOGI(TAG, "resumed profile");
    return PROFILE_RESUME_SUCCESS;
}

profile_event_response ProfileManager::end_profile() {
    esp_err_t err = this->profile->end();
    if (err != ESP_OK) {
        TB_LOGE(TAG, "fail to end profile");
        return PROFILE_END_FAIL;
    }

    TB_LOGI(TAG, "ended profile");
    return PROFILE_END_SUCCESS;
}

void ProfileManager::print_profile_info() {
    this->profile->print_info();
}

void ProfileManager::send_evt(Event *evt) {
    evt->origin = EventOrigin::PROFILE;
    if (xQueueSend(*this->event_queue_handle, &*evt, portMAX_DELAY) != pdTRUE) {
        TB_LOGE(TAG, "event send fail");
    }
}

void ProfileManager::send_evt_start() {
    Event evt = {};
    evt.type = EventType::PROFILE_START;
    this->send_evt(&evt);
}

void ProfileManager::send_evt_stop() {
    Event evt = {};
    evt.type = EventType::PROFILE_STOP;
    this->send_evt(&evt);
}

void ProfileManager::send_evt_resume() {
    Event evt = {};
    evt.type = EventType::PROFILE_RESUME;
    this->send_evt(&evt);
}

void ProfileManager::send_evt_end() {
    Event evt = {};
    evt.type = EventType::PROFILE_END;
    this->send_evt(&evt);
}

void ProfileManager::send_evt_response(profile_event_response response) {
    Event evt = {};
    evt.type = EventType::PROFILE_RESPONSE;
    EventProfileResponse payload = {};
    payload.response = response;
    memcpy(&evt.payload, &payload.buffer, sizeof(EventProfileResponse));
    this->send_evt(&evt);
}

void ProfileManager::send_evt_update() {
    Event evt = {};
    evt.type = EventType::PROFILE_UPDATE;
    EventProfileUpdate payload = {};
    payload.info = this->profile->get_profile_run_info();
    memcpy(&evt.payload, &payload.buffer, sizeof(EventProfileUpdate));
    this->send_evt(&evt);
}
