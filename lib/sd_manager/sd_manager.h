#ifndef LIB_SD_MANAGER_SD_MANAGER_H_
#define LIB_SD_MANAGER_SD_MANAGER_H_


#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "tb_event.h"
#include "sd_card.h"
#include "system_manager.h"


class SDManager {
 private:
    SystemManager *sysMgr;
    QueueHandle_t *event_queue_handle;
    QueueHandle_t *sd_queue_handle;
    SDCard card;

    char *get_args(SDEvent *evt);

    void process_sd_event(SDEvent *evt);
    void poll_sd_events();
 public:
    SDManager();
    ~SDManager();

    void begin();
    void end();
    void process_events();
};


#endif  // LIB_SD_MANAGER_SD_MANAGER_H_
