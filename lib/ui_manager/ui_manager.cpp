#include <cstring>

#include "logger.h"

#include "ui_manager.h"
#include "lcd_controller.h"

const char * const TAG = "UIMgr";


UIManager::UIManager() {
    this->expander = new GPIOExpander();
}

UIManager::~UIManager() {
    delete this->expander;
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
            UIEventBuzzerBeep *payload = reinterpret_cast<UIEventBuzzerBeep*> (evt->payload);

            TB_LOGI(TAG, "Duration: %u", payload->duration);
            this->expander->buzzer_beep(payload->duration);
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

    this->current_scene->process_ui_event(evt);
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
    this->current_scene = Scene::create_scene(target);
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
