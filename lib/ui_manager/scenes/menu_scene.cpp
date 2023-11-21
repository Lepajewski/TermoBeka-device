#include "menu_scene.h"

MenuScene::MenuScene(LCDController *lcd) : Scene(lcd) {
    this->lcd->clear();
}

SceneEnum MenuScene::get_scene_enum() {
    return SceneEnum::menu;
}

void MenuScene::button_callback(Button *button, PressType type) {
    pca9539_pin_num pin_num = button->get_pin_num();

    switch (type) {
        case PressType::SHORT_PRESS: {
            switch (pin_num) {
                case P0_0:
                case P0_1:
                case P0_2:
                case P0_3:
                case P0_4:
                case P0_5: {
                    this->lcd->clear();
                    this->lcd->print_formatted("B%d, SHORT", (int) pin_num);
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
                    this->lcd->clear();
                    this->lcd->print_formatted("B%d, LONG", (int) pin_num);
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
