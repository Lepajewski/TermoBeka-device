#include "start_profile_scene.h"

#include "lcd_controller.h"

StartProfileScene::StartProfileScene() : Scene() {
    LCDController::clear_frame_buf();
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
                }
                break;

                case P0_1: {
                }
                break;

                case P0_2: {
                }
                break;

                case P0_3: {
                    next_scene = SceneEnum::menu;
                    should_be_changed = true;
                }
                break;

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

void StartProfileScene::update(float d_time) {
    LCDController::clear_frame_buf();

    LCDController::display_frame_buf();
}
