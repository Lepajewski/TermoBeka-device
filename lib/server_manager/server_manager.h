#ifndef LIB_SERVER_MANAGER_SERVER_MANAGER_H_
#define LIB_SERVER_MANAGER_SERVER_MANAGER_H_


#include "inttypes.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/ringbuf.h"

#include <pb.h>
#include "profile_status_update.pb.h"

#include "mqtt_driver.h"
#include "system_manager.h"
#include "tb_event.h"
#include "profile_type.h"


class ServerManager {
 private:
    SystemManager *sysMgr;
    QueueHandle_t *event_queue_handle;
    QueueHandle_t *server_queue_handle;
    RingbufHandle_t *sd_ring_buf_handle;

    bool ca_file_loaded;
    bool credentials_loaded;
    bool running;
    bool connected;
    EventGroupHandle_t server_event_group;
    mqtt_driver_config_t config;
    uint8_t mac_address[6] = {};
    mqtt_topic topic_profile_update;
    mqtt_topic topic_regulator_update;
    mqtt_topic topic_server_commands;

    void setup();
    void create_topic(mqtt_topic *topic, const char* prefix, const char* postfix);
    void init_topics();
    void process_mqtt_driver_events();
    void process_server_event(ServerEvent *evt);
    void process_publish_profile_update(ProfileStatusUpdate *info);
    void process_publish_regulator_update();
    void process_read_ca_file();
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
