#include "startup_scene.h"

StartupScene::StartupScene(LCDController* lcd) : Scene(lcd) {
    this->lcd->draw_logo();
}

SceneEnum StartupScene::get_scene_enum() {
    return SceneEnum::startup;
}

void StartupScene::button_callback(Button *button, PressType type) {
    switch (button->get_pin_num()) {
        case P0_0:
        case P0_1:
        case P0_2:
        case P0_3:
        case P0_4:
        case P0_5: {
            this->should_be_changed = true;
            this->next_scene = SceneEnum::menu;
        }
        break;
        default:
        break;
    }
}
