#ifndef LIB_PERIPHERALS_LCD_CONTROLLER_H_
#define LIB_PERIPHERALS_LCD_CONTROLLER_H_


#include "drivers/pcd8544.h"


class LCDController {
 private:
    bool initialized;
    pcd8544_config_t config;

    void finish_frame();

 public:
    LCDController();
    LCDController(pcd8544_config_t config);
    ~LCDController();

    void begin();
    void end();

    void draw_logo();
    void clear();
    void set_cursor(uint8_t row, uint8_t col);
    void draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
    void draw_rectangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
    void draw_bitmap(const uint8_t *bitmap, uint8_t rows, uint8_t cols, bool transparent);
    void print(const char *text);
    void print_formatted(const char *format, ...);
};

#endif  // LIB_PERIPHERALS_LCD_CONTROLLER_H_
