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
    TB_LOGI(TAG, "BUTTON %u, %u", pin_num, type);

    this->buzzer.beep(50);

    // process event
    switch (type) {
        case PressType::SHORT_PRESS:
        {
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
    // setup GPIO expander
    this->expander->set_callback(std::bind(&UIManager::button_callback, this, std::placeholders::_1, std::placeholders::_2));

    this->expander->begin();

    // setup BUZZER & LEDS

    // setup LCD
    this->lcd.begin();
}

void UIManager::process_events() {
    this->expander->poll_intr_events();
}
