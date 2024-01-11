#include "profile_bar.h"

#define MAX_TEMPERATURE_LENGTH FONT5X7_CHAR_WIDTH*5

void ProfileBar::set_remaining_time(uint32_t profile_time, uint32_t profile_duration) {
    uint32_t remainder_ms = profile_duration - profile_time;
    this->remaining_hours = (remainder_ms / 3600000);
    this->remaining_minutes = (remainder_ms / 60000) % 60;
    this->remaining_seconds = (remainder_ms / 1000) % 60;
    this->percent_done = static_cast<float>(profile_time) / static_cast<float>(profile_duration);
}

void ProfileBar::set_temperature(int16_t temperature) {
    this->temperature = temperature / 100;
}

void ProfileBar::draw_percent_bar() {
    Rect boundary_rect = Rect(rect.x, rect.y, rect.width - MAX_TEMPERATURE_LENGTH - 1, rect.height);
    draw_rect(boundary_rect);
    boundary_rect.width *= percent_done;
    draw_rect(boundary_rect, true);
}

void ProfileBar::draw_time_remaining() {
    LCDController::draw_string_formatted(rect.x + (FONT5X7_CHAR_WIDTH / 2), rect.y, "%2d:%02d:%02d", remaining_hours, remaining_minutes, remaining_seconds);
}

void ProfileBar::draw(float d_time) {
    if ((counter += d_time) >= PROFILE_BAR_PAGE_DURATION) {
        int page_int = static_cast<int>(current_page) + 1;
        page_int = page_int % static_cast<int>(Page::enum_count);
        current_page = static_cast<Page>(page_int);
        counter = 0;
    }

    LCDController::draw_string_formatted(rect.width - MAX_TEMPERATURE_LENGTH, rect.y, "%3d*C", temperature);

    switch (current_page) {
        case Page::percent_bar: {
            draw_percent_bar();
        }
        break;

        case Page::time_remaining: {
            draw_time_remaining();
        }
        break;
    
        default:
        break;
    }
}