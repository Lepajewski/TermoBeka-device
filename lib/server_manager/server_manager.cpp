#include "string.h"
#include <stdio.h>

#include "esp_mac.h"
#include "mqtt_driver.h"

#include "from_device_msg.pb.h"
#include "to_device_msg.pb.h"
#include "pb_encode.h"

#include "global_config.h"
#include "logger.h"
#include "server_manager.h"


const char * const TAG = "ServerMgr";


ServerManager::ServerManager() :
    ca_file_loaded(false),
    credentials_loaded(false),
    running(false),
    connected(false)
{
    this->setup();
}

ServerManager::~ServerManager() {
    this->end();
}

void ServerManager::setup() {
    this->init_topics();
    this->config = {};
    this->server_event_group = xEventGroupCreate();
    this->init_driver_queue();
    mqtt_set_event_group(&this->server_event_group);
    mqtt_setup_timer();

    this->sysMgr = get_system_manager();
    this->event_queue_handle = this->sysMgr->get_event_queue();
    this->server_queue_handle = this->sysMgr->get_server_queue();
    this->sd_ring_buf_handle = this->sysMgr->get_sd_ring_buf();

    mqtt_set_queue(&this->driver_queue_handle);
}

void ServerManager::init_driver_queue() {
    this->driver_queue_handle = xQueueCreate(MQTT_DRIVER_QUEUE_SIZE, sizeof(ToDeviceMessage));

    if (this->driver_queue_handle == NULL) {
        TB_LOGE(TAG, "mqtt driver queue init fail");
        vTaskDelay(pdMS_TO_TICKS(2000));
        fflush(stdout);
        esp_restart();
    }
}

void ServerManager::create_topic(mqtt_topic *topic, const char* prefix, const char* postfix) {
    snprintf((char *) topic, sizeof(mqtt_topic), "%s%02X%02X%02X%s",
        prefix,
        this->mac_address[3],
        this->mac_address[4],
        this->mac_address[5],
        postfix
    );
}

void ServerManager::init_topics() {
    if (esp_read_mac(this->mac_address, ESP_MAC_WIFI_STA) != ESP_OK) {
        memset(this->mac_address, 0, sizeof(this->mac_address));
    }

    TB_LOGI(TAG, "MAC address: %02X%02X%02X%02X%02X%02X",
        this->mac_address[0], this->mac_address[1], this->mac_address[2],
        this->mac_address[3], this->mac_address[4], this->mac_address[5]
    );

    this->create_topic(&this->topic_from_device, MQTT_TOPIC_PREFIX, MQTT_TOPIC_FROM_DEVICE);
    this->create_topic(&this->topic_to_device, MQTT_TOPIC_PREFIX, MQTT_TOPIC_TO_DEVICE);

    TB_LOGI(TAG, "topic_from_device: %s", this->topic_from_device);
    TB_LOGI(TAG, "topic_to_device: %s", this->topic_to_device);
}

void ServerManager::subscribe_topic() {
    esp_err_t err = mqtt_subscribe(this->topic_to_device, 2);
    if (err == ESP_FAIL) {
        TB_LOGE(TAG, "%s fail", __func__);
    }
}

void ServerManager::begin() {
    if (!this->ca_file_loaded || !this->credentials_loaded) {
        TB_LOGE(TAG, "no CA file or credentials loaded, aborting");
        return;
    }

    if (!this->running) {
        esp_err_t err = mqtt_begin(&this->config);
        if (err != ESP_OK) {
            TB_LOGE(TAG, "begin: %d", err);
        }
    } else {
        TB_LOGI(TAG, "already connected");
    }
}

void ServerManager::end() {
    if (this->running) {
        esp_err_t err = mqtt_end();
        if (err != ESP_OK) {
            TB_LOGE(TAG, "fail to end: %d", err);
        } else {
            this->running = false;
        }
    } else {
        TB_LOGI(TAG, "already ended");
    }
}

void ServerManager::process_mqtt_driver_events() {
    EventBits_t bits = xEventGroupWaitBits(
        this->server_event_group,
        BIT_MQTT_ALL,
        pdTRUE,
        pdFALSE,
        pdMS_TO_TICKS(10)
    );

    if ((bits & BIT_MQTT_RUNNING) == BIT_MQTT_RUNNING) {
        TB_LOGI(TAG, "mqtt running");
        this->running = true;
    } else if ((bits & BIT_MQTT_STOPPED) == BIT_MQTT_STOPPED) {
        TB_LOGI(TAG, "mqtt stopped");
        this->running = false;
    } else if ((bits & BIT_MQTT_DISCONNECTED) == BIT_MQTT_DISCONNECTED) {
        TB_LOGI(TAG, "mqtt disconnected");
        this->connected = false;
        this->end();
        this->send_evt_disconnected();
    } else if ((bits & BIT_MQTT_CONNECTED) == BIT_MQTT_CONNECTED) {
        TB_LOGI(TAG, "mqtt connected");
        this->connected = true;
        this->subscribe_topic();
        this->send_evt_connected();
    } else if ((bits & BIT_MQTT_CONNECT_TIMEOUT) == BIT_MQTT_CONNECT_TIMEOUT) {
        TB_LOGI(TAG, "mqtt reconnecting...");
        this->begin();
    }
}

void ServerManager::process_server_event(ServerEvent *evt) {
    TB_LOGI(TAG, "%s", server_event_type_to_s(evt->type));
    switch (evt->type) {
        case ServerEventType::CONNECT:
        {
            ServerEventCredentials *payload = reinterpret_cast<ServerEventCredentials*>(evt->payload);
            strlcpy(this->config.credentials.uri, payload->credentials.uri, MQTT_MAX_BROKER_URI_LEN);
            strlcpy(this->config.credentials.username, payload->credentials.username, MQTT_MAX_USERNAME_LEN);
            strlcpy(this->config.credentials.password, payload->credentials.password, MQTT_MAX_PASSWORD_LEN);

            TB_LOGI(TAG, "NEW: uri: %s uname: %s pass: %s", this->config.credentials.uri, this->config.credentials.username, this->config.credentials.password);

            this->credentials_loaded = true;
            this->begin();
            break;
        }
        case ServerEventType::DISCONNECT:
        {
            TB_LOGI(TAG, "disconnecting");
            this->end();
            this->send_evt_disconnected();
            break;
        }
        case ServerEventType::IS_CONNECTED:
        {
            TB_LOGI(TAG, "connected: %s", this->connected ? "true" : "false");
            break;
        }
        case ServerEventType::PUBLISH_REGULATOR_UPDATE:
        {
            ServerEventPubRegulatorUpdate *payload = reinterpret_cast<ServerEventPubRegulatorUpdate*>(evt->payload);
            this->process_publish_regulator_update(&payload->info);
            break;
        }
        case ServerEventType::READ_CA_FILE:
        {
            this->process_read_ca_file();
            break;
        }
        case ServerEventType::NONE:
        default:
            break;
    }
}

void ServerManager::process_publish_regulator_update(RegulatorStatusUpdate *info) {
    if (!this->running || !this->connected) {
        return;
    }

    this->publish_from_device(info);
}

void ServerManager::publish_from_device(RegulatorStatusUpdate *msg) {
    uint8_t buffer[SERVER_QUEUE_MAX_PAYLOAD] = {};
    pb_ostream_t ostream;

    ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    pb_encode(&ostream, &RegulatorStatusUpdate_msg, msg);

    size_t written = ostream.bytes_written;
    TB_LOGI(TAG, "%s written: %d", __func__, written);

    esp_err_t err = mqtt_publish(this->topic_from_device, (const char *) buffer, (uint16_t) written, 1);
    if (err != ESP_OK) {
        TB_LOGE(TAG, "fail to publish");
    }
}

void ServerManager::process_read_ca_file() {
    size_t ca_len;
    char *ca_cert = (char*) xRingbufferReceive(*this->sd_ring_buf_handle, &ca_len, pdMS_TO_TICKS(10));

    if (ca_cert != NULL) {
        mqtt_set_ca_cert(ca_cert, ca_len);
        vRingbufferReturnItem(*this->sd_ring_buf_handle, (void*) ca_cert);

        this->ca_file_loaded = true;
        this->begin();
    } else {
        TB_LOGE(TAG, "fail to set CA cert");
    }
}

void ServerManager::poll_server_events() {
    ServerEvent evt = {};

    while (uxQueueMessagesWaiting(*this->server_queue_handle)) {
        if (xQueueReceive(*this->server_queue_handle, &evt, pdMS_TO_TICKS(10)) == pdPASS) {
            TB_LOGI(TAG, "new event, type: %d", evt.type);
            this->process_server_event(&evt);
        }
    }
}

void ServerManager::process_recv_mqtt_message(ToDeviceMessage *msg) {
    switch (msg->command) {
        case Command_NEW_PROFILE:
        {
            this->send_evt_profile_load(msg);
            break;
        }
        case Command_START_PROFILE:
        {
            this->send_evt_profile_start();
            break;
        }
        case Command_END_PROFILE:
        {
            this->send_evt_profile_stop();
            break;
        }
        case Command_NONE:
        default:
            break;
    }
}

void ServerManager::poll_driver_messages() {
    ToDeviceMessage msg = ToDeviceMessage_init_default;

    while (uxQueueMessagesWaiting(this->driver_queue_handle)) {
        if (xQueueReceive(this->driver_queue_handle, &msg, pdMS_TO_TICKS(10)) == pdPASS) {
            TB_LOGI(TAG, "new server message, command: %d", msg.command);
            this->process_recv_mqtt_message(&msg);
        }
    }
}

void ServerManager::process_events() {
    this->process_mqtt_driver_events();
    this->poll_server_events();
    this->poll_driver_messages();
}

void ServerManager::send_evt(Event *evt) {
    evt->origin = EventOrigin::SERVER;
    if (xQueueSend(*this->event_queue_handle, &*evt, portMAX_DELAY) != pdTRUE) {
        TB_LOGE(TAG, "event send fail");
    }
}

void ServerManager::send_evt_connected() {
    Event evt = {};
    evt.type = EventType::SERVER_CONNECTED;
    this->send_evt(&evt);
}

void ServerManager::send_evt_disconnected() {
    Event evt = {};
    evt.type = EventType::SERVER_DISCONNECTED;
    this->send_evt(&evt);
}

void ServerManager::send_evt_profile_load(ToDeviceMessage *msg) {
    Event evt = {};
    evt.type = EventType::SERVER_PROFILE_LOAD;
    ProfileEventNewProfile payload = {};

    for (uint8_t i = 0; i < msg->data_count; i++) {
        payload.profile.points[i] = {
            .temperature = static_cast<int16_t>(msg->data[i].temperature),
            .time_ms = msg->data[i].time
        };
    }

    strncpy(payload.profile_name, msg->ProfileName, 32);
    
    memcpy(&evt.payload, &payload.buffer, sizeof(ProfileEventNewProfile));
    this->send_evt(&evt);
}

void ServerManager::send_evt_profile_start() {
    Event evt = {};
    evt.type = EventType::SERVER_PROFILE_START;
    this->send_evt(&evt);
}

void ServerManager::send_evt_profile_stop() {
    Event evt = {};
    evt.type = EventType::SERVER_PROFILE_STOP;
    this->send_evt(&evt);
}


