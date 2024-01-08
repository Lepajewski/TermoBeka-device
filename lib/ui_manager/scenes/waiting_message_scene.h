#ifndef LIB_UI_MANAGER_WAITING_MESSAGE_SCENE_H_
#define LIB_UI_MANAGER_WAITING_MESSAGE_SCENE_H_

#include "scene.h"
#include "button.h"

#include <string>
#include <functional>

class WaitingMessageScene : public Scene {
private:
    float counter = 0;
    bool first_frame = true;

    std::string waiting_str[2];
    std::string success_str[2];
    std::string fail_str[2];

    std::function<bool(void)> waiting_function = [](){ return true; };
    std::function<bool(void)> success_function = [](){ return true; };

public:
    WaitingMessageScene(std::shared_ptr<UISystemState> system_state);

    SceneEnum get_scene_enum() override;
    void button_callback(Button *button, PressType type) override;

    void update(float d_time) override;
};

#endif