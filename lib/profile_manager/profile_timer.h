#ifndef LIB_PROFILE_MANAGER_PROFILE_DRIVER_H_
#define LIB_PROFILE_MANAGER_PROFILE_DRIVER_H_


#include "inttypes.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"


#define BIT_PROFILE_TIMER_TIMEOUT           BIT0
#define BIT_PROFILE_UPDATE_TIMER_TIMEOUT    BIT1
#define BIT_PROFILE_START                   BIT2
#define BIT_PROFILE_STOP                    BIT3
#define BIT_PROFILE_RESUME                  BIT4
#define BIT_PROFILE_END                     BIT5
#define BIT_PROFILE_UPDATE                  BIT6

#define BITS_PROFILE_CONTROL    ( \
    BIT_PROFILE_START           | \
    BIT_PROFILE_STOP            | \
    BIT_PROFILE_RESUME          | \
    BIT_PROFILE_END             | \
    BIT_PROFILE_UPDATE          )

#define BITS_PROFILE_TIMERS             ( \
    BIT_PROFILE_TIMER_TIMEOUT           | \
    BIT_PROFILE_UPDATE_TIMER_TIMEOUT    )


#ifdef __cplusplus
extern "C" {
#endif


void profile_timer_set_event_group(EventGroupHandle_t *event_group);
void profile_timer_setup(uint32_t timeout_ms);
void profile_timer_run(uint32_t timeout);
void profile_timer_stop();
bool profile_timer_is_expired();
uint32_t profile_timer_get_time_left();

void profile_update_timer_setup(uint32_t interval_ms);
void profile_update_timer_run(uint32_t timeout);
void profile_update_timer_stop();


#ifdef __cplusplus
}
#endif


#endif  // LIB_PROFILE_MANAGER_PROFILE_DRIVER_H_
