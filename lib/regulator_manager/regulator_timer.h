#ifndef LIB_REGULATOR_MANAGER_REGULATOR_TIMER_H_
#define LIB_REGULATOR_MANAGER_REGULATOR_TIMER_H_


#include "inttypes.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"


#define BIT_REGULATOR_TIMER_TIMEOUT         BIT0
#define BIT_REGULATOR_START                 BIT1
#define BIT_REGULATOR_STOP                  BIT2
#define BIT_REGULATOR_UPDATE                BIT3

#define BITS_REGULATOR_CONTROL      ( \
    BIT_REGULATOR_START             | \
    BIT_REGULATOR_STOP              | \
    BIT_REGULATOR_UPDATE            )


#ifdef __cplusplus
extern "C" {
#endif


void regulator_timer_set_event_group(EventGroupHandle_t *event_group);
void regulator_timer_setup(uint32_t timeout_ms);
void regulator_timer_run(uint32_t timeout);
void regulator_timer_stop();
bool regulator_timer_is_expired();

void regulator_update_timer_setup(uint32_t interval_ms);
void regulator_update_timer_run();
void regulator_update_timer_stop();


#ifdef __cplusplus
}
#endif


#endif  // LIB_REGULATOR_MANAGER_REGULATOR_TIMER_H_
