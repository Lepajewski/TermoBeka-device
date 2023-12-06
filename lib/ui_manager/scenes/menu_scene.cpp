#include "menu_scene.h"

#include "lcd_controller.h"
#include "global_config.h"

#include "logger.h"

MenuScene::MenuScene() : Scene() {
    LCDController::clear_frame_buf();
}

SceneEnum MenuScene::get_scene_enum() {
    return SceneEnum::menu;
}

void MenuScene::button_callback(Button *button, PressType type) {
    pca9539_pin_num pin_num = button->get_pin_num();

    switch (type) {
        case PressType::SHORT_PRESS: {
            switch (pin_num) {
                case P0_0: {
                    this->list.select();
                }
                break;

                case P0_1: {
                    this->list.move_down();
                }
                break;

                case P0_2: {
                    this->list.move_up();
                }
                break;

                case P0_3:
                case P0_4:
                case P0_5:
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

void MenuScene::update(float d_time) {
    LCDController::clear_frame_buf();

    char timestamp[TIMESTAMP_SIZE];
    get_timestamp(timestamp);
    std::string time = std::string(timestamp);
    time = time.substr(time.find('T') + 1, 5);
    status.set_time(std::stoi(time.substr(0, 2)), std::stoi(time.substr(3, 2)));
    
    this->status.draw(d_time);
    this->list.draw();
    LCDController::display_frame_buf();
}
