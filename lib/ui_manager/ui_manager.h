#ifndef LIB_UI_MANAGER_UI_MANAGER_H_
#define LIB_UI_MANAGER_UI_MANAGER_H_


#include "tb_event.h"
#include "gpio_expander.h"
#include "button.h"
#include "system_manager.h"
#include "scenes/scene.h"
#include "ui_system_state.h"

#include <memory>

class GPIOExpander;

class UIManager {
 private:
   SystemManager *sysMgr;
   QueueHandle_t *event_queue_handle;
   QueueHandle_t *ui_queue_handle;
   std::unique_ptr<GPIOExpander> expander;

   std::unique_ptr<Scene> current_scene;
   std::shared_ptr<UISystemState> state;

   void button_callback(Button* button, PressType type);
   void process_ui_event(UIEvent *evt);
   void poll_ui_events();

   void process_profile_response(profile_event_response response);
   void process_new_profile_info(EventNewProfileInfo *payload);
   void process_regulator_update(RegulatorStatusUpdate &info);
   void process_sd_response(SDResponse response);

   void send_evt(Event *evt);
   void send_evt_button_press(pca9539_pin_num pin_num, PressType type);

   void switch_scene(SceneEnum target);
   void check_scene_transition();
 public:
   UIManager();

   void setup();
   void process_events();
   void update(float d_time);
};

#endif
