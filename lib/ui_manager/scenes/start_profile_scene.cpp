#include "start_profile_scene.h"

#include "lcd_controller.h"

#define ERROR_DISPLAY_DURATION 2

StartProfileScene::StartProfileScene(std::shared_ptr<UISystemState> system_state) : Scene(system_state) {
    display_err = system_state->profile_info.profile_state == ProfileState::unloaded || system_state->profile_info.profile_state == ProfileState::loading;
    previous_profile_state = system_state->profile_info.profile_state;

    if (display_err) {
        LCDController::clear_frame_buf();

        LCDController::draw_string(0, (LCD_HEIGHT - FONT5X7_LINE_HEIGHT) / 2 - FONT5X7_LINE_HEIGHT, "No profile");
        LCDController::draw_string(0, (LCD_HEIGHT + FONT5X7_LINE_HEIGHT) / 2 - FONT5X7_LINE_HEIGHT, "       loaded");

        LCDController::display_frame_buf();
    }
    else {
        construct_option_list();
    }
}

void StartProfileScene::construct_option_list() {
    switch (system_state->profile_info.profile_state) {
        case ProfileState::loaded: {
            list = OptionList(list_rect, {
                {"Start", [this](){ this->start_profile(); }},
            });
        }
        break;

        case ProfileState::running: {
            list = OptionList(list_rect, {
                {"Stop", [this](){ this->stop_profile(); }},
            });
        }
        break;
    
        default:
            break;
    }
}

void StartProfileScene::start_profile() {
    this->system_state->profile_info.profile_state = ProfileState::starting;
    
    Event evt = {};
    evt.type = EventType::UI_PROFILE_START;
    this->send_evt(&evt);

    this->system_state->waiting_message_args.type = MessageType::profile_started;
    std::shared_ptr<UISystemState> state = system_state;
    this->system_state->waiting_message_args.waiting_function = [state](){ return state->profile_info.profile_state == ProfileState::starting; };
    this->system_state->waiting_message_args.success_function = [state](){ return state->profile_info.profile_state == ProfileState::running; };
    this->next_scene = SceneEnum::waiting_message;
    this->should_be_changed = true;
}

void StartProfileScene::stop_profile() {
    this->system_state->profile_info.profile_state = ProfileState::stopping;

    Event evt = {};
    evt.type = EventType::UI_PROFILE_STOP;
    this->send_evt(&evt);

    this->system_state->waiting_message_args.type = MessageType::profile_stopped;
    std::shared_ptr<UISystemState> state = system_state;
    this->system_state->waiting_message_args.waiting_function = [state](){ return state->profile_info.profile_state == ProfileState::stopping; };
    this->system_state->waiting_message_args.success_function = [state](){ return state->profile_info.profile_state == ProfileState::loaded; };
    this->next_scene = SceneEnum::waiting_message;
    this->should_be_changed = true;
}

SceneEnum StartProfileScene::get_scene_enum() {
    return SceneEnum::start_profile;
}

void StartProfileScene::button_callback(Button *button, PressType type) {
    ButtonType button_type = button->get_button_type();

    switch (button_type) {
        case ButtonType::L_BOT: {
            if (!display_err) {
                list.select();
            }
        }
        break;

        case ButtonType::L_MID: {
            if (!display_err) {
                list.move_down();
            }
        }
        break;

        case ButtonType::L_UP: {
            if (!display_err) {
                list.move_up();
            }
        }
        break;

        case ButtonType::R_UP:
        case ButtonType::R_MID:
        break;

        case ButtonType::R_BOT:  {
            next_scene = SceneEnum::menu;
            should_be_changed = true;
        }
        break;

        default:
        break;
    }
}

void StartProfileScene::update(float d_time) {
    if (display_err) {
        counter += d_time;
        if (counter >= ERROR_DISPLAY_DURATION) {
            this->next_scene = SceneEnum::menu;
            this->should_be_changed = true;
        }
        return;
    }

    if (previous_profile_state != system_state->profile_info.profile_state) {
        construct_option_list();
    }

    LCDController::clear_frame_buf();

    LCDController::draw_string(0, 0, system_state->profile_info.selected_profile);
    list.draw();

    LCDController::display_frame_buf();

    previous_profile_state = system_state->profile_info.profile_state;
}
