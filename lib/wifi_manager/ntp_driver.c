#include "lwip/inet.h"
#include "esp_sntp.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "logger.h"
#include "global_config.h"
#include "ntp_driver.h"


#define BIT_NTP_GOT_TIME        BIT0


static const char * const TAG = "NTP";
static EventGroupHandle_t ntp_event_group;
static uint8_t ntp_running = 0;


static void time_sync_notification_cb(struct timeval *tv) {
    TB_LOGI(TAG, "synchronized time");
    xEventGroupSetBits(ntp_event_group, BIT_NTP_GOT_TIME);
}


static void ntp_init() {
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, NTP_DEFAULT_SERVER_ADDR);
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_init();
}


esp_err_t ntp_start() {
    TB_LOGI(TAG, "initializing SNTP");
    esp_err_t err = ESP_OK;
    ntp_event_group = xEventGroupCreate();

    if (ntp_running == 1) {
        TB_LOGW(TAG, "already running");
        return ESP_FAIL;
    }
    
    ntp_init();
    ntp_running = 1;

    // wait for time to be set
    TB_LOGI(TAG, "Waiting for system time to be set...");
    EventBits_t bits = xEventGroupWaitBits(
        ntp_event_group,
        BIT_NTP_GOT_TIME,
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(NTP_CONNECT_TIMEOUT_MS)
    );

    if ((bits & BIT_NTP_GOT_TIME) == BIT_NTP_GOT_TIME) {
        // set timezone
        setenv("TZ", NTP_TIME_ZONE, 1);
        tzset();
    } else {
        TB_LOGW(TAG, "could not get time, stopping NTP");
        ntp_stop();
        err = ESP_ERR_TIMEOUT;
    }

    return err;
}

void ntp_stop() {
    TB_LOGI(TAG, "stop");
    if (ntp_running == 1) {
        esp_sntp_stop();
        ntp_running = 0;
    }
}
