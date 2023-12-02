#include "string.h"

#include "mqtt_driver.h"
#include "global_config.h"
#include "logger.h"
#include "server_manager.h"


const char * const TAG = "ServerMgr";


ServerManager::ServerManager() :
    running(false),
    connected(false)
{
    this->setup();
}

ServerManager::~ServerManager() {
    this->end();
}

void ServerManager::setup() {
    this->config = {};
    this->server_event_group = xEventGroupCreate();
    mqtt_set_event_group(&this->server_event_group);
    mqtt_setup_timer();

    this->sysMgr = get_system_manager();
    this->event_queue_handle = this->sysMgr->get_event_queue();
    this->server_queue_handle = this->sysMgr->get_server_queue();
}

void ServerManager::begin() {
    this->end();
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

            TB_LOGI(TAG, "NEW: uri: %s", this->config.credentials.uri);

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
        case ServerEventType::NONE:
        default:
            break;
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

void ServerManager::process_events() {
    this->process_mqtt_driver_events();
    this->poll_server_events();
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

