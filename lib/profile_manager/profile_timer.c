#include "stdbool.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "esp_timer.h"

#include "profile_timer.h"


#define _pdTICKS_TO_MS( xTicks )   ( ( TickType_t ) ( ( uint64_t ) ( xTicks ) * 1000 / configTICK_RATE_HZ ) )


static EventGroupHandle_t profile_event_group;
static TimerHandle_t profile_timer;
static bool is_expired = true;



void profile_timer_set_event_group(EventGroupHandle_t *event_group) {
    profile_event_group = *event_group;
}

static void profile_timer_cb(TimerHandle_t timer) {
    is_expired = true;
    xEventGroupSetBits(profile_event_group, BIT_PROFILE_TIMER_TIMEOUT);
    xTimerStop(profile_timer, 0);
}

void profile_timer_setup(uint32_t timeout_ms) {
    is_expired = false;
    profile_timer = xTimerCreate(
        "profTim",
        pdMS_TO_TICKS(timeout_ms),
        pdFALSE,
        NULL,
        profile_timer_cb
    );
}

void profile_timer_run(uint32_t timeout) {
    is_expired = false;
    xTimerChangePeriod(profile_timer, pdMS_TO_TICKS(timeout), 0);
}

void profile_timer_stop() {
    xTimerStop(profile_timer, 0);
}

bool profile_timer_is_expired() {
    return is_expired;
}

uint32_t profile_timer_get_time_left() {
    return _pdTICKS_TO_MS(xTimerGetExpiryTime(profile_timer) - xTaskGetTickCount());
}
