#include "settings_scene.h"

void SettingsScene::mount_sd() {
    Event evt = {};
    evt.type = EventType::MOUNT_SD;

    system_state->sd_info.status = SDStatus::mounting;

    std::shared_ptr<UISystemState> state = system_state;
    system_state->waiting_message_args.waiting_function = [state](){
        return state->sd_info.status == SDStatus::mounting;
    };
    system_state->waiting_message_args.success_function = [state](){
        return state->sd_info.status != SDStatus::failed;
    };
    system_state->waiting_message_args.waiting_strs[0] = "Mounting SD";
    system_state->waiting_message_args.waiting_strs[1] = "";
    system_state->waiting_message_args.success_strs[0] = "Mounting SD";
    system_state->waiting_message_args.success_strs[1] = "Success";
    system_state->waiting_message_args.fail_strs[0] = "Mounting SD";
    system_state->waiting_message_args.fail_strs[1] = "Failed";

    this->send_evt(&evt);

    next_scene = SceneEnum::waiting_message;
    should_be_changed = true;
}

void SettingsScene::unmount_sd() {
    Event evt = {};
    evt.type = EventType::UNMOUNT_SD;

    system_state->sd_info.status = SDStatus::unmounting;

    std::shared_ptr<UISystemState> state = system_state;
    system_state->waiting_message_args.waiting_function = [state](){
        return state->sd_info.status == SDStatus::unmounting;
    };
    system_state->waiting_message_args.success_function = [](){
        return true;
    };
    system_state->waiting_message_args.waiting_strs[0] = "Unmounting SD";
    system_state->waiting_message_args.waiting_strs[1] = "";
    system_state->waiting_message_args.success_strs[0] = "Unmounted SD";
    system_state->waiting_message_args.success_strs[1] = "";

    this->send_evt(&evt);

    next_scene = SceneEnum::waiting_message;
    should_be_changed = true;
}

void SettingsScene::play_snek() {
    //t
}

SettingsScene::SettingsScene(std::shared_ptr<UISystemState> system_state) : Scene(system_state) {
}

SceneEnum SettingsScene::get_scene_enum() {
    return SceneEnum::settings;
}

void SettingsScene::button_callback(Button *button, PressType type) {
    ButtonType button_type = button->get_button_type();

    switch (button_type) {
        case ButtonType::L_BOT: {
            list.select();
        }
        break;

        case ButtonType::L_MID: {
            list.move_down();
        }
        break;

        case ButtonType::L_UP: {
            list.move_up();
        }
        break;

        case ButtonType::R_UP:
        case ButtonType::R_MID:
        break;

        case ButtonType::R_BOT: {
            next_scene = SceneEnum::menu;
            should_be_changed = true;
        }
        break;

        default:
        break;
    }
}

void SettingsScene::update(float d_time) {
    LCDController::clear_frame_buf();

    LCDController::draw_string(0, 0, "Settings");
    list.draw();

    LCDController::display_frame_buf();
}
