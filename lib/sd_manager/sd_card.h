#ifndef LIB_SD_MANAGER_SD_CARD_H_
#define LIB_SD_MANAGER_SD_CARD_H_


#include "drivers/sd_card_driver.h"
#include "nvs_config.h"


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

    void begin();
    void end();
    bool is_mounted();
    const char *get_mount_point();
    const char* ls(const char *path);
    const char* cat(const char *path);
    void mkdir(const char *path);
    void touch(const char *path);
    void rm(const char *path);
    void rmdir(const char *path);
    void save_buf(const char *path, char *buf);
    bool load_config_ini(const char *path, nvs_device_config_t *config);

    char *get_sd_buf();

};


#endif
