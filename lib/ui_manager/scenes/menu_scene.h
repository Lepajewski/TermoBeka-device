#ifndef LIB_UI_MANAGER_MENU_SCENE_H_
#define LIB_UI_MANAGER_MENU_SCENE_H_

#include "scene.h"
#include "button.h"

class MenuScene : public Scene {
public:
    MenuScene(LCDController* lcd);

    SceneEnum get_scene_enum() override;
    void button_callback(Button *button, PressType type) override;
};

#endif