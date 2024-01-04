#include "lcd_controller.h"

#include "rom/ets_sys.h"

#include "logger.h"
#include "global_config.h"
#include "system_manager.h"
#include "font5x7.h"

#include <cstring>
#include <memory>

#define TAG "LCDController"

#define CMD_BASIC_INSTR 0b00100000
#define CMD_SET_X_ADDR  0b10000000
#define CMD_SET_Y_ADDR  0b01000000
#define CMD_SET_DISP_MOD 0b00001000

#define CMD_EXTEND_INSTR 0b00100001
#define CMD_SET_TMP_COEF 0b00000100
#define CMD_SET_LCD_BIAS 0b00010000
#define CMD_SET_CONTRAST 0b10000000

static LCDController::Config config;
static spi_device_handle_t spi;
static void** data_ptrs;
static uint8_t *frame_buf;
static uint8_t data_count;
static uint8_t trans_queue_size;
static uint32_t last_warned_at;

bool LCDController::initialized = false;

esp_err_t LCDController::init() {
    esp_err_t err = ESP_OK;

    if ((err = init_control_pin()) != ESP_OK) {
        return err;
    }
    
    TB_LOGI(TAG, "PCD8544 Control pins: RST=IO%u, DC=IO%u", 
        config.control_pin.reset_io_num, config.control_pin.dc_io_num);

    if ((err = init_spi()) != ESP_OK) {
        return err;
    }

    TB_LOGI(TAG, "SPI pins: Din=IO%d, Clk=IO%d, CE=IO%d",
            config.spi_pin.mosi_io_num, config.spi_pin.sclk_io_num, config.spi_pin.spics_io_num);

    reset();

    data_count = 0;
    trans_queue_size = 0;
    last_warned_at = 0;
    data_ptrs = static_cast<void**>(malloc_for_queue_trans(LCD_CONTROLLER_TRANS_QUEUE_SIZE * sizeof(void *)));

    set_display_mode_int(DisplayMode::NORMAL);
    set_contrast_int(LCD_DEFAULT_CONTRAST);

    frame_buf = static_cast<uint8_t*>(config.dma_chan > 0 ? heap_caps_malloc(LCD_FRAME_BUF_SIZE, MALLOC_CAP_DMA) : malloc(LCD_FRAME_BUF_SIZE));

    return err;
}

esp_err_t LCDController::init_control_pin() {
    esp_err_t err = ESP_OK;

    gpio_config_t io_conf = {
        .pin_bit_mask = ((1ULL << config.control_pin.reset_io_num) | (1ULL << config.control_pin.dc_io_num)),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    if ((err = gpio_config(&io_conf)) != ESP_OK) {
        return err;
    }

    return gpio_config(&io_conf);
}

esp_err_t LCDController::init_spi() {
    esp_err_t err = ESP_OK;

    // spi_bus_config_t buscfg = {};
    // buscfg.mosi_io_num = config.spi_pin.mosi_io_num;
    // buscfg.miso_io_num = -1;
    // buscfg.sclk_io_num = config.spi_pin.sclk_io_num;
    // buscfg.quadwp_io_num = -1;
    // buscfg.quadhd_io_num = -1;

    spi_device_interface_config_t devcfg = {};
    devcfg.mode = 3;
    devcfg.clock_speed_hz = 4 * 1000 * 1000;
    devcfg.spics_io_num = config.spi_pin.spics_io_num;
    devcfg.queue_size = LCD_CONTROLLER_TRANS_QUEUE_SIZE;
    devcfg.pre_cb = pre_transfer_callback;

    // if ((err = spi_bus_initialize(config.spi_host, &buscfg, config.dma_chan)) != ESP_OK) {
    //     return err;
    // }

    xSemaphoreTake(*get_system_manager()->get_spi_semaphore(), portMAX_DELAY);
    err = spi_bus_add_device(config.spi_host, &devcfg, &spi);
    xSemaphoreGive(*get_system_manager()->get_spi_semaphore());

    return err;
}

void LCDController::pre_transfer_callback(spi_transaction_t *t) {
    int dc = (int) t->user;
    gpio_set_level(config.control_pin.dc_io_num, dc);
}

void LCDController::reset() {
    gpio_set_level(config.control_pin.reset_io_num, 1);
    ets_delay_us(200);
    gpio_set_level(config.control_pin.reset_io_num, 0);
    ets_delay_us(1000);
    gpio_set_level(config.control_pin.reset_io_num, 1);
}

void *LCDController::malloc_for_queue_trans(size_t size) {
    return config.dma_chan > 0 ? heap_caps_malloc(size, MALLOC_CAP_DMA) : malloc(size);
}

void LCDController::register_buf_for_gc(void* buf) {
    data_ptrs[data_count++] = buf;
}

void LCDController::queue_trans(const uint8_t *cmds_or_data, int len, bool dc) {
    size_t length = 8 * sizeof(uint8_t) * len;
    // On large data without DMA-enabled SPI, We have to queue data for each 32 bytes packet.
    if (config.dma_chan == 0 && length > LCD_CONTROLLER_MAX_TRANS_LENGTH_WITHOUT_DMA) {
        int length_left = length;
        void* p = static_cast<void*>(const_cast<uint8_t*>(cmds_or_data));
        while (length_left) {
            spi_transaction_t *t = static_cast<spi_transaction_t*>(malloc(sizeof(spi_transaction_t)));
            memset(t, 0, sizeof(spi_transaction_t));
            length = length_left > LCD_CONTROLLER_MAX_TRANS_LENGTH_WITHOUT_DMA
                    ? LCD_CONTROLLER_MAX_TRANS_LENGTH_WITHOUT_DMA : length_left;
            t->length = length;
            t->tx_buffer = p;
            t->user = reinterpret_cast<void*>(dc ? 1 : 0);
            t->flags = 0;
            check_queue_size();

            xSemaphoreTake(*get_system_manager()->get_spi_semaphore(), portMAX_DELAY);
            ESP_ERROR_CHECK(spi_device_queue_trans(spi, t, portMAX_DELAY));
            xSemaphoreGive(*get_system_manager()->get_spi_semaphore());
            
            trans_queue_size++;
            p = (static_cast<char*>(p) + length / 8);
            length_left -= length;
        }
    } else {
        spi_transaction_t *t = static_cast<spi_transaction_t*>(malloc(sizeof(spi_transaction_t)));
        memset(t, 0, sizeof(spi_transaction_t));
        t->length = length;
        t->tx_buffer = cmds_or_data;
        t->user = reinterpret_cast<void*>(dc ? 1 : 0);
        t->flags = 0;
        check_queue_size();

        xSemaphoreTake(*get_system_manager()->get_spi_semaphore(), portMAX_DELAY);
        ESP_ERROR_CHECK(spi_device_queue_trans(spi, t, portMAX_DELAY));
        xSemaphoreGive(*get_system_manager()->get_spi_semaphore());

        trans_queue_size++;
    }
}

void LCDController::check_queue_size() {
    if (trans_queue_size == LCD_CONTROLLER_TRANS_QUEUE_SIZE) {
        uint32_t now = esp_log_timestamp();
        if (now - last_warned_at >= LCD_CONTROLLER_WARN_SUPPRESS_INTERVAL_MS) {
            TB_LOGW(TAG, "trans_queue_size reaches LCD_CONTROLLER_TRANS_QUEUE_SIZE(%d). Force syncing (may get some latency). Consider increasing LCD_CONTROLLER_TRANS_QUEUE_SIZE for less CPU usage", LCD_CONTROLLER_TRANS_QUEUE_SIZE);
            last_warned_at = now;
        }
        sync_and_gc();
    }
}

void LCDController::send_cmd(const uint8_t *cmds, int len) {
    queue_trans(cmds, len, false);
}

void LCDController::send_data(const uint8_t *data, int len) {
    queue_trans(data, len, true);
}

void LCDController::gc_finished_trans_data() {
    while(data_count) {
        if (config.dma_chan > 0) {
            heap_caps_free(data_ptrs[--data_count]);
        } else {
            free(data_ptrs[--data_count]);
        }
    }
}

void LCDController::sync_and_gc() {
    spi_transaction_t *rtrans;
    while(trans_queue_size) {
        xSemaphoreTake(*get_system_manager()->get_spi_semaphore(), portMAX_DELAY);
        ESP_ERROR_CHECK(spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY));
        xSemaphoreGive(*get_system_manager()->get_spi_semaphore());

        free(rtrans);
        trans_queue_size--;
    }
    gc_finished_trans_data();
}

void LCDController::set_contrast_int(uint8_t vop) {
    uint8_t *cmds = static_cast<uint8_t*>(malloc_for_queue_trans(5));
    register_buf_for_gc(cmds);
    cmds[0] = CMD_EXTEND_INSTR;
    cmds[1] = CMD_SET_TMP_COEF | LCD_TEMPERATURE_COEFFICIENT;
    cmds[2] = CMD_SET_LCD_BIAS | LCD_DEFAULT_BIAS;
    cmds[3] = CMD_SET_CONTRAST | vop;
    cmds[4] = CMD_BASIC_INSTR;
    send_cmd(cmds, 5);
}

void LCDController::set_display_mode_int(DisplayMode mode) {
    uint8_t *cmd = static_cast<uint8_t*>(malloc_for_queue_trans(1));
    register_buf_for_gc(cmd);
    *cmd = CMD_SET_DISP_MOD | (((uint8_t)mode & 2) << 1) | ((uint8_t)mode & 1); // display_mode
    send_cmd(cmd, 1);
}

void LCDController::begin() {
    SpiPinConfig spi_config = {
        .mosi_io_num = PIN_LCD_DIN,
        .sclk_io_num = PIN_LCD_SCLK,
        .spics_io_num = PIN_LCD_CS
    };

    ControlPinConfig control_config = {
        .reset_io_num = PIN_LCD_RST,
        .dc_io_num = PIN_LCD_DC
    };

    begin({
        .spi_host = SPI3_HOST,
        .dma_chan = 0, // SPI_DMA_CH_AUTO,
        .spi_pin = spi_config,
        .control_pin = control_config
    });
}

void LCDController::begin(Config cfg) {
    if (!initialized) {
        while (get_system_manager()->get_spi_semaphore() == NULL) {};

        config = cfg;
        ESP_ERROR_CHECK(init());
        initialized = true;
    }
}

void LCDController::display_frame_buf() {
    if (!initialized) {
        return;
    }

    uint8_t *cmds = static_cast<uint8_t*>(malloc_for_queue_trans(2));
    register_buf_for_gc(cmds);
    cmds[0] = CMD_SET_X_ADDR | 0;
    cmds[1] = CMD_SET_Y_ADDR | 0;
    send_cmd(cmds, 2);

    send_data(frame_buf, LCD_FRAME_BUF_SIZE);

    sync_and_gc();
}

void LCDController::clear_frame_buf() {
    if (!initialized) {
        return;
    }

    memset(frame_buf, 0, LCD_FRAME_BUF_SIZE);
}

void LCDController::set_pixel(uint8_t x, uint8_t y, bool color) {
    if (!initialized) {
        return;
    }

    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
        return;
    }

    if (color)
        frame_buf[x + (y / 8) * LCD_WIDTH] |= 1 << (y % 8);
    else
        frame_buf[x + (y / 8) * LCD_WIDTH] &= ~(1 << (y % 8));
}

void LCDController::draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool color) {
    if (!initialized) {
        return;
    }

    int8_t dx = abs(x1 - x0);
    int8_t dy = -abs(y1 - y0);
    int8_t sx = x0 < x1 ? 1 : -1;
    int8_t sy = y0 < y1 ? 1 : -1; 
    int8_t err = dx + dy; 
    int16_t err2;


    while (1) {
        set_pixel(x0,y0, color);
        
        if (x0 == x1 && y0 == y1) {
            break;
        }
        
        err2 = 2 * err;
        
        if (err2 >= dy) { 
            err += dy; 
            x0 += sx; 

            if (x0 > LCD_WIDTH) {
                break;
            }
        } 
        
        if (err2 <= dx) { 
            err += dx;
            y0 += sy; 

            if (y0 > LCD_HEIGHT) {
                break; 
            }
        }
    }
}

void LCDController::draw_circle(int x0, int y0, int r, bool color) {
    if (!initialized) {
        return;
    }

    int x = -r;
    int y = 0; 
    int err = 2 - 2 * r;
    
    do {
        set_pixel(x0-x, y0+y, color);
        set_pixel(x0-y, y0-x, color);
        set_pixel(x0+x, y0-y, color);
        set_pixel(x0+y, y0+x, color);
        
        r = err;
        
        if (r >  x){
            ++x; 
            err = err + x * 2 + 1;   
        }

        if (r <= y) {
            ++y;
            err = err + y * 2 + 1;
        }
    } while (x < 0);
}

void LCDController::draw_bitmap(int x, int y, int w, int h, const uint8_t bitmap[], bool color) {
    if (!initialized) {
        return;
    }

    int16_t byteWidth = (w + 7) / 8;
    uint8_t b = 0;

    for (int16_t j = 0; j < h; j++, y++) {
        
        for (int16_t i = 0; i < w; i++) {

            if (i & 7)
                b <<= 1;
            else
                b = bitmap[j * byteWidth + i / 8];

            if (b & 0x80) {
                set_pixel(x + i, y, color);
            } 
        }
    }
}

void LCDController::draw_char(int x, int y, char c) {
    if (!initialized) {
        return;
    }

    if (c < 32 || c > 127) {
        return;
    }

    for (int i = 0; i < FONT5X7_WIDTH; i++) {
        int line = font5x7[c - FONT5X7_CHAR_CODE_OFFSET][i];

        for (int j = 0; j < FONT5X7_HEIGHT; j++) {
            set_pixel(x + i, y + j, line & (1 << j));
        }
    }
}

void LCDController::draw_string(int x, int y, std::string s) {
    if (!initialized) {
        return;
    }

    int curr_x = x;
    for (char c : s) {
        draw_char(curr_x, y, c);
        curr_x += FONT5X7_WIDTH + 1;

        if (curr_x >= LCD_WIDTH) {
            break;
        }
    }
}

void LCDController::draw_string_formatted(int x, int y, std::string f, ...) {
    if (!initialized) {
        return;
    }

    va_list args;
    va_start(args, f);

    size_t size = vsnprintf( nullptr, 0, f.c_str(), args) + 1;
    if( size <= 0 ) {
        return;
    }

    std::unique_ptr<char[]> buf( new char[ size ] ); 
    vsnprintf( buf.get(), size, f.c_str(), args);

    va_end(args);

    std::string formatted_string = std::string( buf.get(), buf.get() + size - 1 );
    draw_string(x, y, formatted_string);
}

void LCDController::set_display_mode(DisplayMode mode) {
    if (!initialized) {
        return;
    }

    set_display_mode_int(mode);
}

void LCDController::set_contrast(uint8_t vop) {
    if (!initialized) {
        return;
    }

    set_contrast_int(vop);
}
