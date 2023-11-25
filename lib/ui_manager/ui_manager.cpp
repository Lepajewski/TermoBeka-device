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
    pca9539_pin_num pin_num = button->get_pin_num();

    this->buzzer.beep(50);

    // process event
    switch (type) {
        case PressType::SHORT_PRESS:
        {
            this->send_evt_button_press(pin_num, PressType::SHORT_PRESS);

            switch (pin_num) {
                case P0_0:
                case P0_1:
                case P0_2:
                case P0_3:
                case P0_4:
                case P0_5:
                {
                    this->lcd.clear();
                    this->lcd.print_formatted("B%d, SHORT", (int) pin_num);
                    this->expander->set_backlight_color(Color::NONE);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case PressType::LONG_PRESS:
        {
            this->send_evt_button_press(pin_num, PressType::LONG_PRESS);

            switch (pin_num) {
                case P0_0:
                {
                    this->lcd.draw_logo();
                    break;
                }
                case P0_1:
                {
                    this->lcd.clear();
                    this->expander->set_backlight_color(Color::R);
                    break;
                }
                case P0_2:
                {
                    this->expander->set_backlight_color(Color::G);
                    break;
                }
                case P0_3:
                {
                    this->expander->set_backlight_color(Color::B);
                    break;
                }
                case P0_4:
                {
                    this->expander->set_backlight_color(Color::RGB);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
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
}

void UIManager::process_ui_event(UIEvent *evt) {
    switch (evt->type) {
        case UIEventType::BUZZER_BEEP:
        {
            UIEventBuzzerBeep *payload = reinterpret_cast<UIEventBuzzerBeep*> (evt->payload);

            TB_LOGI(TAG, "Duration: %u", payload->duration);
            this->buzzer.beep(payload->duration);
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

void UIManager::process_events() {
    this->expander->poll_intr_events();
    poll_ui_events();
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
