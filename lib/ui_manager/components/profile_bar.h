#ifndef LIB_UI_MANAGER_PROFILE_BAR_H_
#define LIB_UI_MANAGER_PROFILE_BAR_H_

#include <inttypes.h>

#include "font5x7.h"
#include "rect.h"
#include "lcd_controller.h"

#define PROFILE_BAR_PAGE_DURATION 5

class ProfileBar {
private:
    enum class Page { percent_bar, time_remaining, enum_count };
    
    const Rect rect = Rect(0, LCD_HEIGHT - FONT5X7_LINE_HEIGHT, LCD_WIDTH, FONT5X7_LINE_HEIGHT);

    float counter = 0;
    int temperature = 0;
    float percent_done = 0;

    int remaining_hours = 0;
    int remaining_minutes = 0;
    int remaining_seconds = 0;

    Page current_page = Page::percent_bar;

    void draw_percent_bar();
    void draw_time_remaining();

public:
    void set_remaining_time(uint32_t profile_time, uint32_t profile_duration);
    void set_temperature(int16_t temperature);

    void draw(float d_time);

};

#endif