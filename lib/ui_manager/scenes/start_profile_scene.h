#ifndef LIB_UI_MANAGER_START_PROFILE_SCENE_H_
#define LIB_UI_MANAGER_START_PROFILE_SCENE_H_

#include "scene.h"
#include "button.h"
#include "components/ui_components.h"

#include <memory>

#define START_PROFILE_SCENE_SCROLL_SPEED 2.5f

class StartProfileScene : public Scene {
private:
    const Rect scrolling_text_rect = Rect(0, 0, LCD_WIDTH, FONT5X7_LINE_HEIGHT);
    const Rect list_rect = Rect(0, 8, LCD_WIDTH, LCD_HEIGHT - 16);

    ScrollingText text = ScrollingText(scrolling_text_rect, START_PROFILE_SCENE_SCROLL_SPEED, false);
    ProfileBar profile_bar;

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