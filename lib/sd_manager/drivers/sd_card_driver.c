#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include "string.h"

#include "sd_card_driver.h"


#define PRINT_DIRENT(type) ((type == DT_DIR) ? "D\t" : ((type == DT_REG) ? "F\t" : "U\t"))
#define PRINT_DIRENT_LEN    3



esp_err_t card_mount(sd_card_config_t *config) {
    esp_err_t err = ESP_OK;

    err = spi_bus_initialize(config->host.slot, &config->bus_cfg, config->dma_chan);
    
    if (err == ESP_OK) {
        err = esp_vfs_fat_sdspi_mount(config->mount_point, &config->host, &config->slot_config, &config->mount_config, &config->card);
    }
    return err;
}

esp_err_t card_unmount(sd_card_config_t *config) {
    esp_err_t err = ESP_OK;

   err = esp_vfs_fat_sdcard_unmount(config->mount_point, config->card);

    if (err == ESP_OK) {
        err = spi_bus_free(config->host.slot);
    }
    return err;
}

esp_err_t card_ls(const char* path, char *buf) {
    esp_err_t err = ESP_OK;
    DIR *dir;
    struct dirent *entry;

    struct stat path_stat;
    stat(path, &path_stat);

    if (S_ISDIR(path_stat.st_mode)) {
        dir = opendir(path);
        if (dir == NULL) {
            err = ESP_FAIL;
        } else {
            buf[0] = '\0';
            uint16_t buf_len = 1;

            while ((entry = readdir(dir)) != NULL) {
                uint8_t entry_len = strlen(entry->d_name) + 1;
                buf_len += entry_len + PRINT_DIRENT_LEN;
                
                if (buf_len <= SD_OPERATIONS_BUFFER_SIZE) {
                    strncat(buf, PRINT_DIRENT(entry->d_type), PRINT_DIRENT_LEN);
                    strncat(buf, entry->d_name, entry_len);
                    strncat(buf, "\n", 2);
                } else {
                    err = ESP_ERR_NO_MEM;
                    break;
                }
            }
            closedir(dir);
        }
    } else {
        err = ESP_FAIL;
    }

    return err;
}

esp_err_t card_cat(const char *path, char *buf) {
    esp_err_t err = ESP_OK;
    FILE *file = fopen(path, "r");
    
    if (file == NULL) {
        err = ESP_FAIL;
    } else {
        size_t bytes_read;
        while ((bytes_read = fread(buf, 1, sizeof(buf), file)) > 0) {
            fwrite(buf, 1, bytes_read, stdout);
        }
    }
    fclose(file);
    return err;
}

esp_err_t card_mkdir(const char *path) {
    esp_err_t err = ESP_OK;
    struct stat st = {0};

    if (stat(path, &st) == -1) {
        if (mkdir(path, 0777) != 0) {
            err = ESP_FAIL;
        }
    } else {
        err = ESP_FAIL;
    }
    return err;
}

esp_err_t card_touch(const char *path) {
    esp_err_t err = ESP_OK;

    FILE *file = fopen(path, "w");
    if (file == NULL) {
        err = ESP_FAIL;
    }
    fclose(file);

    return err;
}

esp_err_t card_rm_file(const char *path) {
    esp_err_t err = ESP_OK;

    if (remove(path) != 0) {
        err = ESP_FAIL;
    }

    return err;
}

