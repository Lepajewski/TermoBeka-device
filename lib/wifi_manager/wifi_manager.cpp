#include "string.h"

#include "ntp_driver.h"
#include "global_config.h"
#include "logger.h"
#include "wifi_manager.h"


const char * const TAG = "WiFiMgr";


WiFiManager::WiFiManager() :
    running(false),
    connected(false)
{    
    this->setup();
}

WiFiManager::~WiFiManager() {
    this->end();
}

void WiFiManager::setup() {
    this->wifi_event_group = xEventGroupCreate();
    wifi_set_event_group(&this->wifi_event_group);

    this->sysMgr = get_system_manager();
    this->event_queue_handle = this->sysMgr->get_event_queue();
    this->wifi_queue_handle = this->sysMgr->get_wifi_queue();

    this->config.ntp_config = {
        NTP_DEFAULT_SERVER_ADDR,
        NTP_TIME_ZONE,
        NTP_CONNECT_TIMEOUT_MS,
        NTP_RECONNECT_INTERVAL_MS
    };
}

void WiFiManager::begin() {
    esp_err_t err = wifi_begin(&this->config);
    if (err != ESP_OK) {
        TB_LOGE(TAG, "begin: %d", err);
    }
}

void WiFiManager::end() {
    esp_err_t err = wifi_end();
    if (err != ESP_OK) {
        TB_LOGW(TAG, "fail to disconnect: %d", err);
    } else {
        this->running = false;
        this->connected = false;
    }
    // else not necessary as driver will call events
}

void WiFiManager::process_wifi_driver_events() {
    EventBits_t bits = xEventGroupWaitBits(
        this->wifi_event_group,
        BIT_WIFI_ALL,
        pdTRUE,
        pdFALSE,
        pdMS_TO_TICKS(10)
    );

    if ((bits & BIT_WIFI_RUNNING) == BIT_WIFI_RUNNING) {
        TB_LOGI(TAG, "wifi running");
        this->running = true;
    } else if ((bits & BIT_WIFI_STOPPED) == BIT_WIFI_STOPPED) {
        TB_LOGI(TAG, "wifi stopped");
        this->running = false;
    } else if ((bits & BIT_WIFI_DISCONNECTED) == BIT_WIFI_DISCONNECTED) {
        TB_LOGI(TAG, "wifi disconnected");
        ntp_stop();
        this->running = false;
        this->connected = false;
        this->send_evt_disconnected();
    } else if ((bits & BIT_WIFI_CONNECTED) == BIT_WIFI_CONNECTED) {
        TB_LOGI(TAG, "wifi connected");
        this->running = true;
        this->connected = true;
        this->send_evt_connected();
        xEventGroupSetBits(this->wifi_event_group, BIT_WIFI_NTP_START);
    } else if ((bits & BIT_WIFI_NTP_START) == BIT_WIFI_NTP_START) {
        TB_LOGI(TAG, "NTP start");
        wifi_ntp_connect();
    } else if ((bits & BIT_WIFI_NTP_GOT_TIME) == BIT_WIFI_NTP_GOT_TIME) {
        char timestamp[27];
        get_timestamp(timestamp);
        TB_LOGI(TAG, "got time: %s", timestamp);
        this->send_evt_got_time();
    } else if ((bits & BIT_WIFI_SCAN_DONE) == BIT_WIFI_SCAN_DONE) {
        TB_LOGI(TAG, "wifi scan done");
    }
}

void WiFiManager::process_wifi_event(WiFiEvent *evt) {
    TB_LOGI(TAG, "%s", wifi_event_type_to_s(evt->type));
    switch (evt->type) {
        case WiFiEventType::CONNECT:
        {
            if (!this->running || !this->connected) {
                WiFiEventCredentials *payload = reinterpret_cast<WiFiEventCredentials*>(evt->payload);
                strlcpy(this->config.credentials.ssid, payload->credentials.ssid, WIFI_MAX_SSID_LEN);
                strlcpy(this->config.credentials.pass, payload->credentials.pass, WIFI_MAX_PASS_LEN);

                TB_LOGI(TAG, "NEW: ssid: %s pass: %s", this->config.credentials.ssid, this->config.credentials.pass);

                this->begin();
            } else {
                TB_LOGI(TAG, "already connected");
            }
            break;
        }
        case WiFiEventType::DISCONNECT:
        {
            if (this->running) {
                TB_LOGI(TAG, "disconnecting");
                this->end();
                this->send_evt_disconnected();
            } else {
                TB_LOGI(TAG, "already disconnected");
            }
            break;
        }
        case WiFiEventType::IS_CONNECTED:
        {
            TB_LOGI(TAG, "connected: %s", this->connected ? "true" : "false");
            break;
        }
        case WiFiEventType::GET_TIME:
        {
            char timestamp[27];
            get_timestamp(timestamp);
            TB_LOGI(TAG, "time is: %s", timestamp);
            break;
        }
        case WiFiEventType::SCAN:
        {
            if (this->running) {
                esp_err_t err = wifi_scan();
                if (err == ESP_OK) {
                    TB_LOGI(TAG, "scan done");
                } else {
                    TB_LOGW(TAG, "scan failed: %d", err);
                }
            } else {
                TB_LOGW(TAG, "wifi not running");
            }
            break;
        }
        case WiFiEventType::NONE:
        default:
            break;
    }
}

void WiFiManager::poll_wifi_events() {
    WiFiEvent evt = {};

    while (uxQueueMessagesWaiting(*this->wifi_queue_handle)) {
        if (xQueueReceive(*this->wifi_queue_handle, &evt, pdMS_TO_TICKS(10)) == pdPASS) {
            TB_LOGI(TAG, "new event, type: %d", evt.type);
            process_wifi_event(&evt);
        }
    }
}

void WiFiManager::process_events() {
    process_wifi_driver_events();
    poll_wifi_events();
}

void WiFiManager::send_evt(Event *evt) {
    evt->origin = EventOrigin::WIFI;
    if (xQueueSend(*this->event_queue_handle, &*evt, portMAX_DELAY) != pdTRUE) {
        TB_LOGE(TAG, "event send fail");
    }
}

void WiFiManager::send_evt_connected() {
    Event evt = {};
    evt.type = EventType::WIFI_CONNECTED;

    this->send_evt(&evt);
}

void WiFiManager::send_evt_disconnected() {
    Event evt = {};
    evt.type = EventType::WIFI_DISCONNECTED;

    this->send_evt(&evt);
}

void WiFiManager::send_evt_got_time() {
    Event evt = {};
    evt.type = EventType::WIFI_GOT_TIME;

    this->send_evt(&evt);
}
