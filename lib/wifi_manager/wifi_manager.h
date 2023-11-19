#ifndef LIB_WIFI_MANAGER_WIFI_MANAGER_H_
#define LIB_WIFI_MANAGER_WIFI_MANAGER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "system_manager.h"
#include "tb_event.h"
#include "wifi_driver.h"


class WiFiManager {
 private:
    SystemManager *sysMgr;
    QueueHandle_t *event_queue_handle;
    QueueHandle_t *wifi_queue_handle;

    bool running;
    bool connected;
    EventGroupHandle_t wifi_event_group;
    wifi_driver_config_t config;

    void setup();
    void process_wifi_driver_events();
    void process_wifi_event(WiFiEvent *evt);
    void poll_wifi_events();
 public:
    WiFiManager();
    ~WiFiManager();

    void begin();
    void end();
    void process_events();
};


#endif  // LIB_WIFI_MANAGER_WIFI_MANAGER_H_
