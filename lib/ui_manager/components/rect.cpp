#include "rect.h"

#include "lcd_controller.h"

Rect::Rect() {
    this->x = 0;
    this->y = 0;
    this->width = 0;
    this->height = 0;
}

Rect::Rect(int x, int y) {
    this->x = x;
    this->y = y;
    this->width = 0;
    this->height = 0;
}

Rect::Rect(int x, int y, int width, int height) {
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
}

void draw_rect(Rect &rect, bool filled) {
    if (rect.height <= 0 || rect.width <= 0) {
        return;
    }

    int x_start = rect.x;
    int x_end = rect.x + rect.width;
    int y_start = rect.y;
    int y_end = rect.y + rect.height;

    for (int x = x_start; x < x_end; x++) {
        for (int y = y_start; y < y_end; y++) {
            if (!filled &&
                (x > x_start && x < x_end - 1) &&
                (y > y_start && y < y_end - 1)) {
                continue;
            }
            LCDController::set_pixel(x, y);
        }
    }
}
