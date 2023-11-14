#include <cstring>
#include "global_config.h"

#include "drivers/sd_card_driver.h"
#include "sd_card.h"


SDCard::SDCard() :
    running(false),
    mount_point("/sd")
{
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
        .disk_status_check_enable = false
    };

    spi_bus_config_t bus_cfg;
    memset(&bus_cfg, 0, sizeof(spi_bus_config_t));
    bus_cfg.mosi_io_num = PIN_SD_MOSI;
    bus_cfg.miso_io_num = PIN_SD_MISO;
    bus_cfg.sclk_io_num = PIN_SD_SCLK;
    bus_cfg.quadwp_io_num = GPIO_NUM_NC;
    bus_cfg.quadhd_io_num = GPIO_NUM_NC;

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_SD_CS;
    slot_config.host_id = SPI2_HOST;

    this->config.mount_config = mount_config;
    this->config.host = host;
    this->config.bus_cfg = bus_cfg;
    this->config.slot_config = slot_config;
    this->config.dma_chan = SPI_DMA_CH_AUTO;
    this->config.mount_point = this->mount_point;
}

SDCard::SDCard(sd_card_config_t config) :
    running(false),
    config(config),
    mount_point("/sd")
{}

SDCard::~SDCard() {}

void SDCard::begin() {
    if (!this->running) {
        ESP_ERROR_CHECK(card_mount(&this->config));

        sdmmc_card_print_info(stdout, this->config.card);
        this->running = true;
    }
}

void SDCard::end() {
    if (this->running) {
        ESP_ERROR_CHECK(card_unmount(&this->config));
        this->running = false;
    }
}

bool SDCard::is_mounted() {
    return this->running;
}

const char *SDCard::get_mount_point() {
    return this->mount_point;
}

const char* SDCard::list_files(const char *path) {
    if (this->running) {
        esp_err_t err = card_ls(path, this->sd_buf);
        switch (err) {
            case ESP_OK:
            {
                printf("%s\n", sd_buf);
                break;
            }
            case ESP_ERR_NO_MEM:
            {
                printf("Failed to list, buffer too small\n");
                break;
            }
            default:
            {
                printf("Failed to list: %s\n", path);
                break;
            }
        }
        
    }

    return "NO SD CARD";
}

void SDCard::mkdir(const char *path) {
    if (card_mkdir(path) == ESP_OK) {
        printf("mkdir %s\n", path);
    } else {
        printf("mkdir %s fail\n", path);
    }
}

void SDCard::touch(const char *path) {
    if (card_touch(path) == ESP_OK) {
        printf("touch %s\n", path);
    } else {
        printf("touch %s fail\n", path);
    }
}

void SDCard::rm_file(const char *path) {
    if (card_rm_file(path) == ESP_OK) {
        printf("rm file: %s\n", path);
    } else {
        printf("rm file: %s fail\n", path);
    }
}
