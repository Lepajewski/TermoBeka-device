#ifndef LIB_UI_MANAGER_START_PROFILE_SCENE_H_
#define LIB_UI_MANAGER_START_PROFILE_SCENE_H_

#include "components/ui_components.h"
#include "scene.h"
#include "button.h"

class StartProfileScene : public Scene {
private:

public:
    StartProfileScene();

    SceneEnum get_scene_enum() override;
    void button_callback(Button *button, PressType type) override;

    void update(float d_time) override;

};

#endif