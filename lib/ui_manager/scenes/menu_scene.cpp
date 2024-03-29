#include "menu_scene.h"

#include "lcd_controller.h"
#include "global_config.h"

#include "logger.h"

MenuScene::MenuScene(std::shared_ptr<UISystemState> system_state) : Scene(system_state) {
    LCDController::clear_frame_buf();
}

SceneEnum MenuScene::get_scene_enum() {
    return SceneEnum::menu;
}

void MenuScene::button_callback(Button *button, PressType type) {
    ButtonType button_type = button->get_button_type();

    switch (button_type) {
        case ButtonType::L_BOT: {
            this->list.select();
        }
        break;

        case ButtonType::L_MID: {
            this->list.move_down();
        }
        break;

        case ButtonType::L_UP: {
            this->list.move_up();
        }
        break;

        case ButtonType::R_UP:
        case ButtonType::R_MID:
        break;

        case ButtonType::R_BOT:  {
            next_scene = SceneEnum::startup;
            should_be_changed = true;
        }
        break;

        default:
        break;
    }
}

void MenuScene::update(float d_time) {
    LCDController::clear_frame_buf();

    char timestamp[TIMESTAMP_SIZE];
    get_timestamp(timestamp);
    std::string time = std::string(timestamp);
    time = time.substr(time.find('T') + 1, 5);
    status.set_time(std::stoi(time.substr(0, 2)), std::stoi(time.substr(3, 2)));

    status.set_wifi_strength(system_state->wifi_rssi);

    if (system_state->profile_info.profile_state == ProfileState::running) {
        profile_bar.set_remaining_time(system_state->profile_info.profile_time, system_state->profile_info.profile_duration);
        profile_bar.set_temperature(system_state->profile_info.avg_temperature);

        profile_bar.draw(d_time);
    }
    
    status.draw(d_time);
    list.draw();
    LCDController::display_frame_buf();
}
