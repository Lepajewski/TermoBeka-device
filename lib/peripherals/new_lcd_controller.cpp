#include "new_lcd_controller.h"

#include "rom/ets_sys.h"

#include "logger.h"
#include "global_config.h"

#define TAG "NewLCDController"

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
    data_ptrs = (void**)malloc_for_queue_trans(LCD_CONTROLLER_TRANS_QUEUE_SIZE * sizeof(void *));

    set_display_mode_int(DisplayMode::NORMAL);
    set_contrast_int(LCD_CONTROLLER_DEFAULT_CONTRAST);

    frame_buf = (uint8_t*)(config.dma_chan > 0 ? heap_caps_malloc(LCD_CONTROLLER_FRAME_BUF_SIZE, MALLOC_CAP_DMA) : malloc(LCD_CONTROLLER_FRAME_BUF_SIZE));

    return err;
}

#pragma region INIT_REGION

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

#pragma endregion INIT_REGION

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

void NewLCDController::set_contrast_int(uint8_t vop) {
    uint8_t *cmds = (uint8_t*)malloc_for_queue_trans(5);
    register_buf_for_gc(cmds);
    cmds[0] = 0b00100001; // extended instruction set
    cmds[1] = 0b00000100 | LCD_CONTROLLER_TEMPERATURE_COEFFICIENT; // templerature coefficient
    cmds[2] = 0b00010000 | LCD_CONTROLLER_DEFAULT_BIAS; // bias system
    cmds[3] = 0b10000000 | vop; // Vop (contrast)
    cmds[4] = 0b00100000; // basic instruction set
    pcd8544_cmds(cmds, 5);
}

void NewLCDController::set_display_mode_int(DisplayMode mode) {
    uint8_t *cmd = (uint8_t*)malloc_for_queue_trans(1);
    register_buf_for_gc(cmd);
    *cmd = 0b00001000 | (((uint8_t)mode & 2) << 1) | ((uint8_t)mode & 1); // display_mode
    pcd8544_cmds(cmd, 1);
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
