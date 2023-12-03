#ifndef LIB_SERVER_MANAGER_SERVER_MANAGER_H_
#define LIB_SERVER_MANAGER_SERVER_MANAGER_H_


#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "mqtt_driver.h"
#include "system_manager.h"
#include "tb_event.h"


class ServerManager {
 private:
    SystemManager *sysMgr;
    QueueHandle_t *event_queue_handle;
    QueueHandle_t *server_queue_handle;

    bool running;
    bool connected;
    EventGroupHandle_t server_event_group;
    mqtt_driver_config_t config;

    void setup();
    void process_mqtt_driver_events();
    void process_server_event(ServerEvent *evt);
    void poll_server_events();

    void send_evt(Event *evt);
    void send_evt_connected();
    void send_evt_disconnected();
 public:
    ServerManager();
    ~ServerManager();

    void begin();
    void end();
    void process_events();
};


#endif  // LIB_SERVER_MANAGER_SERVER_MANAGER_H_
