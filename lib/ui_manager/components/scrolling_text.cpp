#include "scrolling_text.h"

#include "lcd_controller.h"

ScrollingText::ScrollingText(Rect rect, float speed, bool always_scroll) {
    this->rect = rect;
    this->speed = speed;
    this->always_scroll = always_scroll;

    this->length = rect.width / FONT5X7_CHAR_WIDTH;
}

void ScrollingText::set_text(std::string &text) {
    text_size = text.size();
    offset = 0;
    counter = 0;

    this->text = text;

    should_scroll = always_scroll || text_size > length;

    if (!should_scroll) {
        return;
    }

    for (int i = 0; text_size * i < length; i++) {
        this->text += sep + text;
    }
}

void ScrollingText::draw(float d_time) {
    if (!should_scroll) {
        LCDController::draw_string(rect.x, rect.y, text);
        return;
    }

    if ((counter += (d_time * speed)) > 1) {
        counter--;
        offset++;
        offset = offset % (text_size + sep.size());
    }
    
    LCDController::draw_string(rect.x, rect.y, text.substr(offset, length));
}
