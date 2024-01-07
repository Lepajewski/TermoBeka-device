#ifndef LIB_UI_MANAGER_SELECT_PROFILE_SCENE_H_
#define LIB_UI_MANAGER_SELECT_PROFILE_SCENE_H_

#include <string>
#include <memory>
#include <vector>

#include "components/ui_components.h"
#include "scene.h"
#include "button.h"
#include "font5x7.h"

#define START_PROFILE_SCENE_SCROLL_SPEED 2.5f

class SelectProfileScene : public Scene {
private:
    const Rect scrolling_text_rect = Rect(FONT5X7_WIDTH * 5, 0, LCD_WIDTH, FONT5X7_LINE_HEIGHT);
    const Rect option_list_rect = Rect(0, FONT5X7_LINE_HEIGHT, LCD_WIDTH, LCD_HEIGHT - FONT5X7_LINE_HEIGHT);

    ScrollingText scrolling_text = ScrollingText(scrolling_text_rect, START_PROFILE_SCENE_SCROLL_SPEED);
    std::unique_ptr<OptionList> option_list;
    std::vector<std::string> folder_stack;
    std::string current_path;

    void setup_options_list(std::string &ls_response);
    void update_current_path();
    void back_button();
    void send_load_profiles();
    void send_profile_chosen(std::string filename);

    bool profiles_loaded = false;
public:
    SelectProfileScene(std::shared_ptr<UISystemState> system_state);

    SceneEnum get_scene_enum() override;
    void button_callback(Button *button, PressType type) override;

    void update(float d_time) override;
    void process_ui_event(UIEvent *evt) override;
};

#endif