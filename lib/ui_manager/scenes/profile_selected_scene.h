#ifndef LIB_UI_MANAGER_PROFILE_SELECTED_SCENE_H_
#define LIB_UI_MANAGER_PROFILE_SELECTED_SCENE_H_

#include "scene.h"

class ProfileSelectedScene : public Scene {
private:
    float counter = 0;
public:
    ProfileSelectedScene(std::shared_ptr<UISystemState> system_state);

    SceneEnum get_scene_enum() override;
    void button_callback(Button *button, PressType type) override;

    void update(float d_time) override;
};

#endif