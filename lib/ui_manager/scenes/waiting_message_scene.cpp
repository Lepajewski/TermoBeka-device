#include "waiting_message_scene.h"

#include "lcd_controller.h"
#include "font5x7.h"

#define SCENE_DURATION 2

WaitingMessageScene::WaitingMessageScene(std::shared_ptr<UISystemState> system_state) : Scene(system_state) {
    switch (system_state->waiting_message_args.type) {
        case MessageType::profile_selected: {
            waiting_str[0] = "Loading:";
            waiting_str[1] = system_state->profile_info.selected_profile;
            success_str[0] = "Load success";
            success_str[1] = system_state->profile_info.selected_profile;
            fail_str[0] = "Load failed";
            fail_str[1] = system_state->profile_info.selected_profile;
            waiting_function = system_state->waiting_message_args.waiting_function;
            success_function = system_state->waiting_message_args.success_function;
        }
        break;

        case MessageType::profile_started: {
            waiting_str[0] = "Starting:";
            waiting_str[1] = system_state->profile_info.selected_profile;
            success_str[0] = "Start success";
            success_str[1] = system_state->profile_info.selected_profile;
            fail_str[0] = "Start failed";
            fail_str[1] = system_state->profile_info.selected_profile;
            waiting_function = system_state->waiting_message_args.waiting_function;
            success_function = system_state->waiting_message_args.success_function;
        }
        break;

        case MessageType::profile_stopped: {
            waiting_str[0] = "Stopping:";
            waiting_str[1] = system_state->profile_info.selected_profile;
            success_str[0] = "Stop success";
            success_str[1] = system_state->profile_info.selected_profile;
            fail_str[0] = "Stop failed";
            fail_str[1] = system_state->profile_info.selected_profile;
            waiting_function = system_state->waiting_message_args.waiting_function;
            success_function = system_state->waiting_message_args.success_function;
        }
        break;
    
        default:
        break;
    }
    system_state->waiting_message_args.type = MessageType::none;

    LCDController::clear_frame_buf();

    LCDController::draw_string(0, (LCD_HEIGHT - FONT5X7_LINE_HEIGHT) / 2 - FONT5X7_LINE_HEIGHT, waiting_str[0]);
    LCDController::draw_string(0, (LCD_HEIGHT + FONT5X7_LINE_HEIGHT) / 2 - FONT5X7_LINE_HEIGHT, waiting_str[1]);

    LCDController::display_frame_buf();
}

SceneEnum WaitingMessageScene::get_scene_enum() {
    return SceneEnum::waiting_message;
}

void WaitingMessageScene::button_callback(Button *button, PressType type) {

}

void WaitingMessageScene::update(float d_time) {
    if (waiting_function()) {
        return;
    }

    if (first_frame) {
        LCDController::clear_frame_buf();

        if (success_function()) {
            LCDController::draw_string(0, (LCD_HEIGHT - FONT5X7_LINE_HEIGHT) / 2 - FONT5X7_LINE_HEIGHT, success_str[0]);
            LCDController::draw_string(0, (LCD_HEIGHT + FONT5X7_LINE_HEIGHT) / 2 - FONT5X7_LINE_HEIGHT, success_str[1]);
        }
        else {
            LCDController::draw_string(0, (LCD_HEIGHT - FONT5X7_LINE_HEIGHT) / 2 - FONT5X7_LINE_HEIGHT, fail_str[0]);
            LCDController::draw_string(0, (LCD_HEIGHT + FONT5X7_LINE_HEIGHT) / 2 - FONT5X7_LINE_HEIGHT, fail_str[1]);
        }

        LCDController::display_frame_buf();

        first_frame = false;
    }

    counter += d_time;
    if (counter >= SCENE_DURATION) {
        this->next_scene = SceneEnum::menu;
        this->should_be_changed = true;
    }
}