#include "startup_scene.h"

#include "icons/icons.h"
#include "lcd_controller.h"

StartupScene::StartupScene() : Scene() {
    LCDController::clear_frame_buf();
    LCDController::draw_bitmap(0, 0, logo_width, logo_height, logo_bitmap);
    LCDController::display_frame_buf();
}

SceneEnum StartupScene::get_scene_enum() {
    return SceneEnum::startup;
}

void StartupScene::button_callback(Button *button, PressType type) {
    switch (button->get_button_type()) {
        case ButtonType::L_BOT:
        case ButtonType::L_MID:
        case ButtonType::L_UP:
        case ButtonType::R_BOT:
        case ButtonType::R_MID:
        case ButtonType::R_UP: {
            this->should_be_changed = true;
            this->next_scene = SceneEnum::menu;
        }
        break;

        default:
        break;
    }
}
