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
    TB_LOGI(TAG, "BUTTON %u, %u", button->get_pin_num(), type);

    // process event
}

void UIManager::setup() {
    // setup BUZZER & LEDS

    // setup GPIO expander
    this->expander->set_callback(std::bind(&UIManager::button_callback, this, std::placeholders::_1, std::placeholders::_2));

    this->expander->begin();
}

void UIManager::process_events() {
    this->expander->poll_intr_events();
}
