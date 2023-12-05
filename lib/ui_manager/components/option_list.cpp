#include "option_list.h"

#include "font5x7.h"
#include "logger.h"

OptionList::OptionList(Rect rect, std::vector<OptionEntry> options) {
    this->rect = rect;
    this->options = options;
    this->visible_char_count = rect.width / FONT5X7_WIDTH;
    this->visible_entries_count = rect.height / (FONT5X7_HEIGHT + 1);
}

void OptionList::draw() {
    int y = rect.y;

    for (int i = min_visible_index; i < min_visible_index + visible_entries_count && i < options.size(); i++) {
        std::string visible_string = options[i].option_name.substr(0, visible_char_count - 1);
        visible_string = (i == current_index ? ">" : " ") + visible_string;
        LCDController::draw_string(rect.x, y, visible_string);
        
        y += FONT5X7_HEIGHT + 1;
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
