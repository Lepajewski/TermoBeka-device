#include "string.h"

#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"

#include "logger.h"
#include "global_config.h"

#include "ntp_driver.h"
#include "wifi_driver.h"


static const char * const TAG = "WIFI";
static EventGroupHandle_t wifi_event_group;
static TimerHandle_t ntp_reconnect_timer;
static uint16_t connection_retries = 0;
static esp_netif_t* esp_netif_sta;


static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        connection_retries = 0;
        TB_LOGI(TAG, "started, connecting...");
        esp_wifi_connect();
        xEventGroupSetBits(wifi_event_group, BIT_WIFI_RUNNING);
    } else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_STOP) {
        TB_LOGI(TAG, "stopped");
        xEventGroupSetBits(wifi_event_group, BIT_WIFI_STOPPED);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        connection_retries++;
        esp_wifi_connect();
        TB_LOGI(TAG, "disconnected... retrying... %" PRIu16, connection_retries);
        xEventGroupSetBits(wifi_event_group, BIT_WIFI_DISCONNECTED);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        TB_LOGI(TAG, "connected");
        xEventGroupSetBits(wifi_event_group, BIT_WIFI_CONNECTED);
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

void wifi_set_event_group(EventGroupHandle_t *event_group) {
    wifi_event_group = *event_group;
}

esp_err_t wifi_begin(wifi_driver_config_t *cfg) {
    esp_err_t err = ESP_OK;
    ntp_reconnect_timer = xTimerCreate(
        "ntpReconn",
        pdMS_TO_TICKS(NTP_RECONNECT_INTERVAL_MS),
        pdFALSE,
        NULL,
        ntp_reconnect_timer_cb
    );

    // initialize wifi in STA (stationary) mode
    if ((err = esp_netif_init()) != ESP_OK) {
        return err;
    }

    if ((err = esp_event_loop_create_default()) != ESP_OK) {
        return err;
    }

    esp_netif_sta = esp_netif_create_default_wifi_sta();

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    if ((err = esp_wifi_init(&config)) != ESP_OK) {
        return err;
    }

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    err = esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &event_handler,
        NULL,
        &instance_any_id
    );
    if (err != ESP_OK) {
        return err;
    }

    err = esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &event_handler,
        NULL,
        &instance_got_ip
    );
    if (err != ESP_OK) {
        return err;
    }

    wifi_config_t wifi_config = {0};
    memcpy(wifi_config.sta.ssid, cfg->ssid, WIFI_MAX_SSID_LEN);
    memcpy(wifi_config.sta.password, cfg->pass, WIFI_MAX_PASS_LEN);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

    if ((err = esp_wifi_set_mode(WIFI_MODE_STA)) != ESP_OK) {
        return err;
    }
    if ((err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config)) != ESP_OK) {
        return err;
    }
    if ((err = esp_wifi_start()) != ESP_OK) {
        return err;
    }

    TB_LOGI(TAG, "init finished");
    return err;
}

esp_err_t wifi_end() {
    esp_err_t err = ESP_OK;

    ntp_stop();

    if ((err = esp_event_loop_delete_default()) != ESP_OK) {
        return err;
    }
    
    esp_netif_destroy_default_wifi(esp_netif_sta);

    if ((err = esp_wifi_disconnect()) != ESP_OK) {
        return err;
    }
    return esp_wifi_stop();
}

esp_err_t wifi_scan() {
    esp_err_t err = ESP_OK;

    uint16_t number = SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    if ((err = esp_wifi_scan_start(NULL, true)) != ESP_OK) {
        return err;
    }

    if ((err = esp_wifi_scan_get_ap_records(&number, ap_info)) != ESP_OK) {
        return err;
    }

    if ((err = esp_wifi_scan_get_ap_num(&ap_count)) != ESP_OK) {
        return err;
    }

    for (int i = 0; i < number; i++) {
        printf("SSID:\t%-32s RSSI:\t%-3d Authmode:\t%d Channel:\t%d\n", 
            ap_info[i].ssid, 
            ap_info[i].rssi,
            ap_info[i].authmode,
            ap_info[i].primary);
    }

    return err;
}

void wifi_ntp_connect() {
    esp_err_t err = ntp_start();
    if (err == ESP_ERR_TIMEOUT) {
        TB_LOGW(TAG, "NTP timeout. Retrying... in %ds", NTP_RECONNECT_INTERVAL_MS / 1000);
        xTimerStart(ntp_reconnect_timer, 0);
    } else {
        xEventGroupSetBits(wifi_event_group, BIT_WIFI_NTP_GOT_TIME);
    }
}
