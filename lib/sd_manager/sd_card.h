#ifndef LIB_SD_MANAGER_SD_CARD_H_
#define LIB_SD_MANAGER_SD_CARD_H_


#include "drivers/sd_card_driver.h"


class SDCard {
 private:
    bool running;
    sd_card_config_t config;
    const char* mount_point;
    char sd_buf[SD_OPERATIONS_BUFFER_SIZE];

 public:
    SDCard();
    SDCard(sd_card_config_t config);
    ~SDCard();

    esp_err_t begin();
    esp_err_t end();
    bool is_mounted();
    const char *get_mount_point();
    esp_err_t ls(const char *path);
    esp_err_t cat(const char *path);
    void mkdir(const char *path);
    void touch(const char *path);
    void rm(const char *path);
    void rmdir(const char *path);
    void save_buf(const char *path, char *buf);

    char *get_sd_buf();

};


#endif
