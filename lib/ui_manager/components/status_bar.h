#ifndef LIB_UI_MANAGER_STATUS_BAR_H_
#define LIB_UI_MANAGER_STATUS_BAR_H_

#include "font5x7.h"

#define STATUS_BAR_HEIGHT FONT5X7_HEIGHT+1
#define STATUS_BAR_BLINK_TIME_SECONDS 1

class StatusBar {
private:
    int current_hours = 0;
    int current_minutes = 0;

    float timer = 0;
    char current_sep = ':';
public:
    void set_time(int hours, int minutes);

    void draw(float d_time);
};

#endif