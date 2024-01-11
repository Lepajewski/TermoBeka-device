#include <cstring>

#include "logger.h"

#include "ui_manager.h"
#include "lcd_controller.h"

const char * const TAG = "UIMgr";


UIManager::UIManager() {
    this->expander = std::make_unique<GPIOExpander>();
    this->state = std::make_shared<UISystemState>();
}

void UIManager::button_callback(Button *button, PressType type) {
    this->current_scene->button_callback(button, type);
    send_evt_button_press(button->get_pin_num(), type);
}

void UIManager::setup() {
    this->sysMgr = get_system_manager();
    this->event_queue_handle = sysMgr->get_event_queue();
    this->ui_queue_handle = sysMgr->get_ui_queue();

    // setup GPIO expander
    this->expander->set_callback(std::bind(&UIManager::button_callback, this, std::placeholders::_1, std::placeholders::_2));

    // setup BUTTONS & LEDS
    this->expander->begin();

    // setup LCD
    LCDController::begin();

    switch_scene(SceneEnum::startup);
}

void UIManager::process_ui_event(UIEvent *evt) {
    switch (evt->type) {
        case UIEventType::BUZZER_BEEP:
        {
            UIEventBuzzerBeep *payload = reinterpret_cast<UIEventBuzzerBeep*>(evt->payload);

            TB_LOGI(TAG, "Duration: %u", payload->duration);
            this->expander->buzzer_beep(payload->duration);
            break;
        }
        case UIEventType::ERROR_SHOW:
        {
            break;
        }
        case UIEventType::PROFILE_RESPONSE:
        {
            EventProfileResponse *payload = reinterpret_cast<EventProfileResponse*>(evt->payload);

            process_profile_response(payload->response);
            break;
        }
        case UIEventType::NEW_PROFILE_INFO:
        {
            EventNewProfileInfo *payload = reinterpret_cast<EventNewProfileInfo*>(evt->payload);

            process_new_profile_info(payload);
            break;
        }
        case UIEventType::REGULATOR_UPDATE: 
        {
            EventRegulatorUpdate *payload = reinterpret_cast<EventRegulatorUpdate*>(evt->payload);
            
            process_regulator_update(payload->info);
            break;
        }
        case UIEventType::NONE:
        default:
            break;
    }

    this->current_scene->process_ui_event(evt);
}

void UIManager::poll_ui_events() {
    UIEvent evt;

    while (uxQueueMessagesWaiting(*this->ui_queue_handle)) {
        if (xQueueReceive(*this->ui_queue_handle, &evt, pdMS_TO_TICKS(10)) == pdPASS) {
            TB_LOGI(TAG, "new event, type: %d", evt.type);
            process_ui_event(&evt);

            if (evt.type == UIEventType::WIFI_STRENGTH) {
                memcpy(&state->wifi_rssi, evt.payload, sizeof(int));
            }
        }
    }
}

void UIManager::process_profile_response(profile_event_response response) {
    switch (response) {
        case PROFILE_LOAD_SUCCESS: {
            state->profile_info.profile_state = ProfileState::loaded;
        }
        break;

        case PROFILE_LOAD_FAIL: {
            state->profile_info.profile_state = ProfileState::unloaded;
            state->profile_info.selected_profile = "";
        }
        break;

        case PROFILE_START_SUCCESS: {
            state->profile_info.profile_state = ProfileState::running;
        }
        break;

        case PROFILE_START_FAIL:
        case PROFILE_END_SUCCESS: {
            state->profile_info.profile_state = ProfileState::loaded;
        }
        break;

        case PROFILE_END_FAIL:
        default:
        break;
    }
}

void UIManager::process_new_profile_info(EventNewProfileInfo *payload) {
    state->profile_info.profile_duration = payload->info.duration;
}

void UIManager::process_regulator_update(RegulatorStatusUpdate &info) {
    state->profile_info.profile_time = info.time;
    state->profile_info.avg_temperature = info.avg_chamber_temperature;
}

void UIManager::send_evt(Event *evt) {
    evt->origin = EventOrigin::UI;
    if (xQueueSend(*this->event_queue_handle, &*evt, portMAX_DELAY) != pdTRUE) {
        TB_LOGE(TAG, "event send fail");
    }
}

void UIManager::send_evt_button_press(pca9539_pin_num pin_num, PressType type) {
    Event evt = {};
    evt.type = EventType::UI_BUTTON_PRESS;
    EventUIButtonPress payload = {};
    payload.press_data.num = static_cast<uint8_t>(pin_num);
    payload.press_data.type = static_cast<uint8_t>(type);

    memcpy(&evt.payload, &payload.buffer, sizeof(payload));

    this->send_evt(&evt);
}

void UIManager::switch_scene(SceneEnum target) {
    this->current_scene = Scene::create_scene(target, state);
}

void UIManager::check_scene_transition() {
    if (this->current_scene->get_should_be_changed()) {
        switch_scene(this->current_scene->get_next_scene());
    }
}

void UIManager::process_events() {
    this->expander->poll_intr_events();
    poll_ui_events();
}

void UIManager::update(float d_time) {
    check_scene_transition();

    this->current_scene->update(d_time);
}
