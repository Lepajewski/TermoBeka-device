#ifndef LIB_UI_MANAGER_START_PROFILE_SCENE_H_
#define LIB_UI_MANAGER_START_PROFILE_SCENE_H_

#include "scene.h"
#include "button.h"

#include <memory>

class StartProfileScene : public Scene {
private:
    const Rect list_rect = Rect(0, 8, LCD_WIDTH, LCD_HEIGHT - 8);

    OptionList list = OptionList(list_rect, {{"", [](){}}});

    bool display_err = false;
    float counter = 0;

    ProfileState previous_profile_state;

    void construct_option_list();

    void start_profile();
    void stop_profile();

public:
    StartProfileScene(std::shared_ptr<UISystemState> system_state);

    SceneEnum get_scene_enum() override;
    void button_callback(Button *button, PressType type) override;

    void update(float d_time) override;
};

#endif