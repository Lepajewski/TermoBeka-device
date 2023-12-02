#include "new_lcd_controller.h"

#include "rom/ets_sys.h"

#include "logger.h"
#include "global_config.h"
#include "font5x7.h"

#include <cstring>

#define TAG "NewLCDController"

#define CMD_BASIC_INSTR 0b00100000
#define CMD_SET_X_ADDR  0b10000000
#define CMD_SET_Y_ADDR  0b01000000
#define CMD_SET_DISP_MOD 0b00001000

#define CMD_EXTEND_INSTR 0b00100001
#define CMD_SET_TMP_COEF 0b00000100
#define CMD_SET_LCD_BIAS 0b00010000
#define CMD_SET_CONTRAST 0b10000000

bool NewLCDController::initialized = false;

esp_err_t NewLCDController::init() {
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

esp_err_t NewLCDController::init_control_pin() {
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

esp_err_t NewLCDController::init_spi() {
    esp_err_t err = ESP_OK;

    spi_bus_config_t buscfg = {
        .mosi_io_num = config.spi_pin.mosi_io_num,
        .miso_io_num = -1,
        .sclk_io_num = config.spi_pin.sclk_io_num,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    spi_device_interface_config_t devcfg = {
        .mode = 0,
        .clock_speed_hz = 4 * 1000 * 1000,
        .spics_io_num = config.spi_pin.spics_io_num,
        .queue_size = LCD_CONTROLLER_TRANS_QUEUE_SIZE,
        .pre_cb = pre_transfer_callback,
    };

    if ((err = spi_bus_initialize(config.spi_host, &buscfg, config.dma_chan)) != ESP_OK) {
        return err;
    }
    if ((err = spi_bus_add_device(config.spi_host, &devcfg, &spi)) != ESP_OK) {
        return err;
    }
    return err;
}

void NewLCDController::pre_transfer_callback(spi_transaction_t *t) {
    int dc = (int) t->user;
    gpio_set_level(config.control_pin.dc_io_num, dc);
}

void NewLCDController::reset() {
    gpio_set_level(config.control_pin.reset_io_num, 1);
    ets_delay_us(200);
    gpio_set_level(config.control_pin.reset_io_num, 0);
    ets_delay_us(1000);
    gpio_set_level(config.control_pin.reset_io_num, 1);
}

void *NewLCDController::malloc_for_queue_trans(size_t size) {
    return config.dma_chan > 0 ? heap_caps_malloc(size, MALLOC_CAP_DMA) : malloc(size);
}

void NewLCDController::register_buf_for_gc(void* buf) {
    data_ptrs[data_count++] = buf;
}

void NewLCDController::queue_trans(const uint8_t *cmds_or_data, int len, bool dc) {
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
            ESP_ERROR_CHECK( spi_device_queue_trans(spi, t, portMAX_DELAY) );
            trans_queue_size++;
            p += length / 8;
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
        ESP_ERROR_CHECK( spi_device_queue_trans(spi, t, portMAX_DELAY) );
        trans_queue_size++;
    }
}

void NewLCDController::check_queue_size() {
    if (trans_queue_size == LCD_CONTROLLER_TRANS_QUEUE_SIZE) {
        uint32_t now = esp_log_timestamp();
        if (now - last_warned_at >= LCD_CONTROLLER_WARN_SUPPRESS_INTERVAL_MS) {
            TB_LOGW(TAG, "trans_queue_size reaches LCD_CONTROLLER_TRANS_QUEUE_SIZE(%d). Force syncing (may get some latency). Consider increasing LCD_CONTROLLER_TRANS_QUEUE_SIZE for less CPU usage", LCD_CONTROLLER_TRANS_QUEUE_SIZE);
            last_warned_at = now;
        }
        sync_and_gc();
    }
}

void NewLCDController::send_cmd(const uint8_t *cmds, int len) {
    queue_trans(cmds, len, false);
}

void NewLCDController::send_data(const uint8_t *data, int len) {
    queue_trans(data, len, true);
}

void NewLCDController::gc_finished_trans_data() {
    while(data_count) {
        if (config.dma_chan > 0) {
            heap_caps_free(data_ptrs[--data_count]);
        } else {
            free(data_ptrs[--data_count]);
        }
    }
}

void NewLCDController::sync_and_gc() {
    spi_transaction_t *rtrans;
    while(trans_queue_size) {
        ESP_ERROR_CHECK(spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY));
        free(rtrans);
        trans_queue_size--;
    }
    gc_finished_trans_data();
}

void NewLCDController::set_contrast_int(uint8_t vop) {
    uint8_t *cmds = static_cast<uint8_t*>(malloc_for_queue_trans(5));
    register_buf_for_gc(cmds);
    cmds[0] = CMD_EXTEND_INSTR; // extended instruction set
    cmds[1] = CMD_SET_TMP_COEF | LCD_TEMPERATURE_COEFFICIENT; // templerature coefficient
    cmds[2] = CMD_SET_LCD_BIAS | LCD_DEFAULT_BIAS; // bias system
    cmds[3] = CMD_SET_CONTRAST | vop; // Vop (contrast)
    cmds[4] = CMD_BASIC_INSTR; // basic instruction set
    send_cmd(cmds, 5);
}

void NewLCDController::set_display_mode_int(DisplayMode mode) {
    uint8_t *cmd = static_cast<uint8_t*>(malloc_for_queue_trans(1));
    register_buf_for_gc(cmd);
    *cmd = CMD_SET_DISP_MOD | (((uint8_t)mode & 2) << 1) | ((uint8_t)mode & 1); // display_mode
    send_cmd(cmd, 1);
}

void NewLCDController::begin() {
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
        .dma_chan = SPI_DMA_CH_AUTO,
        .spi_pin = spi_config,
        .control_pin = control_config
    });
}

void NewLCDController::begin(Config config) {
    if (!initialized) {
        NewLCDController::config = config;
        ESP_ERROR_CHECK(init());
        initialized = true;
    }
}

void NewLCDController::display_frame_buf() {
    uint8_t *cmds = static_cast<uint8_t*>(malloc_for_queue_trans(2));
    register_buf_for_gc(cmds);
    cmds[0] = CMD_SET_X_ADDR | 0;
    cmds[1] = CMD_SET_Y_ADDR | 0;
    send_cmd(cmds, 2);

    send_data(frame_buf, LCD_FRAME_BUF_SIZE);
}

void NewLCDController::clear_frame_buf() {
    memset(frame_buf, 0, LCD_FRAME_BUF_SIZE);
}

void NewLCDController::set_display_mode(DisplayMode mode) {
    if (!initialized) {
        return;
    }

    set_display_mode_int(mode);
}

void NewLCDController::set_contrast(uint8_t vop) {
    if (!initialized) {
        return;
    }

    set_contrast_int(vop);
}
