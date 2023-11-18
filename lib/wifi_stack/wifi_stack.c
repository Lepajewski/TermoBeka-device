#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "logger.h"

#include "wifi_stack.h"


#define BIT_WIFI_CONNECTED      BIT0
#define BIT_WIFI_FAIL           BIT1

#define WIFI_SSID       "Orange_Swiatlowod_3760"
#define WIFI_PASS       "2FT752946HF7"


static const char * const TAG = "WIFI";
static EventGroupHandle_t wifi_event_group;
static uint16_t connection_retries = 0;


static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        connection_retries = 0;
        esp_err_t err = esp_wifi_connect();
        TB_LOGI(TAG, "started, connected: %s", err == ESP_OK ? "success" : "fail");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        connection_retries++;
        esp_wifi_connect();
        TB_LOGI(TAG, "disconnected... retrying... %" PRIu16, connection_retries);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t*) event_data;
        TB_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void wifi_init(void) {
    TB_LOGI(TAG, "start");

    wifi_event_group = xEventGroupCreate();

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
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    TB_LOGI(TAG, "init finished");

    EventBits_t bits = xEventGroupWaitBits(
        wifi_event_group,
        BIT_WIFI_CONNECTED | BIT_WIFI_FAIL,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    if (bits & BIT_WIFI_CONNECTED) {
        TB_LOGI(TAG, "connected to %s", WIFI_SSID);
        // synchronize time
        // setup_ntp();
    } else if (bits & BIT_WIFI_FAIL) {
        TB_LOGI(TAG, "failed to connect to %s, connection tries: %" PRIu16, WIFI_SSID, connection_retries);
    } else {
        TB_LOGI(TAG, "unexpected event");
    }
}
