#ifndef LIB_UI_MANAGER_UI_MANAGER_H_
#define LIB_UI_MANAGER_UI_MANAGER_H_


#include "tb_event.h"
#include "gpio_expander.h"
#include "button.h"
#include "buzzer.h"
#include "system_manager.h"
#include "scenes/scene.h"

#include <memory>

class GPIOExpander;

class UIManager {
 private:
   SystemManager *sysMgr;
   QueueHandle_t *event_queue_handle;
   QueueHandle_t *ui_queue_handle;
   GPIOExpander *expander;
   Buzzer buzzer;

   std::unique_ptr<Scene> current_scene;

   void button_callback(Button* button, PressType type);
   void process_ui_event(UIEvent *evt);
   void poll_ui_events();

   void switch_scene(SceneEnum target);
   void check_scene_transition();

    void send_evt(Event *evt);
    void send_evt_button_press(pca9539_pin_num pin_num, PressType type);
 public:
   UIManager();
   ~UIManager();

   void setup();
   void process_events();
   void update(float d_time);
};

#endif
