#ifndef LIB_SD_MANAGER_DRIVERS_SD_CARD_DRIVER_H_
#define LIB_SD_MANAGER_DRIVERS_SD_CARD_DRIVER_H_


#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"


#define SD_OPERATIONS_BUFFER_SIZE      4096


typedef struct {
    esp_vfs_fat_sdmmc_mount_config_t mount_config;
    sdmmc_host_t host;
    spi_bus_config_t bus_cfg;
    sdspi_device_config_t slot_config;
    spi_dma_chan_t dma_chan;
    sdmmc_card_t *card;
    const char *mount_point;
} sd_card_config_t;


#ifdef __cplusplus
extern "C" {
#endif

esp_err_t card_mount(sd_card_config_t *config);
esp_err_t card_unmount(sd_card_config_t *config);
esp_err_t card_ls(const char* path, char *buf);
esp_err_t card_cat(const char *path, char *buf, size_t *bytes_read_last);
esp_err_t card_mkdir(const char *path);
esp_err_t card_touch(const char *path);
esp_err_t card_rm(const char *path);
esp_err_t card_rmdir(const char *path);
esp_err_t card_save_to_file(const char *path, char *buf);

#ifdef __cplusplus
}
#endif

#endif  // LIB_SD_MANAGER_DRIVERS_SD_CARD_DRIVER_H_
