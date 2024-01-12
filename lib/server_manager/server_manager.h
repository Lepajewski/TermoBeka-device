#ifndef LIB_SERVER_MANAGER_SERVER_MANAGER_H_
#define LIB_SERVER_MANAGER_SERVER_MANAGER_H_


#include "inttypes.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/ringbuf.h"

#include <pb.h>
#include "from_device_msg.pb.h"

#include "mqtt_driver.h"
#include "system_manager.h"
#include "tb_event.h"
#include "profile_type.h"


class ServerManager {
 private:
    SystemManager *sysMgr;
    QueueHandle_t *event_queue_handle;
    QueueHandle_t *server_queue_handle;
    QueueHandle_t driver_queue_handle;
    RingbufHandle_t *sd_ring_buf_handle;

    bool ca_file_loaded;
    bool credentials_loaded;
    bool running;
    bool connected;
    EventGroupHandle_t server_event_group;
    mqtt_driver_config_t config;
    uint8_t mac_address[6] = {};
    mqtt_topic topic_from_device;
    mqtt_topic topic_to_device;

    void setup();
    void init_driver_queue();
    void create_topic(mqtt_topic *topic, const char* prefix, const char* postfix);
    void init_topics();
    void subscribe_topic();
    void process_mqtt_driver_events();
    void process_server_event(ServerEvent *evt);
    void process_publish_regulator_update(RegulatorStatusUpdate *info);
    void publish_from_device(RegulatorStatusUpdate *msg);
    void process_read_ca_file();
    void poll_server_events();
    void process_recv_mqtt_message(ToDeviceMessage *msg);
    void poll_driver_messages();

    void send_evt(Event *evt);
    void send_evt_connected();
    void send_evt_disconnected();
    void send_evt_profile_load(ToDeviceMessage *msg);
    void send_evt_profile_start();
    void send_evt_profile_stop();

 public:
    ServerManager();
    ~ServerManager();

    void begin();
    void end();
    void process_events();
};


#endif  // LIB_SERVER_MANAGER_SERVER_MANAGER_H_
