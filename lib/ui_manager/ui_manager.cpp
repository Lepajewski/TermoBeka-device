#include <cstring>

#include "logger.h"

#include "ui_manager.h"


const char * const TAG = "UIMgr";


UIManager::UIManager() {
    this->expander = new GPIOExpander();
}

UIManager::~UIManager() {
    delete this->expander;
}

void UIManager::button_callback(Button *button, PressType type) {
    this->current_scene->button_callback(button, type);
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
    this->lcd.begin();

    switch_scene(SceneEnum::startup);
}

void UIManager::process_ui_event(UIEvent *evt) {
    switch (evt->type) {
        case UIEventType::BUZZER_BEEP:
        {
            uint32_t duration = 0;
            memcpy(&duration, evt->payload, sizeof(duration));

            TB_LOGI(TAG, "Duration: %u", duration);
            this->buzzer.beep(duration);
            break;
        }
        case UIEventType::ERROR_SHOW:
        {
            break;
        }
        case UIEventType::NONE:
        default:
            break;
    }
}

void UIManager::poll_ui_events() {
    UIEvent evt;

    while (uxQueueMessagesWaiting(*this->ui_queue_handle)) {
        if (xQueueReceive(*this->ui_queue_handle, &evt, pdMS_TO_TICKS(10)) == pdPASS) {
            TB_LOGI(TAG, "new event, type: %d", evt.type);
            process_ui_event(&evt);
        }
    }
}

void UIManager::switch_scene(SceneEnum target) {
    this->current_scene = Scene::create_scene(target, &this->lcd);
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
