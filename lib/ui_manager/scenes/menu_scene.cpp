#include "menu_scene.h"

#include "lcd_controller.h"

MenuScene::MenuScene() : Scene() {
    LCDController::clear_frame_buf();
    this->list.draw();
    LCDController::display_frame_buf();
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
                    LCDController::clear_frame_buf();
                    LCDController::draw_string_formatted(0, 0, "B%d, SHORT", static_cast<int>(pin_num));
                    this->list.draw();
                    LCDController::display_frame_buf();
                }
                break;

                case P0_1: {
                    this->list.move_down();
                    LCDController::clear_frame_buf();
                    LCDController::draw_string_formatted(0, 0, "B%d, SHORT", static_cast<int>(pin_num));
                    this->list.draw();
                    LCDController::display_frame_buf();
                }
                break;

                case P0_2: {
                    this->list.move_up();
                    LCDController::clear_frame_buf();
                    LCDController::draw_string_formatted(0, 0, "B%d, SHORT", static_cast<int>(pin_num));
                    this->list.draw();
                    LCDController::display_frame_buf();
                }
                break;

                case P0_3:
                case P0_4:
                case P0_5: {
                    LCDController::clear_frame_buf();
                    LCDController::draw_string_formatted(0, 0, "B%d, SHORT", static_cast<int>(pin_num));
                    this->list.draw();
                    LCDController::display_frame_buf();
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
                case P0_5: {
                    LCDController::clear_frame_buf();
                    LCDController::draw_string_formatted(0, 0, "B%d, LONG", static_cast<int>(pin_num));
                    this->list.draw();
                    LCDController::display_frame_buf();
                }
                break;
                default:
                break;
            }
        }
        default:
        break;
    }
}
