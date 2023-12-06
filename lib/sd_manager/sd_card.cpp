#include <cstring>
#include "global_config.h"

#include "logger.h"

#include "drivers/sd_card_driver.h"
#include "sd_card.h"

#include <fstream>
#include "inipp.h"

#define TAG "SDCard"

SDCard::SDCard() :
    running(false),
    mount_point(SD_CARD_MOUNT_POINT)
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
    mount_point(SD_CARD_MOUNT_POINT)
{}

SDCard::~SDCard() {}

esp_err_t SDCard::begin() {
    if (!this->running) {
        if (card_mount(&this->config) == ESP_OK) {
            printf("SD card mounted\n");
            sdmmc_card_print_info(stdout, this->config.card);
            this->running = true;
            return ESP_OK;
        }
        printf("SD card fail to mount\n");
    }
    
    return ESP_FAIL;
}

esp_err_t SDCard::end() {
    if (this->running) {
        if (card_unmount(&this->config) == ESP_OK) {
            printf("SD card unmounted\n");
            this->running = false;
            return ESP_OK;
        }
        printf("SD card fail to unmount\n");
    }

    printf("No SD card mounted\n");
    return ESP_FAIL;
}

bool SDCard::is_mounted() {
    return this->running;
}

const char *SDCard::get_mount_point() {
    return this->mount_point;
}

esp_err_t SDCard::ls(const char *path) {
    esp_err_t err = ESP_OK;

    if (this->running) {
        memset(this->sd_buf, 0, sizeof(this->sd_buf));
        switch (card_ls(path, this->sd_buf)) {
            case ESP_OK:
            {
                printf("%s\n", this->sd_buf);
                err = ESP_OK;
                break;
            }
            case ESP_ERR_NO_MEM:
            {
                printf("Failed to list, buffer too small\n");
                err = ESP_FAIL;
                break;
            }
            default:
            {
                printf("Failed to list: %s\n", path);
                err = ESP_FAIL;
                break;
            }
        }
    } else {
        printf("No SD card mounted\n");
        err = ESP_FAIL;
    }

    return err;
}

esp_err_t SDCard::cat(const char *path) {
    if (this->running) {
        size_t bytes_read_total = 0;
        uint16_t block_no = 0;

        do {
            memset(this->sd_buf, 0, sizeof(this->sd_buf));
            esp_err_t err = card_cat(path, this->sd_buf, &bytes_read_total);

            if (err == ESP_OK) {
                printf("\t\t\tcat %s, read %d bytes, block: %" PRIu16 "\n", path, bytes_read_total, block_no);
                printf("%s\n", this->sd_buf);
                block_no++;
                // handle here the file block, parse/extract data, etc...
            } else {
                printf("cat %s fail\n", path);
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        } while (((bytes_read_total % SD_OPERATIONS_BUFFER_SIZE) == 0) && (bytes_read_total != 0));
        
        return ESP_OK;
    }

    printf("No SD card mounted\n");
    return ESP_FAIL;
}

void SDCard::mkdir(const char *path) {
    if (this->running) {
        if (card_mkdir(path) == ESP_OK) {
            printf("mkdir %s\n", path);
        } else {
            printf("mkdir %s fail\n", path);
        }
    } else {
        printf("No SD card mounted\n");
    }
}

void SDCard::touch(const char *path) {
    if (this->running) {
        if (card_touch(path) == ESP_OK) {
            printf("touch %s\n", path);
        } else {
            printf("touch %s fail\n", path);
        }
    } else {
        printf("No SD card mounted\n");
    }
}

void SDCard::rm(const char *path) {
    if (this->running) {
        if (card_rm(path) == ESP_OK) {
            printf("rm file: %s\n", path);
        } else {
            printf("rm file: %s fail\n", path);
        }
    } else {
        printf("No SD card mounted\n");
    }
}

void SDCard::rmdir(const char *path) {
    if (this->running) {
        if (card_rmdir(path) == ESP_OK) {
            printf("rmdir: %s\n", path);
        } else {
            printf("rmdir: %s fail\n", path);
        }
    } else {
        printf("No SD card mounted\n");
    }
}

void SDCard::save_buf(const char *path, char *buf) {
    if (this->running) {
        if (card_save_buf(path, buf) == ESP_OK) {
            printf("save buf: %s to: %s\n", buf, path);
        } else {
            printf("save buf: %s to: %s fail\n", buf, path);
        }
    } else {
        printf("No SD card mounted\n");
    }
}

bool SDCard::load_config_ini(const char *path, nvs_device_config_t *config) {
    inipp::Ini<char> ini;
    std::ifstream file(path);
    if (!file.is_open()) {
        TB_LOGE(TAG, "error oppening config ini (file: %s)", path);
        return false;
    }

    ini.parse(file);
    file.close();

    std::string value;
    if (!inipp::get_value(ini.sections["debug"], "log_level", value)) {
        TB_LOGE(TAG, "error reading debug.log_level");
        return false;
    }
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    if (value == "none") {
        config->log_level = ESP_LOG_NONE;
    }
    else if (value == "error") {
        config->log_level = ESP_LOG_ERROR;
    }
    else if (value == "warn") {
        config->log_level = ESP_LOG_WARN;
    }
    else if (value == "info") {
        config->log_level = ESP_LOG_INFO;
    }
    else if (value == "debug") {
        config->log_level = ESP_LOG_DEBUG;
    }
    else if (value == "verbose") {
        config->log_level = ESP_LOG_VERBOSE;
    }
    else {
        TB_LOGE(TAG, "unknown debug.log_level value (%s)", value.c_str());
        return false;
    }

    if (!inipp::get_value(ini.sections["wifi"], "ssid", value)) {
        TB_LOGE(TAG, "error reading wifi.ssid");
        return false;
    }
    strcpy(config->wifi_ssid, value.c_str());
    if (!inipp::get_value(ini.sections["wifi"], "password", value)) {
        TB_LOGE(TAG, "error reading wifi.password");
        return false;
    }
    strcpy(config->wifi_pass, value.c_str());

    if (!inipp::get_value(ini.sections["mqtt"], "broker_uri", value)) {
        TB_LOGE(TAG, "error reading mqtt.broker_uri");
        return false;
    }
    strcpy(config->mqtt_broker_uri, value.c_str());
    if (!inipp::get_value(ini.sections["mqtt"], "username", value)) {
        TB_LOGE(TAG, "error reading mqtt.username");
        return false;
    }
    strcpy(config->mqtt_username, value.c_str());
    if (!inipp::get_value(ini.sections["mqtt"], "password", value)) {
        TB_LOGE(TAG, "error reading mqtt.password");
        return false;
    }
    strcpy(config->mqtt_password, value.c_str());

    return true;
}

char *SDCard::get_sd_buf() {
    return this->sd_buf;
}
