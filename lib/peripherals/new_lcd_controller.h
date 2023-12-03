#ifndef LIB_PERIPHERALS_NEW_LCD_CONTROLLER_H_
#define LIB_PERIPHERALS_NEW_LCD_CONTROLLER_H_

#include "driver/gpio.h"
#include "driver/spi_master.h"

#include <string>

#define LCD_CONTROLLER_TRANS_QUEUE_SIZE 32
#define LCD_CONTROLLER_MAX_TRANS_LENGTH_WITHOUT_DMA 256 // 32 bytes * 8 bits = 256 bits

#define LCD_WIDTH 84
#define LCD_BYTE_HEIGHT 6
#define LCD_HEIGHT LCD_BYTE_HEIGHT*8
#define LCD_FRAME_BUF_SIZE LCD_WIDTH*LCD_BYTE_HEIGHT

#define LCD_DEFAULT_CONTRAST 50
#define LCD_TEMPERATURE_COEFFICIENT 2
#define LCD_DEFAULT_BIAS 5

#define LCD_CONTROLLER_WARN_SUPPRESS_INTERVAL_MS 5000

class NewLCDController {
public:
    struct SpiPinConfig {
        gpio_num_t mosi_io_num;
        gpio_num_t sclk_io_num;
        gpio_num_t spics_io_num;
    };

    struct ControlPinConfig {
        gpio_num_t reset_io_num;
        gpio_num_t dc_io_num;
    };

    struct Config {
        spi_host_device_t spi_host;
        spi_dma_chan_t dma_chan;
        SpiPinConfig spi_pin;
        ControlPinConfig control_pin;
    };

    enum class DisplayMode {
        BLANK = 0b00,
        ALL_SEGMENTS_ON = 0b01,
        NORMAL = 0b10,
        INVERSE = 0b11,
    };

private:
    static bool initialized;
    static Config config;
    static spi_device_handle_t spi;
    static void** data_ptrs;
    static uint8_t *frame_buf;
    static uint8_t data_count;
    static uint8_t trans_queue_size;
    static uint32_t last_warned_at;

    static esp_err_t init();
    static esp_err_t init_control_pin();
    static esp_err_t init_spi();
    static void pre_transfer_callback(spi_transaction_t *t);

    static void reset();

    static void *malloc_for_queue_trans(size_t size);
    static void register_buf_for_gc(void* buf);
    static void gc_finished_trans_data();

    static void sync_and_gc();
    static void check_queue_size();

    static void queue_trans(const uint8_t *cmds_or_data, int len, bool dc);
    static void send_cmd(const uint8_t *cmds, int len);
    static void send_data(const uint8_t *data, int len);

    static void set_display_mode_int(DisplayMode mode);
    static void set_contrast_int(uint8_t vop);

public:
    static void begin();
    static void begin(Config config);

    static void display_frame_buf();
    static void clear_frame_buf();

    static void set_pixel(uint8_t x, uint8_t y, bool color = true);
    static void draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool color = true);
    static void draw_circle(int x0, int y0, int r, bool color = true);
    static void draw_bitmap(int x, int y, int w, int h, uint8_t bitmap[], bool color = true);
    static void draw_char(int x, int y, char c);
    static void draw_string(int x, int y, std::string s);

    static void set_display_mode(DisplayMode mode);
    static void set_contrast(uint8_t vop);
};

#endif