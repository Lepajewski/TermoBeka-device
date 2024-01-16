#include "select_profile_scene.h"

#include "lcd_controller.h"
#include "global_config.h"
#include "logger.h"

#include <vector>
#include <tuple>

#define TAG "StartProfileScene"
#define ERROR_DISPLAY_DURATION 2

void SelectProfileScene::setup_options_list(std::string &ls_response) {
    std::vector<OptionEntry> list;

    if (ls_response.size() == 0) {
        OptionEntry entry;
        entry.option_name = "No profiles";
        entry.select_callback = [this](){ this->back_button(); };

        list.push_back(entry);
    }
    else {
        size_t pos = std::string::npos;
        while ((pos = ls_response.find("\n")) != std::string::npos) {
            OptionEntry entry;

            size_t t_pos = ls_response.find("\t") + 1;
            std::string name = ls_response.substr(t_pos, pos - 2);
            entry.option_name = name;

            char type = ls_response[t_pos - 2];
            switch (type)
            {
                case 'D': {
                    entry.select_callback = [this, name]() {
                        this->folder_stack.push_back(name);
                        this->send_load_profiles();
                    };
                    entry.option_name = "/" + entry.option_name;
                    list.push_back(entry);
                }
                break;

                case 'F': {
                    size_t length = entry.option_name.size();
                    std::string extension = ".csv";
                    if (entry.option_name.substr(length - extension.size()).compare(extension) == 0) {
                        entry.select_callback = [this, name]() {
                            this->system_state->profile_info.profile_state = ProfileState::loading;
                            this->send_profile_chosen(name);

                            std::shared_ptr<UISystemState> state = system_state;
                            this->system_state->waiting_message_args.waiting_strs[0] = "Loading:";
                            this->system_state->waiting_message_args.waiting_strs[1] = name;
                            this->system_state->waiting_message_args.success_strs[0] = "Load success";
                            this->system_state->waiting_message_args.success_strs[1] = name;
                            this->system_state->waiting_message_args.fail_strs[0] = "Load fail";
                            this->system_state->waiting_message_args.fail_strs[1] = name;
                            this->system_state->waiting_message_args.waiting_function = [state](){ 
                                    bool is_loading_profile = state->profile_info.profile_state == ProfileState::loading;
                                    bool is_reading_sd = state->sd_info.status == SDStatus::loading_profile;
                                    bool read_sd_correctly = state->sd_info.status != SDStatus::failed;
                                    return is_reading_sd || (read_sd_correctly && is_loading_profile);
                                };
                            this->system_state->waiting_message_args.success_function = [state](){ 
                                    bool read_sd_correctly = state->sd_info.status != SDStatus::failed;
                                    bool loaded_profile_correctly = state->profile_info.profile_state == ProfileState::loaded;
                                    return read_sd_correctly && loaded_profile_correctly; 
                                };
                            this->next_scene = SceneEnum::waiting_message;
                            this->should_be_changed = true;
                        };

                        list.push_back(entry);
                    }
                }
                
                default:
                break;
            }

            ls_response.erase(0, pos + 1);
        }
    }

    option_list = std::make_unique<OptionList>(option_list_rect, list);
}

void SelectProfileScene::update_current_path() {
    std::string ret = PROFILE_FOLDER_PATH;
    for (std::string v : folder_stack) {
        ret += "/" + v;
    }
    current_path = ret;
    scrolling_text.set_text(ret);
}

void SelectProfileScene::back_button() {
    if (folder_stack.size() < 1) {
        this->next_scene = SceneEnum::menu;
        this->should_be_changed = true;
        return;
    }

    folder_stack.pop_back();
    send_load_profiles();
}

void SelectProfileScene::send_load_profiles() {
    profiles_loaded = false;

    Event evt = {};
    evt.type = EventType::UI_PROFILES_LOAD;

    update_current_path();
    SDEventPathArg arg = {};
    strncpy(arg.path, current_path.c_str(), SD_QUEUE_MAX_PAYLOAD);
    memcpy(evt.payload, arg.buffer, SD_QUEUE_MAX_PAYLOAD);

    send_evt(&evt);
}

void SelectProfileScene::send_profile_chosen(std::string filename) {
    Event evt = {};
    evt.type = EventType::UI_PROFILE_CHOSEN;

    update_current_path();
    SDEventPathArg arg = {};
    std::string filepath = current_path + "/" + filename;
    strncpy(arg.path, filepath.c_str(), SD_QUEUE_MAX_PAYLOAD);
    memcpy(evt.payload, arg.buffer, SD_QUEUE_MAX_PAYLOAD);

    send_evt(&evt);
}

SelectProfileScene::SelectProfileScene(std::shared_ptr<UISystemState> system_state) : Scene(system_state)
{
    err_profile_running = system_state->profile_info.profile_state == ProfileState::running  ||
                          system_state->profile_info.profile_state == ProfileState::starting ||
                          system_state->profile_info.profile_state == ProfileState::stopping;

    LCDController::clear_frame_buf();

    if (err_profile_running) {
        LCDController::draw_string(0, (LCD_HEIGHT / 2) - FONT5X7_LINE_HEIGHT, "Cant select");
        LCDController::draw_string(0, LCD_HEIGHT / 2                        , "when profile");
        LCDController::draw_string(0, (LCD_HEIGHT / 2) + FONT5X7_LINE_HEIGHT, "is running");

        LCDController::display_frame_buf();
    }
    else {
        send_load_profiles();
    }
}

SceneEnum SelectProfileScene::get_scene_enum() {
    return SceneEnum::select_profile;
}

void SelectProfileScene::button_callback(Button *button, PressType type) {
    ButtonType button_type = button->get_button_type();

    switch (button_type) {
        case ButtonType::L_BOT: {
            if (option_list != NULL) {
                this->option_list->select();
            }
        }
        break;

        case ButtonType::L_MID: {
            if (option_list != NULL) {
                this->option_list->move_down();
            }
        }
        break;

        case ButtonType::L_UP: {
            if (option_list != NULL) {
                this->option_list->move_up();
            }
        }
        break;

        case ButtonType::R_UP:
        case ButtonType::R_MID:
        break;

        case ButtonType::R_BOT: {
            if (profiles_loaded) {
                back_button();
            }
        }
        break;

        default:
        break;
    }
}

void SelectProfileScene::update(float d_time) {
    if (err_sd_fail && sd_fail_first_frame) {
        sd_fail_first_frame = false;
        counter = 0;
        
        LCDController::clear_frame_buf();

        LCDController::draw_string(0, LCD_HEIGHT / 2, "Error with SD");

        LCDController::display_frame_buf();
    }

    if (err_profile_running || err_sd_fail) {
        counter += d_time;
        if (counter >= ERROR_DISPLAY_DURATION) {
            this->next_scene = SceneEnum::menu;
            this->should_be_changed = true;
        }
        return;
    }

    LCDController::clear_frame_buf();

    if (option_list != NULL && profiles_loaded) {
        LCDController::draw_string(0, 0, "Dir:");
        scrolling_text.draw(d_time);
        option_list->draw();
    }
    else {
        LCDController::draw_string(0, (LCD_HEIGHT - FONT5X7_LINE_HEIGHT) / 2 - FONT5X7_LINE_HEIGHT, "loading");
        LCDController::draw_string(0, (LCD_HEIGHT + FONT5X7_LINE_HEIGHT) / 2 - FONT5X7_LINE_HEIGHT, "profiles");
    }

    LCDController::display_frame_buf();

    err_sd_fail = system_state->sd_info.status == SDStatus::failed;
}

void SelectProfileScene::process_ui_event(UIEvent *evt) {
    switch (evt->type) {
        case UIEventType::PROFILES_LOAD: {
            RingbufHandle_t *ring_buf = sys_manager->get_sd_ring_buf();

            size_t len;
            char *ls_profiles = (char*) xRingbufferReceive(*ring_buf, &len, pdMS_TO_TICKS(10));
            if (ls_profiles != NULL) {
                std::string ls_resp = std::string(ls_profiles, len);
                vRingbufferReturnItem(*ring_buf, (void*) ls_profiles);

                setup_options_list(ls_resp);
            }
            profiles_loaded = true;
        }
        break;

        default:
        break;
    }
}
