#ifndef LIB_UI_MANAGER_MENU_SCENE_H_
#define LIB_UI_MANAGER_MENU_SCENE_H_

#include "components/ui_components.h"
#include "scene.h"
#include "button.h"

class MenuScene : public Scene {
private:
    OptionList list = OptionList(Rect(0, 8, LCD_WIDTH, LCD_HEIGHT - 8), {
        {"Intro", [this](){this->next_scene = SceneEnum::startup; this->should_be_changed = true;}},
        {"Start profile", [](){}},
        {"Settings", [this](){/* this->next_scene = SceneEnum::settings; this->should_be_changed = true; */}},
        {"Option 4", [](){}},
        {"Option 5", [](){}},
        {"Option 6", [](){}},
        {"Option 7", [](){}},
        {"Option 8", [](){}}
    });

public:
    MenuScene(LCDController* lcd);

    SceneEnum get_scene_enum() override;
    void button_callback(Button *button, PressType type) override;
};

#endif