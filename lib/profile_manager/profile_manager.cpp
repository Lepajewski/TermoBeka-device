#include <time.h>

#include "logger.h"

#include "profile_timer.h"
#include "profile_manager.h"

const char * const TAG = "ProfMgr";


ProfileManager::ProfileManager() {
    profile_t profile = {};
    for (uint8_t i = 0; i < PROFILE_MAX_VERTICES; i++) {
        profile.points[i] = {-1, -1};
    }

    profile.points[0] = {2000, 0};
    profile.points[1] = {2000, 30000};
    profile.points[2] = {5000, 60000};
    profile.points[3] = {5000, 90000};
    profile.points[4] = {3000, 110000};
    profile.points[5] = {3000, 140000};

    this->config.profile = profile;
    this->config.min_temp = PROFILE_MIN_TEMPERATURE;
    this->config.max_temp = PROFILE_MAX_TEMPERATURE;
    this->config.step_time = PROFILE_STEP_TIME_MS;
    this->config.min_duration = PROFILE_MIN_DURATION_S;
    this->config.max_duration = PROFILE_MAX_DURATION_S;

    this->profile = new Profile(this->config);
}

ProfileManager::~ProfileManager() {
    this->end();
}

void ProfileManager::begin() {
    this->sysMgr = get_system_manager();
    this->event_queue_handle = this->sysMgr->get_event_queue();
    this->profile_queue_handle = this->sysMgr->get_profile_queue();
    
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
            this->process_new_profile(&payload->profile);
            break;
        }
        case ProfileEventType::START:
        {
            this->start_profile();
            break;
        }
        case ProfileEventType::STOP:
        {
            this->stop_profile();
            break;
        }
        case ProfileEventType::RESUME:
        {
            this->resume_profile();
            break;
        }
        case ProfileEventType::END:
        {
            this->end_profile();
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
        if (xQueueReceive(*this->profile_queue_handle, &evt, pdMS_TO_TICKS(5)) == pdPASS) {
            TB_LOGI(TAG, "new event, type: %d", evt.type);
            process_profile_queue_event(&evt);
        }
    }
}

void ProfileManager::poll_running_profile_events() {
    EventBits_t bits = xEventGroupWaitBits(
        *this->profile->get_profile_event_group(),
        BIT_PROFILE_ALL,
        pdTRUE,
        pdFALSE,
        pdMS_TO_TICKS(5)
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
    }
}

void ProfileManager::process_events() {
    if (this->profile->is_running()) {
        this->profile->process_next_step();
    }
    this->poll_profile_queue_events();
    this->poll_running_profile_events();
}

void ProfileManager::process_new_profile(profile_t *profile) {
    if (!this->profile->is_running()) {
        delete this->profile;
        memcpy(&this->config.profile, profile, sizeof(profile_t));
        this->profile = new Profile(this->config);
        this->profile->print_raw_profile();
    } else {
        TB_LOGW(TAG, "another profile is running");
    }
}

void ProfileManager::start_profile() {
    esp_err_t err = this->profile->start();
    if (err != ESP_OK) {
        TB_LOGE(TAG, "fail to start profile");
    } else {
        TB_LOGI(TAG, "started profile");
    }
}

void ProfileManager::stop_profile() {
    return;
}

void ProfileManager::resume_profile() {
    return;
}

void ProfileManager::end_profile() {
    esp_err_t err = this->profile->end();
    if (err != ESP_OK) {
        TB_LOGE(TAG, "fail to end profile");
    } else {
        TB_LOGI(TAG, "ended profile");
    }
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

