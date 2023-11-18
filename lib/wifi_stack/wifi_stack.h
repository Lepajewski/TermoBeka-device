#ifndef LIB_PERIPHERALS_WIFI_STACK_H
#define LIB_PERIPHERALS_WIFI_STACK_H


#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"


#ifdef __cplusplus
extern "C" {
#endif


void wifi_init(void);
EventBits_t get_wifi_status();


#ifdef __cplusplus
}
#endif

bool wifi_is_on();
bool wifi_is_connected();
bool wifi_got_ntp_time();
bool wifi_scan_done();

#endif  // LIB_PERIPHERALS_WIFI_STACK_H
