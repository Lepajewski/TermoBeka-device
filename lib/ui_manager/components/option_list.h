#ifndef LIB_UI_MANAGER_OPTION_LIST_H_
#define LIB_UI_MANAGER_OPTION_LIST_H_

#include "lcd_controller.h"

#include "rect.h"

#include <string>
#include <functional>
#include <vector>

struct OptionEntry {
    std::string option_name;
    std::function<void()> select_callback;
};

class OptionList {
private:
    int current_index = 0;
    int min_visible_index = 0;
    int visible_char_count;
    int visible_entries_count;
    Rect rect;
    std::vector<OptionEntry> options;
    
public:
    OptionList(Rect rect, std::vector<OptionEntry> options);

    void draw(LCDController* lcd);
    void move_up();
    void move_down();
    void select();
};

#endif