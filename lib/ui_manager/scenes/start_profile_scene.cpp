#include "start_profile_scene.h"

#include "lcd_controller.h"
#include "global_config.h"
#include "logger.h"

#include <vector>
#include <tuple>

#define TAG "StartProfileScene"

void StartProfileScene::setup_options_list(std::string &ls_response) {
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
                }
                break;
                
                default: {
                    entry.select_callback = [](){};
                }
                break;
            }

            list.push_back(entry);
            ls_response.erase(0, pos + 1);
        }
    }

    option_list = std::make_unique<OptionList>(option_list_rect, list);
}

void StartProfileScene::update_current_path() {
    std::string ret = PROFILE_FOLDER_PATH;
    for (std::string v : folder_stack) {
        ret += "/" + v;
    }
    current_path = ret;
    scrolling_text.set_text(ret);
}

void StartProfileScene::back_button() {
    if (folder_stack.size() < 1) {
        this->next_scene = SceneEnum::menu;
        this->should_be_changed = true;
        return;
    }

    folder_stack.pop_back();
    send_load_profiles();
}

void StartProfileScene::send_load_profiles() {
    profiles_loaded = false;

    Event evt;
    evt.type = EventType::UI_PROFILES_LOAD;

    update_current_path();
    SDEventPathArg arg;
    strncpy(arg.path, current_path.c_str(), SD_QUEUE_MAX_PAYLOAD);
    memcpy(evt.payload, arg.buffer, SD_QUEUE_MAX_PAYLOAD);

    send_evt(&evt);
}

StartProfileScene::StartProfileScene() : Scene()
{
    LCDController::clear_frame_buf();

    send_load_profiles();
}

SceneEnum StartProfileScene::get_scene_enum() {
    return SceneEnum::start_profile;
}

void StartProfileScene::button_callback(Button *button, PressType type) {
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

void StartProfileScene::update(float d_time) {
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
}

void StartProfileScene::process_ui_event(UIEvent *evt) {
    if (evt->type == UIEventType::PROFILES_LOAD) {
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
}
