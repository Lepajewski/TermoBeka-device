#include "option_list.h"

#define FONT_HEIGHT 7
#define FONT_WIDTH 5

OptionList::OptionList(Rect rect, std::vector<OptionEntry> options) {
    this->rect = rect;
    this->options = options;
    this->visible_char_count = rect.width / FONT_WIDTH;
    this->visible_entries_count = rect.height / (FONT_HEIGHT + 1);
}

void OptionList::draw(LCDController *lcd) {
    int y = (rect.y + 7) / 8;
    lcd->set_cursor(rect.x, y);
    for (int i = min_visible_index; i < min_visible_index + visible_entries_count && i < options.size(); i++) {
        std::string visible_string = options[i].option_name.substr(0, visible_char_count - 1);
        visible_string = (i == current_index ? ">" : " ") + visible_string;
        lcd->print(visible_string.c_str());

        y++;
        lcd->set_cursor(rect.x, y);
    }
}

void OptionList::move_up() {
    if (current_index > 0) {
        current_index--;
    }
    if (current_index < min_visible_index) {
        min_visible_index--;
    }
}

void OptionList::move_down() {
    if (current_index < options.size() - 1) {
        current_index++;
    }
    if (current_index > min_visible_index + visible_entries_count - 1) {
        min_visible_index++;
    }
}

void OptionList::select() {
    options[current_index].select_callback();
}
