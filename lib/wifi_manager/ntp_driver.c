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
    
    ntp_init();

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
        esp_sntp_stop();
        err = ESP_ERR_TIMEOUT;
    }

    return err;
}

void ntp_stop() {
    TB_LOGI(TAG, "stop");
    esp_sntp_stop();
}
