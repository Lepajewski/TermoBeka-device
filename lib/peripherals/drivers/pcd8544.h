#ifndef LIB_PERIPHERALS_DRIVERS_PCD8544_H_
#define LIB_PERIPHERALS_DRIVERS_PCD8544_H_


#include "driver/gpio.h"
#include "driver/spi_master.h"


#define PCD8544_FRAME_BUF_SIZE                  504  // 84 rows * 6 cols
#define PCD8544_TRANS_QUEUE_SIZE                32
#define PCD8544_DEFAULT_BIAS                    5
#define PCD8544_DEFAULT_CONTRAST                50
#define PCD8544_TEMPERATURE_COEFFICIENT         2
#define PCD8544_MAX_TRANS_LENGTH_WITHOUT_DMA    256 // 32 bytes * 8 bits = 256 bits
#define PCD8544_WARN_SUPPRESS_INTERVAL_MS       5000




typedef enum {
    PCD8544_ADDRESSING_MODE_HORIZONTAL,
    PCD8544_ADDRESSING_MODE_VERTICAL
} pcd8544_addressing_mode;

typedef enum {
    PCD8544_DISPLAY_MODE_BLANK = 0b00,
    PCD8544_DISPLAY_MODE_ALL_SEGMENTS_ON = 0b01,
    PCD8544_DISPLAY_MODE_NORMAL = 0b10,
    PCD8544_DISPLAY_MODE_INVERSE = 0b11,
} pcd8544_display_mode;


typedef struct {
    gpio_num_t mosi_io_num;
    gpio_num_t sclk_io_num;
    gpio_num_t spics_io_num;
} pcd8544_spi_pin_config_t;

typedef struct {
    gpio_num_t reset_io_num;
    gpio_num_t dc_io_num;
} pcd8544_control_pin_config_t;

typedef struct {
    spi_host_device_t spi_host;
    spi_dma_chan_t dma_chan;
    pcd8544_spi_pin_config_t spi_pin;
    pcd8544_control_pin_config_t control_pin;
} pcd8544_config_t;


#ifdef __cplusplus
extern "C" {
#endif

esp_err_t pcd8544_init(pcd8544_config_t *config);

void pcd8544_set_powerdown_mode(bool powerdown);
void pcd8544_set_display_mode(pcd8544_display_mode mode);
void pcd8544_set_contrast(uint8_t vop);
void pcd8544_clear_display();

void pcd8544_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void pcd8544_draw_rectangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void pcd8544_set_pos(uint8_t row, uint8_t col);
void pcd8544_draw_bitmap(const uint8_t *bitmap, uint8_t rows, uint8_t cols, bool transparent);
void pcd8544_finalize_frame_buf();
void pcd8544_puts(const char* text);
void pcd8544_printf(const char *format, ...);

void pcd8544_sync_and_gc();
void pcd8544_free();

#ifdef __cplusplus
}
#endif


#endif  // LIB_PERIPHERALS_DRIVERS_PCD8544_H_
