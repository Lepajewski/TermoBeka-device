#ifndef LIB_UI_MANAGER_SCENE_H_
#define LIB_UI_MANAGER_SCENE_H_

#include "button.h"

#include <memory>

enum class SceneEnum { startup, menu, settings, none };

class Scene {
protected:
    bool should_be_changed;
    SceneEnum next_scene;

public:
    Scene();

    bool get_should_be_changed();
    SceneEnum get_next_scene();

    virtual SceneEnum get_scene_enum() = 0;
    virtual void button_callback(Button *button, PressType type) = 0;

    virtual void update(float d_time) {}

    static std::unique_ptr<Scene> create_scene(SceneEnum target);
};

#include "startup_scene.h"
#include "menu_scene.h"

#endif