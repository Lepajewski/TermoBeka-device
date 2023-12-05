#include "status_bar.h"

#include "lcd_controller.h"

#define STATUS_BAR_CLOCK_WIDTH (FONT5X7_WIDTH+1)*5

void StatusBar::set_time(int hours, int minutes) {
    current_hours = hours;
    current_minutes = minutes;
}

void StatusBar::draw(float d_time) {
    if ((timer += d_time) >= STATUS_BAR_BLINK_TIME_SECONDS) {
        timer -= STATUS_BAR_BLINK_TIME_SECONDS;
        current_sep = current_sep == ':' ? ' ' : ':';
    }
    LCDController::draw_string_formatted(LCD_WIDTH - STATUS_BAR_CLOCK_WIDTH, 0, "%2d%c%02d", current_hours, current_sep, current_minutes);
}
