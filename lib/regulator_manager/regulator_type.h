#ifndef LIB_REGULATOR_MANAGER_REGULATOR_TYPE_H_
#define LIB_REGULATOR_MANAGER_REGULATOR_TYPE_H_


#include "inttypes.h"

typedef struct {
    int16_t min_temp;
    int16_t max_temp;
    uint32_t sampling_rate;
    uint32_t update_interval;
} regulator_config_t;


#endif  // LIB_REGULATOR_MANAGER_REGULATOR_TYPE_H_
