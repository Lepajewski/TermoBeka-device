#ifndef LIB_UI_MANAGER_SETTINGS_SCENE_H_
#define LIB_UI_MANAGER_SETTINGS_SCENE_H_

#include "scene.h"
#include "button.h"
#include "components/ui_components.h"
#include "font5x7.h"
#include "lcd_controller.h"

#include <memory>

class SettingsScene : public Scene {
private:
    OptionList list = OptionList(Rect(0, FONT5X7_LINE_HEIGHT, LCD_WIDTH, LCD_HEIGHT - FONT5X7_LINE_HEIGHT), {
        {"Mount SD", [this](){ this->mount_sd(); }},
        {"Unmount SD", [this](){ this->unmount_sd(); }}//,
        //{"Snejk", [this](){ this->play_snek(); }}
    });

    void mount_sd();
    void unmount_sd();
    void play_snek();

public:
    SettingsScene(std::shared_ptr<UISystemState> system_state);

    SceneEnum get_scene_enum() override;
    void button_callback(Button *button, PressType type) override;

    void update(float d_time) override;
};

#endif