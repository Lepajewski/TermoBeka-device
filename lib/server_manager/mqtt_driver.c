#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"

#include "logger.h"
#include "global_config.h"

#include "mqtt_driver.h"


static const char * const TAG = "MQTT";
static EventGroupHandle_t mqtt_event_group;
static TimerHandle_t mqtt_reconnect_timer;
static esp_mqtt_client_handle_t client;
static uint16_t connection_retries = 0;


static void event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t evt = event_data;

    switch((esp_mqtt_event_id_t) event_id) {
        case MQTT_EVENT_CONNECTED:
        {
            connection_retries = 0;
            TB_LOGI(TAG, "connected");
            xEventGroupSetBits(mqtt_event_group, BIT_MQTT_CONNECTED);
            break;
        }
        case MQTT_EVENT_DISCONNECTED:
        {
            if (connection_retries < MQTT_RECONNECT_RETRY) {
                connection_retries++;
                TB_LOGI(TAG, "disconnected... retrying... %" PRIu16, connection_retries);
            } else {
                TB_LOGW(TAG, "cannot connect to mqtt broker, retrying in %ds", MQTT_RECONNECT_INTERVAL_MS/1000);
                xEventGroupSetBits(mqtt_event_group, BIT_MQTT_DISCONNECTED);
                xTimerStart(mqtt_reconnect_timer, 0);
            }
            break;
        }
        case MQTT_EVENT_SUBSCRIBED:
        {
            TB_LOGI(TAG, "subscribed");
            break;
        }
        case MQTT_EVENT_PUBLISHED:
        {
            TB_LOGI(TAG, "published, id: %d", evt->msg_id);
            break;
        }
        case MQTT_EVENT_DATA:
        {
            TB_LOGI(TAG, "received data");
            break;
        }
        case MQTT_EVENT_ERROR:
        {
            TB_LOGI(TAG, "event error");
            break;
        }
        default:
        {
            TB_LOGI(TAG, "other event: %d", evt->event_id);
            break;
        }
    }
}


static void mqtt_reconnect_timer_cb(TimerHandle_t timer) {
    xEventGroupSetBits(mqtt_event_group, BIT_MQTT_CONNECT_TIMEOUT);
    connection_retries = 0;
    xTimerStop(mqtt_reconnect_timer, 0);
}

static void stop_timer() {
    xTimerStop(mqtt_reconnect_timer, 0);
}

void mqtt_set_event_group(EventGroupHandle_t *event_group) {
    mqtt_event_group = *event_group;
}

void mqtt_setup_timer() {
    mqtt_reconnect_timer = xTimerCreate(
        "mqttReconn",
        pdMS_TO_TICKS(MQTT_RECONNECT_INTERVAL_MS),
        pdFALSE,
        NULL,
        mqtt_reconnect_timer_cb
    );
}

esp_err_t mqtt_begin(mqtt_driver_config_t *cfg) {
    esp_err_t err = ESP_OK;
    connection_retries = 0;

    stop_timer();

    esp_mqtt_client_config_t mqtt_cfg = {};

    mqtt_cfg.network.reconnect_timeout_ms = MQTT_AUTO_RECONNECT_TIMEOUT_MS;

    client = esp_mqtt_client_init(&mqtt_cfg);
    if ((err = esp_mqtt_client_set_uri(client, cfg->credentials.uri)) != ESP_OK) {
        return err;
    }

    if ((err = esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, event_handler, NULL)) != ESP_OK) {
        return err;
    }

    if ((err = esp_mqtt_client_start(client)) != ESP_OK) {
        return err;
    }

    TB_LOGI(TAG, "init finished");

    xEventGroupSetBits(mqtt_event_group, BIT_MQTT_RUNNING);

    return err;
}

esp_err_t mqtt_end() {
    esp_err_t err = ESP_OK;

    if ((err = esp_mqtt_client_stop(client)) != ESP_OK) {
        return err;
    }

    xEventGroupSetBits(mqtt_event_group, BIT_MQTT_STOPPED);
    return err;
}
