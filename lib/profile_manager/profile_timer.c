#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"

#include "profile_timer.h"


static EventGroupHandle_t profile_event_group;
static TimerHandle_t profile_timer;


void profile_timer_set_event_group(EventGroupHandle_t *event_group) {
    profile_event_group = *event_group;
}

static void profile_timer_cb(TimerHandle_t timer) {
    xEventGroupSetBits(profile_event_group, BIT_PROFILE_TIMER_TIMEOUT);
    xTimerStop(profile_timer, 0);
}

void profile_timer_setup(uint32_t timeout_ms) {
    profile_timer = xTimerCreate(
        "profTim",
        pdMS_TO_TICKS(timeout_ms),
        pdFALSE,
        NULL,
        profile_timer_cb
    );
}

void profile_timer_run(uint32_t timeout) {
    xTimerChangePeriod(profile_timer, pdMS_TO_TICKS(timeout), 0);
}

void profile_timer_stop() {
    xTimerStop(profile_timer, 0);
}
