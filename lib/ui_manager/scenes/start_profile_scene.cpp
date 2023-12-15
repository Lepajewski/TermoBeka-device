#include "start_profile_scene.h"

#include "lcd_controller.h"
#include "logger.h"

#include <vector>
#include <tuple>

#define TAG "StartProfileScene"

void StartProfileScene::setup_options_list(std::string &ls_response) {
    Rect option_list_rect(0, 0, LCD_WIDTH, LCD_HEIGHT);
    std::vector<OptionEntry> list;

    if (ls_response.size() == 0) {
        OptionEntry entry;
        entry.option_name = "No profiles";
        entry.select_callback = [this](){this->next_scene = SceneEnum::menu; this->should_be_changed = true; };

        list.push_back(entry);
    }
    else {
        size_t pos = std::string::npos;
        while ((pos = ls_response.find("\n")) != std::string::npos) {
            OptionEntry entry;

            size_t t_pos = ls_response.find("\t") + 1;
            entry.option_name = ls_response.substr(t_pos, pos - 1);

            entry.select_callback = [](){};

            list.push_back(entry);
            ls_response.erase(0, pos + 1);
        }
    }

    option_list = std::make_unique<OptionList>(option_list_rect, list);
}

StartProfileScene::StartProfileScene() : Scene()
{
    LCDController::clear_frame_buf();

    profiles_loaded = false;

    Event evt;
    evt.type = EventType::UI_PROFILES_LOAD;
    send_evt(&evt);
}

SceneEnum StartProfileScene::get_scene_enum() {
    return SceneEnum::start_profile;
}

void StartProfileScene::button_callback(Button *button, PressType type) {
    pca9539_pin_num pin_num = button->get_pin_num();

    switch (type) {
        case PressType::SHORT_PRESS: {
            switch (pin_num) {
                case P0_0: {
                    if (option_list != NULL) {
                        this->option_list->select();
                    }
                }
                break;

                case P0_1: {
                    if (option_list != NULL) {
                        this->option_list->move_down();
                    }
                }
                break;

                case P0_2: {
                    if (option_list != NULL) {
                        this->option_list->move_up();
                    }
                }
                break;

                case P0_3:
                case P0_4:
                break;

                case P0_5:{
                    if (profiles_loaded) {
                        next_scene = SceneEnum::menu;
                        should_be_changed = true;
                    }
                }
                break;

                default:
                break;
            }
        }
        break;
        case PressType::LONG_PRESS: {
            switch (pin_num) {
                case P0_0:
                case P0_1:
                case P0_2:
                case P0_3:
                case P0_4:
                case P0_5:
                break;

                default:
                break;
            }
        }
        default:
        break;
    }
}

void StartProfileScene::update(float d_time) {
    LCDController::clear_frame_buf();

    if (option_list != NULL) {
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
