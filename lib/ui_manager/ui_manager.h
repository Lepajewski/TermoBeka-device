#ifndef LIB_UI_MANAGER_UI_MANAGER_H_
#define LIB_UI_MANAGER_UI_MANAGER_H_


#include "tb_event.h"
#include "gpio_expander.h"
#include "button.h"
#include "buzzer.h"
#include "lcd_controller.h"
#include "system_manager.h"


class GPIOExpander;


class UIManager {
 private:
    SystemManager *sysMgr;
    QueueHandle_t *event_queue_handle;
    QueueHandle_t *ui_queue_handle;
    GPIOExpander *expander;
    Buzzer buzzer;
    LCDController lcd;


    void button_callback(Button* button, PressType type);
    void process_ui_event(UIEvent *evt);
    void poll_ui_events();
 public:
    UIManager();
    ~UIManager();

    void setup();
    void process_events();
};

#endif
