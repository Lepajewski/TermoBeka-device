#ifndef LIB_UI_MANAGER_SCROLLING_TEXT_H_
#define LIB_UI_MANAGER_SCROLLING_TEXT_H_

#include <string>

#include "rect.h"
#include "font5x7.h"

class ScrollingText {
private:
    const std::string sep = "   ";

    Rect rect;
    std::string text = "";
    float speed;

    uint8_t offset = 0;
    uint8_t length;
    size_t text_size = 0;

    float counter;
public:
    ScrollingText(Rect rect, float speed);

    void set_text(std::string &text);
    void draw(float d_time);
};

#endif