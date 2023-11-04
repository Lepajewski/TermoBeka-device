#ifndef LIB_UI_MANAGER_UI_MANAGER_H_
#define LIB_UI_MANAGER_UI_MANAGER_H_


#include "gpio_expander.h"
#include "button.h"
#include "buzzer.h"


class GPIOExpander;


class UIManager {
 private:
    GPIOExpander *expander;
    Buzzer buzzer;


    void button_callback(Button* button, PressType type);
 public:
    UIManager();
    ~UIManager();

    void setup();
    void process_events();
};

#endif
