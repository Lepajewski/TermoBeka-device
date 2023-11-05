#include <cstdarg>

#include "global_config.h"
#include "lcd_controller.h"


uint8_t example_bitmap[] = {
    0b11100000,
    0b11111000,
    0b11111110,
    0b00111110,
    0b00010111,
    0b00000111,
    0b00000111,
    0b00000111,
    0b00000111,
    0b00000111,
    0b00000111,
    0b00000111,
    0b00011111,
    0b00011110,
    0b00111110,
    0b11111100,
    0b11111000,
    0b11110000,

    0b00011111,
    0b00111111,
    0b01111000,
    0b11110000,
    0b11110000,
    0b11100000,
    0b11100000,
    0b11100000,
    0b11100000,
    0b11100000,
    0b11100000,
    0b11110000,
    0b11110000,
    0b11111000,
    0b11111111,
    0b00111111,
    0b00111111,
    0b00011111,
};


LCDController::LCDController() :
    initialized(false)
{
    pcd8544_spi_pin_config_t spi_config = {
        .mosi_io_num = PIN_LCD_DIN,
        .sclk_io_num = PIN_LCD_SCLK,
        .spics_io_num = PIN_LCD_CS
    };

    pcd8544_control_pin_config_t control_config = {
        .reset_io_num = PIN_LCD_RST,
        .dc_io_num = PIN_LCD_DC
    };

    this->config = {
        .spi_host = SPI2_HOST,
        .dma_chan = SPI_DMA_CH_AUTO,
        .spi_pin = spi_config,
        .control_pin = control_config
    };
}

LCDController::LCDController(pcd8544_config_t config) :
    initialized(false)
{
    this->config = config;
}

LCDController::~LCDController() {
    end();
}

void LCDController::begin() {
    if (!this->initialized) {
        ESP_ERROR_CHECK(pcd8544_init(&this->config));
        this->initialized = true;

        clear();
        draw_line(0, 0, 83, 47);
        draw_line(0, 47, 83, 0);
        draw_line(0, 0, 83, 0);
        draw_line(83, 0, 83, 47);
        draw_line(0, 0, 0, 47);
        draw_line(0, 47, 83, 47);
        finish_frame();
    }
}

void LCDController::end() {
    if (this->initialized) {
        pcd8544_sync_and_gc();
        pcd8544_free();
        this->initialized = false;
    }
}

// important:
// https://esp32-pcd8544.readthedocs.io/en/latest/api-reference/pcd8544/index.html#_CPPv226pcd8544_finalize_frame_bufv
void LCDController::finish_frame() {
    if (this->initialized) {
        pcd8544_finalize_frame_buf();
        pcd8544_sync_and_gc();
    }
}

void LCDController::draw_logo() {
    if (this->initialized) {
        clear();
        draw_bitmap(example_bitmap, 18, 2, false);

        /* transparent on */
        set_cursor(30, 3);
        draw_bitmap(example_bitmap, 18, 2, false);
        set_cursor(35, 4);
        draw_bitmap(example_bitmap, 18, 2, true);

        /* transparent off */
        set_cursor(50, 0);
        draw_bitmap(example_bitmap, 18, 2, false);
        set_cursor(55, 1);
        draw_bitmap(example_bitmap, 18, 2, false);

        finish_frame();
    }
}

void LCDController::clear() {
    if (this->initialized) {
        pcd8544_clear_display();
        pcd8544_finalize_frame_buf();
    }
}

void LCDController::set_cursor(uint8_t row, uint8_t col) {
    if (this->initialized) {
        pcd8544_set_pos(row, col);
    }
}

void LCDController::draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    if (this->initialized) {
        pcd8544_draw_line(x0, y0, x1, y1);
    }
}

void LCDController::draw_rectangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    if (this->initialized) {
        pcd8544_draw_rectangle(x0, y0, x1, y1);
    }
}

void LCDController::draw_bitmap(const uint8_t *bitmap, uint8_t rows, uint8_t cols, bool transparent) {
    if (this->initialized) {
        pcd8544_draw_bitmap(bitmap, rows, cols, transparent);
    }
}

void LCDController::print(const char *text) {
    if (this->initialized) {
        pcd8544_puts(text);
    }
}

void LCDController::print_formatted(const char *format, ...) {
    if (this->initialized) {
        char s[64];
        va_list ap;
        va_start(ap, format);
        vsprintf(s, format, ap);
        va_end(ap);
        pcd8544_puts(s);
    }
}
