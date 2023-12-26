#include "scrolling_text.h"

#include "lcd_controller.h"

ScrollingText::ScrollingText(Rect rect, float speed) {
    this->rect = rect;
    this->speed = speed;

    this->length = rect.width / FONT5X7_WIDTH;
}

void ScrollingText::set_text(std::string &text) {
    text_size = text.size();
    offset = 0;
    counter = 0;

    this->text = text;
    for (int i = 0; text_size * i < length; i++) {
        this->text += sep + text;
    }
}

void ScrollingText::draw(float d_time) {
    if ((counter += (d_time * speed)) > 1) {
        counter--;
        offset++;
        offset = offset % (text_size + sep.size());
    }

    LCDController::draw_string(rect.x, rect.y, text.substr(offset, length));
}
