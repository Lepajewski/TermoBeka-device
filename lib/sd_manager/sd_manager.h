#ifndef LIB_SD_MANAGER_SD_MANAGER_H_
#define LIB_SD_MANAGER_SD_MANAGER_H_


#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"

#include "tb_event.h"
#include "sd_card.h"
#include "system_manager.h"


class SDManager {
 private:
    SystemManager *sysMgr;
    QueueHandle_t *event_queue_handle;
    QueueHandle_t *sd_queue_handle;
    RingbufHandle_t *sd_ring_buf_handle;
    SDCard card;

    

    char *make_path(const char *path);

    void process_sd_event(SDEvent *evt);
    void process_mount_card();
    void process_unmount_card();
    void poll_sd_events();

    void send_evt(Event *evt);
    void send_evt_sd_mounted();
    void send_evt_sd_unmounted();
    void send_evt_sd_config_load();
    void send_evt_sd_load_ca_file();

    esp_err_t process_load_ca_cert(const char *path);
 public:
    SDManager();
    ~SDManager();

    void begin();
    void end();
    void process_events();
};


#endif  // LIB_SD_MANAGER_SD_MANAGER_H_
