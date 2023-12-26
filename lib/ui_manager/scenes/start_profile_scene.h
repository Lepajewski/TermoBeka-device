#ifndef LIB_UI_MANAGER_START_PROFILE_SCENE_H_
#define LIB_UI_MANAGER_START_PROFILE_SCENE_H_

#include <string>
#include <memory>
#include <vector>

#include "components/ui_components.h"
#include "scene.h"
#include "button.h"

class StartProfileScene : public Scene {
private:
    std::unique_ptr<OptionList> option_list;
    std::vector<std::string> folder_stack;
    std::string current_path;

    void setup_options_list(std::string &ls_response);
    void update_current_path();
    void back_button();
    void send_load_profiles();

    bool profiles_loaded = false;
public:
    StartProfileScene();

    SceneEnum get_scene_enum() override;
    void button_callback(Button *button, PressType type) override;

    void update(float d_time) override;
    void process_ui_event(UIEvent *evt) override;
};

#endif