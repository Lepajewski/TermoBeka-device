#ifndef LIB_UI_MANAGER_STARTUP_SCENE_H_
#define LIB_UI_MANAGER_STARTUP_SCENE_H_

#include "scene.h"
#include "button.h"

class StartupScene : public Scene {
public:
    StartupScene();

    SceneEnum get_scene_enum() override;
    void button_callback(Button *button, PressType type) override;
};

#endif