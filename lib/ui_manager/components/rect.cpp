#include "rect.h"

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
