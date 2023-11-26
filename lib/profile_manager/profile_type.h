#ifndef LIB_PROFILE_MANAGER_PROFILE_TYPE_H_
#define LIB_PROFILE_MANAGER_PROFILE_TYPE_H_


#include "inttypes.h"
#include "global_config.h"


typedef struct {
    int16_t temperature;    // temperature in *C * 100
    int32_t time_ms;        // time in ms
} __attribute__ ((packed)) profile_point;


typedef struct {
    profile_point points[PROFILE_MAX_VERTICES];
} profile_t;


#endif  // LIB_PROFILE_MANAGER_PROFILE_TYPE_H_
