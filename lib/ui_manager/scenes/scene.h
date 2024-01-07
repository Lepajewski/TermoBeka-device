#ifndef LIB_UI_MANAGER_SCENE_H_
#define LIB_UI_MANAGER_SCENE_H_

#include "button.h"

#include <memory>

#include "tb_event.h"
#include "system_manager.h"
#include "ui_system_state.h"

enum class SceneEnum { startup, menu, profile_selected, select_profile, start_profile, none };

class Scene {
protected:
    SystemManager *sys_manager;
    std::shared_ptr<UISystemState> system_state;

    bool should_be_changed;
    SceneEnum next_scene;

    void send_evt(Event *evt);
    
public:
    Scene(std::shared_ptr<UISystemState> system_state);

    bool get_should_be_changed();
    SceneEnum get_next_scene();

    virtual SceneEnum get_scene_enum() = 0;
    virtual void button_callback(Button *button, PressType type) = 0;

    virtual void update(float d_time) {}
    virtual void process_ui_event(UIEvent *evt) {}

    static std::unique_ptr<Scene> create_scene(SceneEnum target, std::shared_ptr<UISystemState> system_state);
};

#include "startup_scene.h"
#include "menu_scene.h"
#include "select_profile_scene.h"
#include "profile_selected_scene.h"
#include "start_profile_scene.h"

#endif