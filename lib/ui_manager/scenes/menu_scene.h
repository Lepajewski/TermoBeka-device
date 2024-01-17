#ifndef LIB_UI_MANAGER_MENU_SCENE_H_
#define LIB_UI_MANAGER_MENU_SCENE_H_

#include "components/ui_components.h"
#include "scene.h"
#include "button.h"

class MenuScene : public Scene {
private:
    StatusBar status;
    ProfileBar profile_bar;

    OptionList list = OptionList(Rect(0, 8, LCD_WIDTH, LCD_HEIGHT - 16), {
        {"Select profile", [this](){this->next_scene = SceneEnum::select_profile; this->should_be_changed = true;}},
        {"Control profile", [this](){this->next_scene = SceneEnum::start_profile; this->should_be_changed = true;}},
        {"Settings", [this](){this->next_scene = SceneEnum::settings; this->should_be_changed = true;}},
    });

public:
    MenuScene(std::shared_ptr<UISystemState> system_state);

    SceneEnum get_scene_enum() override;
    void button_callback(Button *button, PressType type) override;

    void update(float d_time) override;
};

#endif