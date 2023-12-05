#include "scene.h"

Scene::Scene() {
    this->should_be_changed = false;
    this->next_scene = SceneEnum::none;
}

bool Scene::get_should_be_changed()
{
    return this->should_be_changed;
}

SceneEnum Scene::get_next_scene()
{
    return this->next_scene;
}

std::unique_ptr<Scene> Scene::create_scene(SceneEnum target)
{
    std::unique_ptr<Scene> result;
    switch (target) {
        case SceneEnum::startup: {
            result = std::make_unique<StartupScene>();
        }
        break;
        case SceneEnum::menu: {
            result = std::make_unique<MenuScene>();
        }
        break;
        case SceneEnum::settings: {

        }
        break;
        default:
        break;
    }
    return result;
}
