#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"

#include "logger.h"
#include "global_config.h"

#include "ntp_stack.h"
#include "wifi_stack.h"


#define BIT_WIFI_ENABLED            BIT0
#define BIT_WIFI_CONNECTED          BIT1
#define BIT_WIFI_NTP_START          BIT2
#define BIT_WIFI_NTP_GOT_TIME       BIT3
#define BIT_WIFI_SCAN_DONE          BIT4

#define BIT_WIFI_ALL    (BIT_WIFI_ENABLED | BIT_WIFI_CONNECTED | BIT_WIFI_NTP_START | BIT_WIFI_GOT_NTP_TIME | BIT_WIFI_SCAN_DONE)


static const char * const TAG = "WIFI";
static EventGroupHandle_t wifi_event_group;
static uint16_t connection_retries = 0;


static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        connection_retries = 0;
        TB_LOGI(TAG, "started, connecting to %s...", WIFI_DEFAULT_SSID);
        esp_wifi_connect();
        xEventGroupSetBits(wifi_event_group, BIT_WIFI_ENABLED);
    } else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_STOP) {
        TB_LOGI(TAG, "stopped");
        xEventGroupClearBits(wifi_event_group, BIT_WIFI_ENABLED);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        TB_LOGI(TAG, "connected to %s", WIFI_DEFAULT_SSID);
        xEventGroupSetBits(wifi_event_group, BIT_WIFI_ENABLED | BIT_WIFI_NTP_START);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        connection_retries++;
        esp_wifi_connect();
        TB_LOGI(TAG, "disconnected... retrying... %" PRIu16, connection_retries);
        xEventGroupClearBits(wifi_event_group, BIT_WIFI_CONNECTED);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t*) event_data;
        TB_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP) {
        TB_LOGI(TAG, "lost ip");
    } else {
        TB_LOGI(TAG, "unknown event");
    }
}

static void ntp_reconnect_timer_cb(TimerHandle_t timer) {
    xEventGroupSetBits(wifi_event_group, BIT_WIFI_NTP_START);
}


void wifi_init(void) {
    TB_LOGI(TAG, "start");
    wifi_event_group = xEventGroupCreate();
    TimerHandle_t ntp_reconnect_timer = xTimerCreate(
        "ntpReconn",
        pdMS_TO_TICKS(NTP_RECONNECT_INTERVAL_MS),
        pdFALSE,
        NULL,
        ntp_reconnect_timer_cb
    );

     // initialize wifi in STA (stationary) mode
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_DEFAULT_SSID,
            .password = WIFI_DEFAULT_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    TB_LOGI(TAG, "init finished");

    // handle wifi events
    while (1) {
        EventBits_t bits = xEventGroupWaitBits(
            wifi_event_group,
            BIT_WIFI_NTP_START,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );

        if ((bits & BIT_WIFI_NTP_START) == BIT_WIFI_NTP_START) {
            esp_err_t err = ntp_start();
            xEventGroupClearBits(wifi_event_group, BIT_WIFI_NTP_START);
            if (err == ESP_ERR_TIMEOUT) {
                TB_LOGW(TAG, "NTP timeout. Retrying... in %ds", NTP_RECONNECT_INTERVAL_MS / 1000);
                xTimerStart(ntp_reconnect_timer, 0);
            } else {
                xEventGroupSetBits(wifi_event_group, BIT_WIFI_NTP_GOT_TIME);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

bool wifi_is_on() {
    return ((xEventGroupGetBits(wifi_event_group) & BIT_WIFI_ENABLED) == BIT_WIFI_ENABLED) ? true : false;
}

bool wifi_is_connected() {
    return ((xEventGroupGetBits(wifi_event_group) & BIT_WIFI_CONNECTED) == BIT_WIFI_CONNECTED) ? true : false;
}

bool wifi_got_ntp_time() {
    return ((xEventGroupGetBits(wifi_event_group) & BIT_WIFI_NTP_GOT_TIME) == BIT_WIFI_NTP_GOT_TIME) ? true : false;
}

bool wifi_scan_done() {
    return ((xEventGroupGetBits(wifi_event_group) & BIT_WIFI_SCAN_DONE) == BIT_WIFI_SCAN_DONE) ? true : false;
}
