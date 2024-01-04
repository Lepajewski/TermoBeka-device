#include "scene.h"

#include "logger.h"

#define TAG "Scene"

void Scene::send_evt(Event *evt) {
    evt->origin = EventOrigin::UI;
    if (xQueueSend(*sys_manager->get_event_queue(), &*evt, portMAX_DELAY) != pdTRUE) {
        TB_LOGE(TAG, "event send fail");
    }
}

Scene::Scene(std::shared_ptr<UISystemState> system_state)
{
    this->system_state = system_state;
    this->should_be_changed = false;
    this->next_scene = SceneEnum::none;
    this->sys_manager = get_system_manager();
}

bool Scene::get_should_be_changed()
{
    return this->should_be_changed;
}

SceneEnum Scene::get_next_scene()
{
    return this->next_scene;
}

std::unique_ptr<Scene> Scene::create_scene(SceneEnum target, std::shared_ptr<UISystemState> system_state)
{
    std::unique_ptr<Scene> result;
    switch (target) {
        case SceneEnum::startup: {
            result = std::make_unique<StartupScene>(system_state);
        }
        break;
        case SceneEnum::menu: {
            result = std::make_unique<MenuScene>(system_state);
        }
        break;
        case SceneEnum::settings: {

        }
        break;
        case SceneEnum::start_profile: {
            result = std::make_unique<StartProfileScene>(system_state);
        }
        break;
        default:
        break;
    }
    return result;
}
