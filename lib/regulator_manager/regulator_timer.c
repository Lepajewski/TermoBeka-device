#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"

#include "regulator_timer.h"


static EventGroupHandle_t regulator_event_group;
static TimerHandle_t regulator_timer;
static bool is_expired = true;

static TimerHandle_t regulator_update_timer;



void regulator_timer_set_event_group(EventGroupHandle_t *event_group) {
    regulator_event_group = *event_group;
}

static void regulator_timer_cb(TimerHandle_t timer) {
    is_expired = true;
    xEventGroupSetBits(regulator_event_group, BIT_REGULATOR_TIMER_TIMEOUT);
    xTimerStop(regulator_timer, 0);
}

void regulator_timer_setup(uint32_t timeout_ms) {
    is_expired = false;
    regulator_timer = xTimerCreate(
        "regTim",
        pdMS_TO_TICKS(timeout_ms),
        pdFALSE,
        NULL,
        regulator_timer_cb
    );
}

void regulator_timer_run(uint32_t timeout) {
    is_expired = false;
    xTimerChangePeriod(regulator_timer, pdMS_TO_TICKS(timeout), 0);
}

void regulator_timer_stop() {
    xTimerStop(regulator_timer, 0);
}

bool regulator_timer_is_expired() {
    return is_expired;
}



static void regulator_update_timer_cb(TimerHandle_t timer) {
    xEventGroupSetBits(regulator_event_group, BIT_REGULATOR_UPDATE);
}

void regulator_update_timer_setup(uint32_t interval_ms) {
    regulator_update_timer = xTimerCreate(
        "regUpdTim",
        pdMS_TO_TICKS(interval_ms),
        pdTRUE,
        NULL,
        regulator_update_timer_cb
    );
}

void regulator_update_timer_run() {
    xTimerStart(regulator_update_timer, 0);
}

void regulator_update_timer_stop() {
    xTimerStop(regulator_update_timer, 0);
}
