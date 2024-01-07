#include "profile_selected_scene.h"

#include "lcd_controller.h"
#include "font5x7.h"

#define PROFILE_SELECTED_SCENE_DURATION 2

ProfileSelectedScene::ProfileSelectedScene(std::shared_ptr<UISystemState> system_state) : Scene(system_state) {
    LCDController::clear_frame_buf();

    LCDController::draw_string(0, (LCD_HEIGHT - FONT5X7_LINE_HEIGHT) / 2 - FONT5X7_LINE_HEIGHT, "Loading:");
    LCDController::draw_string(0, (LCD_HEIGHT + FONT5X7_LINE_HEIGHT) / 2 - FONT5X7_LINE_HEIGHT, system_state->selected_profile);

    LCDController::display_frame_buf();
}

SceneEnum ProfileSelectedScene::get_scene_enum() {
    return SceneEnum::profile_selected;
}

void ProfileSelectedScene::button_callback(Button *button, PressType type) {

}

void ProfileSelectedScene::update(float d_time) {
    if (system_state->profile_state == ProfileState::loading) {
        return;
    }

    if (first_frame) {
        LCDController::clear_frame_buf();

        if (system_state->profile_state == ProfileState::loaded) {
            LCDController::draw_string(0, (LCD_HEIGHT - FONT5X7_LINE_HEIGHT) / 2 - FONT5X7_LINE_HEIGHT, "Load success");
            LCDController::draw_string(0, (LCD_HEIGHT + FONT5X7_LINE_HEIGHT) / 2 - FONT5X7_LINE_HEIGHT, system_state->selected_profile);
        }
        else {
            LCDController::draw_string(0, (LCD_HEIGHT - FONT5X7_LINE_HEIGHT) / 2 - FONT5X7_LINE_HEIGHT, "Load failed");
            LCDController::draw_string(0, (LCD_HEIGHT + FONT5X7_LINE_HEIGHT) / 2 - FONT5X7_LINE_HEIGHT, system_state->selected_profile);
        }

        LCDController::display_frame_buf();

        first_frame = false;
    }

    counter += d_time;
    if (counter >= PROFILE_SELECTED_SCENE_DURATION) {
        this->next_scene = SceneEnum::menu;
        this->should_be_changed = true;
    }
}