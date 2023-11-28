#ifndef LIB_PERIPHERALS_NEW_LCD_CONTROLLER_H_
#define LIB_PERIPHERALS_NEW_LCD_CONTROLLER_H_

#include "driver/gpio.h"
#include "driver/spi_master.h"

#define LCD_CONTROLLER_TRANS_QUEUE_SIZE 32
#define LCD_CONTROLLER_FRAME_BUF_SIZE 504
#define LCD_CONTROLLER_DEFAULT_CONTRAST 50
#define LCD_CONTROLLER_TEMPERATURE_COEFFICIENT 2
#define LCD_CONTROLLER_DEFAULT_BIAS 5

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

    static esp_err_t init();
    static esp_err_t init_control_pin();
    static esp_err_t init_spi();
    static void pre_transfer_callback(spi_transaction_t *t);

    static void reset();

    static void *malloc_for_queue_trans(size_t size);
    static void register_buf_for_gc(void* buf);

    static void set_display_mode_int(DisplayMode mode);
    static void set_contrast_int(uint8_t vop);

public:
    static void begin();
    static void begin(Config config);

    static void set_display_mode(DisplayMode mode);
    static void set_contrast(uint8_t vop);
};

#endif