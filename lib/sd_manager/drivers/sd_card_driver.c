#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include "string.h"

#include "sd_card_driver.h"


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
            uint32_t buf_len = 1;

            while ((entry = readdir(dir)) != NULL) {
                uint32_t entry_len = strlen(entry->d_name);
                if (buf_len + entry_len + 2 <= SD_OPERATIONS_BUFFER_SIZE) {
                    strcat(buf, entry->d_name);
                    strcat(buf, "\n");
                    buf_len += entry_len + 2;
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
